from dataclasses import dataclass
from typing import Literal


@dataclass(frozen=True)
class LoadoutRequirement:
    survivors: int | None = None
    weapon_tiers: int | None = None
    operator: Literal["and", "or"] = "and"


LOADOUT_REQUIREMENTS: dict[str, LoadoutRequirement] = {
    "Dead Center - Hotel: Reach the Banquet Hall": LoadoutRequirement(weapon_tiers=1),
    "Dead Center - Hotel": LoadoutRequirement(weapon_tiers=1),

    "Dead Center - Streets": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Dead Center - Streets: Gun Store Intercom": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Dead Center - Streets: Deliver Soda": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Dead Center - Mall": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "Dead Center - Mall: Disable Alarm": LoadoutRequirement(survivors=2, weapon_tiers=1),

    "Dead Center - Atrium": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "Dead Center - Atrium: Fuel #1": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Dead Center - Atrium: Fuel #2": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Dead Center - Atrium: Fuel #3": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #4": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Dead Center - Atrium: Fuel #5": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "Dead Center - Atrium: Fuel #6": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "Dead Center - Atrium: Fuel #7": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "Dead Center - Atrium: Fuel #8": LoadoutRequirement(survivors=2, weapon_tiers=3),

    "Dark Carnival - Highway": LoadoutRequirement(survivors=1),
    "Dark Carnival - Highway: Get Through the Motel": LoadoutRequirement(survivors=1),

    "Dark Carnival - Fairgrounds": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Dark Carnival - Fairgrounds: Win the Shooting Game": LoadoutRequirement(weapon_tiers=1),
    "Dark Carnival - Fairgrounds: Ride the Slide": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Dark Carnival - Fairgrounds: Activate Carousel": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Dark Carnival - Fairgrounds: Deactivate Carousel": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Dark Carnival - Coaster": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Dark Carnival - Coaster: Start Roller Coaster": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Dark Carnival - Coaster: Stop the Alarm": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Dark Carnival - Barns": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Dark Carnival - Barns: Beat 'Stache Whacker": LoadoutRequirement(survivors=1),
    "Dark Carnival - Barns: Activate Gate": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Dark Carnival - Concert": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Swamp Fever - Plank Country": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Swamp Fever - Plank Country: Cross the River": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Swamp Fever - Swamp": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Swamp Fever - Swamp: Blow the Door": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Swamp Fever - Shantytown": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Swamp Fever - Shantytown: Lower the Bridge": LoadoutRequirement(survivors=1),

    "Swamp Fever - Plantation": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "Swamp Fever - Plantation: Enter the Mansion": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Swamp Fever - Plantation: Call Virgil": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Hard Rain - Milltown": LoadoutRequirement(survivors=1, weapon_tiers=2, operator="or"),
    "Hard Rain - Milltown: Get Past the Garage Sale": LoadoutRequirement(survivors=1, weapon_tiers=1, operator="or"),

    "Hard Rain - Sugar Mill": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Hard Rain - Sugar Mill: Call Elevator": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "Hard Rain - Sugar Mill: Ride Elevator": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Hard Rain - Sugar Mill: Survive the Horde": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Hard Rain - Sugar Mill Escape": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Hard Rain - Sugar Mill Escape: Activate Elevator": LoadoutRequirement(survivors=1),
    "Hard Rain - Sugar Mill Escape: Ride Elevator": LoadoutRequirement(survivors=1),

    "Hard Rain - Return to Milltown": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Hard Rain - Milltown Escape": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Hard Rain - Milltown Escape: Survive": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "The Parish - Waterfront": LoadoutRequirement(survivors=1),
    "The Parish - Waterfront: Get Through the Restaurant": LoadoutRequirement(survivors=1),

    "The Parish - Park": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "The Parish - Park: Enter the Decontamination Unit": LoadoutRequirement(survivors=1),
    "The Parish - Park: Stop the Alarm": LoadoutRequirement(survivors=2, weapon_tiers=1),

    "The Parish - Cemetery": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "The Parish - Cemetery: Get on the Freeway": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "The Parish - Quarter": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "The Parish - Quarter: Reach the Courtyard": LoadoutRequirement(survivors=1),
    "The Parish - Quarter: Start the Tractor": LoadoutRequirement(survivors=1),

    "The Parish - Bridge": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Parish - Bridge: Reach the Off-Ramp": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "The Passing - Riverbank": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "The Passing - Riverbank: Enter the Park": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "The Passing - Riverbank: Change the Song": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "The Passing - Underground": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "The Passing - Underground: Turn on the Jukebox": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "The Passing - Underground: Open Gate": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "The Passing - Underground: Open Second Gate": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "The Passing - Port": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #1": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "The Passing - Port: Fuel #2": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "The Passing - Port: Fuel #3": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #4": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #5": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #6": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #7": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #8": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #9": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Passing - Port: Fuel #10": LoadoutRequirement(survivors=2, weapon_tiers=3),

    "The Sacrifice - Docks": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Sacrifice - Docks: Reach the Wrecked Train": LoadoutRequirement(survivors=1),
    "The Sacrifice - Docks: Open Train Door": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "The Sacrifice - Docks: Get Past the Tank": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "The Sacrifice - Barge": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "The Sacrifice - Barge: Reach the Barge": LoadoutRequirement(survivors=1),

    "The Sacrifice - Port": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Start the First Generator": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Start the Second Generator": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Start the Third Generator": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "The Sacrifice - Port: Lift the Bridge": LoadoutRequirement(survivors=2, weapon_tiers=3),

    "No Mercy - Apartments": LoadoutRequirement(survivors=1),

    "No Mercy - Subway": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "No Mercy - Subway: Open Garage Door": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "No Mercy - Sewers": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "No Mercy - Sewers: Open Shutter Door": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "No Mercy - Sewers: Open Manhole": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "No Mercy - Hospital": LoadoutRequirement(survivors=2, weapon_tiers=1),
    "No Mercy - Hospital: Call Elevator": LoadoutRequirement(survivors=1, weapon_tiers=1, operator="or"),
    "No Mercy - Hospital: Ride Elevator Up": LoadoutRequirement(survivors=2, weapon_tiers=1),

    "No Mercy - Rooftop": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Crash Course - Alleys": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Crash Course - Alleys: Reach the End of the Alleyway": LoadoutRequirement(weapon_tiers=1),
    "Crash Course - Alleys: Reach the Waterfront": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Crash Course - Alleys: Fire the Howitzer": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Crash Course - Alleys: Enter the Bridge": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Crash Course - Truck Depot": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "Crash Course - Truck Depot: Enter the Depot": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Crash Course - Truck Depot: Start the Generator": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Crash Course - Truck Depot: Restart the Generator": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Death Toll - Turnpike": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Death Toll - Turnpike: Exit the Tunnel": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Death Toll - Drains": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Death Toll - Drains: Activate Floodgates": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Death Toll - Church": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Death Toll - Church: Reach Safehouse": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Death Toll - Church: Play the Piano": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Death Toll - Town": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Death Toll - Town: Lower the Forklift": LoadoutRequirement(survivors=1, weapon_tiers=1),

    "Death Toll - Boathouse": LoadoutRequirement(survivors=2, weapon_tiers=3),
    "Death Toll - Boathouse: Activate Radio": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Dead Air - Greenhouse": LoadoutRequirement(survivors=1, weapon_tiers=2, operator="or"),
    "Dead Air - Greenhouse: Reach Ground Level": LoadoutRequirement(survivors=1, weapon_tiers=1, operator="or"),

    "Dead Air - Crane": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Dead Air - Crane: Activate Crane": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "Dead Air - Crane: Navigate the Office Building": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Dead Air - Construction Site": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Dead Air - Terminal": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Dead Air - Terminal: Slam the Van Through the Gate": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "Dead Air - Terminal: Metal Detector": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Dead Air - Runway": LoadoutRequirement(survivors=2, weapon_tiers=3),

    "Blood Harvest - Woods": LoadoutRequirement(survivors=1, weapon_tiers=2, operator="or"),

    "Blood Harvest - Tunnel": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Blood Harvest - Tunnel: Open Emergency Exit": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Blood Harvest - Bridge": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Blood Harvest - Bridge: Decouple Train": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Blood Harvest - Train Station": LoadoutRequirement(survivors=1, weapon_tiers=2, operator="or"),

    "Blood Harvest - Farmhouse": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Blood Harvest - Farmhouse: Enter the Cornfield": LoadoutRequirement(survivors=1),
    "Blood Harvest - Farmhouse: Activate Radio": LoadoutRequirement(survivors=1),

    "Cold Stream - Alpine Creek": LoadoutRequirement(survivors=1),
    "Cold Stream - Alpine Creek: Open the Bunker Door": LoadoutRequirement(survivors=1),

    "Cold Stream - South Pine Stream": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Cold Stream - South Pine Stream: Blow the Barricades": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "Cold Stream - Memorial Bridge": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Cold Stream - Memorial Bridge: Tanker Explosion": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "Cold Stream - Memorial Bridge: Bridge Explosion": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "Cold Stream - Cutthroat Creek": LoadoutRequirement(survivors=1, weapon_tiers=2),

    "The Last Stand - Junkyard": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Shop Yard Entrance": LoadoutRequirement(survivors=1),
    "The Last Stand - Junkyard: Turn the Fuel Valve": LoadoutRequirement(survivors=1, weapon_tiers=1),
    "The Last Stand - Junkyard: Jumpstart Generator #1": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Jumpstart Generator #2": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Jumpstart Generator #3": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Jumpstart Generator #4": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Last Stand - Junkyard: Drop the Container": LoadoutRequirement(survivors=2, weapon_tiers=2),

    "The Last Stand - Lighthouse": LoadoutRequirement(survivors=3, weapon_tiers=4),
    "The Last Stand - Lighthouse: Start the Generator": LoadoutRequirement(survivors=1, weapon_tiers=2),
    "The Last Stand - Lighthouse: Survive the First Horde": LoadoutRequirement(survivors=2, weapon_tiers=2),
    "The Last Stand - Lighthouse: Fuel #1": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #2": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #3": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #4": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #5": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #6": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #7": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Fuel #8": LoadoutRequirement(survivors=2, weapon_tiers=4),
    "The Last Stand - Lighthouse: Survive the Second Horde": LoadoutRequirement(survivors=3, weapon_tiers=4),
}
