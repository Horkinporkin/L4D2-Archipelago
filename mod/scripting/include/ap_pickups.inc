#if defined _ap_pickups_included
    #endinput
#endif
#define _ap_pickups_included

// AP Item Carriers: once per chapter a common infected carries the chapter's AP item. Killing it drops the item,
// and using the drop sends "<Chapter>: AP Item Carrier". A director timer decides when to arm a carrier
// (map flow progress, hordes, finale start); lost carriers and stranded drops are re-armed later.

#define AP_ITEM_MODEL       "models/archipelago/ap_item.mdl"
#define AP_MARKER_MODEL     "models/archipelago/ap_waypoint.mdl"
#define AP_PICKUP_SOUND     "ui/pickup_secret01.wav" // TODO: fix

// Behavior tuning
// all distances in source units
#define AP_PK_CARRIER_MIN_DIST  250.0    // don't pick a zombie already on top of the player
#define AP_PK_CARRIER_MAX_DIST  1500.0
#define AP_PK_BACKSTOP_FLOW     0.92     // arm regardless of target past this
#define AP_PK_CULL_DIST         1000.0   // carrier vanished with a survivor this close = it was killed, not culled
#define AP_PK_STRAND_TIME       30.0     // seconds before a far away drop is pulled
#define AP_PK_STRAND_DIST       1800.0
#define AP_PK_GLOW_COLOR        (160 + (80 << 8) + (255 << 16))   // purple, R + G*256 + B*65536

// Arming "schedule"
// Director uses 'flow' to determine player progress
#define AP_PK_HIGH_FLOW          0.90    // flow past which a re-roll is no longer "ahead"
#define AP_PK_INITIAL_FLOW_MIN   0.05    // earliest flow the first carrier can be armed at. consider omitting this?
#define AP_PK_INITIAL_ARM_DELAY  10.0    // seconds after map start before the first arm attempt
#define AP_PK_ARM_RETRY_DELAY     5.0    // seconds to wait after an arm attempt found no carrier
#define AP_PK_HORDE_ARM_DELAY     2.0    // seconds to let a horde finish spawning before arming
#define AP_PK_FINALE_ARM_MIN     10.0    // random window used during a finale chapter, in which flow stops
#define AP_PK_FINALE_ARM_MAX     30.0
#define AP_PK_DROP_SPAWN_GRACE    1.0    // drop that dies inside this window means the model has no .phy

// Waypoint marker animation
#define AP_PK_MARKER_BOB_AMP    6.0      // bob amplitude in units
#define AP_PK_MARKER_BOB_SPEED  1.5      // radians/sec
#define AP_PK_MARKER_SPIN       90.0     // degrees/sec

// !appickup moves the carrier this far in front of the survivor
#define AP_PK_SUMMON_DISTANCE   90.0
#define AP_PK_SUMMON_HEIGHT     8.0

// Model orientation
static const float g_pkMarkerAngles[3] = { 90.0, 0.0, 0.0 };   // waypoint
static const float g_pkDropAngles[3]   = { 0.0, 0.0, 0.0 };   // dropped physics prop

enum PickupState
{
    PK_IDLE = 0,
    PK_CARRIER,
    PK_DROPPED,
    PK_PENDING_DROP,
    PK_DONE
};

PickupState g_pkState;
int g_pkCarrierRef = INVALID_ENT_REFERENCE;
int g_pkMarkerRef = INVALID_ENT_REFERENCE;
int g_pkDropRef = INVALID_ENT_REFERENCE;
int g_pkCarrierIndex = -1;
float g_pkLastPos[3];
float g_pkPendingPos[3];         // where the drop will spawn
float g_pkTarget;          // flow fraction the next carrier is armed at
float g_pkNextArm;         // GetGameTime() before nothing is armed
float g_pkDropTime;
char g_pkLocation[128];    // "<Chapter>: AP Item Carrier" for the current map
float g_pkMarkerBirth;     // GetGameTime() when the current marker was attached
float g_pkMarkerBaseZ;     // marker's resting local Z, the bob oscillates around it

ConVar g_cvPkHordeChance;
ConVar g_cvPkCooldownMin;
ConVar g_cvPkCooldownMax;

