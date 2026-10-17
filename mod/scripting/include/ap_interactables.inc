#if defined _ap_interactables_included
    #endinput
#endif
#define _ap_interactables_included

// Detects LocationType.INTERACTABLE locations in game (buttons, actions,script triggers, etc.)
// Adds custom triggers for checks as well

StringMap g_interactableQueues;          // output key -> ArrayList of location names still to report
StringMap g_hookedInteractableOutputs;   // "classname:output" pairs already hooked (hooks are global)
char g_interactablesMap[AP_MAP_LEN];     // map that the queues were registered for


// Custom region tables
// -----------------------------
// So far, only first levels have extra checks to improve route gen. Time consuming to add more
enum struct CustomRegion
{
    char map[32];
    char targetname[64];
    char location[128];
    float center[3];
    float extent[3];
}

CustomRegion g_customRegions[] =
{
    { "c1m1_hotel",
        "ap_hotel",
        "Dead Center - Hotel: Reach the Banquet Hall",
        { 176.0, 5685.0, 1245.0 }, { 34.0, 5.0, 61.0 } },
    { "c2m1_highway",
        "ap_motel_entrance1",
        "Dark Carnival - Highway: Reach the Motel",
        { 2336.0, 6547.0, -586.0 }, { 96.0, 19.0, 93.0 } },
    { "c2m1_highway",
        "ap_motel_entrance2",
        "Dark Carnival - Highway: Reach the Motel",
        { 3069.0, 5443.0, -927.0 }, { 6.0, 200.0, 32.0 } },
    { "c2m1_highway",
        "ap_motel",
        "Dark Carnival - Highway: Get Through the Motel",
        { 2212.0, 3648.0, -759.0 }, { 100.0, 72.0, 64.0 } },
    { "c4m1_milltown_a",
        "ap_milltown1",
        "Hard Rain - Milltown: Jump the Wall",
        { -4089.0, 7118.0, 287.0 }, { 16.0, 194.0, 128.0 } },
    { "c4m1_milltown_a",
        "ap_milltown2",
        "Hard Rain - Milltown: Get Past the Garage Sale",
        { 2824.0, 2976.0, 242.0 }, { 8.0, 928.0, 142.0 } },
    { "c5m1_waterfront",
        "ap_waterfront1",
        "The Parish - Waterfront: Reach the Bus",
        { -928.0, -1238.0, -252.0 }, { 273.0, 5.0, 128.0 } },
    { "c5m1_waterfront",
        "ap_waterfront2",
        "The Parish - Waterfront: Get Through the Restaurant",
        { -3208.0, -460.0, -322.0 }, { 8.0, 128.0, 53.0 } },
    { "c6m1_riverbank",
        "ap_riverbank1",
        "The Passing - Riverbank: Get to the Alleyway",
        { 3983.0, 2007.0, 264.0 }, { 6.0, 32.0, 64.0 } },
    { "c6m1_riverbank",
        "ap_riverbank2",
        "The Passing - Riverbank: Enter the Park",
        { 1320.0, 390.0, 616.0 }, { 550.0, 8.0, 103.0 } },
    { "c7m1_docks",
        "ap_docks1",
        "The Sacrifice - Docks: Reach the Wrecked Train",
        { 8385.0, 943.0, 233.0 }, { 446.0, 10.0, 105.0 } },
    { "c7m1_docks",
        "ap_docks2",
        "The Sacrifice - Docks: Get Past the Tank",
        { 6929.0, 952.0, 216.0 }, { 131.0, 10.0, 88.0 } },
    { "c8m1_apartment",
        "ap_apartment1",
        "No Mercy - Apartments: Reach Ground Level",
        { 2292.0, 1022.0, 78.0 }, { 4.0, 32.0, 56.0 } },
    { "c8m1_apartment",
        "ap_apartment2",
        "No Mercy - Apartments: Enter the Subway",
        { 2803.0, 4192.0, 64.0 }, { 10.0, 96.0, 64.0 } },
    { "c9m1_alleys",
        "ap_alleys1",
        "Crash Course - Alleys: Reach the End of the Alleyway",
        { -4253.0, -10130.0, 79.0 }, { 9.0, 151.0, 79.0 } },
    { "c9m1_alleys",
        "ap_alleys2",
        "Crash Course - Alleys: Reach the Waterfront",
        { -3006.0, -5632.0, 76.0 }, { 194.0, 10.0, 76.0 } },
    { "c10m1_caves",
        "ap_turnpike1",
        "Death Toll - Turnpike: Enter the Tunnel",
        { -12372.0, -9822.0, 64.0 }, { 231.0, 10.0, 128.0 } },
    { "c10m1_caves",
        "ap_turnpike2",
        "Death Toll - Turnpike: Exit the Tunnel",
        { -12313.0, -5600.0, -28.0 }, { 10.0, 58.0, 47.0 } },
    { "c11m1_greenhouse",
        "ap_greenhouse1",
        "Dead Air - Greenhouse: Climb the Ladder",
        { 2954.0, 508.0, 655.0 }, { 75.0, 5.0, 44.0 } },
    { "c11m1_greenhouse",
        "ap_greenhouse2",
        "Dead Air - Greenhouse: Reach Ground Level",
        { 3396.0, 2235.0, 324.0 }, { 5.0, 32.0, 43.0 } },
    { "c12m1_hilltop",
        "ap_woods",
        "Blood Harvest - Woods: Cross the Bridge",
        { -10842.0, -11058.0, 477.0 }, { 115.0, 10.0, 64.0 } },
    { "c13m1_alpinecreek",
        "ap_creek",
        "Cold Stream - Alpine Creek: Reach the Fallen Bridge",
        { -988.0, 3593.0, 781.0 }, { 8.0, 210.0, 101.0 } },
    { "c14m1_junkyard",
        "ap_junkyard1",
        "The Last Stand - Junkyard: Under the Bridge",
        { -2557.0, -5730.0, -319.0 }, { 10.0, 208.0, 64.0 } },
    { "c14m1_junkyard",
        "ap_junkyard2",
        "The Last Stand - Junkyard: Shop Yard Entrance",
        { -4113.0, -1865.0, 50.0 }, { 131.0, 10.0, 90.0 } },
};
// -----------------------------


