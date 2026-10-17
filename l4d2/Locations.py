from dataclasses import dataclass
from enum import Flag, auto

from BaseClasses import Location

l4d2_location_base_id = 8462000


class LocationType(Flag):
    CHAPTER = auto()
    FINALE = auto()
    INTERACTABLE = auto()
    ITEM = auto()
    SURVIVAL = auto()


@dataclass
class L4D2LocationData:
    name: str
    location_type: LocationType
    campaign: str
    map_name: str
    ap_id: int = 0

    @property
    def is_chapter(self) -> bool:
        return bool(self.location_type & LocationType.CHAPTER)

    @property
    def is_finale(self) -> bool:
        return bool(self.location_type & LocationType.FINALE)

    @property
    def is_survival(self) -> bool:
        return bool(self.location_type & LocationType.SURVIVAL)

    @property
    def is_item(self) -> bool:
        return bool(self.location_type & LocationType.ITEM)


class L4D2Location(Location):
    game = "Left 4 Dead 2"


# Chapter and interactable checks, grouped by their campaign, in vanilla order
# I am not sticking to 120 char per line for this :(
CAMPAIGN_LOCATIONS: dict[str, list[L4D2LocationData]] = {
    "Dead Center": [
        L4D2LocationData("Dead Center - Hotel", LocationType.CHAPTER, "Dead Center", "c1m1_hotel"),
        L4D2LocationData("Dead Center - Hotel: Reach Stairwell", LocationType.INTERACTABLE, "Dead Center", "c1m1_hotel"),
        L4D2LocationData("Dead Center - Hotel: Elevator", LocationType.INTERACTABLE, "Dead Center", "c1m1_hotel"),
        L4D2LocationData("Dead Center - Hotel: Reach the Banquet Hall", LocationType.INTERACTABLE, "Dead Center", "c1m1_hotel"),

        L4D2LocationData("Dead Center - Streets", LocationType.CHAPTER, "Dead Center", "c1m2_streets"),
        L4D2LocationData("Dead Center - Streets: Gun Store Intercom", LocationType.INTERACTABLE, "Dead Center", "c1m2_streets"),
        L4D2LocationData("Dead Center - Streets: Deliver Soda", LocationType.INTERACTABLE, "Dead Center", "c1m2_streets"),

        L4D2LocationData("Dead Center - Mall", LocationType.CHAPTER, "Dead Center", "c1m3_mall"),
        L4D2LocationData("Dead Center - Mall: Disable Alarm", LocationType.INTERACTABLE, "Dead Center", "c1m3_mall"),

        L4D2LocationData("Dead Center - Atrium", LocationType.CHAPTER | LocationType.FINALE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #1", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #2", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #3", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #4", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #5", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #6", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #7", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium: Fuel #8", LocationType.INTERACTABLE, "Dead Center", "c1m4_atrium")
    ],

    "Dark Carnival": [
        L4D2LocationData("Dark Carnival - Highway", LocationType.CHAPTER, "Dark Carnival", "c2m1_highway"),
        L4D2LocationData("Dark Carnival - Highway: Reach the Motel", LocationType.INTERACTABLE, "Dark Carnival", "c2m1_highway"),
        L4D2LocationData("Dark Carnival - Highway: Get Through the Motel", LocationType.INTERACTABLE, "Dark Carnival", "c2m1_highway"),

        L4D2LocationData("Dark Carnival - Fairgrounds", LocationType.CHAPTER, "Dark Carnival", "c2m2_fairgrounds"),
        L4D2LocationData("Dark Carnival - Fairgrounds: Win the Shooting Game", LocationType.INTERACTABLE, "Dark Carnival", "c2m2_fairgrounds"),
        L4D2LocationData("Dark Carnival - Fairgrounds: Ride the Slide", LocationType.INTERACTABLE, "Dark Carnival", "c2m2_fairgrounds"),
        L4D2LocationData("Dark Carnival - Fairgrounds: Activate Carousel", LocationType.INTERACTABLE, "Dark Carnival", "c2m2_fairgrounds"),
        L4D2LocationData("Dark Carnival - Fairgrounds: Deactivate Carousel", LocationType.INTERACTABLE, "Dark Carnival", "c2m2_fairgrounds"),

        L4D2LocationData("Dark Carnival - Coaster", LocationType.CHAPTER, "Dark Carnival", "c2m3_coaster"),
        L4D2LocationData("Dark Carnival - Coaster: Start Roller Coaster", LocationType.INTERACTABLE, "Dark Carnival", "c2m3_coaster"),
        L4D2LocationData("Dark Carnival - Coaster: Stop the Alarm", LocationType.INTERACTABLE, "Dark Carnival", "c2m3_coaster"),

        L4D2LocationData("Dark Carnival - Barns", LocationType.CHAPTER, "Dark Carnival", "c2m4_barns"),
        L4D2LocationData("Dark Carnival - Barns: Beat 'Stache Whacker", LocationType.INTERACTABLE, "Dark Carnival", "c2m4_barns"),
        L4D2LocationData("Dark Carnival - Barns: Activate Gate", LocationType.INTERACTABLE, "Dark Carnival", "c2m4_barns"),

        L4D2LocationData("Dark Carnival - Concert", LocationType.CHAPTER | LocationType.FINALE, "Dark Carnival", "c2m5_concert"),
        L4D2LocationData("Dark Carnival - Concert: Hit the Lights", LocationType.INTERACTABLE, "Dark Carnival", "c2m5_concert"),
        L4D2LocationData("Dark Carnival - Concert: Activate the Sparklers", LocationType.INTERACTABLE, "Dark Carnival", "c2m5_concert"),
        L4D2LocationData("Dark Carnival - Concert: Start the Show", LocationType.INTERACTABLE, "Dark Carnival", "c2m5_concert")
    ],

    "Swamp Fever": [
        L4D2LocationData("Swamp Fever - Plank Country", LocationType.CHAPTER, "Swamp Fever", "c3m1_plankcountry"),
        L4D2LocationData("Swamp Fever - Plank Country: Turn on the Jukebox", LocationType.INTERACTABLE, "Swamp Fever", "c3m1_plankcountry"),
        L4D2LocationData("Swamp Fever - Plank Country: Start the Winch", LocationType.INTERACTABLE, "Swamp Fever", "c3m1_plankcountry"),
        L4D2LocationData("Swamp Fever - Plank Country: Cross the River", LocationType.INTERACTABLE, "Swamp Fever", "c3m1_plankcountry"),

        L4D2LocationData("Swamp Fever - Swamp", LocationType.CHAPTER, "Swamp Fever", "c3m2_swamp"),
        L4D2LocationData("Swamp Fever - Swamp: Blow the Door", LocationType.INTERACTABLE, "Swamp Fever", "c3m2_swamp"),

        L4D2LocationData("Swamp Fever - Shantytown", LocationType.CHAPTER, "Swamp Fever", "c3m3_shantytown"),
        L4D2LocationData("Swamp Fever - Shantytown: Lower the Bridge", LocationType.INTERACTABLE, "Swamp Fever", "c3m3_shantytown"),

        L4D2LocationData("Swamp Fever - Plantation", LocationType.CHAPTER | LocationType.FINALE, "Swamp Fever", "c3m4_plantation"),
        L4D2LocationData("Swamp Fever - Plantation: Enter the Mansion", LocationType.INTERACTABLE, "Swamp Fever", "c3m4_plantation"),
        L4D2LocationData("Swamp Fever - Plantation: Call Virgil", LocationType.INTERACTABLE, "Swamp Fever", "c3m4_plantation")
    ],

    "Hard Rain": [
        L4D2LocationData("Hard Rain - Milltown", LocationType.CHAPTER, "Hard Rain", "c4m1_milltown_a"),
        L4D2LocationData("Hard Rain - Milltown: Jump the Wall", LocationType.INTERACTABLE, "Hard Rain", "c4m1_milltown_a"),
        L4D2LocationData("Hard Rain - Milltown: Get Past the Garage Sale", LocationType.INTERACTABLE, "Hard Rain", "c4m1_milltown_a"),

        L4D2LocationData("Hard Rain - Sugar Mill", LocationType.CHAPTER, "Hard Rain", "c4m2_sugarmill_a"),
        L4D2LocationData("Hard Rain - Sugar Mill: Call Elevator", LocationType.INTERACTABLE, "Hard Rain", "c4m2_sugarmill_a"),
        L4D2LocationData("Hard Rain - Sugar Mill: Ride Elevator", LocationType.INTERACTABLE, "Hard Rain", "c4m2_sugarmill_a"),
        L4D2LocationData("Hard Rain - Sugar Mill: Survive the Horde", LocationType.INTERACTABLE, "Hard Rain", "c4m2_sugarmill_a"),

        L4D2LocationData("Hard Rain - Sugar Mill Escape", LocationType.CHAPTER, "Hard Rain", "c4m3_sugarmill_b"),
        L4D2LocationData("Hard Rain - Sugar Mill Escape: Activate Elevator", LocationType.INTERACTABLE, "Hard Rain", "c4m3_sugarmill_b"),
        L4D2LocationData("Hard Rain - Sugar Mill Escape: Ride Elevator", LocationType.INTERACTABLE, "Hard Rain", "c4m3_sugarmill_b"),

        L4D2LocationData("Hard Rain - Return to Milltown", LocationType.CHAPTER, "Hard Rain", "c4m4_milltown_b"),

        L4D2LocationData("Hard Rain - Milltown Escape", LocationType.CHAPTER | LocationType.FINALE, "Hard Rain", "c4m5_milltown_escape"),
        L4D2LocationData("Hard Rain - Milltown Escape: Survive", LocationType.INTERACTABLE, "Hard Rain", "c4m5_milltown_escape")
    ],

    "The Parish": [
        L4D2LocationData("The Parish - Waterfront", LocationType.CHAPTER, "The Parish", "c5m1_waterfront"),
        L4D2LocationData("The Parish - Waterfront: Reach the Bus", LocationType.INTERACTABLE, "The Parish", "c5m1_waterfront"),
        L4D2LocationData("The Parish - Waterfront: Turn on the Jukebox", LocationType.INTERACTABLE, "The Parish", "c5m1_waterfront"),
        L4D2LocationData("The Parish - Waterfront: Get Through the Restaurant", LocationType.INTERACTABLE, "The Parish", "c5m1_waterfront"),

        L4D2LocationData("The Parish - Park", LocationType.CHAPTER, "The Parish", "c5m2_park"),
        L4D2LocationData("The Parish - Park: Enter the Decontamination Unit", LocationType.INTERACTABLE, "The Parish", "c5m2_park"),
        L4D2LocationData("The Parish - Park: Stop the Alarm", LocationType.INTERACTABLE, "The Parish", "c5m2_park"),

        L4D2LocationData("The Parish - Cemetery", LocationType.CHAPTER, "The Parish", "c5m3_cemetery"),
        L4D2LocationData("The Parish - Cemetery: Get on the Freeway", LocationType.INTERACTABLE, "The Parish", "c5m3_cemetery"),

        L4D2LocationData("The Parish - Quarter", LocationType.CHAPTER, "The Parish", "c5m4_quarter"),
        L4D2LocationData("The Parish - Quarter: Reach the Courtyard", LocationType.INTERACTABLE, "The Parish", "c5m4_quarter"),
        L4D2LocationData("The Parish - Quarter: Start the Tractor", LocationType.INTERACTABLE, "The Parish", "c5m4_quarter"),

        L4D2LocationData("The Parish - Bridge", LocationType.CHAPTER | LocationType.FINALE, "The Parish", "c5m5_bridge"),
        L4D2LocationData("The Parish - Bridge: Activate the Radio", LocationType.INTERACTABLE, "The Parish", "c5m5_bridge"),
        L4D2LocationData("The Parish - Bridge: Reach the Off-Ramp", LocationType.INTERACTABLE, "The Parish", "c5m5_bridge")
    ],

    "The Passing": [
        L4D2LocationData("The Passing - Riverbank", LocationType.CHAPTER, "The Passing", "c6m1_riverbank"),
        L4D2LocationData("The Passing - Riverbank: Open the Footlocker", LocationType.INTERACTABLE, "The Passing", "c6m1_riverbank"),
        L4D2LocationData("The Passing - Riverbank: Get to the Alleyway", LocationType.INTERACTABLE, "The Passing", "c6m1_riverbank"),
        L4D2LocationData("The Passing - Riverbank: Enter the Park", LocationType.INTERACTABLE, "The Passing", "c6m1_riverbank"),
        L4D2LocationData("The Passing - Riverbank: Change the Song", LocationType.INTERACTABLE, "The Passing", "c6m1_riverbank"),

        L4D2LocationData("The Passing - Underground", LocationType.CHAPTER, "The Passing", "c6m2_bedlam"),
        L4D2LocationData("The Passing - Underground: Open the Footlocker", LocationType.INTERACTABLE, "The Passing", "c6m2_bedlam"),
        L4D2LocationData("The Passing - Underground: Turn on the Jukebox", LocationType.INTERACTABLE, "The Passing", "c6m2_bedlam"),
        L4D2LocationData("The Passing - Underground: Open Gate", LocationType.INTERACTABLE, "The Passing", "c6m2_bedlam"),
        L4D2LocationData("The Passing - Underground: Open Second Gate", LocationType.INTERACTABLE, "The Passing", "c6m2_bedlam"),

        L4D2LocationData("The Passing - Port", LocationType.CHAPTER | LocationType.FINALE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Meet the Old Crew", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Turn on the Jukebox", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #1", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #2", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #3", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #4", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #5", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #6", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #7", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #8", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #9", LocationType.INTERACTABLE, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port: Fuel #10", LocationType.INTERACTABLE, "The Passing", "c6m3_port")
    ],

    "The Sacrifice": [
        L4D2LocationData("The Sacrifice - Docks", LocationType.CHAPTER, "The Sacrifice", "c7m1_docks"),
        L4D2LocationData("The Sacrifice - Docks: Reach the Wrecked Train", LocationType.INTERACTABLE, "The Sacrifice", "c7m1_docks"),
        L4D2LocationData("The Sacrifice - Docks: Open Train Door", LocationType.INTERACTABLE, "The Sacrifice", "c7m1_docks"),
        L4D2LocationData("The Sacrifice - Docks: Get Past the Tank", LocationType.INTERACTABLE, "The Sacrifice", "c7m1_docks"),

        L4D2LocationData("The Sacrifice - Barge", LocationType.CHAPTER, "The Sacrifice", "c7m2_barge"),
        L4D2LocationData("The Sacrifice - Barge: Reach the Barge", LocationType.INTERACTABLE, "The Sacrifice", "c7m2_barge"),

        L4D2LocationData("The Sacrifice - Port", LocationType.CHAPTER | LocationType.FINALE, "The Sacrifice", "c7m3_port"),
        L4D2LocationData("The Sacrifice - Port: Turn on the Jukebox", LocationType.INTERACTABLE, "The Sacrifice", "c7m3_port"),
        L4D2LocationData("The Sacrifice - Port: Start the First Generator", LocationType.INTERACTABLE, "The Sacrifice", "c7m3_port"),
        L4D2LocationData("The Sacrifice - Port: Start the Second Generator", LocationType.INTERACTABLE, "The Sacrifice", "c7m3_port"),
        L4D2LocationData("The Sacrifice - Port: Start the Third Generator", LocationType.INTERACTABLE, "The Sacrifice", "c7m3_port"),
        L4D2LocationData("The Sacrifice - Port: Lift the Bridge", LocationType.INTERACTABLE, "The Sacrifice", "c7m3_port")
    ],

    "No Mercy": [
        L4D2LocationData("No Mercy - Apartments", LocationType.CHAPTER, "No Mercy", "c8m1_apartment"),
        L4D2LocationData("No Mercy - Apartments: Reach Ground Level", LocationType.INTERACTABLE, "No Mercy", "c8m1_apartment"),
        L4D2LocationData("No Mercy - Apartments: Enter the Subway", LocationType.INTERACTABLE, "No Mercy", "c8m1_apartment"),

        L4D2LocationData("No Mercy - Subway", LocationType.CHAPTER, "No Mercy", "c8m2_subway"),
        L4D2LocationData("No Mercy - Subway: Open Garage Door", LocationType.INTERACTABLE, "No Mercy", "c8m2_subway"),

        L4D2LocationData("No Mercy - Sewers", LocationType.CHAPTER, "No Mercy", "c8m3_sewers"),
        L4D2LocationData("No Mercy - Sewers: Activate Lift", LocationType.INTERACTABLE, "No Mercy", "c8m3_sewers"),
        L4D2LocationData("No Mercy - Sewers: Open Shutter Door", LocationType.INTERACTABLE, "No Mercy", "c8m3_sewers"),
        L4D2LocationData("No Mercy - Sewers: Open Manhole", LocationType.INTERACTABLE, "No Mercy", "c8m3_sewers"),

        L4D2LocationData("No Mercy - Hospital", LocationType.CHAPTER, "No Mercy", "c8m4_interior"),
        L4D2LocationData("No Mercy - Hospital: Call Elevator", LocationType.INTERACTABLE, "No Mercy", "c8m4_interior"),
        L4D2LocationData("No Mercy - Hospital: Ride Elevator Up", LocationType.INTERACTABLE, "No Mercy", "c8m4_interior"),

        L4D2LocationData("No Mercy - Rooftop", LocationType.CHAPTER | LocationType.FINALE, "No Mercy", "c8m5_rooftop"),
        L4D2LocationData("No Mercy - Rooftop: Activate Radio", LocationType.INTERACTABLE, "No Mercy", "c8m5_rooftop")
    ],

    "Crash Course": [
        L4D2LocationData("Crash Course - Alleys", LocationType.CHAPTER, "Crash Course", "c9m1_alleys"),
        L4D2LocationData("Crash Course - Alleys: Reach the End of the Alleyway", LocationType.INTERACTABLE, "Crash Course", "c9m1_alleys"),
        L4D2LocationData("Crash Course - Alleys: Reach the Waterfront", LocationType.INTERACTABLE, "Crash Course", "c9m1_alleys"),
        L4D2LocationData("Crash Course - Alleys: Fire the Howitzer", LocationType.INTERACTABLE, "Crash Course", "c9m1_alleys"),
        L4D2LocationData("Crash Course - Alleys: Enter the Bridge", LocationType.INTERACTABLE, "Crash Course", "c9m1_alleys"),

        L4D2LocationData("Crash Course - Truck Depot", LocationType.CHAPTER | LocationType.FINALE, "Crash Course", "c9m2_lots"),
        L4D2LocationData("Crash Course - Truck Depot: Enter the Depot", LocationType.INTERACTABLE, "Crash Course", "c9m2_lots"),
        L4D2LocationData("Crash Course - Truck Depot: Start the Generator", LocationType.INTERACTABLE, "Crash Course", "c9m2_lots"),
        L4D2LocationData("Crash Course - Truck Depot: Restart the Generator", LocationType.INTERACTABLE, "Crash Course", "c9m2_lots")
    ],

    "Death Toll": [
        L4D2LocationData("Death Toll - Turnpike", LocationType.CHAPTER, "Death Toll", "c10m1_caves"),
        L4D2LocationData("Death Toll - Turnpike: Enter the Tunnel", LocationType.INTERACTABLE, "Death Toll", "c10m1_caves"),
        L4D2LocationData("Death Toll - Turnpike: Exit the Tunnel", LocationType.INTERACTABLE, "Death Toll", "c10m1_caves"),

        L4D2LocationData("Death Toll - Drains", LocationType.CHAPTER, "Death Toll", "c10m2_drainage"),
        L4D2LocationData("Death Toll - Drains: Activate Floodgates", LocationType.INTERACTABLE, "Death Toll", "c10m2_drainage"),

        L4D2LocationData("Death Toll - Church", LocationType.CHAPTER, "Death Toll", "c10m3_ranchhouse"),
        L4D2LocationData("Death Toll - Church: Reach Safehouse", LocationType.INTERACTABLE, "Death Toll", "c10m3_ranchhouse"),
        L4D2LocationData("Death Toll - Church: Play the Piano", LocationType.INTERACTABLE, "Death Toll", "c10m3_ranchhouse"),

        L4D2LocationData("Death Toll - Town", LocationType.CHAPTER, "Death Toll", "c10m4_mainstreet"),
        L4D2LocationData("Death Toll - Town: Lower the Forklift", LocationType.INTERACTABLE, "Death Toll", "c10m4_mainstreet"),

        L4D2LocationData("Death Toll - Boathouse", LocationType.CHAPTER | LocationType.FINALE, "Death Toll", "c10m5_houseboat"),
        L4D2LocationData("Death Toll - Boathouse: Activate Radio", LocationType.INTERACTABLE, "Death Toll", "c10m5_houseboat")
    ],

    "Dead Air": [
        L4D2LocationData("Dead Air - Greenhouse", LocationType.CHAPTER, "Dead Air", "c11m1_greenhouse"),
        L4D2LocationData("Dead Air - Greenhouse: Climb the Ladder", LocationType.INTERACTABLE, "Dead Air", "c11m1_greenhouse"),
        L4D2LocationData("Dead Air - Greenhouse: Reach Ground Level", LocationType.INTERACTABLE, "Dead Air", "c11m1_greenhouse"),

        L4D2LocationData("Dead Air - Crane", LocationType.CHAPTER, "Dead Air", "c11m2_offices"),
        L4D2LocationData("Dead Air - Crane: Activate Crane", LocationType.INTERACTABLE, "Dead Air", "c11m2_offices"),
        L4D2LocationData("Dead Air - Crane: Navigate the Office Building", LocationType.INTERACTABLE, "Dead Air", "c11m2_offices"),

        L4D2LocationData("Dead Air - Construction Site", LocationType.CHAPTER, "Dead Air", "c11m3_garage"),

        L4D2LocationData("Dead Air - Terminal", LocationType.CHAPTER, "Dead Air", "c11m4_terminal"),
        L4D2LocationData("Dead Air - Terminal: Start the Van", LocationType.INTERACTABLE, "Dead Air", "c11m4_terminal"),
        L4D2LocationData("Dead Air - Terminal: Slam the Van Through the Gate", LocationType.INTERACTABLE, "Dead Air", "c11m4_terminal"),
        L4D2LocationData("Dead Air - Terminal: Metal Detector", LocationType.INTERACTABLE, "Dead Air", "c11m4_terminal"),

        L4D2LocationData("Dead Air - Runway", LocationType.CHAPTER | LocationType.FINALE, "Dead Air", "c11m5_runway"),
        L4D2LocationData("Dead Air - Runway: Activate Radio", LocationType.INTERACTABLE, "Dead Air", "c11m5_runway")
    ],

    "Blood Harvest": [
        L4D2LocationData("Blood Harvest - Woods", LocationType.CHAPTER, "Blood Harvest", "c12m1_hilltop"),
        L4D2LocationData("Blood Harvest - Woods: Cross the Bridge", LocationType.INTERACTABLE, "Blood Harvest", "c12m1_hilltop"),

        L4D2LocationData("Blood Harvest - Tunnel", LocationType.CHAPTER, "Blood Harvest", "c12m2_traintunnel"),
        L4D2LocationData("Blood Harvest - Tunnel: Open Emergency Exit", LocationType.INTERACTABLE, "Blood Harvest", "c12m2_traintunnel"),

        L4D2LocationData("Blood Harvest - Bridge", LocationType.CHAPTER, "Blood Harvest", "c12m3_bridge"),
        L4D2LocationData("Blood Harvest - Bridge: Decouple Train", LocationType.INTERACTABLE, "Blood Harvest", "c12m3_bridge"),

        L4D2LocationData("Blood Harvest - Train Station", LocationType.CHAPTER, "Blood Harvest", "c12m4_barn"),

        L4D2LocationData("Blood Harvest - Farmhouse", LocationType.CHAPTER | LocationType.FINALE, "Blood Harvest", "c12m5_cornfield"),
        L4D2LocationData("Blood Harvest - Farmhouse: Enter the Cornfield", LocationType.INTERACTABLE, "Blood Harvest", "c12m5_cornfield"),
        L4D2LocationData("Blood Harvest - Farmhouse: Activate Radio", LocationType.INTERACTABLE, "Blood Harvest", "c12m5_cornfield")
    ],

    "Cold Stream": [
        L4D2LocationData("Cold Stream - Alpine Creek", LocationType.CHAPTER, "Cold Stream", "c13m1_alpinecreek"),
        L4D2LocationData("Cold Stream - Alpine Creek: Reach the Fallen Bridge", LocationType.INTERACTABLE, "Cold Stream", "c13m1_alpinecreek"),
        L4D2LocationData("Cold Stream - Alpine Creek: Open the Bunker Door", LocationType.INTERACTABLE, "Cold Stream", "c13m1_alpinecreek"),

        L4D2LocationData("Cold Stream - South Pine Stream", LocationType.CHAPTER, "Cold Stream", "c13m2_southpinestream"),
        L4D2LocationData("Cold Stream - South Pine Stream: Blow the Barricades", LocationType.INTERACTABLE, "Cold Stream", "c13m2_southpinestream"),

        L4D2LocationData("Cold Stream - Memorial Bridge", LocationType.CHAPTER, "Cold Stream", "c13m3_memorialbridge"),
        L4D2LocationData("Cold Stream - Memorial Bridge: Tanker Explosion", LocationType.INTERACTABLE, "Cold Stream", "c13m3_memorialbridge"),
        L4D2LocationData("Cold Stream - Memorial Bridge: Bridge Explosion", LocationType.INTERACTABLE, "Cold Stream", "c13m3_memorialbridge"),

        L4D2LocationData("Cold Stream - Cutthroat Creek", LocationType.CHAPTER | LocationType.FINALE, "Cold Stream", "c13m4_cutthroatcreek"),
        L4D2LocationData("Cold Stream - Cutthroat Creek: Activate Radio", LocationType.INTERACTABLE, "Cold Stream", "c13m4_cutthroatcreek"),
        L4D2LocationData("Cold Stream - Cutthroat Creek: Open Shutter Door", LocationType.INTERACTABLE, "Cold Stream", "c13m4_cutthroatcreek")
    ],

    "The Last Stand": [
        L4D2LocationData("The Last Stand - Junkyard", LocationType.CHAPTER, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Under the Bridge", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Shop Yard Entrance", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Turn the Fuel Valve", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Jumpstart Generator #1", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Jumpstart Generator #2", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Jumpstart Generator #3", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Jumpstart Generator #4", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard: Drop the Container", LocationType.INTERACTABLE, "The Last Stand", "c14m1_junkyard"),

        L4D2LocationData("The Last Stand - Lighthouse", LocationType.CHAPTER | LocationType.FINALE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Open the Footlocker", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Start the Generator", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Survive the First Horde", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #1", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #2", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #3", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #4", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #5", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #6", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #7", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Fuel #8", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse: Survive the Second Horde", LocationType.INTERACTABLE, "The Last Stand", "c14m2_lighthouse")
    ],
}

SURVIVAL_LOCATIONS: dict[str, list[L4D2LocationData]] = {
    "Dead Center - Streets (Survival)": [
        L4D2LocationData("Dead Center - Streets (Survival): Bronze", LocationType.SURVIVAL, "Dead Center", "c1m2_streets"),
        L4D2LocationData("Dead Center - Streets (Survival): Silver", LocationType.SURVIVAL, "Dead Center", "c1m2_streets"),
        L4D2LocationData("Dead Center - Streets (Survival): Gold", LocationType.SURVIVAL, "Dead Center", "c1m2_streets"),
    ],
    "Dead Center - Atrium (Survival)": [
        L4D2LocationData("Dead Center - Atrium (Survival): Bronze", LocationType.SURVIVAL, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium (Survival): Silver", LocationType.SURVIVAL, "Dead Center", "c1m4_atrium"),
        L4D2LocationData("Dead Center - Atrium (Survival): Gold", LocationType.SURVIVAL, "Dead Center", "c1m4_atrium"),
    ],

    "Dark Carnival - Highway (Survival)": [
        L4D2LocationData("Dark Carnival - Highway (Survival): Bronze", LocationType.SURVIVAL, "Dark Carnival", "c2m1_highway"),
        L4D2LocationData("Dark Carnival - Highway (Survival): Silver", LocationType.SURVIVAL, "Dark Carnival", "c2m1_highway"),
        L4D2LocationData("Dark Carnival - Highway (Survival): Gold", LocationType.SURVIVAL, "Dark Carnival", "c2m1_highway"),
    ],
    "Dark Carnival - Barns (Survival)": [
        L4D2LocationData("Dark Carnival - Barns (Survival): Bronze", LocationType.SURVIVAL, "Dark Carnival", "c2m4_barns"),
        L4D2LocationData("Dark Carnival - Barns (Survival): Silver", LocationType.SURVIVAL, "Dark Carnival", "c2m4_barns"),
        L4D2LocationData("Dark Carnival - Barns (Survival): Gold", LocationType.SURVIVAL, "Dark Carnival", "c2m4_barns"),
    ],
    "Dark Carnival - Concert (Survival)": [
        L4D2LocationData("Dark Carnival - Concert (Survival): Bronze", LocationType.SURVIVAL, "Dark Carnival", "c2m5_concert"),
        L4D2LocationData("Dark Carnival - Concert (Survival): Silver", LocationType.SURVIVAL, "Dark Carnival", "c2m5_concert"),
        L4D2LocationData("Dark Carnival - Concert (Survival): Gold", LocationType.SURVIVAL, "Dark Carnival", "c2m5_concert"),
    ],

    "Swamp Fever - Plank Country (Survival)": [
        L4D2LocationData("Swamp Fever - Plank Country (Survival): Bronze", LocationType.SURVIVAL, "Swamp Fever", "c3m1_plankcountry"),
        L4D2LocationData("Swamp Fever - Plank Country (Survival): Silver", LocationType.SURVIVAL, "Swamp Fever", "c3m1_plankcountry"),
        L4D2LocationData("Swamp Fever - Plank Country (Survival): Gold", LocationType.SURVIVAL, "Swamp Fever", "c3m1_plankcountry"),
    ],
    "Swamp Fever - Shantytown (Survival)": [
        L4D2LocationData("Swamp Fever - Shantytown (Survival): Bronze", LocationType.SURVIVAL, "Swamp Fever", "c3m3_shantytown"),
        L4D2LocationData("Swamp Fever - Shantytown (Survival): Silver", LocationType.SURVIVAL, "Swamp Fever", "c3m3_shantytown"),
        L4D2LocationData("Swamp Fever - Shantytown (Survival): Gold", LocationType.SURVIVAL, "Swamp Fever", "c3m3_shantytown"),
    ],
    "Swamp Fever - Plantation (Survival)": [
        L4D2LocationData("Swamp Fever - Plantation (Survival): Bronze", LocationType.SURVIVAL, "Swamp Fever", "c3m4_plantation"),
        L4D2LocationData("Swamp Fever - Plantation (Survival): Silver", LocationType.SURVIVAL, "Swamp Fever", "c3m4_plantation"),
        L4D2LocationData("Swamp Fever - Plantation (Survival): Gold", LocationType.SURVIVAL, "Swamp Fever", "c3m4_plantation"),
    ],

    "Hard Rain - Milltown (Survival)": [
        L4D2LocationData("Hard Rain - Milltown (Survival): Bronze", LocationType.SURVIVAL, "Hard Rain", "c4m1_milltown_a"),
        L4D2LocationData("Hard Rain - Milltown (Survival): Silver", LocationType.SURVIVAL, "Hard Rain", "c4m1_milltown_a"),
        L4D2LocationData("Hard Rain - Milltown (Survival): Gold", LocationType.SURVIVAL, "Hard Rain", "c4m1_milltown_a"),
    ],
    "Hard Rain - Sugar Mill (Survival)": [
        L4D2LocationData("Hard Rain - Sugar Mill (Survival): Bronze", LocationType.SURVIVAL, "Hard Rain", "c4m2_sugarmill_a"),
        L4D2LocationData("Hard Rain - Sugar Mill (Survival): Silver", LocationType.SURVIVAL, "Hard Rain", "c4m2_sugarmill_a"),
        L4D2LocationData("Hard Rain - Sugar Mill (Survival): Gold", LocationType.SURVIVAL, "Hard Rain", "c4m2_sugarmill_a"),
    ],
    "Hard Rain - Sugar Mill Escape (Survival)": [
        L4D2LocationData("Hard Rain - Sugar Mill Escape (Survival): Bronze", LocationType.SURVIVAL, "Hard Rain", "c4m3_sugarmill_b"),
        L4D2LocationData("Hard Rain - Sugar Mill Escape (Survival): Silver", LocationType.SURVIVAL, "Hard Rain", "c4m3_sugarmill_b"),
        L4D2LocationData("Hard Rain - Sugar Mill Escape (Survival): Gold", LocationType.SURVIVAL, "Hard Rain", "c4m3_sugarmill_b"),
    ],

    "The Parish - Waterfront (Survival)": [
        L4D2LocationData("The Parish - Waterfront (Survival): Bronze", LocationType.SURVIVAL, "The Parish", "c5m1_waterfront"),
        L4D2LocationData("The Parish - Waterfront (Survival): Silver", LocationType.SURVIVAL, "The Parish", "c5m1_waterfront"),
        L4D2LocationData("The Parish - Waterfront (Survival): Gold", LocationType.SURVIVAL, "The Parish", "c5m1_waterfront"),
    ],
    "The Parish - Park (Survival)": [
        L4D2LocationData("The Parish - Park (Survival): Bronze", LocationType.SURVIVAL, "The Parish", "c5m2_park"),
        L4D2LocationData("The Parish - Park (Survival): Silver", LocationType.SURVIVAL, "The Parish", "c5m2_park"),
        L4D2LocationData("The Parish - Park (Survival): Gold", LocationType.SURVIVAL, "The Parish", "c5m2_park"),
    ],
    "The Parish - Cemetery (Survival)": [
        L4D2LocationData("The Parish - Cemetery (Survival): Bronze", LocationType.SURVIVAL, "The Parish", "c5m3_cemetery"),
        L4D2LocationData("The Parish - Cemetery (Survival): Silver", LocationType.SURVIVAL, "The Parish", "c5m3_cemetery"),
        L4D2LocationData("The Parish - Cemetery (Survival): Gold", LocationType.SURVIVAL, "The Parish", "c5m3_cemetery"),
    ],
    "The Parish - Quarter (Survival)": [
        L4D2LocationData("The Parish - Quarter (Survival): Bronze", LocationType.SURVIVAL, "The Parish", "c5m4_quarter"),
        L4D2LocationData("The Parish - Quarter (Survival): Silver", LocationType.SURVIVAL, "The Parish", "c5m4_quarter"),
        L4D2LocationData("The Parish - Quarter (Survival): Gold", LocationType.SURVIVAL, "The Parish", "c5m4_quarter"),
    ],
    "The Parish - Bridge (Survival)": [
        L4D2LocationData("The Parish - Bridge (Survival): Bronze", LocationType.SURVIVAL, "The Parish", "c5m5_bridge"),
        L4D2LocationData("The Parish - Bridge (Survival): Silver", LocationType.SURVIVAL, "The Parish", "c5m5_bridge"),
        L4D2LocationData("The Parish - Bridge (Survival): Gold", LocationType.SURVIVAL, "The Parish", "c5m5_bridge"),
    ],

    "The Passing - Riverbank (Survival)": [
        L4D2LocationData("The Passing - Riverbank (Survival): Bronze", LocationType.SURVIVAL, "The Passing", "c6m1_riverbank"),
        L4D2LocationData("The Passing - Riverbank (Survival): Silver", LocationType.SURVIVAL, "The Passing", "c6m1_riverbank"),
        L4D2LocationData("The Passing - Riverbank (Survival): Gold", LocationType.SURVIVAL, "The Passing", "c6m1_riverbank"),
    ],
    "The Passing - Underground (Survival)": [
        L4D2LocationData("The Passing - Underground (Survival): Bronze", LocationType.SURVIVAL, "The Passing", "c6m2_bedlam"),
        L4D2LocationData("The Passing - Underground (Survival): Silver", LocationType.SURVIVAL, "The Passing", "c6m2_bedlam"),
        L4D2LocationData("The Passing - Underground (Survival): Gold", LocationType.SURVIVAL, "The Passing", "c6m2_bedlam"),
    ],
    "The Passing - Port (Survival)": [
        L4D2LocationData("The Passing - Port (Survival): Bronze", LocationType.SURVIVAL, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port (Survival): Silver", LocationType.SURVIVAL, "The Passing", "c6m3_port"),
        L4D2LocationData("The Passing - Port (Survival): Gold", LocationType.SURVIVAL, "The Passing", "c6m3_port"),
    ],

    "The Sacrifice - Docks (Survival)": [
        L4D2LocationData("The Sacrifice - Docks (Survival): Bronze", LocationType.SURVIVAL, "The Sacrifice", "c7m1_docks"),
        L4D2LocationData("The Sacrifice - Docks (Survival): Silver", LocationType.SURVIVAL, "The Sacrifice", "c7m1_docks"),
        L4D2LocationData("The Sacrifice - Docks (Survival): Gold", LocationType.SURVIVAL, "The Sacrifice", "c7m1_docks"),
    ],
    "The Sacrifice - Barge (Survival)": [
        L4D2LocationData("The Sacrifice - Barge (Survival): Bronze", LocationType.SURVIVAL, "The Sacrifice", "c7m2_barge"),
        L4D2LocationData("The Sacrifice - Barge (Survival): Silver", LocationType.SURVIVAL, "The Sacrifice", "c7m2_barge"),
        L4D2LocationData("The Sacrifice - Barge (Survival): Gold", LocationType.SURVIVAL, "The Sacrifice", "c7m2_barge"),
    ],
    "The Sacrifice - Port (Survival)": [
        L4D2LocationData("The Sacrifice - Port (Survival): Bronze", LocationType.SURVIVAL, "The Sacrifice", "c7m3_port"),
        L4D2LocationData("The Sacrifice - Port (Survival): Silver", LocationType.SURVIVAL, "The Sacrifice", "c7m3_port"),
        L4D2LocationData("The Sacrifice - Port (Survival): Gold", LocationType.SURVIVAL, "The Sacrifice", "c7m3_port"),
    ],

    "No Mercy - Subway (Survival)": [
        L4D2LocationData("No Mercy - Subway (Survival): Bronze", LocationType.SURVIVAL, "No Mercy", "c8m2_subway"),
        L4D2LocationData("No Mercy - Subway (Survival): Silver", LocationType.SURVIVAL, "No Mercy", "c8m2_subway"),
        L4D2LocationData("No Mercy - Subway (Survival): Gold", LocationType.SURVIVAL, "No Mercy", "c8m2_subway"),
    ],
    "No Mercy - Sewers (Survival)": [
        L4D2LocationData("No Mercy - Sewers (Survival): Bronze", LocationType.SURVIVAL, "No Mercy", "c8m3_sewers"),
        L4D2LocationData("No Mercy - Sewers (Survival): Silver", LocationType.SURVIVAL, "No Mercy", "c8m3_sewers"),
        L4D2LocationData("No Mercy - Sewers (Survival): Gold", LocationType.SURVIVAL, "No Mercy", "c8m3_sewers"),
    ],
    "No Mercy - Hospital (Survival)": [
        L4D2LocationData("No Mercy - Hospital (Survival): Bronze", LocationType.SURVIVAL, "No Mercy", "c8m4_interior"),
        L4D2LocationData("No Mercy - Hospital (Survival): Silver", LocationType.SURVIVAL, "No Mercy", "c8m4_interior"),
        L4D2LocationData("No Mercy - Hospital (Survival): Gold", LocationType.SURVIVAL, "No Mercy", "c8m4_interior"),
    ],
    "No Mercy - Rooftop (Survival)": [
        L4D2LocationData("No Mercy - Rooftop (Survival): Bronze", LocationType.SURVIVAL, "No Mercy", "c8m5_rooftop"),
        L4D2LocationData("No Mercy - Rooftop (Survival): Silver", LocationType.SURVIVAL, "No Mercy", "c8m5_rooftop"),
        L4D2LocationData("No Mercy - Rooftop (Survival): Gold", LocationType.SURVIVAL, "No Mercy", "c8m5_rooftop"),
    ],

    "Crash Course - Alleys (Survival)": [
        L4D2LocationData("Crash Course - Alleys (Survival): Bronze", LocationType.SURVIVAL, "Crash Course", "c9m1_alleys"),
        L4D2LocationData("Crash Course - Alleys (Survival): Silver", LocationType.SURVIVAL, "Crash Course", "c9m1_alleys"),
        L4D2LocationData("Crash Course - Alleys (Survival): Gold", LocationType.SURVIVAL, "Crash Course", "c9m1_alleys"),
    ],
    "Crash Course - Truck Depot (Survival)": [
        L4D2LocationData("Crash Course - Truck Depot (Survival): Bronze", LocationType.SURVIVAL, "Crash Course", "c9m2_lots"),
        L4D2LocationData("Crash Course - Truck Depot (Survival): Silver", LocationType.SURVIVAL, "Crash Course", "c9m2_lots"),
        L4D2LocationData("Crash Course - Truck Depot (Survival): Gold", LocationType.SURVIVAL, "Crash Course", "c9m2_lots"),
    ],

    "Death Toll - Drains (Survival)": [
        L4D2LocationData("Death Toll - Drains (Survival): Bronze", LocationType.SURVIVAL, "Death Toll", "c10m2_drainage"),
        L4D2LocationData("Death Toll - Drains (Survival): Silver", LocationType.SURVIVAL, "Death Toll", "c10m2_drainage"),
        L4D2LocationData("Death Toll - Drains (Survival): Gold", LocationType.SURVIVAL, "Death Toll", "c10m2_drainage"),
    ],
    "Death Toll - Church (Survival)": [
        L4D2LocationData("Death Toll - Church (Survival): Bronze", LocationType.SURVIVAL, "Death Toll", "c10m3_ranchhouse"),
        L4D2LocationData("Death Toll - Church (Survival): Silver", LocationType.SURVIVAL, "Death Toll", "c10m3_ranchhouse"),
        L4D2LocationData("Death Toll - Church (Survival): Gold", LocationType.SURVIVAL, "Death Toll", "c10m3_ranchhouse"),
    ],
    "Death Toll - Town (Survival)": [
        L4D2LocationData("Death Toll - Town (Survival): Bronze", LocationType.SURVIVAL, "Death Toll", "c10m4_mainstreet"),
        L4D2LocationData("Death Toll - Town (Survival): Silver", LocationType.SURVIVAL, "Death Toll", "c10m4_mainstreet"),
        L4D2LocationData("Death Toll - Town (Survival): Gold", LocationType.SURVIVAL, "Death Toll", "c10m4_mainstreet"),
    ],
    "Death Toll - Boathouse (Survival)": [
        L4D2LocationData("Death Toll - Boathouse (Survival): Bronze", LocationType.SURVIVAL, "Death Toll", "c10m5_houseboat"),
        L4D2LocationData("Death Toll - Boathouse (Survival): Silver", LocationType.SURVIVAL, "Death Toll", "c10m5_houseboat"),
        L4D2LocationData("Death Toll - Boathouse (Survival): Gold", LocationType.SURVIVAL, "Death Toll", "c10m5_houseboat"),
    ],

    "Dead Air - Crane (Survival)": [
        L4D2LocationData("Dead Air - Crane (Survival): Bronze", LocationType.SURVIVAL, "Dead Air", "c11m2_offices"),
        L4D2LocationData("Dead Air - Crane (Survival): Silver", LocationType.SURVIVAL, "Dead Air", "c11m2_offices"),
        L4D2LocationData("Dead Air - Crane (Survival): Gold", LocationType.SURVIVAL, "Dead Air", "c11m2_offices"),
    ],
    "Dead Air - Construction Site (Survival)": [
        L4D2LocationData("Dead Air - Construction Site (Survival): Bronze", LocationType.SURVIVAL, "Dead Air", "c11m3_garage"),
        L4D2LocationData("Dead Air - Construction Site (Survival): Silver", LocationType.SURVIVAL, "Dead Air", "c11m3_garage"),
        L4D2LocationData("Dead Air - Construction Site (Survival): Gold", LocationType.SURVIVAL, "Dead Air", "c11m3_garage"),
    ],
    "Dead Air - Terminal (Survival)": [
        L4D2LocationData("Dead Air - Terminal (Survival): Bronze", LocationType.SURVIVAL, "Dead Air", "c11m4_terminal"),
        L4D2LocationData("Dead Air - Terminal (Survival): Silver", LocationType.SURVIVAL, "Dead Air", "c11m4_terminal"),
        L4D2LocationData("Dead Air - Terminal (Survival): Gold", LocationType.SURVIVAL, "Dead Air", "c11m4_terminal"),
    ],
    "Dead Air - Runway (Survival)": [
        L4D2LocationData("Dead Air - Runway (Survival): Bronze", LocationType.SURVIVAL, "Dead Air", "c11m5_runway"),
        L4D2LocationData("Dead Air - Runway (Survival): Silver", LocationType.SURVIVAL, "Dead Air", "c11m5_runway"),
        L4D2LocationData("Dead Air - Runway (Survival): Gold", LocationType.SURVIVAL, "Dead Air", "c11m5_runway"),
    ],

    "Blood Harvest - Tunnel (Survival)": [
        L4D2LocationData("Blood Harvest - Tunnel (Survival): Bronze", LocationType.SURVIVAL, "Blood Harvest", "c12m2_traintunnel"),
        L4D2LocationData("Blood Harvest - Tunnel (Survival): Silver", LocationType.SURVIVAL, "Blood Harvest", "c12m2_traintunnel"),
        L4D2LocationData("Blood Harvest - Tunnel (Survival): Gold", LocationType.SURVIVAL, "Blood Harvest", "c12m2_traintunnel"),
    ],
    "Blood Harvest - Bridge (Survival)": [
        L4D2LocationData("Blood Harvest - Bridge (Survival): Bronze", LocationType.SURVIVAL, "Blood Harvest", "c12m3_bridge"),
        L4D2LocationData("Blood Harvest - Bridge (Survival): Silver", LocationType.SURVIVAL, "Blood Harvest", "c12m3_bridge"),
        L4D2LocationData("Blood Harvest - Bridge (Survival): Gold", LocationType.SURVIVAL, "Blood Harvest", "c12m3_bridge"),
    ],
    "Blood Harvest - Farmhouse (Survival)": [
        L4D2LocationData("Blood Harvest - Farmhouse (Survival): Bronze", LocationType.SURVIVAL, "Blood Harvest", "c12m5_cornfield"),
        L4D2LocationData("Blood Harvest - Farmhouse (Survival): Silver", LocationType.SURVIVAL, "Blood Harvest", "c12m5_cornfield"),
        L4D2LocationData("Blood Harvest - Farmhouse (Survival): Gold", LocationType.SURVIVAL, "Blood Harvest", "c12m5_cornfield"),
    ],

    "Cold Stream - Memorial Bridge (Survival)": [
        L4D2LocationData("Cold Stream - Memorial Bridge (Survival): Bronze", LocationType.SURVIVAL, "Cold Stream", "c13m3_memorialbridge"),
        L4D2LocationData("Cold Stream - Memorial Bridge (Survival): Silver", LocationType.SURVIVAL, "Cold Stream", "c13m3_memorialbridge"),
        L4D2LocationData("Cold Stream - Memorial Bridge (Survival): Gold", LocationType.SURVIVAL, "Cold Stream", "c13m3_memorialbridge"),
    ],
    "Cold Stream - Cutthroat Creek (Survival)": [
        L4D2LocationData("Cold Stream - Cutthroat Creek (Survival): Bronze", LocationType.SURVIVAL, "Cold Stream", "c13m4_cutthroatcreek"),
        L4D2LocationData("Cold Stream - Cutthroat Creek (Survival): Silver", LocationType.SURVIVAL, "Cold Stream", "c13m4_cutthroatcreek"),
        L4D2LocationData("Cold Stream - Cutthroat Creek (Survival): Gold", LocationType.SURVIVAL, "Cold Stream", "c13m4_cutthroatcreek"),
    ],

    "The Last Stand - Junkyard (Survival)": [
        L4D2LocationData("The Last Stand - Junkyard (Survival): Bronze", LocationType.SURVIVAL, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard (Survival): Silver", LocationType.SURVIVAL, "The Last Stand", "c14m1_junkyard"),
        L4D2LocationData("The Last Stand - Junkyard (Survival): Gold", LocationType.SURVIVAL, "The Last Stand", "c14m1_junkyard"),
    ],
    "The Last Stand - Lighthouse (Survival)": [
        L4D2LocationData("The Last Stand - Lighthouse (Survival): Bronze", LocationType.SURVIVAL, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse (Survival): Silver", LocationType.SURVIVAL, "The Last Stand", "c14m2_lighthouse"),
        L4D2LocationData("The Last Stand - Lighthouse (Survival): Gold", LocationType.SURVIVAL, "The Last Stand", "c14m2_lighthouse"),
    ],
}


def survival_checks(medal_count: int, maps: set[str]) -> dict[str, list[L4D2LocationData]]:
    return {
        level: rows[:medal_count]
        for level, rows in SURVIVAL_LOCATIONS.items()
        if rows[:medal_count] and rows[0].map_name in maps
    }


def _survival_rows(campaign: str) -> list[L4D2LocationData]:
    return [
        loc
        for level_rows in SURVIVAL_LOCATIONS.values()
        for loc in level_rows
        if loc.campaign == campaign
    ]


def _item_carrier(chapter: L4D2LocationData) -> L4D2LocationData:
    return L4D2LocationData(
        f"{chapter.name}: AP Item Carrier", LocationType.ITEM, chapter.campaign, chapter.map_name
    )


carrier_by_chapter: dict[str, L4D2LocationData] = {
    row.name: _item_carrier(row)
    for rows in CAMPAIGN_LOCATIONS.values()
    for row in rows
    if row.is_chapter
}

# Item carrier checks, one per chapter
CARRIER_LOCATIONS: dict[str, list[L4D2LocationData]] = {
    campaign: [carrier_by_chapter[row.name] for row in rows if row.is_chapter]
    for campaign, rows in CAMPAIGN_LOCATIONS.items()
}

location_table: dict[str, list[L4D2LocationData]] = {
    campaign: rows + CARRIER_LOCATIONS[campaign] + _survival_rows(campaign)
    for campaign, rows in CAMPAIGN_LOCATIONS.items()
}

# Location ids are assigned in this specific order
for _index, _location in enumerate(
    [location for rows in CAMPAIGN_LOCATIONS.values() for location in rows]
    + [location for rows in CARRIER_LOCATIONS.values() for location in rows]
    + [location for rows in SURVIVAL_LOCATIONS.values() for location in rows]
):
    _location.ap_id = l4d2_location_base_id + _index


def _group_locations_by_chapter() -> dict[str, list[L4D2LocationData]]:
    chapter_by_map = {
        location.map_name: location.name
        for rows in location_table.values()
        for location in rows
        if location.is_chapter
    }
    grouped: dict[str, list[L4D2LocationData]] = {chapter: [] for chapter in chapter_by_map.values()}
    for rows in location_table.values():
        for location in rows:
            if not location.is_survival and not location.is_item:
                grouped[chapter_by_map[location.map_name]].append(location)
    return grouped


locations_by_chapter: dict[str, list[L4D2LocationData]] = _group_locations_by_chapter()


def get_location_names() -> dict[str, int]:
    return {
        location.name: location.ap_id
        for locations in location_table.values()
        for location in locations
    }
