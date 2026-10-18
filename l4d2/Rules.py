from typing import TYPE_CHECKING

from rule_builder.options import OptionFilter
from rule_builder.rules import AtLeast, CanReachLocation, Has, Rule

from .Items import PROGRESSIVE_SURVIVOR, PROGRESSIVE_WEAPON_TIER
from .Options import LoadoutLogic, ProgressiveSurvivors, ProgressiveWeaponTiers

if TYPE_CHECKING:
    from . import L4D2World


def loadout(survivors: int = 0, weapon_tiers: int = 0) -> Rule:
    return (
        Has(PROGRESSIVE_SURVIVOR, 
            survivors, 
            options=OptionFilter(ProgressiveSurvivors, ProgressiveSurvivors.option_true), 
            filtered_resolution=True)
        & Has(PROGRESSIVE_WEAPON_TIER, 
              weapon_tiers, 
              options=OptionFilter(ProgressiveWeaponTiers, ProgressiveWeaponTiers.option_true),
              filtered_resolution=True)
    )


LOADOUT_REQUIREMENTS: dict[str, Rule] = {
    "Dead Center - Hotel: Reach the Banquet Hall": loadout(weapon_tiers=1),
    "Dead Center - Hotel": loadout(weapon_tiers=1),

    "Dead Center - Streets": loadout(survivors=1, weapon_tiers=2),
    "Dead Center - Streets: Gun Store Intercom": loadout(survivors=1, weapon_tiers=1),
    "Dead Center - Streets: Deliver Soda": loadout(survivors=1, weapon_tiers=2),

    "Dead Center - Mall": loadout(survivors=2, weapon_tiers=1),
    "Dead Center - Mall: Disable Alarm": loadout(survivors=2, weapon_tiers=1),

    "Dead Center - Atrium": loadout(survivors=2, weapon_tiers=3) | loadout(survivors=3, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #1": loadout(survivors=1, weapon_tiers=1) | loadout(survivors=2),
    "Dead Center - Atrium: Fuel #2": loadout(survivors=1, weapon_tiers=1) | loadout(survivors=2),
    "Dead Center - Atrium: Fuel #3": loadout(survivors=2, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #4": loadout(survivors=2, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #5": loadout(survivors=2, weapon_tiers=3) | loadout(survivors=3, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #6": loadout(survivors=2, weapon_tiers=3) | loadout(survivors=3, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #7": loadout(survivors=2, weapon_tiers=3) | loadout(survivors=3, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #8": loadout(survivors=2, weapon_tiers=3) | loadout(survivors=3, weapon_tiers=2),

    "Dark Carnival - Highway": loadout(survivors=1),
    "Dark Carnival - Highway: Get Through the Motel": loadout(survivors=1),

    "Dark Carnival - Fairgrounds": loadout(survivors=1, weapon_tiers=2),
    "Dark Carnival - Fairgrounds: Win the Shooting Game": loadout(weapon_tiers=1),
    "Dark Carnival - Fairgrounds: Ride the Slide": loadout(survivors=1, weapon_tiers=1),
    "Dark Carnival - Fairgrounds: Activate Carousel": loadout(survivors=1, weapon_tiers=2),
    "Dark Carnival - Fairgrounds: Deactivate Carousel": loadout(survivors=1, weapon_tiers=2),

    "Dark Carnival - Coaster": loadout(survivors=2, weapon_tiers=2),
    "Dark Carnival - Coaster: Start Roller Coaster": loadout(survivors=1, weapon_tiers=1),
    "Dark Carnival - Coaster: Stop the Alarm": loadout(survivors=2, weapon_tiers=2),

    "Dark Carnival - Barns": loadout(survivors=2, weapon_tiers=2),
    "Dark Carnival - Barns: Beat 'Stache Whacker": loadout(survivors=1),
    "Dark Carnival - Barns: Activate Gate": loadout(survivors=1, weapon_tiers=1),

    "Dark Carnival - Concert": loadout(survivors=2, weapon_tiers=2),

    "Swamp Fever - Plank Country": loadout(survivors=1, weapon_tiers=1),
    "Swamp Fever - Plank Country: Cross the River": loadout(survivors=1, weapon_tiers=1),

    "Swamp Fever - Swamp": loadout(survivors=2, weapon_tiers=2),
    "Swamp Fever - Swamp: Blow the Door": loadout(survivors=1, weapon_tiers=1),

    "Swamp Fever - Shantytown": loadout(survivors=1, weapon_tiers=1),
    "Swamp Fever - Shantytown: Lower the Bridge": loadout(survivors=1),

    "Swamp Fever - Plantation": loadout(survivors=2, weapon_tiers=1),
    "Swamp Fever - Plantation: Enter the Mansion": loadout(survivors=1, weapon_tiers=1),
    "Swamp Fever - Plantation: Call Virgil": loadout(survivors=1, weapon_tiers=1),

    "Hard Rain - Milltown": loadout(survivors=1) | loadout(weapon_tiers=2),
    "Hard Rain - Milltown: Get Past the Garage Sale": loadout(survivors=1) | loadout(weapon_tiers=1),

    "Hard Rain - Sugar Mill": loadout(survivors=2, weapon_tiers=2),
    "Hard Rain - Sugar Mill: Call Elevator": loadout(survivors=2, weapon_tiers=1),
    "Hard Rain - Sugar Mill: Ride Elevator": loadout(survivors=2, weapon_tiers=2),
    "Hard Rain - Sugar Mill: Survive the Horde": loadout(survivors=2, weapon_tiers=2),

    "Hard Rain - Sugar Mill Escape": loadout(survivors=1, weapon_tiers=1),
    "Hard Rain - Sugar Mill Escape: Activate Elevator": loadout(survivors=1),
    "Hard Rain - Sugar Mill Escape: Ride Elevator": loadout(survivors=1),

    "Hard Rain - Return to Milltown": loadout(survivors=1, weapon_tiers=1),

    "Hard Rain - Milltown Escape": loadout(survivors=2, weapon_tiers=2),
    "Hard Rain - Milltown Escape: Survive": loadout(survivors=1, weapon_tiers=1),

    "The Parish - Waterfront": loadout(survivors=1),
    "The Parish - Waterfront: Get Through the Restaurant": loadout(survivors=1),

    "The Parish - Park": loadout(survivors=2, weapon_tiers=1),
    "The Parish - Park: Enter the Decontamination Unit": loadout(survivors=1),
    "The Parish - Park: Stop the Alarm": loadout(survivors=2, weapon_tiers=1),

    "The Parish - Cemetery": loadout(survivors=1, weapon_tiers=1),
    "The Parish - Cemetery: Get on the Freeway": loadout(survivors=1, weapon_tiers=1),

    "The Parish - Quarter": loadout(survivors=2, weapon_tiers=1),
    "The Parish - Quarter: Reach the Courtyard": loadout(survivors=1),
    "The Parish - Quarter: Start the Tractor": loadout(survivors=1),

    "The Parish - Bridge": loadout(survivors=2, weapon_tiers=2),
    "The Parish - Bridge: Reach the Off-Ramp": loadout(survivors=2, weapon_tiers=2),

    "The Passing - Riverbank": loadout(survivors=2, weapon_tiers=1),
    "The Passing - Riverbank: Enter the Park": loadout(survivors=1, weapon_tiers=1),
    "The Passing - Riverbank: Change the Song": loadout(survivors=1, weapon_tiers=1),

    "The Passing - Underground": loadout(survivors=1, weapon_tiers=2),
    "The Passing - Underground: Turn on the Jukebox": loadout(survivors=1, weapon_tiers=1),
    "The Passing - Underground: Open Gate": loadout(survivors=1, weapon_tiers=2),
    "The Passing - Underground: Open Second Gate": loadout(survivors=1, weapon_tiers=2),

    "The Passing - Port": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #1": loadout(survivors=1, weapon_tiers=2),
    "The Passing - Port: Fuel #2": loadout(survivors=1, weapon_tiers=2),
    "The Passing - Port: Fuel #3": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #4": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #5": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #6": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #7": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #8": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #9": loadout(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #10": loadout(survivors=2, weapon_tiers=3),

    "The Sacrifice - Docks": loadout(survivors=2, weapon_tiers=2),
    "The Sacrifice - Docks: Reach the Wrecked Train": loadout(survivors=1),
    "The Sacrifice - Docks: Open Train Door": loadout(survivors=1, weapon_tiers=2),
    "The Sacrifice - Docks: Get Past the Tank": loadout(survivors=2, weapon_tiers=2),

    "The Sacrifice - Barge": loadout(survivors=1, weapon_tiers=1),
    "The Sacrifice - Barge: Reach the Barge": loadout(survivors=1),

    "The Sacrifice - Port": loadout(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Start the First Generator": loadout(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Start the Second Generator": loadout(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Start the Third Generator": loadout(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Lift the Bridge": loadout(survivors=2, weapon_tiers=3),

    "No Mercy - Apartments": loadout(survivors=1),

    "No Mercy - Subway": loadout(survivors=2, weapon_tiers=1),
    "No Mercy - Subway: Open Garage Door": loadout(survivors=1, weapon_tiers=1),

    "No Mercy - Sewers": loadout(survivors=2, weapon_tiers=2),
    "No Mercy - Sewers: Open Shutter Door": loadout(survivors=1, weapon_tiers=2),
    "No Mercy - Sewers: Open Manhole": loadout(survivors=2, weapon_tiers=2),

    "No Mercy - Hospital": loadout(survivors=2, weapon_tiers=1),
    "No Mercy - Hospital: Call Elevator": loadout(survivors=1) | loadout(weapon_tiers=1),
    "No Mercy - Hospital: Ride Elevator Up": loadout(survivors=2, weapon_tiers=1),

    "No Mercy - Rooftop": loadout(survivors=2, weapon_tiers=2),

    "Crash Course - Alleys": loadout(survivors=1, weapon_tiers=1),
    "Crash Course - Alleys: Reach the End of the Alleyway": loadout(weapon_tiers=1),
    "Crash Course - Alleys: Reach the Waterfront": loadout(survivors=1, weapon_tiers=1),
    "Crash Course - Alleys: Fire the Howitzer": loadout(survivors=1, weapon_tiers=1),
    "Crash Course - Alleys: Enter the Bridge": loadout(survivors=1, weapon_tiers=1),

    "Crash Course - Truck Depot": loadout(survivors=2, weapon_tiers=3),
    "Crash Course - Truck Depot: Enter the Depot": loadout(survivors=1, weapon_tiers=2),
    "Crash Course - Truck Depot: Start the Generator": loadout(survivors=1, weapon_tiers=2),
    "Crash Course - Truck Depot: Restart the Generator": loadout(survivors=2, weapon_tiers=2),

    "Death Toll - Turnpike": loadout(survivors=1, weapon_tiers=1),
    "Death Toll - Turnpike: Exit the Tunnel": loadout(survivors=1, weapon_tiers=1),

    "Death Toll - Drains": loadout(survivors=1, weapon_tiers=2),
    "Death Toll - Drains: Activate Floodgates": loadout(survivors=1, weapon_tiers=2),

    "Death Toll - Church": loadout(survivors=1, weapon_tiers=2),
    "Death Toll - Church: Reach Safehouse": loadout(survivors=1, weapon_tiers=1),
    "Death Toll - Church: Play the Piano": loadout(survivors=1, weapon_tiers=1),

    "Death Toll - Town": loadout(survivors=1, weapon_tiers=2),
    "Death Toll - Town: Lower the Forklift": loadout(survivors=1, weapon_tiers=1),

    "Death Toll - Boathouse": loadout(survivors=2, weapon_tiers=3),
    "Death Toll - Boathouse: Activate Radio": loadout(survivors=1, weapon_tiers=2),

    "Dead Air - Greenhouse": loadout(survivors=1) | loadout(weapon_tiers=2),
    "Dead Air - Greenhouse: Reach Ground Level": loadout(survivors=1) | loadout(weapon_tiers=1),

    "Dead Air - Crane": loadout(survivors=1, weapon_tiers=2),
    "Dead Air - Crane: Activate Crane": loadout(survivors=1, weapon_tiers=2),
    "Dead Air - Crane: Navigate the Office Building": loadout(survivors=1, weapon_tiers=2),

    "Dead Air - Construction Site": loadout(survivors=2, weapon_tiers=2),

    "Dead Air - Terminal": loadout(survivors=2, weapon_tiers=2),
    "Dead Air - Terminal: Slam the Van Through the Gate": loadout(survivors=1, weapon_tiers=1),
    "Dead Air - Terminal: Metal Detector": loadout(survivors=1, weapon_tiers=2),

    "Dead Air - Runway": loadout(survivors=2, weapon_tiers=3),

    "Blood Harvest - Woods": loadout(survivors=1) | loadout(weapon_tiers=2),

    "Blood Harvest - Tunnel": loadout(survivors=2, weapon_tiers=2),
    "Blood Harvest - Tunnel: Open Emergency Exit": loadout(survivors=1, weapon_tiers=2),

    "Blood Harvest - Bridge": loadout(survivors=2, weapon_tiers=2),
    "Blood Harvest - Bridge: Decouple Train": loadout(survivors=2, weapon_tiers=2),

    "Blood Harvest - Train Station": loadout(survivors=1) | loadout(weapon_tiers=2),

    "Blood Harvest - Farmhouse": loadout(survivors=2, weapon_tiers=2),
    "Blood Harvest - Farmhouse: Enter the Cornfield": loadout(survivors=1),
    "Blood Harvest - Farmhouse: Activate Radio": loadout(survivors=1),

    "Cold Stream - Alpine Creek": loadout(survivors=1),
    "Cold Stream - Alpine Creek: Open the Bunker Door": loadout(survivors=1),

    "Cold Stream - South Pine Stream": loadout(survivors=2, weapon_tiers=2),
    "Cold Stream - South Pine Stream: Blow the Barricades": loadout(survivors=1, weapon_tiers=2),

    "Cold Stream - Memorial Bridge": loadout(survivors=2, weapon_tiers=2),
    "Cold Stream - Memorial Bridge: Tanker Explosion": loadout(survivors=2, weapon_tiers=2),
    "Cold Stream - Memorial Bridge: Bridge Explosion": loadout(survivors=2, weapon_tiers=2),

    "Cold Stream - Cutthroat Creek": loadout(survivors=1, weapon_tiers=2),

    "The Last Stand - Junkyard": loadout(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Shop Yard Entrance": loadout(survivors=1),
    "The Last Stand - Junkyard: Turn the Fuel Valve": loadout(survivors=1, weapon_tiers=1),
    "The Last Stand - Junkyard: Jumpstart Generator #1": loadout(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Jumpstart Generator #2": loadout(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Jumpstart Generator #3": loadout(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Jumpstart Generator #4": loadout(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Drop the Container": loadout(survivors=2, weapon_tiers=2),

    "The Last Stand - Lighthouse": loadout(survivors=3, weapon_tiers=4),
    "The Last Stand - Lighthouse: Start the Generator": loadout(survivors=1, weapon_tiers=2),
    "The Last Stand - Lighthouse: Survive the First Horde": loadout(survivors=2, weapon_tiers=2),
    "The Last Stand - Lighthouse: Fuel #1": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #2": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #3": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #4": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #5": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #6": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #7": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #8": loadout(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Survive the Second Horde": loadout(survivors=3, weapon_tiers=4),
}


def _set_loadout_rules(world: "L4D2World") -> None:
    for location in world.multiworld.get_locations(world.player):
        rule = LOADOUT_REQUIREMENTS.get(location.name)
        if rule is not None:
            world.set_rule(location, rule)


def _completion_condition(world: "L4D2World") -> Rule:
    finales = [CanReachLocation(chapter.name) for chapter in world.chapters if chapter.finale]
    return AtLeast(world.options.goal.value, *finales)


def set_rules(world: "L4D2World") -> None:
    if world.options.loadout_logic.value == LoadoutLogic.option_enabled:
        _set_loadout_rules(world)

    world.set_completion_rule(_completion_condition(world))
