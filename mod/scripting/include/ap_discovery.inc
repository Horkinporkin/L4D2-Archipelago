#if defined _ap_discovery_included
    #endinput
#endif
#define _ap_discovery_included

// Dev tool for finding interactable hooks: logs every distinct entity output it sees as a ready-made
// Interactables_RegisterOutput() line. Enable it by uncommenting its #include and its Discovery_Init() call in
// archipelago.sp.
// More registers can be beneficial to add. Useful to use Hammer to explore map entities

#define AP_DISCOVERY_LOG_FILE "ap_discovery.log"

ConVar g_cvDiscoveryMode;
StringMap g_discoverySeen;

void Discovery_Init()
{
    g_cvDiscoveryMode = CreateConVar("l4d2_ap_discovery", "1",
        "Log entity I/O and game events broadly, to help find interactable hooks. Disable once done mapping campaigns.");

    g_discoverySeen = new StringMap();

    if (!g_cvDiscoveryMode.BoolValue)
    {
        return;
    }

    Discovery_RegisterCommonHooks();
    RegAdminCmd("sm_ap_dump_logic", Command_DumpLogicEntities, ADMFLAG_GENERIC,
        "List targetnames of common logic entities on the current map.");
    RegAdminCmd("sm_ap_dump_all", Command_DumpAllEntities, ADMFLAG_GENERIC, "Dump every entity and targetname on the current map.");
}

void Discovery_HookPair(const char[] classname, const char[] output)
{
    HookEntityOutput(classname, output, Hook_DiscoveryOutput);
}

void Discovery_RegisterCommonHooks()
{
    Discovery_HookPair("logic_relay", "OnTrigger");
    Discovery_HookPair("logic_case", "OnDefault");
    for (int i = 1; i <= 16; i++)
    {
        char output[16];
        Format(output, sizeof(output), "OnCase%02d", i);
        Discovery_HookPair("logic_case", output);
    }
    Discovery_HookPair("logic_branch", "OnTrue");
    Discovery_HookPair("logic_branch", "OnFalse");
    Discovery_HookPair("logic_auto", "OnMapSpawn");
    Discovery_HookPair("logic_timer", "OnTimer");
    Discovery_HookPair("logic_timer", "OnTimerHigh");
    Discovery_HookPair("logic_timer", "OnTimerLow");

    Discovery_HookPair("math_counter", "OnHitMax");
    Discovery_HookPair("math_counter", "OnHitMin");
    Discovery_HookPair("math_counter", "OnHalfMax");
    Discovery_HookPair("math_counter", "OnChangedFromMin");
    Discovery_HookPair("math_counter", "OnChangedFromMax");
    Discovery_HookPair("math_counter", "OnGetValue");

    Discovery_HookPair("trigger_multiple", "OnStartTouch");
    Discovery_HookPair("trigger_multiple", "OnEndTouch");
    Discovery_HookPair("trigger_multiple", "OnTrigger");
    Discovery_HookPair("trigger_once", "OnStartTouch");
    Discovery_HookPair("trigger_once", "OnTrigger");

    Discovery_HookPair("func_button", "OnPressed");
    Discovery_HookPair("func_button", "OnIn");
    Discovery_HookPair("func_button", "OnOut");
    Discovery_HookPair("func_button", "OnDamaged");
    Discovery_HookPair("func_button", "OnUseLocked");

    Discovery_HookPair("func_button_timed", "OnPressed");
    Discovery_HookPair("func_button_timed", "OnIn");
    Discovery_HookPair("func_button_timed", "OnOut");
    Discovery_HookPair("func_button_timed", "OnDamaged");
    Discovery_HookPair("func_button_timed", "OnUseLocked");
    Discovery_HookPair("func_button_timed", "OnTimeUp");

    Discovery_HookPair("func_door", "OnFullyOpen");
    Discovery_HookPair("func_door", "OnFullyClosed");
    Discovery_HookPair("func_door", "OnOpen");
    Discovery_HookPair("func_door", "OnClose");
    Discovery_HookPair("prop_door_rotating", "OnFullyOpen");
    Discovery_HookPair("prop_door_rotating", "OnFullyClosed");
    Discovery_HookPair("prop_door_rotating", "OnOpen");
    Discovery_HookPair("prop_door_rotating", "OnClose");

    Discovery_HookPair("func_breakable", "OnBreak");
    Discovery_HookPair("func_breakable", "OnHealthChanged");
    Discovery_HookPair("point_template", "OnEntitySpawned");

    Discovery_HookPair("info_director", "OnTeamScored");

    static char commonClassnames[][32] = {
        "logic_relay", "logic_case", "logic_branch", "logic_timer",
        "math_counter", "trigger_multiple", "trigger_once", "func_button",
        "func_door", "prop_door_rotating", "func_breakable"
    };
    for (int i = 0; i < sizeof(commonClassnames); i++)
    {
        Discovery_HookPair(commonClassnames[i], "OnUser1");
        Discovery_HookPair(commonClassnames[i], "OnUser2");
        Discovery_HookPair(commonClassnames[i], "OnUser3");
        Discovery_HookPair(commonClassnames[i], "OnUser4");
    }
}

