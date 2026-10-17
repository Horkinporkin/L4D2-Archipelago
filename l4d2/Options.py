from dataclasses import dataclass

from Options import Choice, DeathLink, DefaultOnToggle, OptionGroup, PerGameCommonOptions, Range, Toggle


class Randomization(Choice):
    """Controls how the route is generated.

    **Campaign:** shuffle the campaign order, keeping each campaign's chapters together.
    **Chapter:** shuffles all chapters together, creating "custom" campaigns.
    """

    display_name = "Randomization"
    option_campaign = 0
    option_chapter = 1
    default = option_chapter


class ProgressiveWeaponTiers(DefaultOnToggle):
    """Start with only melee weapons usable and unlock stronger weapon tiers progressively.

    Each tier is a separate Progressive Weapon Tier item.
    0. Melee
    1. Pistols
    2. SMGs and weaker shotguns
    3. Mid-tier shotguns, assault rifles and sniper rifles
    4. Stronger assault rifles, sniper rifles and shotguns
    5. Heavy weapons

    You can add the "Progressive Weapon Tier" item to "start_inventory" to start with a higher tier.
    """

    display_name = "Progressive Weapon Tiers"


class ProgressiveSurvivors(DefaultOnToggle):
    """Add three Progressive Survivor items, each unlocking one additional survivor.

    With this option enabled, you will begin the game alone.

    You can add the "Progressive Survivor" item to "start_inventory" to start with additional survivors.
    """

    display_name = "Progressive AI Survivors"


class SurvivalMedals(Choice):
    """Add one Survival level access item per Survival level, and a medal check for each medal tier.

    **Disabled:** no Survival mode checks.
    **Bronze:** every Survival level awards a check for achieving the Bronze time.
    **Silver:** every Survival level additionally awards a check for achieving the Silver time.
    **Gold:** every Survival level additionally awards a check for achieving the Gold time.

    If using anything higher than 'Bronze' as a solo player, it is recommended to adjust 'Survival Time Scale' below.
    Survival levels are independent of Progressive Weapon Tiers and Progressive AI Survivors, you will have your
    full loadout.
    """

    display_name = "Survival Medals"
    option_disabled = 0
    option_bronze = 1
    option_silver = 2
    option_gold = 3
    default = option_disabled


class SurvivalTimeScale(Range):
    """Scale the Survival medal timers as a percentage.

    100 is normal (Bronze 4:00, Silver 7:00, Gold 10:00).
    50 cuts timers in half (2:00/3:30/5:00). Useful for solo playthroughs.
    200 doubles them for a harder challenge (good luck).
    Scales all medals equally.

    Only used when Survival Medals is not Disabled.
    """

    display_name = "Survival Medal Time Scale"
    range_start = 10
    range_end = 200
    default = 100


class LoadoutLogic(Choice):
    """Controls Survivor and Weapon Tier access requirements.

    **Disabled:** only route progression is required. Every check in a chapter can be done as soon as
        the chapter is unlocked. Can cause certain parts of the game to be very hard.
    **Enabled:** every chapter check requires the curated Survivor and Weapon Tier amount.
        A more "balanced" experience.

    While the logic is not entirely sound right now, in theory, it enhances the experience.
    """

    display_name = "Loadout Logic"
    option_disabled = 0
    option_enabled = 1
    default = option_enabled


class Goal(Range):
    """Number of finales to complete in order to win.

    This is also the number of campaigns in the seed, so it effectively sets the length of the run.

    1-3: ~1-2 hours
    4-7: ~3-5 hours
    8-10: ~5-8 hours
    11-14: ~8+ hours
    Times are rough estimates.
    """

    display_name = "Finales to Complete"
    range_start = 1
    range_end = 14
    default = 5


class StartCampaign(Choice):
    """Campaign the run starts in.

    With campaign randomization, the run starts by playing this campaign.
    With chapter randomization, only the first chapter of this campaign is placed at the start of the route.
    """

    display_name = "Starting Campaign"
    option_dead_center = 0
    option_dark_carnival = 1
    option_swamp_fever = 2
    option_hard_rain = 3
    option_the_parish = 4
    option_the_passing = 5
    option_the_sacrifice = 6
    option_no_mercy = 7
    option_crash_course = 8
    option_death_toll = 9
    option_dead_air = 10
    option_blood_harvest = 11
    option_cold_stream = 12
    option_the_last_stand = 13
    default = option_dead_center


class AllCampaignsStart(Toggle):
    """With campaign randomization, start with the first chapter of every campaign unlocked.

    Later chapters remain gated by their chapter items. Finales to Complete still decides
    which campaigns are part of the run. This option is not compatible with chapter randomization.
    """

    display_name = "All Campaigns Start"


class DeathlinkTrigger(Choice):
    """Choose whether to send a Deathlink on being incapacitated or on dying."""

    display_name = "Deathlink Trigger"
    option_downed = 0
    option_death = 1
    default = option_death


class EnableTrapItems(DefaultOnToggle):
    """Allow special-infected trap items to replace filler items in the pool. Received traps enter a
    queue and spawn at the next available trap spawn point.
    """

    display_name = "Enable Trap Items"


class TrapFillPercentage(Range):
    """Percentage of filler items replaced by traps when trap items are enabled."""

    display_name = "Trap Fill Percentage"
    range_start = 0
    range_end = 100
    default = 15


# Save options to slot data for UT
SLOT_DATA_OPTIONS: tuple[str, ...] = (
    "randomization",
    "loadout_logic",
    "start_campaign",
    "all_campaigns_start",
    "progressive_weapon_tiers",
    "progressive_survivors",
    "survival_medals",
    "survival_time_scale",
    "enable_trap_items",
    "trap_fill_percentage",
    "death_link",
    "deathlink_trigger",
    "goal",
)


@dataclass
class L4D2Options(PerGameCommonOptions):
    randomization: Randomization
    loadout_logic: LoadoutLogic
    progressive_weapon_tiers: ProgressiveWeaponTiers
    progressive_survivors: ProgressiveSurvivors
    survival_medals: SurvivalMedals
    survival_time_scale: SurvivalTimeScale
    start_campaign: StartCampaign
    all_campaigns_start: AllCampaignsStart
    death_link: DeathLink
    deathlink_trigger: DeathlinkTrigger
    enable_trap_items: EnableTrapItems
    trap_fill_percentage: TrapFillPercentage
    goal: Goal


l4d2_option_groups = [
    OptionGroup("Route Options", [
        Randomization,
        StartCampaign,
        AllCampaignsStart
    ]),
    OptionGroup("Difficulty & Progression", [
        ProgressiveWeaponTiers,
        ProgressiveSurvivors,
        LoadoutLogic,
        SurvivalMedals,
        SurvivalTimeScale
    ]),
    OptionGroup("Traps", [
        EnableTrapItems,
        TrapFillPercentage
    ]),
]
