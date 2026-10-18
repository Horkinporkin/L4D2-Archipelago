from __future__ import annotations

import asyncio
import copy
import logging
import sys
from typing import TYPE_CHECKING, Any

from CommonClient import get_base_parser, gui_enabled, handle_url_arg, server_loop
from NetUtils import ClientStatus, JSONtoTextParser
from Utils import async_start, init_logging

from ..Items import MAX_SURVIVOR_COMPANIONS, PROGRESSIVE_SURVIVOR, PROGRESSIVE_WEAPON_TIER
from .DeathMessages import format_death_message
from .Protocol import DEFAULT_BRIDGE_HOST, DEFAULT_BRIDGE_PORT, BridgeConnection, BridgeMessage, BridgeProtocolError

tracker_loaded = False
try:
    from worlds.tracker.TrackerClient import TrackerCommandProcessor as ClientCommandProcessor
    from worlds.tracker.TrackerClient import TrackerGameContext as CommonContext

    tracker_loaded = True
except ImportError:
    from CommonClient import ClientCommandProcessor, CommonContext

if TYPE_CHECKING:
    from kvui import GameManager


logger = logging.getLogger("L4D2")

BRIDGE_RECONNECT_DELAY = 5
MAX_PENDING_DEATHLINKS = 2
CARRIER_SUFFIX = ": AP Item Carrier"
CHAT_PREFIX = "§5[AP]§1 "
# AP colors mapped to usable ones in-game
CHAT_COLORS = {
    "yellow": 3,
    "magenta": 3,
    "plum": 4,
    "slateblue": 4,
    "salmon": 4,
    "cyan": 4,
    "green": 5,
    "blue": 5,
}


def _chat_safe(text: object) -> str:
    """Strip characters L4D2 chat cannot print."""
    safe_text = "".join(character for character in str(text) if character.isprintable() or character in "\n\t")
    return safe_text.replace('"', "'")


class ChatParser(JSONtoTextParser):
    def _handle_text(self, node: dict[str, Any]) -> str:
        return _chat_safe(node.get("text", ""))

    def _handle_color(self, node: dict[str, Any]) -> str:
        color = next(
            (CHAT_COLORS[name] for name in str(node.get("color", "")).split(";") if name in CHAT_COLORS),
            1,
        )
        return f"§{color}{self._handle_text(node)}§1"


class L4D2CommandProcessor(ClientCommandProcessor):
    ctx: L4D2Context

    def _cmd_deathlink(self):
        """Toggle deathlink from client. Overrides default setting."""
        if isinstance(self.ctx, L4D2Context):
            async_start(
                self.ctx.set_deathlink_enabled(not self.ctx.deathlink_enabled()),
                name="toggle L4D2 Deathlink",
            )