void Interactables_Init()
{
    g_interactableQueues = new StringMap();
    g_hookedInteractableOutputs = new StringMap();

    // On a late load the map is already running
    Interactables_RegisterAll();
}

void Interactables_OnMapStart()
{
    Interactables_OnRoundStart();
}

void Interactables_OnRoundStart()
{
    Interactables_RegisterAll();
    Interactables_CreateForCurrentMap();
}

void Interactables_OnNewSession()
{
    Interactables_RegisterAll();
}

void Interactables_RegisterOutput(const char[] classname, const char[] output, const char[] map,
    const char[] targetnameFilter, const char[] locationName)
{
    if (!StrEqual(map, g_interactablesMap, false))
    {
        return;
    }

    char key[256];
    if (targetnameFilter[0] != '\0')
    {
        FormatEx(key, sizeof(key), "output:%s:%s:%s|%s", classname, output, targetnameFilter, g_interactablesMap);
    }
    else
    {
        FormatEx(key, sizeof(key), "output:%s:%s|%s", classname, output, g_interactablesMap);
    }
    Interactables_Queue(key, locationName);

    char pairKey[128];
    FormatEx(pairKey, sizeof(pairKey), "%s:%s", classname, output);
    if (!g_hookedInteractableOutputs.ContainsKey(pairKey))
    {
        HookEntityOutput(classname, output, Hook_InteractableOutput);
        g_hookedInteractableOutputs.SetValue(pairKey, 1);
    }
}

// Create custom scripted triggers
void Interactables_CreateForCurrentMap()
{
    for (int i = 0; i < sizeof(g_customRegions); i++)
    {
        if (!StrEqual(g_interactablesMap, g_customRegions[i].map, false))
        {
            continue;
        }

        Interactables_SpawnRegion(
            g_customRegions[i].targetname,
            g_customRegions[i].center,
            g_customRegions[i].extent
        );
    }
}

