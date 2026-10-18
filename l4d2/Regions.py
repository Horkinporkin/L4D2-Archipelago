from typing import TYPE_CHECKING

from BaseClasses import Region
from rule_builder.rules import Has, HasAny

from .Locations import L4D2Location, carrier_by_chapter, locations_by_chapter

if TYPE_CHECKING:
    from . import L4D2World


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
            world.create_entrance(
                route_region, chapter_region, Has(chapter.name), f"{route.campaign} to {chapter.name}"
            )

    # Survival levels
    for level, rows in world.survival_checks.items():
        region = Region(level, player, multiworld)
        region.add_locations({location.name: location.ap_id for location in rows}, L4D2Location)
        multiworld.regions.append(region)
        world.create_entrance(menu, region, Has(level), f"Menu to {level}")

    survival_by_map = {rows[0].map_name: level for level, rows in world.survival_checks.items()}
    for chapter in world.chapters:
        carrier = carrier_by_chapter[chapter.name]
        access_items = (chapter.name,)
        if carrier.map_name in survival_by_map:
            access_items += (survival_by_map[carrier.map_name],)

        region = Region(f"{carrier.name} Region", player, multiworld)
        region.add_locations({carrier.name: carrier.ap_id}, L4D2Location)
        multiworld.regions.append(region)
        world.create_entrance(menu, region, HasAny(*access_items), f"Menu to {carrier.name}")