class L4D2Context(CommonContext):
    game = "Left 4 Dead 2"
    tags = {"AP"}
    items_handling = 0b111
    command_processor = L4D2CommandProcessor

    def __init__(
        self,
        server_address: str | None = None,
        password: str | None = None,
        bridge_host: str = DEFAULT_BRIDGE_HOST,
        bridge_port: int = DEFAULT_BRIDGE_PORT,
    ) -> None:
        super().__init__(server_address, password)
        self.bridge = BridgeConnection(bridge_host, bridge_port)
        self.bridge_task: asyncio.Task[None] | None = None
        self.game_ready = False
        self.bridge_synced = False
        self.bridge_syncing = False

        self.ap_slot_data: dict[str, Any] = {}
        self.processed_item_count = 0
        self.pending_location_checks: set[str] = set()
        self.pending_deathlinks: list[dict[str, str]] = []
        self.deathlink_override: bool | None = None
        self._last_map: str | None = None
        self._location_name_to_id: dict[str, int] = {}
        self._finale_location_ids: set[int] = set()
        self._route_chapter_names: set[str] = set()
        self._survival_level_maps: dict[str, str] = {}
        self._survival_checks: dict[tuple[str, int], str] = {}
        self._item_lock = asyncio.Lock()
        self._deathlink_lock = asyncio.Lock()
        self._chat_lock = asyncio.Lock()
        self.chat_parser = ChatParser(self)

    async def server_auth(self, password_requested: bool = False) -> None:
        if password_requested and not self.password:
            await super().server_auth(password_requested)
        await self.get_username()
        await self.send_connect(game=self.game)

    def session_identity(self) -> str | None:
        if self.server_seed_name and self.team is not None and self.slot is not None:
            return f"{self.server_seed_name}|{self.team}|{self.slot}"
        return None

    def on_package(self, cmd: str, args: dict[str, Any]) -> None:
        if cmd == "Connected":
            self.ap_slot_data = args.get("slot_data") or {}

        super().on_package(cmd, args)

        if cmd == "Connected":
            self.read_slot_data()
            self.rebuild_location_lookup()
            self.build_finale_location_ids()
            self.report_goal()
            async_start(self.update_death_link(self.deathlink_enabled()), name="update L4D2 Deathlink")
            async_start(self.try_sync_bridge(), name="sync L4D2 bridge")
        elif cmd == "RoomUpdate":
            self.report_goal()
        elif cmd == "ReceivedItems":
            async_start(self.give_pending_items(), name="give pending L4D2 items")

    def read_slot_data(self) -> None:
        self._route_chapter_names = {
            chapter["name"]
            for route in self.ap_slot_data.get("route", [])
            for chapter in route.get("chapters", [])
        }

        self._survival_level_maps = {}
        self._survival_checks = {}
        for level in self.survival_slot_data().get("levels", []):
            if not isinstance(level, dict):
                continue
            if level.get("name") and level.get("map"):
                self._survival_level_maps[level["name"]] = level["map"]
            if not isinstance(level.get("map"), str):
                continue
            for medal, location_name in (level.get("checks") or {}).items():
                if not isinstance(location_name, str):
                    continue
                try:
                    self._survival_checks[(level["map"], int(medal))] = location_name
                except ValueError:
                    logger.warning("Slot data has an unusable Survival medal %r.", medal)

    def survival_slot_data(self) -> dict[str, Any]:
        survival = self.ap_slot_data.get("survival")
        return survival if isinstance(survival, dict) else {}

    async def bridge_loop(self) -> None:
        reported_unavailable = False

        while not self.exit_event.is_set():
            try:
                await self.bridge.connect()
                logger.info("Connected to the bridge at %s:%d.", self.bridge.host, self.bridge.port)
                reported_unavailable = False

                while not self.exit_event.is_set():
                    self.handle_bridge_message(await self.bridge.receive())
            except asyncio.CancelledError:
                raise
            except (ConnectionError, OSError, BridgeProtocolError) as error:
                # Do not spam logs with retry
                if not reported_unavailable:
                    logger.info(
                        "Bridge unavailable at %s:%d (%s), retrying every %d seconds.",
                        self.bridge.host,
                        self.bridge.port,
                        error,
                        BRIDGE_RECONNECT_DELAY,
                    )
                    reported_unavailable = True
            except Exception:
                logger.exception("Unexpected error on the bridge.")
            finally:
                self.game_ready = False
                self.bridge_synced = False
                self.bridge_syncing = False
                await self.bridge.close()

            if not self.exit_event.is_set():
                await asyncio.sleep(BRIDGE_RECONNECT_DELAY)

    async def send_bridge(self, message_type: str, payload: dict[str, Any]) -> None:
        await self.bridge.send(BridgeMessage(message_type, payload))

    def handle_bridge_message(self, message: BridgeMessage) -> None:
        logger.debug("Left 4 Dead 2 bridge message: %s %s", message.type, message.payload)

        if message.type == "READY":
            self.game_ready = True
            async_start(self.try_sync_bridge(), name="sync L4D2 bridge")
        elif message.type in {"LOCATION_CHECKED", "CHAPTER_COMPLETED", "FINALE_COMPLETED", "INTERACTABLE_COMPLETED"}:
            self.handle_location_report(message)
        elif message.type == "SURVIVAL_MEDAL":
            self.handle_survival_medal(message)
        elif message.type == "GAME_STATE":
            self.handle_game_state(message)
        elif message.type == "DEATHLINK":
            self.handle_game_deathlink(message)
        elif message.type == "ERROR":
            logger.error("Plugin reported an error: %s", message.payload.get("message"))
        else:
            logger.debug("Ignoring unknown bridge message type %s.", message.type)

    def handle_location_report(self, message: BridgeMessage) -> None:
        location_name = message.payload.get("location")
        if not isinstance(location_name, str):
            logger.warning("Plugin sent %s without a location: %s", message.type, message.payload)
            return
        self.check_location_name(location_name)

    def check_location_name(self, location_name: str) -> None:
        location_id = self._location_name_to_id.get(location_name)
        if location_id is None:
            self.pending_location_checks.add(location_name)
            logger.info("Queued %s until the data package is ready.", location_name)
            return

        if location_id in self.checked_locations or location_id in self.locations_checked:
            logger.debug("Ignoring duplicate Left 4 Dead 2 check: %s (%d)", location_name, location_id)
            return

        self.locations_checked.add(location_id)
        logger.info("Left 4 Dead 2 check: %s (%d)", location_name, location_id)
        async_start(self.check_locations({location_id}), name=f"check {location_name}")

    def handle_survival_medal(self, message: BridgeMessage) -> None:
        map_name = message.payload.get("map")
        medal = message.payload.get("medal")
        if not isinstance(map_name, str) or not isinstance(medal, int):
            logger.warning("Plugin sent an unusable Survival medal: %s", message.payload)
            return

        location_name = self._survival_checks.get((map_name, medal))
        if location_name is None:
            logger.info("There is no check in the seed for Survival medal %d on %s.", medal, map_name)
            return

        self.check_location_name(location_name)

    def handle_game_state(self, message: BridgeMessage) -> None:
        current_map = message.payload.get("map")
        if current_map == self._last_map:
            return
        self._last_map = current_map
        logger.info(
            "Left 4 Dead 2 state: map=%s chapter=%s campaign=%s survivors=%s",
            current_map or "?",
            message.payload.get("chapter", ""),
            message.payload.get("campaign", ""),
            message.payload.get("survivors", "?"),
        )

    def handle_game_deathlink(self, message: BridgeMessage) -> None:
        if not self.deathlink_enabled():
            return

        trigger = message.payload.get("trigger") or "death"
        is_death = trigger != "downed"
        player_name = self.player_names.get(self.slot, self.auth)
        base_cause = (message.payload.get("cause") or "").strip()

        text = format_death_message(base_cause, is_death, player_name)

        async_start(self.send_death(text), name="send L4D2 Deathlink")

    async def try_sync_bridge(self) -> None:
        if not self.game_ready or self.bridge_synced or self.bridge_syncing or not self.ap_slot_data:
            return

        self.bridge_syncing = True
        self.processed_item_count = 0

        try:
            await self.configure_deathlink_bridge()
            await self.send_bridge(
                "CONFIGURE_ROUTE",
                {
                    "route": self.ap_slot_data.get("route", []),
                    "vanilla_campaign_order": self.ap_slot_data.get("vanilla_campaign_order", []),
                    "session_id": self.session_identity() or "",
                },
            )

            weapon_tiers = self.ap_slot_data.get("weapon_tiers", {})
            await self.send_bridge(
                "CONFIGURE_WEAPON_TIERS",
                {
                    "enabled": bool(weapon_tiers.get("enabled", False)),
                    "max_tier": int(weapon_tiers.get("max_tier", 0)),
                    "tiers": [{"item": name, "tier": tier} for name, tier in weapon_tiers.get("tiers", {}).items()],
                },
            )

            survival = self.survival_slot_data()
            times = survival.get("times", {}) if isinstance(survival.get("times"), dict) else {}
            await self.send_bridge(
                "CONFIGURE_SURVIVAL",
                {
                    "enabled": bool(survival.get("enabled", False)),
                    "medal": int(survival.get("medal", 0)),
                    "bronze_time": int(times.get("bronze", 240)),
                    "silver_time": int(times.get("silver", 420)),
                    "gold_time": int(times.get("gold", 600)),
                    "levels": [
                        {
                            "map": level.get("map", ""),
                            "campaign": level.get("campaign", ""),
                            "name": level.get("name", ""),
                        }
                        for level in survival.get("levels", [])
                        if isinstance(level, dict) and level.get("map")
                    ],
                },
            )

            survivors = self.ap_slot_data.get("survivor_progression", {})
            await self.send_bridge(
                "CONFIGURE_SURVIVORS",
                {
                    "enabled": bool(survivors.get("enabled", False)),
                    "max_companions": int(survivors.get("max_companions", 0)),
                },
            )

            completed_chapters, checked_pickups = self.checked_route_progress()
            await self.send_bridge(
                "SYNC_STATE",
                {
                    "completed_chapters": completed_chapters,
                    "checked_pickups": checked_pickups,
                },
            )

            async with self._item_lock:
                await self.deliver_items_locked()
                await self.send_bridge("SYNC_SURVIVORS", {"companions": self.received_survivor_companions()})

            await self.send_bridge("SYNC_COMPLETE", {})
            self.bridge_synced = True
            logger.info("Left 4 Dead 2 bridge synced.")

            await self.deliver_pending_deathlinks()
            self.deliver_pending_location_checks()
        finally:
            self.bridge_syncing = False

        if self.bridge_synced:
            await self.give_pending_items()

    def checked_route_progress(self) -> tuple[list[str], list[str]]:
        completed_chapters = []
        checked_pickups = []
        for location_id in self.checked_locations:
            location_name = self.location_names.lookup_in_game(location_id)
            if location_name in self._route_chapter_names:
                completed_chapters.append(location_name)
            elif (
                location_name.endswith(CARRIER_SUFFIX)
                and location_name.removesuffix(CARRIER_SUFFIX) in self._route_chapter_names
            ):
                checked_pickups.append(location_name)
        return sorted(completed_chapters), sorted(checked_pickups)

    def deliver_pending_location_checks(self) -> None:
        for location_name in tuple(self.pending_location_checks):
            self.pending_location_checks.discard(location_name)
            self.check_location_name(location_name)

        if self.pending_location_checks:
            logger.warning(
                "The game reported location(s) that are not part of this seed: %s",
                ", ".join(sorted(self.pending_location_checks)),
            )

    def reset_session_state(self) -> None:
        super().reset_session_state()
        self.bridge_synced = False
        self.processed_item_count = 0
        self.pending_location_checks.clear()
        self.pending_deathlinks.clear()
        self._last_map = None

    def rebuild_location_lookup(self) -> None:
        self._location_name_to_id = {
            self.location_names.lookup_in_game(location_id): location_id
            for location_id in self.missing_locations | self.checked_locations
        }
        logger.debug(
            "Location lookup ready: %d names from %d missing locations and %d checked locations.",
            len(self._location_name_to_id),
            len(self.missing_locations),
            len(self.checked_locations),
        )

    def build_finale_location_ids(self) -> None:
        self._finale_location_ids = {
            self._location_name_to_id[chapter["name"]]
            for route in self.ap_slot_data.get("route", [])
            for chapter in route.get("chapters", [])
            if chapter.get("finale") and chapter.get("name") in self._location_name_to_id
        }

    def report_goal(self) -> None:
        if self.finished_game:
            return

        options = self.ap_slot_data.get("options", {})
        goal = options.get("goal") if isinstance(options, dict) else None
        if not isinstance(goal, int):
            return
        if len(self._finale_location_ids & self.checked_locations) < goal:
            return

        self.finished_game = True
        logger.info("Left 4 Dead 2 goal reached.")
        async_start(
            self.send_msgs([{"cmd": "StatusUpdate", "status": ClientStatus.CLIENT_GOAL}]),
            name="report L4D2 goal",
        )

    async def give_pending_items(self) -> None:
        if not self.game_ready or not self.bridge_synced:
            return
        async with self._item_lock:
            await self.deliver_items_locked()

    async def deliver_items_locked(self) -> None:
        while self.processed_item_count < len(self.items_received):
            item_index = self.processed_item_count
            item_name = self.item_names.lookup_in_game(self.items_received[item_index].item, self.game)
            await self.give_item(item_name, item_index)
            self.processed_item_count += 1

    async def give_item(self, item_name: str, item_index: int) -> None:
        delivery = {"item_index": item_index}

        if item_name == PROGRESSIVE_WEAPON_TIER:
            await self.send_bridge("UNLOCK_WEAPON_TIER", delivery)
        elif item_name == PROGRESSIVE_SURVIVOR:
            await self.send_bridge("UNLOCK_SURVIVOR", delivery)
        elif item_name.startswith("Trap: "):
            await self.send_bridge("ACTIVATE_TRAP", {**delivery, "trap": item_name.removeprefix("Trap: ")})
        elif item_name in self._route_chapter_names:
            await self.send_bridge("UNLOCK_CHAPTER", {**delivery, "chapter": item_name})
        elif item_name in self._survival_level_maps:
            await self.send_bridge("UNLOCK_SURVIVAL_LEVEL", {**delivery, "map": self._survival_level_maps[item_name]})
        else:
            await self.send_bridge("GIVE_ITEM", {**delivery, "item": item_name})

    def received_survivor_companions(self) -> int:
        received = self.items_received[: self.processed_item_count]
        survivors = sum(
            1 for item in received if self.item_names.lookup_in_game(item.item, self.game) == PROGRESSIVE_SURVIVOR
        )
        return min(survivors, MAX_SURVIVOR_COMPANIONS)

    def deathlink_enabled(self) -> bool:
        if self.deathlink_override is not None:
            return self.deathlink_override
        options = self.ap_slot_data.get("options", {})
        return isinstance(options, dict) and bool(options.get("death_link", False))

    def deathlink_trigger(self) -> str:
        options = self.ap_slot_data.get("options", {})
        if isinstance(options, dict) and options.get("deathlink_trigger", 1) == 0:
            return "downed"
        return "death"

    async def configure_deathlink_bridge(self) -> None:
        await self.send_bridge("CONFIGURE_DEATHLINK", {
            "enabled": self.deathlink_enabled(),
            "trigger": self.deathlink_trigger(),
        })

    async def set_deathlink_enabled(self, enabled: bool) -> None:
        self.deathlink_override = enabled
        async with self._deathlink_lock:
            self.pending_deathlinks.clear()
        await self.update_death_link(enabled)
        if self.game_ready and self.bridge_synced:
            try:
                await self.configure_deathlink_bridge()
            except ConnectionError:
                pass
        logger.info("Deathlink %s for this client session.", "enabled" if enabled else "disabled")

    def on_deathlink(self, data: dict[str, Any]) -> None:
        super().on_deathlink(data)
        if not self.deathlink_enabled() or len(self.pending_deathlinks) >= MAX_PENDING_DEATHLINKS:
            return

        self.pending_deathlinks.append({
            "source": str(data.get("source", "")),
            "cause": str(data.get("cause", "")),
        })
        if self.game_ready and self.bridge_synced:
            async_start(self.deliver_pending_deathlinks(), name="deliver L4D2 Deathlink")

    async def deliver_pending_deathlinks(self) -> None:
        async with self._deathlink_lock:
            while self.pending_deathlinks:
                try:
                    await self.send_bridge("RECEIVE_DEATHLINK", self.pending_deathlinks[0])
                except ConnectionError:
                    return
                self.pending_deathlinks.pop(0)

    def on_print_json(self, args: dict[str, Any]) -> None:
        super().on_print_json(args)
        # The plugin wraps long lines to the chat width
        text = self.chat_parser(copy.deepcopy(args["data"]))
        if text:
            async_start(self.send_chat([f"{CHAT_PREFIX}{text}"]), name="send L4D2 chat")

    async def send_chat(self, lines: list[str]) -> None:
        async with self._chat_lock:
            for line in lines:
                if not self.game_ready or not self.bridge_synced:
                    return
                try:
                    await self.send_bridge("CHAT_LINE", {"line": line})
                except ConnectionError:
                    return

    def make_gui(self) -> type[GameManager]:
        ui = super().make_gui()
        ui.base_title = "Left 4 Dead 2 Archipelago Client"
        return ui

    async def shutdown(self) -> None:
        if self.bridge_task is not None:
            self.bridge_task.cancel()
            try:
                await self.bridge_task
            except asyncio.CancelledError:
                pass
        await super().shutdown()