// Spawning of custom triggers in current map
bool Interactables_SpawnRegion(
    const char[] targetname,
    const float center[3],
    const float extent[3])
{
    int ent = CreateEntityByName("script_trigger_multiple");
    if (ent == -1)
    {
        LogError("[AP] Could not create region '%s'.", targetname);
        return false;
    }

    DispatchKeyValue(ent, "targetname", targetname);
    DispatchKeyValue(ent, "spawnflags", "1");
    DispatchKeyValue(ent, "StartDisabled", "0");
    DispatchKeyValueVector(ent, "origin", center);
    DispatchKeyValueVector(ent, "extent", extent);

    if (!DispatchSpawn(ent))
    {
        if (IsValidEntity(ent))
        {
            RemoveEntity(ent);
        }

        LogError("[AP] DispatchSpawn failed for region '%s'.", targetname);
        return false;
    }

    ActivateEntity(ent);
    AcceptEntityInput(ent, "Enable");
    return true;
}

void Interactables_Queue(const char[] key, const char[] locationName)
{
    if (Route_IsLocationReported(locationName))
    {
        return;
    }

    ArrayList queue;
    if (!g_interactableQueues.GetValue(key, queue))
    {
        queue = new ArrayList(ByteCountToCells(128));
        StringMap_SetArrayValue(g_interactableQueues, key, queue);
    }
    queue.PushString(locationName);
}

public void Hook_InteractableOutput(const char[] output, int caller, int activator, float delay)
{
    if (Survival_IsInSurvivalMode())
    {
        return;
    }

    char classname[64];
    GetEntityClassname(caller, classname, sizeof(classname));

    // Plugin regions only count living survivors
    if (StrEqual(classname, "script_trigger_multiple", false) && !Players_IsSurvivor(activator, true))
    {
        return;
    }

    char targetname[64];
    targetname[0] = '\0';
    if (HasEntProp(caller, Prop_Data, "m_iName"))
    {
        GetEntPropString(caller, Prop_Data, "m_iName", targetname, sizeof(targetname));
    }

    // Some triggers have no targetname, fall back to the classname entry for a specific map
    char key[256];
    if (targetname[0] != '\0')
    {
        FormatEx(key, sizeof(key), "output:%s:%s:%s|%s", classname, output, targetname, g_interactablesMap);
        if (g_interactableQueues.ContainsKey(key))
        {
            Interactables_Consume(key);
            return;
        }
    }
    FormatEx(key, sizeof(key), "output:%s:%s|%s", classname, output, g_interactablesMap);
    Interactables_Consume(key);
}

void Interactables_Consume(const char[] key)
{
    ArrayList queue;
    if (!g_interactableQueues.GetValue(key, queue) || queue == null || queue.Length == 0)
    {
        return;
    }

    char locationName[128];
    queue.GetString(0, locationName, sizeof(locationName));
    queue.Erase(0);

    LogMessage("[AP] interactable trigger '%s' -> '%s' (%d left queued).",
        key, locationName, queue.Length);

    Route_ReportInteractableCompletion(locationName);
}

// Rebuilds the current map's queues, skipping locations that were already reported
void Interactables_RegisterAll()
{
    StringMap_ClearArrayValues(g_interactableQueues);
    GetCurrentMap(g_interactablesMap, sizeof(g_interactablesMap));

    Interactables_RegisterDeadCenter();
    Interactables_RegisterDarkCarnival();
    Interactables_RegisterSwampFever();
    Interactables_RegisterHardRain();
    Interactables_RegisterTheParish();
    Interactables_RegisterThePassing();
    Interactables_RegisterTheSacrifice();
    Interactables_RegisterNoMercy();
    Interactables_RegisterCrashCourse();
    Interactables_RegisterDeathToll();
    Interactables_RegisterDeadAir();
    Interactables_RegisterBloodHarvest();
    Interactables_RegisterColdStream();
    Interactables_RegisterTheLastStand();
    Interactables_RegisterCustomRegions();
}

