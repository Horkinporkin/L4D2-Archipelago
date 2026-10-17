#if defined _ap_players_included
    #endinput
#endif
#define _ap_players_included

// Handles player events and spawning issues
// Still requires some work. Survivor spawn issues is apparently an engine issue
// https://forums.alliedmods.net/showthread.php?t=335117

#define AP_SPAWN_FIX_MAX_ATTEMPTS 5
#define AP_SPAWN_GRACE 15.0
#define AP_FORCED_TRAVEL_REPAIR_DELAY 1.0

bool g_forcedTravelPending;
bool g_forcedTravelRecovery;
int g_forcedTravelRecoveryId;
int g_spawnFixAttempts[MAXPLAYERS + 1];
float g_spawnGraceUntil;

void Players_OnForcedTravel()
{
    g_forcedTravelPending = true;
}

void Players_OnMapStart()
{
    g_spawnGraceUntil = GetTickedTime() + AP_SPAWN_GRACE;
    g_forcedTravelRecovery = g_forcedTravelPending;
    g_forcedTravelPending = false;
    if (g_forcedTravelRecovery)
    {
        g_forcedTravelRecoveryId++;
        CreateTimer(AP_FORCED_TRAVEL_REPAIR_DELAY, Timer_RepairForcedTravelSurvivors,
            g_forcedTravelRecoveryId, TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action Timer_RepairForcedTravelSurvivors(Handle timer, any recoveryId)
{
    if (!g_forcedTravelRecovery || recoveryId != g_forcedTravelRecoveryId)
    {
        return Plugin_Stop;
    }
    Players_FixTransitionedSurvivors();
    return Plugin_Stop;
}

bool Players_IsSurvivor(int client, bool requireAlive = false)
{
    if (client <= 0 || client > MaxClients || !IsClientInGame(client)
        || GetClientTeam(client) != 2)
    {
        return false;
    }
    return !requireAlive || IsPlayerAlive(client);
}

int Players_GetFirstSurvivor()
{
    int firstBot = -1;
    for (int i = 1; i <= MaxClients; i++)
    {
        if (!Players_IsSurvivor(i, true))
        {
            continue;
        }
        if (!IsFakeClient(i))
        {
            return i;
        }
        if (firstBot == -1)
        {
            firstBot = i;
        }
    }
    return firstBot;
}

int Players_GetNearestSurvivor(const float from[3])
{
    int best = -1;
    float bestDistance = 0.0;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!Players_IsSurvivor(client, true))
        {
            continue;
        }
        float origin[3];
        GetClientAbsOrigin(client, origin);
        float distance = GetVectorDistance(from, origin);
        if (best == -1 || distance < bestDistance)
        {
            best = client;
            bestDistance = distance;
        }
    }
    return best;
}

void Players_GetPointInFront(int client, float distance, float height, float origin[3], float angles[3])
{
    float direction[3];
    GetClientAbsOrigin(client, origin);
    GetClientEyeAngles(client, angles);
    angles[0] = 0.0;
    angles[2] = 0.0;
    GetAngleVectors(angles, direction, NULL_VECTOR, NULL_VECTOR);
    origin[0] += direction[0] * distance;
    origin[1] += direction[1] * distance;
    origin[2] += height;
}

bool Players_IsOriginSafe(float origin[3])
{
    return !TR_PointOutsideWorld(origin);
}

bool Players_GetEntityWorldOrigin(int entity, float origin[3])
{
    if (HasEntProp(entity, Prop_Send, "m_vecOrigin"))
    {
        GetEntPropVector(entity, Prop_Send, "m_vecOrigin", origin);
        return true;
    }
    if (HasEntProp(entity, Prop_Data, "m_vecOrigin"))
    {
        GetEntPropVector(entity, Prop_Data, "m_vecOrigin", origin);
        return true;
    }
    return false;
}

bool Players_FindNearestSafeOrigin(const char[] classname, const float from[3], float outOrigin[3])
{
    bool found = false;
    float bestDist = -1.0;
    int entity = -1;
    while ((entity = FindEntityByClassname(entity, classname)) != -1)
    {
        float origin[3];
        if (!Players_GetEntityWorldOrigin(entity, origin) || !Players_IsOriginSafe(origin))
        {
            continue;
        }
        float dist = GetVectorDistance(from, origin);
        if (!found || dist < bestDist)
        {
            found = true;
            bestDist = dist;
            outOrigin = origin;
        }
    }
    return found;
}

bool Players_FindStartingSafeOrigin(const char[] classname, float outOrigin[3])
{
    int entity = -1;
    while ((entity = FindEntityByClassname(entity, classname)) != -1)
    {
        float origin[3];
        if (!Players_GetEntityWorldOrigin(entity, origin) || !Players_IsOriginSafe(origin)
            || !L4D_IsPositionInFirstCheckpoint(origin))
        {
            continue;
        }
        outOrigin = origin;
        return true;
    }
    return false;
}

bool Players_FindTransitionSafeOrigin(float outOrigin[3])
{
    return Players_FindStartingSafeOrigin("info_survivor_position", outOrigin)
        || Players_FindStartingSafeOrigin("info_player_start", outOrigin);
}

bool Players_FixUndergroundSpawn(int client)
{
    float origin[3];
    GetClientAbsOrigin(client, origin);
    bool onGround = (GetEntityFlags(client) & FL_ONGROUND) != 0;
    bool originSafe = Players_IsOriginSafe(origin);
    bool useTransitionAnchor = g_forcedTravelRecovery && GetTickedTime() <= g_spawnGraceUntil;

    float target[3];
    bool haveTarget = useTransitionAnchor
        ? Players_FindTransitionSafeOrigin(target)
        : Players_FindNearestSafeOrigin("info_survivor_position", origin, target);
    if (!haveTarget && !useTransitionAnchor)
    {
        haveTarget = Players_FindNearestSafeOrigin("info_player_start", origin, target);
    }

    if (originSafe && onGround)
    {
        if (!haveTarget || origin[2] > target[2] - 500.0)
        {
            return true;
        }
    }

    if (!haveTarget)
    {
        for (int other = 1; other <= MaxClients; other++)
        {
            if (other == client || !Players_IsSurvivor(other, true))
            {
                continue;
            }
            float otherOrigin[3];
            GetClientAbsOrigin(other, otherOrigin);
            if (Players_IsOriginSafe(otherOrigin) && (GetEntityFlags(other) & FL_ONGROUND) != 0)
            {
                target = otherOrigin;
                haveTarget = true;
                break;
            }
        }
    }

    if (!haveTarget)
    {
        return false;
    }

    TeleportEntity(client, target, NULL_VECTOR, NULL_VECTOR);
    L4D_WarpToValidPositionIfStuck(client);
    LogMessage("[AP] moved survivor %N to a safe spawn after map transition.", client);
    return true;
}

void Players_FixSurvivorSpawn(int client)
{
    g_spawnFixAttempts[client] = 0;
    RequestFrame(Frame_FixSpawnPosition, GetClientUserId(client));
}

void Players_OnTransitionRestore(int client)
{
    if (!Players_IsSurvivor(client, true))
    {
        return;
    }
    Players_FixSurvivorSpawn(client);
    Items_ScheduleFlush();
}

void Players_FixTransitionedSurvivors()
{
    bool foundSurvivor = false;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (!Players_IsSurvivor(client, true))
        {
            continue;
        }
        foundSurvivor = true;
        Players_FixSurvivorSpawn(client);
    }
    if (foundSurvivor)
    {
        Items_ScheduleFlush();
    }
}