async def main(args: Any) -> None:
    ctx = L4D2Context(args.connect, args.password, args.bridge_host, args.bridge_port)
    ctx.auth = args.name
    ctx.server_task = asyncio.create_task(server_loop(ctx), name="Archipelago server loop")
    ctx.bridge_task = asyncio.create_task(ctx.bridge_loop(), name="Left 4 Dead 2 bridge loop")

    if tracker_loaded:
        ctx.run_generator()
    else:
        logger.warning("Universal Tracker not found; the tracker tab is not available.")

    if gui_enabled:
        ctx.run_gui()
    ctx.run_cli()

    await ctx.exit_event.wait()
    await ctx.shutdown()


def launch(*launch_args: str) -> None:
    init_logging("L4D2", exception_logger="Client")
    parser = get_base_parser(description="Left 4 Dead 2 Archipelago Client")
    parser.add_argument("--bridge-host", default=DEFAULT_BRIDGE_HOST, help="Host the L4D2 SourceMod plugin listens on.")
    parser.add_argument(
        "--bridge-port", default=DEFAULT_BRIDGE_PORT, type=int,
        help="Port the L4D2 SourceMod plugin listens on.",
    )
    parser.add_argument("--name", default=None, help="Slot Name to connect as.")
    parser.add_argument("url", nargs="?", help="Archipelago connection URL.")
    args = handle_url_arg(parser.parse_args(launch_args), parser)

    import colorama

    colorama.just_fix_windows_console()
    asyncio.run(main(args))
    colorama.deinit()


if __name__ == "__main__":
    launch(*sys.argv[1:])
