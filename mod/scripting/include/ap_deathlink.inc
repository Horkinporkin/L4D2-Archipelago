#if defined _ap_deathlink_included
    #endinput
#endif
#define _ap_deathlink_included

// Sends the player's downs/deaths (with a cause that the client turns into a custom message) and applies
// DeathLinks by incapacitating or killing

#define AP_DEATHLINK_MAX_PENDING 2
#define AP_DEATHLINK_TARGET_LEN 64

enum DeathlinkTrigger
{
    DeathlinkTrigger_Downed,
    DeathlinkTrigger_Death
}

// Maps attackers to usable names known in the client
enum struct DeathCause
{
    char pattern[32];
    char cause[16];
    bool exact;          // whole-string match instead of substring
}

// Checked in order, first match wins
static const DeathCause g_deathlinkWeaponCauses[] =
{
    { "inferno", "Fire", false },
    { "fire", "Fire", false },
    { "spitter_claw", "Spitter Claw", false },
    { "insect_swarm", "Spitter", false },
    { "splat", "Spitter", true },
    { "hunter_claw", "Hunter Claw", false },
    { "pounce", "Hunter", false },
    { "lounge", "Hunter", false },
    { "smoker_claw", "Smoker Claw", false },
    { "smoker", "Smoker", false },
    { "tongue", "Smoker", false },
    { "choke", "Smoker", false },
    { "pull", "Smoker", false },
    { "boomer_claw", "Boomer Claw", false },
    { "boomer", "Boomer", false },
    { "vomit", "Boomer", true },
    { "jockey_claw", "Jockey Claw", false },
    { "jockey", "Jockey", false },
    { "charger_claw", "Charger Claw", false },
    { "charger", "Charger", false },
    { "tank_claw", "Tank Claw", false },
    { "tank_rock", "Tank Rock", false },
    { "rock", "Tank Rock", true },
    { "tank", "Tank", false },
    { "witch", "Witch", false },
    { "infected", "Common", false },
    { "zombie", "Common", false },
    { "trigger_hurt_ghost", "Water", false},
    { "trigger_hurt", "Water", false}
};

static const DeathCause g_deathlinkAttackerCauses[] =
{
    { "smoker", "Smoker", false },
    { "boomer", "Boomer", false },
    { "hunter", "Hunter", false },
    { "spitter", "Spitter", false },
    { "jockey", "Jockey", false },
    { "charger", "Charger", false },
    { "tank", "Tank", false },
    { "witch", "Witch", false },
    { "infected", "Common", false }
};

ConVar g_cvDeathlinkTarget;
ConVar g_cvDeathlinkMaxIncapacitations;
bool g_deathlinkEnabled;
DeathlinkTrigger g_deathlinkTrigger;
bool g_deathlinkSuppressOutgoing;
bool g_deathlinkReceiveQueued;
bool g_deathlinkTargetWarningShown;
char g_deathlinkAutoTargetAuth[AP_DEATHLINK_TARGET_LEN];
int g_deathlinkPending;

// Last cause for sent deathlink
char g_deathlinkLastCause[64];
char g_deathlinkLastAttacker[64];
char g_deathlinkLastWeapon[64];
char g_deathlinkLastVictim[64];
int g_deathlinkLastType;

// Raw values for debugging via sm_ap_deathlink_cause
char g_deathlinkLastRawAttacker[64];
char g_deathlinkLastRawWeapon[64];

// use last incapacitation cause for death
char g_deathlinkLastIncapCause[64];
char g_deathlinkLastIncapAttacker[64];
char g_deathlinkLastIncapWeapon[64];
float g_deathlinkLastIncapTime;

void Deathlink_Init()
{
    // TODO: figure out multiplayer
    g_cvDeathlinkTarget = CreateConVar("l4d2_ap_deathlink_target", "",
        "SteamID64 of the local Archipelago survivor. Empty auto-binds the only human player.");
    g_cvDeathlinkMaxIncapacitations = FindConVar("survivor_max_incapacitated_count");
    g_deathlinkTrigger = DeathlinkTrigger_Death;
}

