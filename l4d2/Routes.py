from dataclasses import dataclass
from random import Random
from typing import Any

from .Locations import location_table
from .Options import Randomization

CAMPAIGNS: tuple[str, ...] = tuple(location_table)


@dataclass(frozen=True)
class Chapter:
    name: str
    vanilla_campaign: str
    map_name: str
    finale: bool
    vanilla_order: int


@dataclass(frozen=True)
class Route:
    campaign: str
    chapters: tuple[Chapter, ...]


def _campaign_chapters() -> dict[str, tuple[Chapter, ...]]:
    chapters: dict[str, tuple[Chapter, ...]] = {}
    for campaign, locations in location_table.items():
        chapter_rows = [location for location in locations if location.is_chapter]
        chapters[campaign] = tuple(
            Chapter(row.name, campaign, row.map_name, row.is_finale, order)
            for order, row in enumerate(chapter_rows)
        )
    return chapters


CHAPTERS_BY_CAMPAIGN: dict[str, tuple[Chapter, ...]] = _campaign_chapters()
CHAPTERS_BY_NAME: dict[str, Chapter] = {
    chapter.name: chapter for chapters in CHAPTERS_BY_CAMPAIGN.values() for chapter in chapters
}


def build_routes(random: Random, start_campaign: str, randomization: int, goal: int) -> tuple[Route, ...]:
    if randomization == Randomization.option_campaign:
        return _campaign_routes(random, start_campaign, goal)
    if randomization == Randomization.option_chapter:
        return _chapter_routes(random, start_campaign, goal)
    raise ValueError(f"Unknown randomization option {randomization!r}.")


def _campaign_routes(random: Random, start_campaign: str, goal: int) -> tuple[Route, ...]:
    campaigns = list(CAMPAIGNS)
    random.shuffle(campaigns)
    campaigns.remove(start_campaign)
    campaigns.insert(0, start_campaign)

    return tuple(
        Route(campaign, CHAPTERS_BY_CAMPAIGN[campaign]) for campaign in campaigns[:goal]
    )


def _chapter_routes(random: Random, start_campaign: str, goal: int) -> tuple[Route, ...]:
    route_count = max(goal, 3)
    start_chapter = CHAPTERS_BY_CAMPAIGN[start_campaign][0]

    other_campaigns = [campaign for campaign in CAMPAIGNS if campaign != start_campaign]
    random.shuffle(other_campaigns)
    selected_campaigns = [start_campaign, *other_campaigns[: route_count - 1]]

    finales = [
        chapter
        for campaign in selected_campaigns
        for chapter in CHAPTERS_BY_CAMPAIGN[campaign]
        if chapter.finale
    ]
    if len(finales) != route_count:
        raise ValueError(
            f"Expected exactly one finale in each of the {route_count} selected campaigns, found {len(finales)}."
        )

    chapter_pool = [
        chapter
        for campaign in CAMPAIGNS
        for chapter in CHAPTERS_BY_CAMPAIGN[campaign]
        if not chapter.finale and chapter != start_chapter
    ]
    random.shuffle(finales)
    random.shuffle(chapter_pool)

    routes: list[Route] = []
    cursor = 0
    for index, finale in enumerate(finales):
        head = (start_chapter,) if index == 0 else ()
        body_size = len(CHAPTERS_BY_CAMPAIGN[finale.vanilla_campaign]) - len(head) - 1
        body = chapter_pool[cursor : cursor + body_size]
        cursor += body_size
        routes.append(Route(finale.vanilla_campaign, (*head, *body, finale)))

    if cursor > len(chapter_pool):
        raise ValueError("Not enough chapters to fill every route.")
    return tuple(routes)


def starting_chapters(routes: tuple[Route, ...], all_campaigns_start: bool) -> list[str]:
    if all_campaigns_start:
        return [route.chapters[0].name for route in routes]
    return [routes[0].chapters[0].name]


def route_slot_data(routes: tuple[Route, ...]) -> list[dict[str, Any]]:
    return [
        {
            "campaign": route.campaign,
            "chapters": [
                {
                    "name": chapter.name,
                    "map": chapter.map_name,
                    "finale": chapter.finale,
                    "vanilla_campaign": chapter.vanilla_campaign,
                    "vanilla_order": chapter.vanilla_order,
                }
                for chapter in route.chapters
            ],
        }
        for route in routes
    ]


def routes_from_slot_data(route_data: Any) -> tuple[Route, ...]:
    """Rebuild the routes of a seed from its slot data for UT."""
    if not isinstance(route_data, list) or not route_data:
        raise ValueError("Left 4 Dead 2 slot data does not contain a route.")

    routes: list[Route] = []
    seen_chapters: set[str] = set()
    for entry in route_data:
        if not isinstance(entry, dict):
            raise ValueError("Left 4 Dead 2 slot data contains a malformed route.")
        campaign = entry["campaign"]
        chapters: list[Chapter] = []
        for chapter_entry in entry["chapters"]:
            name = chapter_entry["name"]
            if name in seen_chapters:
                raise ValueError(f"Left 4 Dead 2 slot data contains chapter {name!r} twice.")
            if name not in CHAPTERS_BY_NAME:
                raise ValueError(f"Left 4 Dead 2 slot data contains unknown chapter {name!r}.")
            seen_chapters.add(name)
            chapters.append(CHAPTERS_BY_NAME[name])
        routes.append(Route(campaign, tuple(chapters)))
    return tuple(routes)