void Interactables_RegisterDeadCenter()
{
    Interactables_RegisterOutput("trigger_once", "OnStartTouch", "c1m1_hotel",
        "ds-f1ceiling_end", "Dead Center - Hotel: Reach Stairwell");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c1m1_hotel",
        "relay_elevator_reached_bottom", "Dead Center - Hotel: Elevator");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c1m2_streets",
        "gunshop_door_relay", "Dead Center - Streets: Gun Store Intercom");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c1m2_streets",
        "gunshop_button_relay", "Dead Center - Streets: Deliver Soda");

    Interactables_RegisterOutput("func_button", "OnPressed", "c1m3_mall", "",
        "Dead Center - Mall: Disable Alarm");

    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #1");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #2");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #3");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #4");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #5");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #6");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #7");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c1m4_atrium",
        "@director", "Dead Center - Atrium: Fuel #8");
}

void Interactables_RegisterDarkCarnival()
{
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m2_fairgrounds",
        "shootgame_prize_award_rl", "Dark Carnival - Fairgrounds: Win the Shooting Game");
    Interactables_RegisterOutput("trigger_multiple", "OnTrigger", "c2m2_fairgrounds",
        "", "Dark Carnival - Fairgrounds: Ride the Slide");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m2_fairgrounds",
        "carousel_lights_relay", "Dark Carnival - Fairgrounds: Activate Carousel");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m2_fairgrounds",
        "carousel_stop_relay", "Dark Carnival - Fairgrounds: Deactivate Carousel");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m3_coaster",
        "relay_start_onslaught", "Dark Carnival - Coaster: Start Roller Coaster");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m3_coaster",
        "relay_end_onslaught", "Dark Carnival - Coaster: Stop the Alarm");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m4_barns",
        "wam_break_machine_rl", "Dark Carnival - Barns: Beat 'Stache Whacker");
    Interactables_RegisterOutput("func_door", "OnOpen", "c2m4_barns",
        "minifinale_gates_slide_door", "Dark Carnival - Barns: Activate Gate");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m5_concert",
        "stadium_lights_relay", "Dark Carnival - Concert: Hit the Lights");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m5_concert",
        "pyrotechnics_relay", "Dark Carnival - Concert: Activate the Sparklers");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c2m5_concert",
        "stage_sound_startup_relay", "Dark Carnival - Concert: Start the Show");
}

void Interactables_RegisterSwampFever()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c3m1_plankcountry",
        "jukebox_button", "Swamp Fever - Plank Country: Turn on the Jukebox");
    Interactables_RegisterOutput("func_button", "OnPressed", "c3m1_plankcountry",
        "ferry_button", "Swamp Fever - Plank Country: Start the Winch");
    Interactables_RegisterOutput("func_door", "OnFullyOpen", "c3m1_plankcountry",
        "ferry_door_right_exit", "Swamp Fever - Plank Country: Cross the River");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c3m2_swamp",
        "Blow_door", "Swamp Fever - Swamp: Blow the Door");

    Interactables_RegisterOutput("func_button", "OnPressed", "c3m3_shantytown",
        "bridge_button", "Swamp Fever - Shantytown: Lower the Bridge");

    Interactables_RegisterOutput("trigger_once", "OnStartTouch", "c3m4_plantation",
        "", "Swamp Fever - Plantation: Enter the Mansion");
    Interactables_RegisterOutput("func_button", "OnIn", "c3m4_plantation",
        "escape_gate_button", "Swamp Fever - Plantation: Call Virgil");
}

void Interactables_RegisterHardRain()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c4m2_sugarmill_a",
        "button_callelevator", "Hard Rain - Sugar Mill: Call Elevator");
    Interactables_RegisterOutput("func_button", "OnPressed", "c4m2_sugarmill_a",
        "button_inelevator", "Hard Rain - Sugar Mill: Ride Elevator");
    Interactables_RegisterOutput("trigger_multiple", "OnEndTouch", "c4m2_sugarmill_a",
        "trigger_elevator", "Hard Rain - Sugar Mill: Survive the Horde");

    Interactables_RegisterOutput("func_button", "OnPressed", "c4m3_sugarmill_b",
        "button_inelevator", "Hard Rain - Sugar Mill Escape: Activate Elevator");
    Interactables_RegisterOutput("trigger_multiple", "OnEndTouch", "c4m3_sugarmill_b",
        "trigger_elevator", "Hard Rain - Sugar Mill Escape: Ride Elevator");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c4m5_milltown_escape",
        "relay_start_boat", "Hard Rain - Milltown Escape: Survive");
}

