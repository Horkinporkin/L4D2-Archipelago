import logging
from typing import Any

from BaseClasses import Tutorial
from Options import OptionError
from worlds.AutoWorld import WebWorld, World
from worlds.LauncherComponents import Component, Type, components, icon_paths, launch as launch_component

from .Items import MAX_SURVIVOR_COMPANIONS, PROGRESSIVE_SURVIVOR, PROGRESSIVE_WEAPON_TIER, L4D2Item, \
    filler_item_names, item_groups, item_name_to_id, item_table, max_weapon_tier, trap_item_names, weapon_tiers
from .Locations import L4D2LocationData, get_location_names, survival_checks
from .Options import SLOT_DATA_OPTIONS, L4D2Options, Randomization, l4d2_option_groups
from .Regions import create_regions
from .Routes import CAMPAIGNS, Chapter, Route, build_routes, route_slot_data, routes_from_slot_data, starting_chapters
from .Rules import set_rules


logger = logging.getLogger("L4D2")


def launch_client(*args: str) -> None:
    from .client.Client import launch

    launch_component(launch, name="Left 4 Dead 2 Client", args=args)


components.append(
    Component(
        "Left 4 Dead 2 Client",
        component_type=Type.CLIENT,
        func=launch_client,
        game_name="Left 4 Dead 2",
        supports_uri=True,
        icon="l4d2_client",
    )
)

icon_paths["l4d2_client"] = f"ap:{__name__}/icons/l4d2.png"


class L4D2Web(WebWorld):
    theme = "ice"
    rich_text_options_doc = True
    option_groups = l4d2_option_groups

    tutorials = [
        Tutorial(
            "Multiworld Setup Guide",
            "A guide to setting up the Left 4 Dead 2 Archipelago.",
            "English",
            "setup_en.md",
            "setup/en",
            ["Horkinporkin"],
        )
    ]


class L4D2World(World):
    """The apocalypse is here! Team up with a band of very useful and skilled survivors to fight through relentless
    hordes of zombies. Navigate the treacherous environments and overcome the deadly threats that await you."""

    game = "Left 4 Dead 2"
    web = L4D2Web()
    options_dataclass = L4D2Options
    options: L4D2Options
    item_name_to_id = item_name_to_id
    location_name_to_id = get_location_names()
    item_name_groups = item_groups

    ut_can_gen_without_yaml = True

    routes: tuple[Route, ...]
    chapters: tuple[Chapter, ...]
    starting_items: list[str]
    survival_checks: dict[str, list[L4D2LocationData]]

    @staticmethod
    def interpret_slot_data(slot_data: dict[str, Any]) -> dict[str, Any] | None:
        """Tell Universal Tracker to regenerate this world from the slot data."""
        if not isinstance(slot_data, dict) or not isinstance(slot_data.get("route"), list) or not slot_data["route"]:
            logger.error("Universal Tracker passed slot data without a route.")
            return None
        return slot_data

    def _tracker_slot_data(self) -> dict[str, Any] | None:
        re_gen_passthrough = getattr(self.multiworld, "re_gen_passthrough", {})
        if re_gen_passthrough and self.game in re_gen_passthrough:
            return re_gen_passthrough[self.game]
        return None

    def generate_early(self) -> None:
        tracker_slot_data = self._tracker_slot_data()
        if tracker_slot_data is None:
            self._roll_route()
        else:
            self._restore_route(tracker_slot_data)

        self.chapters = tuple(chapter for route in self.routes for chapter in route.chapters)
        self.starting_items = starting_chapters(self.routes, bool(self.options.all_campaigns_start.value))
        self.survival_checks = survival_checks(
            self.options.survival_medals.value, {chapter.map_name for chapter in self.chapters}
        )

    def _roll_route(self) -> None:
        if self.options.all_campaigns_start.value and self.options.randomization != Randomization.option_campaign:
            raise OptionError(
                f"{self.player_name}: All Campaigns Start is only available with Campaign randomization."
            )

        self.routes = build_routes(
            self.random,
            CAMPAIGNS[self.options.start_campaign.value],
            self.options.randomization.value,
            self.options.goal.value,
        )

    def _restore_route(self, slot_data: dict[str, Any]) -> None:
        """Restore the route of the original generation, for Universal Tracker."""
        for option_name, value in slot_data.get("options", {}).items():
            option = getattr(self.options, option_name, None)
            if option is not None:
                setattr(self.options, option_name, option.from_any(value))

        self.routes = routes_from_slot_data(slot_data.get("route"))

    def create_regions(self) -> None:
        create_regions(self)

    def create_item(self, name: str) -> L4D2Item:
        data = item_table[name]
        return L4D2Item(name, data.classification, data.code, self.player)

    def create_items(self) -> None:
        for name in self.starting_items:
            self.multiworld.push_precollected(self.create_item(name))

        pool = [chapter.name for chapter in self.chapters if chapter.name not in self.starting_items]
        if self.options.progressive_weapon_tiers.value:
            pool += [PROGRESSIVE_WEAPON_TIER] * max_weapon_tier
        if self.options.progressive_survivors.value:
            pool += [PROGRESSIVE_SURVIVOR] * MAX_SURVIVOR_COMPANIONS
        if self.options.survival_medals.value:
            pool += list(self.survival_checks)

        location_count = len(self.multiworld.get_unfilled_locations(self.player))
        filler_count = location_count - len(pool)
        if filler_count < 0:
            raise OptionError(
                f"{self.player_name}: {len(pool)} required items do not fit into {location_count} locations."
            )

        if self.options.enable_trap_items.value:
            trap_count = filler_count * self.options.trap_fill_percentage.value // 100
            pool += self.random.choices(trap_item_names, k=trap_count)
            filler_count -= trap_count

        pool += [filler_item_names[index % len(filler_item_names)] for index in range(filler_count)]
        self.multiworld.itempool += [self.create_item(name) for name in pool]

    def set_rules(self) -> None:
        set_rules(self)

    def fill_slot_data(self) -> dict[str, Any]:
        scale = self.options.survival_time_scale.value
        survival = {
            "enabled": bool(self.options.survival_medals.value),
            "medal": self.options.survival_medals.value,
            "scale": scale,
            # Base times 4:00/7:00/10:00 scaled
            "times": {
                "bronze": int(240 * scale / 100),
                "silver": int(420 * scale / 100),
                "gold": int(600 * scale / 100),
            },
            "levels": [
                {
                    "map": rows[0].map_name,
                    "campaign": rows[0].campaign,
                    "name": level,
                    "checks": {str(medal): location.name for medal, location in enumerate(rows, start=1)},
                }
                for level, rows in self.survival_checks.items()
            ],
        }
        return {
            "route": route_slot_data(self.routes),
            "vanilla_campaign_order": list(CAMPAIGNS),
            "weapon_tiers": {
                "enabled": bool(self.options.progressive_weapon_tiers.value),
                "max_tier": max_weapon_tier,
                "tiers": dict(weapon_tiers),
            },
            "survival": survival,
            "survivor_progression": {
                "enabled": bool(self.options.progressive_survivors.value),
                "max_companions": MAX_SURVIVOR_COMPANIONS,
            },
            "options": self.options.as_dict(*SLOT_DATA_OPTIONS),
            "total_locations": len(self.multiworld.get_locations(self.player)),
        }

    def get_filler_item_name(self) -> str:
        return filler_item_names[0]
