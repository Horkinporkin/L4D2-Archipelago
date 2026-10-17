from dataclasses import dataclass

from BaseClasses import Item, ItemClassification

from .Locations import SURVIVAL_LOCATIONS, location_table

l4d2_item_base_id = 8463000

PROGRESSIVE_WEAPON_TIER = "Progressive Weapon Tier"
PROGRESSIVE_SURVIVOR = "Progressive Survivor"
MAX_SURVIVOR_COMPANIONS = 3


@dataclass
class L4D2ItemData:
    name: str
    classification: ItemClassification
    code: int = 0


class L4D2Item(Item):
    game = "Left 4 Dead 2"


chapter_items: tuple[str, ...] = tuple(
    location.name
    for locations in location_table.values()
    for location in locations
    if location.is_chapter
)

progressive_items: list[L4D2ItemData] = [
    L4D2ItemData(PROGRESSIVE_WEAPON_TIER, ItemClassification.progression),
    L4D2ItemData(PROGRESSIVE_SURVIVOR, ItemClassification.progression),
    *(L4D2ItemData(name, ItemClassification.progression) for name in chapter_items),
]

# One access item per Survival level, named after it like chapter items
survival_items: list[L4D2ItemData] = [
    L4D2ItemData(level, ItemClassification.progression) for level in SURVIVAL_LOCATIONS
]

filler_items: list[L4D2ItemData] = [
    # Healing Items
    L4D2ItemData("First Aid Kit", ItemClassification.filler),
    L4D2ItemData("Defibrillator", ItemClassification.filler),

    # Temporary Healing Items
    L4D2ItemData("Pain Pills", ItemClassification.filler),
    L4D2ItemData("Adrenaline", ItemClassification.filler),

    # Weapon Upgrades
    L4D2ItemData("Laser Sights", ItemClassification.filler),
    L4D2ItemData("Incendiary Ammo", ItemClassification.filler),
    L4D2ItemData("Explosive Ammo", ItemClassification.filler),

    # Grenades
    L4D2ItemData("Molotov", ItemClassification.filler),
    L4D2ItemData("Pipe Bomb", ItemClassification.filler),
    L4D2ItemData("Bile Bomb", ItemClassification.filler),

    # Explosives
    L4D2ItemData("Gas Can", ItemClassification.filler),
    L4D2ItemData("Oxygen Tank", ItemClassification.filler),
    L4D2ItemData("Propane Tank", ItemClassification.filler),
    L4D2ItemData("Fireworks", ItemClassification.filler),

    # Miscellaneous
    L4D2ItemData("Gnome Chompski", ItemClassification.filler),
    L4D2ItemData("Ammo Pile", ItemClassification.filler),
]

trap_items: list[L4D2ItemData] = [
    # Special Infected
    L4D2ItemData("Trap: Boomer", ItemClassification.trap),
    L4D2ItemData("Trap: Hunter", ItemClassification.trap),
    L4D2ItemData("Trap: Smoker", ItemClassification.trap),
    L4D2ItemData("Trap: Tank", ItemClassification.trap),
    L4D2ItemData("Trap: Witch", ItemClassification.trap),
    L4D2ItemData("Trap: Charger", ItemClassification.trap),
    L4D2ItemData("Trap: Jockey", ItemClassification.trap),
    L4D2ItemData("Trap: Spitter", ItemClassification.trap),
]

full_item_list: list[L4D2ItemData] = progressive_items + filler_items + trap_items + survival_items
for _index, _item in enumerate(full_item_list):
    _item.code = l4d2_item_base_id + _index

item_table: dict[str, L4D2ItemData] = {item.name: item for item in full_item_list}
item_name_to_id: dict[str, int] = {item.name: item.code for item in full_item_list}

filler_item_names: tuple[str, ...] = tuple(item.name for item in filler_items)
trap_item_names: tuple[str, ...] = tuple(item.name for item in trap_items)

item_groups: dict[str, set[str]] = {
    "Chapters": set(chapter_items),
}

# Weapon display name -> weapon tier. Weapons themselves are not items, but the tiers are
weapon_tiers: dict[str, int] = {
    # Melee weapons are always usable
    "Fireaxe": 0,
    "Baseball Bat": 0,
    "Cricket Bat": 0,
    "Crowbar": 0,
    "Frying Pan": 0,
    "Golf Club": 0,
    "Guitar": 0,
    "Katana": 0,
    "Machete": 0,
    "Nightstick": 0,
    "Pitchfork": 0,
    "Shovel": 0,
    "Knife": 0,
    "Chainsaw": 0,

    # Tier 1
    "Pistol": 1,
    "Magnum": 1,
    "Dual Pistols": 1,

    # Tier 2
    "SMG": 2,
    "Silenced SMG": 2,
    "MP5": 2,
    "Pump Shotgun": 2,
    "Chrome Shotgun": 2,

    # Tier 3
    "Tactical Shotgun": 3,
    "M16": 3,
    "SG552": 3,
    "AUG": 3,
    "Hunting Rifle": 3,
    "SCAR": 3,
    "Scout": 3,

    # Tier 4
    "AK-47": 4,
    "Spas-12": 4,
    "Auto Sniper": 4,
    "AWP": 4,

    # Tier 5
    "Grenade Launcher": 5,
    "M60": 5,
}

max_weapon_tier: int = max(weapon_tiers.values())