void Interactables_RegisterTheParish()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c5m1_waterfront",
        "jukebox_button", "The Parish - Waterfront: Turn on the Jukebox");

    Interactables_RegisterOutput("trigger_multiple", "OnStartTouch", "c5m2_park",
        "finale_decon_trigger", "The Parish - Park: Enter the Decontamination Unit");
    Interactables_RegisterOutput("func_button", "OnPressed", "c5m2_park",
        "finale_alarm_stop_button", "The Parish - Park: Stop the Alarm");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c5m3_cemetery",
        "jets_attack", "The Parish - Cemetery: Get on the Freeway");

    Interactables_RegisterOutput("trigger_multiple", "OnStartTouch", "c5m4_quarter",
        "courtyard_trigger_multiple", "The Parish - Quarter: Reach the Courtyard");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c5m4_quarter",
        "tractor_start_relay", "The Parish - Quarter: Start the Tractor");

    Interactables_RegisterOutput("func_button", "OnPressed", "c5m5_bridge",
        "radio_fake_button", "The Parish - Bridge: Activate the Radio");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c5m5_bridge",
        "relay_end_finale", "The Parish - Bridge: Reach the Off-Ramp");
}

void Interactables_RegisterThePassing()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m1_riverbank",
        "button_locker-1", "The Passing - Riverbank: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m1_riverbank",
        "button_locker-2", "The Passing - Riverbank: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m1_riverbank",
        "button_locker-3", "The Passing - Riverbank: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m1_riverbank",
        "button_locker-4", "The Passing - Riverbank: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m1_riverbank",
        "button_locker-5", "The Passing - Riverbank: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m1_riverbank",
        "button_locker-6", "The Passing - Riverbank: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m1_riverbank",
        "stage_lights_button", "The Passing - Riverbank: Change the Song");

    Interactables_RegisterOutput("func_button", "OnPressed", "c6m2_bedlam",
        "jukebox_button", "The Passing - Underground: Turn on the Jukebox");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m2_bedlam",
        "locker1-button_locker", "The Passing - Underground: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m2_bedlam",
        "locker2-button_locker", "The Passing - Underground: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m2_bedlam",
        "locker3-button_locker", "The Passing - Underground: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m2_bedlam",
        "locker4-button_locker", "The Passing - Underground: Open the Footlocker");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m2_bedlam",
        "locker5-button_locker", "The Passing - Underground: Open the Footlocker");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c6m2_bedlam",
        "relay_minifinale", "The Passing - Underground: Open Gate");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c6m2_bedlam",
        "relay_second_gate", "The Passing - Underground: Open Second Gate");

    Interactables_RegisterOutput("trigger_once", "OnTrigger", "c6m3_port",
        "l4d1_script_trigger", "The Passing - Port: Meet the Old Crew");
    Interactables_RegisterOutput("func_button", "OnPressed", "c6m3_port",
        "jukebox_button", "The Passing - Port: Turn on the Jukebox");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #1");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #2");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #3");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #4");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #5");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #6");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #7");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #8");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #9");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c6m3_port",
        "@director", "The Passing - Port: Fuel #10");
}

void Interactables_RegisterTheSacrifice()
{
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c7m1_docks",
        "panic_event_relay", "The Sacrifice - Docks: Open Train Door");

    Interactables_RegisterOutput("trigger_once", "OnTrigger", "c7m2_barge",
        "panic_trigger", "The Sacrifice - Barge: Reach the Barge");

    Interactables_RegisterOutput("func_button", "OnPressed", "c7m3_port",
        "InstanceAuto1-jukebox_button", "The Sacrifice - Port: Turn on the Jukebox");
    Interactables_RegisterOutput("math_counter", "OnHitMax", "c7m3_port",
        "button_counter_1", "The Sacrifice - Port: Start the First Generator");
    Interactables_RegisterOutput("math_counter", "OnHitMax", "c7m3_port",
        "button_counter_2", "The Sacrifice - Port: Start the Second Generator");
    Interactables_RegisterOutput("math_counter", "OnHitMax", "c7m3_port",
        "button_counter_3", "The Sacrifice - Port: Start the Third Generator");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c7m3_port",
        "tank_spawner_coop_relay", "The Sacrifice - Port: Lift the Bridge");
}