public Action Command_DeathlinkBind(int client, int args)
{
    if (client <= 0 || !IsClientInGame(client) || IsFakeClient(client))
    {
        ReplyToCommand(client, "[AP] This command must be run by a human player.");
        return Plugin_Handled;
    }

    char auth[AP_DEATHLINK_TARGET_LEN];
    if (!GetClientAuthId(client, AuthId_SteamID64, auth, sizeof(auth)))
    {
        ReplyToCommand(client, "[AP] Your SteamID64 is not available yet.");
        return Plugin_Handled;
    }

    g_cvDeathlinkTarget.SetString(auth);
    ReplyToCommand(client, "[AP] Deathlink is now bound to your survivor.");
    return Plugin_Handled;
}

public Action Command_DeathlinkCause(int client, int args)
{
    if (g_deathlinkLastCause[0] == '\0')
    {
        ReplyToCommand(client, "[AP] No death cause recorded yet.");
        return Plugin_Handled;
    }

    ReplyToCommand(client,
        "[AP] Last cause: %s | attacker=%s weapon=%s victim=%s type=%d raw=%s/%s",
        g_deathlinkLastCause,
        g_deathlinkLastAttacker,
        g_deathlinkLastWeapon,
        g_deathlinkLastVictim,
        g_deathlinkLastType,
        g_deathlinkLastRawAttacker,
        g_deathlinkLastRawWeapon);

    return Plugin_Handled;
}

void Deathlink_Configure(bool enabled, DeathlinkTrigger trigger)
{
    g_deathlinkEnabled = enabled;
    g_deathlinkTrigger = trigger;
    g_deathlinkTargetWarningShown = false;

    if (!enabled)
    {
        g_deathlinkSuppressOutgoing = false;
        g_deathlinkPending = 0;
        g_deathlinkReceiveQueued = false;
    }

    LogMessage("[AP] DeathLink %s; trigger=%s.",
        enabled ? "enabled" : "disabled",
        trigger == DeathlinkTrigger_Downed ? "downed" : "death");
}

void Deathlink_OnMapStart()
{
    g_deathlinkSuppressOutgoing = false;
}

void Deathlink_OnMapEnd()
{
    g_deathlinkSuppressOutgoing = false;
    g_deathlinkPending = 0;
    g_deathlinkReceiveQueued = false;
}

bool Deathlink_IsHumanClient(int client)
{
    return client > 0
        && client <= MaxClients
        && IsClientInGame(client)
        && !IsFakeClient(client);
}

int Deathlink_FindHumanByAuth(const char[] targetAuth)
{
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!Deathlink_IsHumanClient(client))
        {
            continue;
        }

        char auth[AP_DEATHLINK_TARGET_LEN];
        if (GetClientAuthId(client, AuthId_SteamID64, auth, sizeof(auth))
            && StrEqual(auth, targetAuth))
        {
            return client;
        }
    }

    return 0;
}

int Deathlink_GetTargetClient()
{
    char targetAuth[AP_DEATHLINK_TARGET_LEN];
    g_cvDeathlinkTarget.GetString(targetAuth, sizeof(targetAuth));
    TrimString(targetAuth);

    if (targetAuth[0] != '\0')
    {
        return Deathlink_FindHumanByAuth(targetAuth);
    }

    if (g_deathlinkAutoTargetAuth[0] != '\0')
    {
        return Deathlink_FindHumanByAuth(g_deathlinkAutoTargetAuth);
    }

    int candidate = 0;

    for (int client = 1; client <= MaxClients; client++)
    {
        if (!Deathlink_IsHumanClient(client))
        {
            continue;
        }

        if (candidate != 0)
        {
            if (!g_deathlinkTargetWarningShown)
            {
                LogMessage("[AP] Deathlink needs l4d2_ap_deathlink_target or sm_ap_deathlink_bind when multiple human players are connected.");
                g_deathlinkTargetWarningShown = true;
            }

            return 0;
        }

        candidate = client;
    }

    if (candidate != 0 && g_deathlinkEnabled)
    {
        GetClientAuthId(candidate, AuthId_SteamID64, g_deathlinkAutoTargetAuth, sizeof(g_deathlinkAutoTargetAuth));
    }

    return candidate;
}