void Pickups_Init()
{
    g_cvPkHordeChance = CreateConVar("l4d2_ap_pickup_horde_chance", "0.35",
        "Chance (0-1) that a horde arms a carrier when none is live.", _, true, 0.0, true, 1.0);
    g_cvPkCooldownMin = CreateConVar("l4d2_ap_pickup_cooldown_min", "20.0",
        "Minimum seconds before re-arming after a lost carrier/drop.");
    g_cvPkCooldownMax = CreateConVar("l4d2_ap_pickup_cooldown_max", "60.0",
        "Maximum seconds before re-arming after a lost carrier/drop.");

    Pickups_ResetState();
}

void Pickups_OnMapStart()
{
    PrecacheModel(AP_ITEM_MODEL, true);
    PrecacheModel(AP_MARKER_MODEL, true);
    PrecacheSound(AP_PICKUP_SOUND, true);

    Pickups_ResetState();

    CreateTimer(1.0, Timer_PkDirector, _, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
    CreateTimer(0.2, Timer_PkDrop, _, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
    CreateTimer(0.1, Timer_PkMarkerAnim, _, TIMER_REPEAT | TIMER_FLAG_NO_MAPCHANGE);
}

void Pickups_OnNewSession()
{
    Pickups_Cleanup();
    Pickups_ResetState();
}

void Pickups_OnMapEnd()
{
    Pickups_Cleanup();
    Pickups_ResetState();
}

void Pickups_OnPluginEnd()
{
    Pickups_Cleanup();   // remove the glow, marker and drop entities
}

void Pickups_ResetState()
{
    g_pkState = PK_IDLE;
    g_pkCarrierRef = INVALID_ENT_REFERENCE;
    g_pkMarkerRef = INVALID_ENT_REFERENCE;
    g_pkDropRef = INVALID_ENT_REFERENCE;
    g_pkCarrierIndex = -1;
    g_pkLocation[0] = '\0';
    g_pkNextArm = GetGameTime() + AP_PK_INITIAL_ARM_DELAY;
    g_pkTarget = GetRandomFloat(AP_PK_INITIAL_FLOW_MIN, AP_PK_HIGH_FLOW);
}

bool Pickups_ResolveLocation()
{
    if (g_pkLocation[0] != '\0')
    {
        return true;
    }
    char map[AP_MAP_LEN];
    char chapter[AP_NAME_LEN];
    GetCurrentMap(map, sizeof(map));
    if (!g_mapToChapter.GetString(map, chapter, sizeof(chapter)))
    {
        return false;
    }
    Format(g_pkLocation, sizeof(g_pkLocation), "%s: AP Item Carrier", chapter);
    return true;
}

bool Pickups_IsChecked()
{
    return Route_IsLocationReported(g_pkLocation);
}

// Director decides when to arm
public Action Timer_PkDirector(Handle timer)
{
    if (!g_stateSynced || g_mapToChapter.Size == 0 || !Pickups_ResolveLocation())
    {
        return Plugin_Continue;
    }

    if (g_pkState != PK_DONE && Pickups_IsChecked())
    {
        Pickups_Cleanup();
        g_pkState = PK_DONE;
    }

    if (g_pkState == PK_IDLE)
    {
        if (GetGameTime() < g_pkNextArm || Players_GetFirstSurvivor() == -1)
        {
            return Plugin_Continue;
        }
        float pct = Pickups_GetFlowPct();
        if (pct >= g_pkTarget || pct >= AP_PK_BACKSTOP_FLOW)
        {
            Pickups_Arm();
        }
    }
    else if (g_pkState == PK_CARRIER)
    {
        if (Pickups_CarrierAlive())
        {
            GetEntPropVector(g_pkCarrierIndex, Prop_Data, "m_vecAbsOrigin", g_pkLastPos);
        }
        else
        {
            Pickups_CarrierGone(g_pkLastPos, false);
        }
    }
    return Plugin_Continue;
}

float Pickups_GetFlowPct()
{
    float maxFlow = L4D2Direct_GetMapMaxFlowDistance();
    if (maxFlow <= 0.0)
    {
        return 0.0;
    }
    float best = 0.0;
    for (int client = 1; client <= MaxClients; client++)
    {
        if (Players_IsSurvivor(client, true))
        {
            float flow = L4D2Direct_GetFlowDistance(client);
            if (flow > best)
            {
                best = flow;
            }
        }
    }
    float pct = best / maxFlow;
    return pct > 1.0 ? 1.0 : pct;
}

// Reroll after a loss
void Pickups_Cooldown()
{
    g_pkNextArm = GetGameTime() + GetRandomFloat(g_cvPkCooldownMin.FloatValue, g_cvPkCooldownMax.FloatValue);
    float pct = Pickups_GetFlowPct();
    g_pkTarget = (pct >= AP_PK_HIGH_FLOW) ? pct : pct + GetRandomFloat(0.0, AP_PK_HIGH_FLOW - pct);
}

bool Pickups_Arm()
{
    int carrier = Pickups_FindCarrier();
    if (carrier == -1)
    {
        carrier = Pickups_SpawnCarrier();
    }
    if (carrier == -1)
    {
        g_pkNextArm = GetGameTime() + AP_PK_ARM_RETRY_DELAY;
        return false;
    }
    Pickups_SetCarrier(carrier);
    return true;
}

static const char g_pkUncommonTokens[][] =
{
    "ceda",
    "clown",
    "mud",
    "riot",
    "roadcrew",
    "fallen",
    "jimmy"
};

bool Pickups_IsUncommon(int entity)
{
    char model[PLATFORM_MAX_PATH];
    GetEntPropString(entity, Prop_Data, "m_ModelName", model, sizeof(model));
    for (int i = 0; i < sizeof(g_pkUncommonTokens); i++)
    {
        if (StrContains(model, g_pkUncommonTokens[i], false) != -1)
        {
            return true;
        }
    }
    return false;
}

int Pickups_FindCarrier()
{
    ArrayList candidates = new ArrayList();
    int entity = -1;
    while ((entity = FindEntityByClassname(entity, "infected")) != -1)
    {
        if (GetEntProp(entity, Prop_Data, "m_iHealth") <= 0)
        {
            continue;
        }
        if (Pickups_IsUncommon(entity))
        {
            continue;
        }
        float origin[3];
        GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", origin);
        int survivor = Players_GetNearestSurvivor(origin);
        if (survivor == -1)
        {
            continue;
        }
        float survivorOrigin[3];
        GetClientAbsOrigin(survivor, survivorOrigin);
        float dist = GetVectorDistance(origin, survivorOrigin);
        if (dist >= AP_PK_CARRIER_MIN_DIST && dist <= AP_PK_CARRIER_MAX_DIST)
        {
            candidates.Push(EntIndexToEntRef(entity));
        }
    }

    int result = -1;
    if (candidates.Length > 0)
    {
        result = EntRefToEntIndex(candidates.Get(GetRandomInt(0, candidates.Length - 1)));
        if (result == INVALID_ENT_REFERENCE)
        {
            result = -1;
        }
    }
    delete candidates;
    return result;
}

// Fallback: ask left4dhooks for a valid spawn position and spawn a common zombie there.
int Pickups_SpawnCarrier()
{
    int survivor = Players_GetFirstSurvivor();
    if (survivor == -1)
    {
        return -1;
    }
    float pos[3];
    float ang[3];
    if (!L4D_GetRandomPZSpawnPosition(survivor, 1, 10, pos))
    {
        return -1;
    }
    ang[1] = GetRandomFloat(0.0, 360.0);
    int entity = L4D_SpawnCommonInfected(pos, ang);
    return (entity > MaxClients && IsValidEntity(entity)) ? entity : -1;
}

void Pickups_SetCarrier(int entity)
{
    g_pkCarrierRef = EntIndexToEntRef(entity);
    g_pkCarrierIndex = entity;
    GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", g_pkLastPos);
    Pickups_SetGlow(entity, true);

    float maxs[3];
    GetEntPropVector(entity, Prop_Send, "m_vecMaxs", maxs);
    float head[3];
    head[2] = maxs[2] + 12.0;
    g_pkMarkerRef = Pickups_AttachProp(entity, AP_MARKER_MODEL, head, g_pkMarkerAngles);
    g_pkMarkerBirth = GetGameTime();
    g_pkMarkerBaseZ = head[2];

    g_pkState = PK_CARRIER;
    LogMessage("[AP] pickup carrier armed for '%s' (entity %d).", g_pkLocation, entity);
}

bool Pickups_CarrierAlive()
{
    int carrier = EntRefToEntIndex(g_pkCarrierRef);
    return carrier != INVALID_ENT_REFERENCE
        && IsValidEntity(carrier)
        && GetEntProp(carrier, Prop_Data, "m_iHealth") > 0;
}

// Carrier vanished without dying
void Pickups_LoseCarrier()
{
    Pickups_Cleanup();
    g_pkState = PK_IDLE;
    Pickups_Cooldown();
    LogMessage("[AP] pickup carrier lost without dropping; re-arming later.");
}

// Drop is OOB, lost, stuck, etc.
bool Pickups_ReturnToCarrier(const char[] reason)
{
    Pickups_Cleanup();
    g_pkState = PK_IDLE;
    if (Pickups_Arm())
    {
        LogMessage("[AP] %s; item returned to a new carrier.", reason);
        return true;
    }
    LogMessage("[AP] %s; no carrier could be armed yet, retrying.", reason);
    return false;
}

void Pickups_OnEntityDestroyed(int entity)
{
    if (g_pkState != PK_CARRIER || entity <= MaxClients || entity != g_pkCarrierIndex)
    {
        return;
    }
    float pos[3];
    GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", pos);
    bool dead = GetEntProp(entity, Prop_Data, "m_iHealth") <= 0;
    Pickups_CarrierGone(pos, dead);
}

void Pickups_CarrierGone(const float lastPos[3], bool knownDeath)
{
    if (g_pkState != PK_CARRIER)
    {
        return;
    }

    bool died = knownDeath;
    if (!died)
    {
        int survivor = Players_GetNearestSurvivor(lastPos);
        if (survivor != -1)
        {
            float survivorOrigin[3];
            GetClientAbsOrigin(survivor, survivorOrigin);
            died = GetVectorDistance(lastPos, survivorOrigin) < AP_PK_CULL_DIST;
        }
    }
    LogMessage("[AP] pickup carrier gone (known death: %d, treated as death: %d).", knownDeath, died);

    if (!died)
    {
        Pickups_LoseCarrier();
        return;
    }

    g_pkPendingPos = lastPos;
    g_pkState = PK_PENDING_DROP;
    CreateTimer(0.1, Timer_PkSpawnDrop, _, TIMER_FLAG_NO_MAPCHANGE);
}

public Action Timer_PkSpawnDrop(Handle timer)
{
    if (g_pkState != PK_PENDING_DROP)
    {
        return Plugin_Stop;
    }
    float pos[3];
    pos = g_pkPendingPos;
    Pickups_Cleanup();
    Pickups_SpawnDrop(pos);
    return Plugin_Stop;
}

void Pickups_SpawnDrop(float pos[3])
{
    pos[2] += 16.0;
    int prop = CreateEntityByName("prop_physics_override");
    if (prop == -1)
    {
        g_pkState = PK_IDLE;
        Pickups_Cooldown();
        return;
    }
    DispatchKeyValue(prop, "model", AP_ITEM_MODEL);
    DispatchKeyValue(prop, "spawnflags", "256");
    DispatchSpawn(prop);
    TeleportEntity(prop, pos, g_pkDropAngles, NULL_VECTOR);
    Pickups_SetGlow(prop, true);
    SDKHook(prop, SDKHook_Use, Pk_OnDropUsed);

    // Keep a marker hovering over the drop so it stays visible
    float above[3] = { 0.0, 0.0, 30.0 };
    float flat[3];   // angles are driven by Timer_PkMarkerAnim from here on
    g_pkMarkerRef = Pickups_AttachProp(prop, AP_MARKER_MODEL, above, flat);
    g_pkMarkerBirth = GetGameTime();
    g_pkMarkerBaseZ = above[2];

    g_pkDropRef = EntIndexToEntRef(prop);
    g_pkDropTime = GetGameTime();
    g_pkState = PK_DROPPED;
}

public Action Pk_OnDropUsed(int entity, int activator, int caller, UseType type, float value)
{
    if (g_pkState != PK_DROPPED || EntRefToEntIndex(g_pkDropRef) != entity)
    {
        return Plugin_Continue;
    }
    int client = (activator >= 1 && activator <= MaxClients) ? activator : caller;

    if (!Players_IsSurvivor(client, true))
    {
        return Plugin_Continue;
    }
    Pickups_Collect();
    return Plugin_Handled;
}

public Action Timer_PkMarkerAnim(Handle timer)
{
    int marker = EntRefToEntIndex(g_pkMarkerRef);
    if (marker == INVALID_ENT_REFERENCE || !IsValidEntity(marker))
    {
        return Plugin_Continue;
    }

    float t = GetGameTime() - g_pkMarkerBirth;
    float offset[3];
    offset[2] = g_pkMarkerBaseZ + Sine(t * AP_PK_MARKER_BOB_SPEED) * AP_PK_MARKER_BOB_AMP;

    float angles[3];
    angles[0] = g_pkMarkerAngles[0];
    angles[1] = g_pkMarkerAngles[1] + t * AP_PK_MARKER_SPIN;
    angles[2] = g_pkMarkerAngles[2];

    TeleportEntity(marker, offset, angles, NULL_VECTOR);
    return Plugin_Continue;
}

public Action Timer_PkDrop(Handle timer)
{
    if (g_pkState != PK_DROPPED)
    {
        return Plugin_Continue;
    }

    int drop = EntRefToEntIndex(g_pkDropRef);
    if (drop == INVALID_ENT_REFERENCE || !IsValidEntity(drop))
    {
        if ((GetGameTime() - g_pkDropTime) < AP_PK_DROP_SPAWN_GRACE)
        {
            Pickups_Cleanup();
            g_pkState = PK_IDLE;
            Pickups_Cooldown();
            return Plugin_Continue;
        }
        Pickups_ReturnToCarrier("pickup drop was culled");
        return Plugin_Continue;
    }

    float origin[3];
    GetEntPropVector(drop, Prop_Data, "m_vecAbsOrigin", origin);

    int nearest = Players_GetNearestSurvivor(origin);
    if (nearest == -1)
    {
        return Plugin_Continue;
    }

    float survivorOrigin[3];
    GetClientAbsOrigin(nearest, survivorOrigin);
    float dist = GetVectorDistance(origin, survivorOrigin);

    // Failsafes so the check can't be stuck
    bool outside = TR_PointOutsideWorld(origin);
    bool stranded = !outside && dist > AP_PK_STRAND_DIST
        && (GetGameTime() - g_pkDropTime) > AP_PK_STRAND_TIME;
    if (outside || stranded)
    {
        Pickups_ReturnToCarrier(outside ? "pickup drop was outside the world" : "pickup drop was stranded");
    }
    return Plugin_Continue;
}

void Pickups_Collect()
{
    EmitSoundToAll(AP_PICKUP_SOUND);
    Pickups_Cleanup();
    g_pkState = PK_DONE;
    Route_ReportInteractableCompletion(g_pkLocation);
}

public Action L4D_OnMobRushStart()
{
    Pickups_OnHorde();
    return Plugin_Continue;
}

public Action L4D_OnSpawnMob(int &amount)
{
    Pickups_OnHorde();
    return Plugin_Continue;
}

void Pickups_OnHorde()
{
    if (g_pkState != PK_IDLE || GetGameTime() < g_pkNextArm)
    {
        return;
    }
    if (GetRandomFloat() > g_cvPkHordeChance.FloatValue)
    {
        return;
    }
    CreateTimer(AP_PK_HORDE_ARM_DELAY, Timer_PkHordeArm, _, TIMER_FLAG_NO_MAPCHANGE);   // let the horde finish spawning
}

public Action Timer_PkHordeArm(Handle timer)
{
    if (g_pkState == PK_IDLE && g_stateSynced && Pickups_ResolveLocation() && !Pickups_IsChecked())
    {
        Pickups_Arm();
    }
    return Plugin_Stop;
}

// Flow stops during a finale, so arm on a random delay instead
void Pickups_OnFinaleStart()
{
    if (g_pkState == PK_IDLE)
    {
        g_pkTarget = 0.0;
        g_pkNextArm = GetGameTime() + GetRandomFloat(AP_PK_FINALE_ARM_MIN, AP_PK_FINALE_ARM_MAX);
    }
}

// cheater command/fallback
public Action Command_PkBring(int client, int args)
{
    if (!Players_IsSurvivor(client, true))
    {
        return Plugin_Handled;
    }
    if (!g_stateSynced || !Pickups_ResolveLocation())
    {
        ReplyToCommand(client, "%sNot on an Archipelago chapter.", AP_CHAT_PREFIX);
        return Plugin_Handled;
    }

    switch (g_pkState)
    {
        case PK_DONE:
        {
            ReplyToCommand(client, "%sThis chapter's item is already collected.", AP_CHAT_PREFIX);
        }
        case PK_DROPPED:
        {
            if (!Pickups_ReturnToCarrier("pickup summoned by command"))
            {
                ReplyToCommand(client, "%sCouldn't find a spawn spot, try again in a moment.", AP_CHAT_PREFIX);
            }
        }
        case PK_CARRIER:
        {
            int carrier = EntRefToEntIndex(g_pkCarrierRef);
            if (carrier != INVALID_ENT_REFERENCE && IsValidEntity(carrier))
            {
                Pickups_MoveNear(carrier, client);
            }
        }
        case PK_IDLE:
        {
            if (Pickups_IsChecked())
            {
                g_pkState = PK_DONE;
            }
            else if (!Pickups_Arm())
            {
                ReplyToCommand(client, "%sCouldn't find a spawn spot, try again in a moment.", AP_CHAT_PREFIX);
            }
        }
    }
    return Plugin_Handled;
}

void Pickups_MoveNear(int entity, int client)
{
    float origin[3];
    float angles[3];
    Players_GetPointInFront(client, AP_PK_SUMMON_DISTANCE, AP_PK_SUMMON_HEIGHT, origin, angles);
    float zero[3];
    TeleportEntity(entity, origin, NULL_VECTOR, zero);
}

void Pickups_SetGlow(int entity, bool on)
{
    if (!HasEntProp(entity, Prop_Send, "m_iGlowType"))
    {
        return;
    }
    SetEntProp(entity, Prop_Send, "m_iGlowType", on ? 3 : 0);
    SetEntProp(entity, Prop_Send, "m_glowColorOverride", AP_PK_GLOW_COLOR);
    SetEntProp(entity, Prop_Send, "m_nGlowRange", 0);   // 0 = unlimited range
}

int Pickups_AttachProp(int parent, const char[] model, const float offset[3], const float angles[3])
{
    int prop = CreateEntityByName("prop_dynamic_override");
    if (prop == -1)
    {
        return INVALID_ENT_REFERENCE;
    }
    DispatchKeyValue(prop, "model", model);
    DispatchKeyValue(prop, "solid", "0");
    DispatchSpawn(prop);

    SetVariantString("!activator");
    AcceptEntityInput(prop, "SetParent", parent);
    TeleportEntity(prop, offset, angles, NULL_VECTOR);
    return EntIndexToEntRef(prop);
}

void Pickups_KillRef(int &ref)
{
    int entity = EntRefToEntIndex(ref);
    if (entity != INVALID_ENT_REFERENCE && IsValidEntity(entity))
    {
        RemoveEntity(entity);
    }
    ref = INVALID_ENT_REFERENCE;
}

void Pickups_Cleanup()
{
    int carrier = EntRefToEntIndex(g_pkCarrierRef);
    if (carrier != INVALID_ENT_REFERENCE && IsValidEntity(carrier))
    {
        Pickups_SetGlow(carrier, false);
    }
    g_pkCarrierRef = INVALID_ENT_REFERENCE;
    g_pkCarrierIndex = -1;
    Pickups_KillRef(g_pkMarkerRef);
    Pickups_KillRef(g_pkDropRef);
}