public void Hook_DiscoveryOutput(const char[] output, int caller, int activator, float delay)
{
    char classname[64];
    GetEntityClassname(caller, classname, sizeof(classname));

    char targetname[64];
    targetname[0] = '\0';

    if (HasEntProp(caller, Prop_Data, "m_iName"))
    {
        GetEntPropString(caller, Prop_Data, "m_iName", targetname, sizeof(targetname));
    }

    char map[64];
    GetCurrentMap(map, sizeof(map));

    bool isNoise =
        (StrEqual(classname, "logic_timer") &&
            (StrEqual(output, "OnTimer") ||
             StrEqual(output, "OnTimerHigh") ||
             StrEqual(output, "OnTimerLow")))
        || (StrEqual(classname, "logic_auto") && StrEqual(output, "OnMapSpawn"))
        || (StrEqual(classname, "point_template") && StrEqual(output, "OnEntitySpawned"));

    if (isNoise)
    {
        return;
    }

    char key[256];
    Format(key, sizeof(key), "%s|%s|%s|%s",
        map,
        classname,
        output,
        targetname);

    if (g_discoverySeen.ContainsKey(key))
    {
        return;
    }

    g_discoverySeen.SetValue(key, 1);

    LogToFileEx(AP_DISCOVERY_LOG_FILE,
        "Interactables_RegisterOutput(\"%s\", \"%s\", \"%s\",\n\t\"%s\", \"TODO: Check Name\");",
        classname,
        output,
        map,
        targetname);

    LogMessage("[AP DISCOVERY] Interactables_RegisterOutput(\"%s\", \"%s\", \"%s\", \"%s\", \"TODO: Check Name\");",
        classname,
        output,
        map,
        targetname);
}

public Action Command_DumpLogicEntities(int client, int args)
{
    static char classnames[][32] = {
        "logic_relay", "logic_case", "logic_branch", "logic_timer",
        "math_counter", "trigger_multiple", "trigger_once", "func_button",
        "func_door", "prop_door_rotating", "func_breakable", "info_game_event_proxy"
    };

    for (int i = 0; i < sizeof(classnames); i++)
    {
        int entity = -1;
        while ((entity = FindEntityByClassname(entity, classnames[i])) != -1)
        {
            char targetname[64];
            targetname[0] = '\0';
            if (HasEntProp(entity, Prop_Data, "m_iName"))
            {
                GetEntPropString(entity, Prop_Data, "m_iName", targetname, sizeof(targetname));
            }

            char extra[128];
            extra[0] = '\0';
            if (StrEqual(classnames[i], "info_game_event_proxy") && HasEntProp(entity, Prop_Data, "m_iszEventName"))
            {
                GetEntPropString(entity, Prop_Data, "m_iszEventName", extra, sizeof(extra));
            }

            ReplyToCommand(client, "[AP] %s: %s%s%s", classnames[i],
                targetname[0] != '\0' ? targetname : "<unnamed>",
                extra[0] != '\0' ? " event_name=" : "", extra);
        }
    }
    return Plugin_Handled;
}

public Action Command_DumpAllEntities(int client, int args)
{
    char classname[64];
    char targetname[128];

    for (int entity = MaxClients + 1; entity < GetMaxEntities(); entity++)
    {
        if (!IsValidEntity(entity))
        {
            continue;
        }

        GetEntityClassname(entity, classname, sizeof(classname));

        targetname[0] = '\0';
        if (HasEntProp(entity, Prop_Data, "m_iName"))
        {
            GetEntPropString(entity, Prop_Data, "m_iName",
                targetname, sizeof(targetname));
        }

        ReplyToCommand(client, "[AP] %d: %s targetname=%s",
            entity,
            classname,
            targetname[0] != '\0' ? targetname : "<unnamed>");
    }

    return Plugin_Handled;
}