bool Deathlink_IsTargetPlayer(int client)
{
    return Deathlink_IsHumanClient(client)
        && client == Deathlink_GetTargetClient();
}

bool Deathlink_IsTargetSurvivor(int client)
{
    return Deathlink_IsTargetPlayer(client)
        && Players_IsSurvivor(client);
}

bool Deathlink_IsDowned(int client)
{
    return (HasEntProp(client, Prop_Send, "m_isIncapacitated") && GetEntProp(client, Prop_Send, "m_isIncapacitated") != 0)
        || (HasEntProp(client, Prop_Send, "m_isHangingFromLedge") && GetEntProp(client, Prop_Send, "m_isHangingFromLedge") != 0)
        || (HasEntProp(client, Prop_Send, "m_isFallingFromLedge") && GetEntProp(client, Prop_Send, "m_isFallingFromLedge") != 0);
}

void Deathlink_OnTargetRestored(int client)
{
    if (!g_deathlinkEnabled || !Deathlink_IsTargetPlayer(client))
    {
        return;
    }

    g_deathlinkSuppressOutgoing = false;

    // Clear so next life doesn't reuse old cause
    g_deathlinkLastCause[0] = '\0';
    g_deathlinkLastAttacker[0] = '\0';
    g_deathlinkLastWeapon[0] = '\0';
    g_deathlinkLastIncapCause[0] = '\0';
    g_deathlinkLastIncapAttacker[0] = '\0';
    g_deathlinkLastIncapWeapon[0] = '\0';
    g_deathlinkLastIncapTime = 0.0;
}

bool Deathlink_CanSendOutgoing(DeathlinkTrigger trigger)
{
    return g_deathlinkEnabled
        && g_deathlinkTrigger == trigger
        && !g_deathlinkSuppressOutgoing;
}

bool Deathlink_MatchCause(const DeathCause[] rules, int count, const char[] text, char[] cause, int maxlen)
{
    for (int i = 0; i < count; i++)
    {
        if (rules[i].exact ? StrEqual(text, rules[i].pattern) : StrContains(text, rules[i].pattern) != -1)
        {
            strcopy(cause, maxlen, rules[i].cause);
            return true;
        }
    }
    return false;
}

