from typing import TYPE_CHECKING

from BaseClasses import CollectionRule
from Options import OptionError
from worlds.generic.Rules import set_rule

from .Items import PROGRESSIVE_SURVIVOR, PROGRESSIVE_WEAPON_TIER
from .LoadoutRules import LOADOUT_REQUIREMENTS, LoadoutRequirement
from .Options import LoadoutLogic

if TYPE_CHECKING:
    from . import L4D2World


REQUIRED_SURVIVORS: int = max((req.survivors or 0 for req in LOADOUT_REQUIREMENTS.values()), default=0)
REQUIRED_WEAPON_TIERS: int = max((req.weapon_tiers or 0 for req in LOADOUT_REQUIREMENTS.values()), default=0)


def validate_options(world: "L4D2World") -> None:
    if world.options.loadout_logic.value != LoadoutLogic.option_enabled:
        return

    if REQUIRED_SURVIVORS and not world.options.progressive_survivors.value:
        raise OptionError(
            f"{world.player_name}: Loadout Logic is enabled and requires Progressive Survivor items, "
            "but Progressive AI Survivors is disabled."
        )
    if REQUIRED_WEAPON_TIERS and not world.options.progressive_weapon_tiers.value:
        raise OptionError(
            f"{world.player_name}: Loadout Logic is enabled and requires Progressive Weapon Tier items, "
            "but Progressive Weapon Tiers is disabled."
        )


def _loadout_rule(player: int, requirement: LoadoutRequirement) -> CollectionRule:
    item_counts: dict[str, int] = {}
    if requirement.survivors is not None:
        item_counts[PROGRESSIVE_SURVIVOR] = requirement.survivors
    if requirement.weapon_tiers is not None:
        item_counts[PROGRESSIVE_WEAPON_TIER] = requirement.weapon_tiers

    if requirement.operator == "or":
        return lambda state: state.has_any_count(item_counts, player)
    return lambda state: state.has_all_counts(item_counts, player)


def _set_loadout_rules(world: "L4D2World") -> None:
    for location in world.multiworld.get_locations(world.player):
        requirement = LOADOUT_REQUIREMENTS.get(location.name)
        if requirement is not None:
            set_rule(location, _loadout_rule(world.player, requirement))


def _completion_condition(world: "L4D2World") -> CollectionRule:
    goal = world.options.goal.value
    finales = [
        world.multiworld.get_location(chapter.name, world.player) for chapter in world.chapters if chapter.finale
    ]
    return lambda state: sum(finale.can_reach(state) for finale in finales) >= goal


def set_rules(world: "L4D2World") -> None:
    if world.options.loadout_logic.value == LoadoutLogic.option_enabled:
        _set_loadout_rules(world)

    world.multiworld.completion_condition[world.player] = _completion_condition(world)