void Interactables_RegisterNoMercy()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c8m2_subway",
        "", "No Mercy - Subway: Open Garage Door");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c8m3_sewers",
        "washer_lift_up_relay", "No Mercy - Sewers: Activate Lift");
    Interactables_RegisterOutput("func_door", "OnFullyOpen", "c8m3_sewers",
        "door_sliding", "No Mercy - Sewers: Open Shutter Door");
    Interactables_RegisterOutput("trigger_once", "OnTrigger", "c8m3_sewers",
        "manhole_button1_trigger", "No Mercy - Sewers: Open Manhole");
    Interactables_RegisterOutput("trigger_once", "OnTrigger", "c8m3_sewers",
        "manhole_button3_trigger", "No Mercy - Sewers: Open Manhole");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c8m4_interior",
        "elevator_number_relay", "No Mercy - Hospital: Call Elevator");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c8m4_interior",
        "elevator_top_relay", "No Mercy - Hospital: Ride Elevator Up");

    Interactables_RegisterOutput("func_button", "OnPressed", "c8m5_rooftop",
        "radio_button", "No Mercy - Rooftop: Activate Radio");
}

void Interactables_RegisterCrashCourse()
{
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c9m1_alleys",
        "howitzer_holdout_relay", "Crash Course - Alleys: Fire the Howitzer");
    Interactables_RegisterOutput("trigger_once", "OnTrigger", "c9m1_alleys",
        "bridge_entrance_trigger", "Crash Course - Alleys: Enter the Bridge");

    Interactables_RegisterOutput("trigger_multiple", "OnTrigger", "c9m2_lots",
        "finale_button_unlocker", "Crash Course - Truck Depot: Enter the Depot");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c9m2_lots",
        "survivalmode_exempt", "Crash Course - Truck Depot: Start the Generator");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c9m2_lots",
        "generator_breaker_relay", "Crash Course - Truck Depot: Restart the Generator");
}

void Interactables_RegisterDeathToll()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c10m2_drainage",
        "button_minifinale", "Death Toll - Drains: Activate Floodgates");

    Interactables_RegisterOutput("func_button", "OnPressed", "c10m3_ranchhouse",
        "button_safedoor_PANIC", "Death Toll - Church: Reach Safehouse");
    Interactables_RegisterOutput("func_button", "OnPressed", "c10m3_ranchhouse",
        "", "Death Toll - Church: Play the Piano");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c10m4_mainstreet",
        "forklift_relay", "Death Toll - Town: Lower the Forklift");

    Interactables_RegisterOutput("func_button", "OnPressed", "c10m5_houseboat",
        "", "Death Toll - Boathouse: Activate Radio");
}

void Interactables_RegisterDeadAir()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c11m2_offices",
        "crane button", "Dead Air - Crane: Activate Crane");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c11m2_offices",
        "caralarm1_ready_relay", "Dead Air - Crane: Navigate the Office Building");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c11m4_terminal",
        "van_start_relay", "Dead Air - Terminal: Start the Van");
    Interactables_RegisterOutput("trigger_once", "OnTrigger", "c11m4_terminal",
        "van_follow_trigger", "Dead Air - Terminal: Slam the Van Through the Gate");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c11m4_terminal",
        "alarm_on_relay", "Dead Air - Terminal: Metal Detector");

    Interactables_RegisterOutput("func_button", "OnPressed", "c11m5_runway",
        "radio_fake_button", "Dead Air - Runway: Activate Radio");
}

void Interactables_RegisterBloodHarvest()
{
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c12m2_traintunnel",
        "emergency_door_relay", "Blood Harvest - Tunnel: Open Emergency Exit");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c12m3_bridge",
        "train_engine_relay", "Blood Harvest - Bridge: Decouple Train");

    Interactables_RegisterOutput("trigger_once", "OnTrigger", "c12m5_cornfield",
        "cornfield_trigger", "Blood Harvest - Farmhouse: Enter the Cornfield");
    Interactables_RegisterOutput("func_button", "OnPressed", "c12m5_cornfield",
        "radio_button", "Blood Harvest - Farmhouse: Activate Radio");
}