void Deathlink_CaptureCause(Event event, bool isDeath)
{
    char attackerName[64];
    char victimName[64];
    char weapon[64];
    char rawAttacker[64];
    char rawWeapon[64];
    int type = event.GetInt("type");
    event.GetString("weapon", weapon, sizeof(weapon));
    strcopy(rawWeapon, sizeof(rawWeapon), weapon);

    if (isDeath)
    {
        event.GetString("attackername", attackerName, sizeof(attackerName));
        event.GetString("victimname", victimName, sizeof(victimName));
        strcopy(rawAttacker, sizeof(rawAttacker), attackerName);
    }
    else
    {
        // player_incapacitated has no attacker name, only the attacking entity
        int attackerEntity = event.GetInt("attackerentid");
        if (attackerEntity > 0 && IsValidEntity(attackerEntity))
        {
            GetEntityClassname(attackerEntity, rawAttacker, sizeof(rawAttacker));
            Deathlink_MatchCause(g_deathlinkAttackerCauses, sizeof(g_deathlinkAttackerCauses),
                rawAttacker, attackerName, sizeof(attackerName));
        }
    }

    char baseCause[64];
    if (weapon[0] != '\0')
    {
        if (StrEqual(weapon, "world") || StrEqual(weapon, "worldspawn") || StrEqual(weapon, "trigger_hurt"))
        {
            strcopy(baseCause, sizeof(baseCause), type == DMG_FALL ? "Fall" : "World");
        }
        else if (!Deathlink_MatchCause(g_deathlinkWeaponCauses, sizeof(g_deathlinkWeaponCauses),
            weapon, baseCause, sizeof(baseCause)))
        {
            strcopy(baseCause, sizeof(baseCause), weapon);
        }
    }
    if (baseCause[0] == '\0' && attackerName[0] != '\0')
    {
        strcopy(baseCause, sizeof(baseCause), attackerName);
    }
    if (baseCause[0] == '\0')
    {
        if (type == DMG_FALL)
        {
            strcopy(baseCause, sizeof(baseCause), "Fall");
        }
        else if (type == DMG_BURN)
        {
            strcopy(baseCause, sizeof(baseCause), "Fire");
        }
        else
        {
            strcopy(baseCause, sizeof(baseCause), "Unknown");
        }
    }

    // If death is generic (World/Unknown), use last incap cause
    if (isDeath)
    {
        bool isGeneric = StrEqual(baseCause, "World") || StrEqual(baseCause, "Unknown") || baseCause[0] == '\0';
        bool incapIsSpecific = !(StrEqual(g_deathlinkLastIncapCause, "World") || StrEqual(g_deathlinkLastIncapCause, "Unknown") || g_deathlinkLastIncapCause[0] == '\0');
        float timeSinceIncap = GetGameTime() - g_deathlinkLastIncapTime;

        if (isGeneric && incapIsSpecific && timeSinceIncap < 90.0)
        {
            LogMessage("[AP] Death was generic '%s', using last specific incap '%s' (%.1fs ago)",
                baseCause, g_deathlinkLastIncapCause, timeSinceIncap);
            strcopy(baseCause, sizeof(baseCause), g_deathlinkLastIncapCause);
            strcopy(attackerName, sizeof(attackerName), g_deathlinkLastIncapAttacker);
            strcopy(weapon, sizeof(weapon), g_deathlinkLastIncapWeapon);
        }

        // Clear incap so next life doesn't reuse it
        g_deathlinkLastIncapCause[0] = '\0';
        g_deathlinkLastIncapTime = 0.0;
    }
    else
    {
        strcopy(g_deathlinkLastIncapCause, sizeof(g_deathlinkLastIncapCause), baseCause);
        strcopy(g_deathlinkLastIncapAttacker, sizeof(g_deathlinkLastIncapAttacker), attackerName);
        strcopy(g_deathlinkLastIncapWeapon, sizeof(g_deathlinkLastIncapWeapon), weapon);
        g_deathlinkLastIncapTime = GetGameTime();
    }

    strcopy(g_deathlinkLastCause, sizeof(g_deathlinkLastCause), baseCause);
    strcopy(g_deathlinkLastAttacker, sizeof(g_deathlinkLastAttacker), attackerName);
    strcopy(g_deathlinkLastWeapon, sizeof(g_deathlinkLastWeapon), weapon);
    strcopy(g_deathlinkLastVictim, sizeof(g_deathlinkLastVictim), victimName);
    strcopy(g_deathlinkLastRawAttacker, sizeof(g_deathlinkLastRawAttacker), rawAttacker);
    strcopy(g_deathlinkLastRawWeapon, sizeof(g_deathlinkLastRawWeapon), rawWeapon);
    g_deathlinkLastType = type;

    LogMessage("[AP] Death cause: isDeath=%d base=%s attacker=%s weapon=%s victim=%s type=%d raw=%s/%s",
        isDeath, baseCause, attackerName, weapon, victimName, type, rawAttacker, rawWeapon);
}

