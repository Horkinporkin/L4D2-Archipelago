from typing import TYPE_CHECKING

from BaseClasses import CollectionRule, Region

from .Locations import L4D2Location, carrier_by_chapter, locations_by_chapter

if TYPE_CHECKING:
    from . import L4D2World


def _has(item: str, player: int) -> CollectionRule:
    return lambda state: state.has(item, player)


def _has_any(items: tuple[str, ...], player: int) -> CollectionRule:
    return lambda state: state.has_any(items, player)


def create_regions(world: "L4D2World") -> None:
    multiworld = world.multiworld
    player = world.player

    menu = Region("Menu", player, multiworld)
    multiworld.regions.append(menu)

    for route in world.routes:
        route_region = Region(route.campaign, player, multiworld)
        multiworld.regions.append(route_region)
        menu.connect(route_region, f"Menu to {route.campaign}")

        for chapter in route.chapters:
            chapter_region = Region(chapter.name, player, multiworld)
            chapter_region.add_locations(
                {location.name: location.ap_id for location in locations_by_chapter[chapter.name]},
                L4D2Location,
            )
            multiworld.regions.append(chapter_region)
            route_region.connect(chapter_region, f"{route.campaign} to {chapter.name}", _has(chapter.name, player))

    # Survival levels
    for level, rows in world.survival_checks.items():
        region = Region(level, player, multiworld)
        region.add_locations({location.name: location.ap_id for location in rows}, L4D2Location)
        multiworld.regions.append(region)
        menu.connect(region, f"Menu to {level}", _has(level, player))

    # AP Item Carriers
    survival_by_map = {rows[0].map_name: level for level, rows in world.survival_checks.items()}
    for chapter in world.chapters:
        carrier = carrier_by_chapter[chapter.name]
        access_items = (chapter.name,)
        if carrier.map_name in survival_by_map:
            access_items += (survival_by_map[carrier.map_name],)

        region = Region(f"{carrier.name} Region", player, multiworld)
        region.add_locations({carrier.name: carrier.ap_id}, L4D2Location)
        multiworld.regions.append(region)
        menu.connect(region, f"Menu to {carrier.name}", _has_any(access_items, player))