void Interactables_RegisterColdStream()
{
    Interactables_RegisterOutput("func_button_timed", "OnPressed", "c13m1_alpinecreek",
        "bunker_button", "Cold Stream - Alpine Creek: Open the Bunker Door");

    Interactables_RegisterOutput("func_button", "OnPressed", "c13m2_southpinestream",
        "bridge_button", "Cold Stream - South Pine Stream: Blow the Barricades");

    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c13m3_memorialbridge",
        "tanker_destruction_relay", "Cold Stream - Memorial Bridge: Tanker Explosion");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c13m3_memorialbridge",
        "relay_bridge_event", "Cold Stream - Memorial Bridge: Bridge Explosion");

    Interactables_RegisterOutput("func_button", "OnPressed", "c13m4_cutthroatcreek",
        "startbldg_door_button", "Cold Stream - Cutthroat Creek: Activate Radio");
    Interactables_RegisterOutput("func_door", "OnOpen", "c13m4_cutthroatcreek",
        "startbldg_door", "Cold Stream - Cutthroat Creek: Open Shutter Door");
}

void Interactables_RegisterTheLastStand()
{
    Interactables_RegisterOutput("func_button", "OnPressed", "c14m1_junkyard",
        "fuel_button", "The Last Stand - Junkyard: Turn the Fuel Valve");
    Interactables_RegisterOutput("logic_case", "OnCase01", "c14m1_junkyard",
        "generator_panic_case", "The Last Stand - Junkyard: Jumpstart Generator #1");
    Interactables_RegisterOutput("logic_case", "OnCase02", "c14m1_junkyard",
        "generator_panic_case", "The Last Stand - Junkyard: Jumpstart Generator #2");
    Interactables_RegisterOutput("logic_case", "OnCase03", "c14m1_junkyard",
        "generator_panic_case", "The Last Stand - Junkyard: Jumpstart Generator #3");
    Interactables_RegisterOutput("logic_case", "OnCase04", "c14m1_junkyard",
        "generator_panic_case", "The Last Stand - Junkyard: Jumpstart Generator #4");
    Interactables_RegisterOutput("func_button", "OnPressed", "c14m1_junkyard",
        "drop_button", "The Last Stand - Junkyard: Drop the Container");

    Interactables_RegisterOutput("logic_case", "OnCase01", "c14m2_lighthouse",
        "case_footlocker", "The Last Stand - Lighthouse: Open the Footlocker");
    Interactables_RegisterOutput("logic_case", "OnCase02", "c14m2_lighthouse",
        "case_footlocker", "The Last Stand - Lighthouse: Open the Footlocker");
    Interactables_RegisterOutput("logic_case", "OnCase03", "c14m2_lighthouse",
        "case_footlocker", "The Last Stand - Lighthouse: Open the Footlocker");
    Interactables_RegisterOutput("logic_case", "OnCase04", "c14m2_lighthouse",
        "case_footlocker", "The Last Stand - Lighthouse: Open the Footlocker");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c14m2_lighthouse",
        "relay_boat_coming1", "The Last Stand - Lighthouse: Start the Generator");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c14m2_lighthouse",
        "relay_boat_coming2", "The Last Stand - Lighthouse: Survive the First Horde");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #1");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #2");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #3");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #4");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #5");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #6");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #7");
    Interactables_RegisterOutput("info_director", "OnTeamScored", "c14m2_lighthouse",
        "director", "The Last Stand - Lighthouse: Fuel #8");
    Interactables_RegisterOutput("logic_relay", "OnTrigger", "c14m2_lighthouse",
        "relay_start_boat", "The Last Stand - Lighthouse: Survive the Second Horde");
}

void Interactables_RegisterCustomRegions()
{
    for (int i = 0; i < sizeof(g_customRegions); i++)
    {
        Interactables_RegisterOutput(
            "script_trigger_multiple",
            "OnStartTouch",
            g_customRegions[i].map,
            g_customRegions[i].targetname,
            g_customRegions[i].location
        );
    }
}