bool Deathlink_SendOutgoing(int client, DeathlinkTrigger trigger)
{
    if (!Deathlink_CanSendOutgoing(trigger) || !Deathlink_IsTargetSurvivor(client))
    {
        return false;
    }

    if (g_client == null)
    {
        LogError("[AP] Bridge is disconnected. Could not send DeathLink.");
        return false;
    }

    char triggerName[8];
    strcopy(triggerName, sizeof(triggerName), trigger == DeathlinkTrigger_Downed ? "downed" : "death");

    char escapedCause[sizeof(g_deathlinkLastCause) * 2];
    char escapedAttacker[sizeof(g_deathlinkLastAttacker) * 2];
    char escapedWeapon[sizeof(g_deathlinkLastWeapon) * 2];
    JSON_Escape(g_deathlinkLastCause, escapedCause, sizeof(escapedCause));
    JSON_Escape(g_deathlinkLastAttacker, escapedAttacker, sizeof(escapedAttacker));
    JSON_Escape(g_deathlinkLastWeapon, escapedWeapon, sizeof(escapedWeapon));

    char payload[512];
    FormatEx(payload, sizeof(payload),
        "{\"trigger\":\"%s\",\"cause\":\"%s\",\"attacker\":\"%s\",\"weapon\":\"%s\",\"type\":%d}",
        triggerName, escapedCause, escapedAttacker, escapedWeapon, g_deathlinkLastType);

    Bridge_Send("DEATHLINK", payload);
    g_deathlinkSuppressOutgoing = true;

    Chat_PrintLine("§5[AP]§1 DeathLink sent.");

    LogMessage("[AP] sent DeathLink (%s) cause=%s.", triggerName, g_deathlinkLastCause);
    return true;
}

void Deathlink_OnPlayerIncapacitated(Event event, int client)
{
    if (!Deathlink_IsTargetSurvivor(client))
    {
        return;
    }

    Deathlink_CaptureCause(event, false);
    Deathlink_SendOutgoing(client, DeathlinkTrigger_Downed);
}

void Deathlink_OnPlayerDeath(Event event, int client)
{
    if (!Deathlink_IsTargetSurvivor(client))
    {
        return;
    }

    Deathlink_CaptureCause(event, true);
    Deathlink_SendOutgoing(client, DeathlinkTrigger_Death);
}

void Deathlink_OnMissionLost()
{
    if (!Deathlink_CanSendOutgoing(DeathlinkTrigger_Death))
    {
        return;
    }

    float timeSinceIncap = GetGameTime() - g_deathlinkLastIncapTime;

    if (g_deathlinkLastIncapCause[0] != '\0' && timeSinceIncap < 30.0)
    {
        strcopy(g_deathlinkLastCause, sizeof(g_deathlinkLastCause), g_deathlinkLastIncapCause);
        strcopy(g_deathlinkLastAttacker, sizeof(g_deathlinkLastAttacker), g_deathlinkLastIncapAttacker);
        strcopy(g_deathlinkLastWeapon, sizeof(g_deathlinkLastWeapon), g_deathlinkLastIncapWeapon);
    }
    else
    {
        strcopy(g_deathlinkLastCause, sizeof(g_deathlinkLastCause), "Mission Lost");
        strcopy(g_deathlinkLastAttacker, sizeof(g_deathlinkLastAttacker), "Team Wipe");
        g_deathlinkLastType = 0;
    }

    CreateTimer(5.0, Timer_MissionLostDeathlink);
}

public Action Timer_MissionLostDeathlink(Handle timer)
{
    Deathlink_SendOutgoing(Deathlink_GetTargetClient(), DeathlinkTrigger_Death);
    return Plugin_Stop;
}

void Deathlink_SanitizeChatText(char[] text, int maxlen)
{
    ReplaceString(text, maxlen, "§", "?");
    ReplaceString(text, maxlen, "\n", " ");
    ReplaceString(text, maxlen, "\r", " ");
    ReplaceString(text, maxlen, "\t", " ");
}