public void Frame_FixSpawnPosition(any userId)
{
    Players_TryFixSpawn(userId);
}

public Action Timer_FixSpawnPosition(Handle timer, any userId)
{
    Players_TryFixSpawn(userId);
    return Plugin_Stop;
}

// Retries once a second until the survivor stands somewhere safe, up to AP_SPAWN_FIX_MAX_ATTEMPTS times
// Needs work
void Players_TryFixSpawn(int userId)
{
    int client = GetClientOfUserId(userId);
    if (!Players_IsSurvivor(client, true) || Players_FixUndergroundSpawn(client))
    {
        return;
    }

    g_spawnFixAttempts[client]++;
    if (g_spawnFixAttempts[client] >= AP_SPAWN_FIX_MAX_ATTEMPTS)
    {
        LogError("[AP] could not find a safe spawn for %N after map change (gave up after %d attempts).",
            client, g_spawnFixAttempts[client]);
        return;
    }

    CreateTimer(1.0, Timer_FixSpawnPosition, userId, TIMER_FLAG_NO_MAPCHANGE);
}

void Players_OnPlayerSpawn(int client)
{
    Players_FixSurvivorSpawn(client);
    Items_ScheduleFlush();
    CreateTimer(AP_SPAWN_MELEE_DELAY, Timer_GiveSpawnMelee, GetClientUserId(client),
        TIMER_FLAG_NO_MAPCHANGE);

    if (g_inventoryRouteVerified && g_inventoryRestorePending[client])
    {
        CreateTimer(1.0, Timer_RestoreClientInventory, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
    }
}

// Bots sometimes die early during the transition
void Players_OnPlayerDeath(int victim)
{
    if (GetTickedTime() > g_spawnGraceUntil || !Players_IsSurvivor(victim) || !IsFakeClient(victim))
    {
        return;
    }

    CreateTimer(0.3, Timer_RespawnEarlyDeathBot, GetClientUserId(victim), TIMER_FLAG_NO_MAPCHANGE);
}

public Action Timer_RespawnEarlyDeathBot(Handle timer, any userId)
{
    if (GetTickedTime() > g_spawnGraceUntil)
    {
        return Plugin_Stop;
    }

    int client = GetClientOfUserId(userId);
    if (!Players_IsSurvivor(client) || !IsFakeClient(client) || IsPlayerAlive(client))
    {
        return Plugin_Stop;
    }

    L4D_RespawnPlayer(client, false);
    LogMessage("[AP] respawned %N after an early post-transition death.", client);
    return Plugin_Stop;
}