void Deathlink_AnnounceReceived(const char[] payload)
{
    char source[AP_NAME_LEN];
    char cause[512];
    char line[1024];

    source[0] = '\0';
    cause[0] = '\0';

    JSON_GetField(payload, "source", source, sizeof(source));
    JSON_GetField(payload, "cause", cause, sizeof(cause));

    Deathlink_SanitizeChatText(source, sizeof(source));
    Deathlink_SanitizeChatText(cause, sizeof(cause));

    if (source[0] != '\0' && cause[0] != '\0')
    {
        Format(line, sizeof(line), "§5[AP]§1 DeathLink received from %s: %s", source, cause);
    }
    else if (source[0] != '\0')
    {
        Format(line, sizeof(line), "§5[AP]§1 DeathLink received from %s.", source);
    }
    else if (cause[0] != '\0')
    {
        Format(line, sizeof(line), "§5[AP]§1 DeathLink received: %s", cause);
    }
    else
    {
        strcopy(line, sizeof(line), "§5[AP]§1 DeathLink received.");
    }

    Chat_PrintLine(line);
}

void Deathlink_Receive(const char[] payload)
{
    if (!g_deathlinkEnabled)
    {
        return;
    }

    Deathlink_AnnounceReceived(payload);

    if (g_deathlinkPending < AP_DEATHLINK_MAX_PENDING)
    {
        g_deathlinkPending++;
    }

    Deathlink_ScheduleReceive();
}

void Deathlink_ScheduleReceive()
{
    if (g_deathlinkReceiveQueued || g_deathlinkPending == 0)
    {
        return;
    }

    g_deathlinkReceiveQueued = true;
    RequestFrame(Frame_DeathlinkReceive);
}

bool Deathlink_ForceIncapacitation(int client)
{
    int maxIncapacitations = g_cvDeathlinkMaxIncapacitations == null ? 0 : g_cvDeathlinkMaxIncapacitations.IntValue;

    if (maxIncapacitations <= 0 || !HasEntProp(client, Prop_Send, "m_currentReviveCount"))
    {
        LogError("[AP] cannot apply a received Deathlink without survivor incapacitation enabled.");
        return false;
    }

    int reviveCount = GetEntProp(client, Prop_Send, "m_currentReviveCount");

    if (reviveCount >= maxIncapacitations)
    {
        SetEntProp(client, Prop_Send, "m_currentReviveCount", maxIncapacitations - 1);
    }

    if (HasEntProp(client, Prop_Send, "m_bIsOnThirdStrike"))
    {
        SetEntProp(client, Prop_Send, "m_bIsOnThirdStrike", 0);
    }

    if (HasEntProp(client, Prop_Send, "m_isGoingToDie"))
    {
        SetEntProp(client, Prop_Send, "m_isGoingToDie", 0);
    }

    if (HasEntProp(client, Prop_Send, "m_healthBuffer"))
    {
        SetEntPropFloat(client, Prop_Send, "m_healthBuffer", 0.0);
    }

    SetVariantInt(0);
    return AcceptEntityInput(client, "SetHealth", client, client);
}

public void Frame_DeathlinkReceive(any data)
{
    g_deathlinkReceiveQueued = false;

    if (!g_deathlinkEnabled || g_deathlinkPending == 0)
    {
        return;
    }

    int client = Deathlink_GetTargetClient();

    if (!Deathlink_IsTargetSurvivor(client) || !IsPlayerAlive(client))
    {
        g_deathlinkPending = 0;
        return;
    }

    g_deathlinkPending--;
    g_deathlinkSuppressOutgoing = true;

    if (Deathlink_IsDowned(client))
    {
        ForcePlayerSuicide(client);
    }
    else if (!Deathlink_ForceIncapacitation(client))
    {
        g_deathlinkSuppressOutgoing = false;
    }

    if (g_deathlinkPending > 0)
    {
        Deathlink_ScheduleReceive();
    }
}
