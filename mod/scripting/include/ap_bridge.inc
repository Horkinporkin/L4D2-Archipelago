#if defined _ap_bridge_included
    #endinput
#endif
#define _ap_bridge_included

// Bridge transport for the Archipelago client and JSON
// Complete lines are sent to ap_protocol.inc. Also houses the small JSON reader/writer

#define AP_PROTOCOL_VERSION  1
#define AP_SEND_BUFFER_SIZE  1024

ConVar g_cvHost;
ConVar g_cvPort;
Handle g_listener;
Handle g_client;
char g_receiveBuffer[AP_RECV_BUFFER_SIZE];
int g_receiveLength;                   // bytes buffered in g_receiveBuffer
char g_lastReportedState[640];         // last GAME_STATE payload sent on this connection

void Bridge_Init()
{
    g_cvHost = CreateConVar("l4d2_ap_host", "127.0.0.1", "Archipelago client host.");
    g_cvPort = CreateConVar("l4d2_ap_port", "12138", "Archipelago client TCP port.", FCVAR_PROTECTED);
}

public Action Command_BridgeStatus(int client, int args)
{
    ReplyToCommand(client,
        "[AP] bridge %s | %d route campaigns | synced %s | unlocked %d | completed %d",
        g_client == null ? "disconnected" : "connected",
        g_routeCampaigns.Length,
        g_stateSynced ? "yes" : "no",
        g_unlockedChapters.Size,
        g_completedChapters.Size);
    return Plugin_Handled;
}

void Bridge_Start()
{
    delete g_listener;

    char host[64];
    g_cvHost.GetString(host, sizeof(host));
    int port = g_cvPort.IntValue;

    g_listener = SocketCreate(SOCKET_TCP, OnSocketError);
    if (g_listener == null)
    {
        LogError("[AP] could not create bridge listener.");
        return;
    }

    SocketSetOption(g_listener, SocketReuseAddr, 1);
    if (!SocketBind(g_listener, host, port) || !SocketListen(g_listener, OnSocketIncoming))
    {
        LogError("[AP] unable to bind bridge to %s:%d.", host, port);
        delete g_listener;
    }
}

void Bridge_Close()
{
    Bridge_CloseSocket(g_client);
    Bridge_CloseSocket(g_listener);
}

// Forgets the socket before closing it. Callbacks fired while it closes cannot reach it again
void Bridge_CloseSocket(Handle socket)
{
    if (socket == null)
    {
        return;
    }
    Bridge_ForgetHandle(socket);
    delete socket;
}

void Bridge_ForgetHandle(Handle socket)
{
    if (socket == g_client)
    {
        g_client = null;
        g_receiveBuffer[0] = '\0';
        g_receiveLength = 0;
        Survivors_OnBridgeDisconnected();
    }
    else if (socket == g_listener)
    {
        g_listener = null;
    }
}

public void OnSocketIncoming(Handle listener, Handle socket, const char[] remoteIP, int remotePort, any data)
{
    // Only one client at a time, a new connection replaces the old one
    Bridge_CloseSocket(g_client);

    g_client = socket;
    // Leave one byte for NUL
    SocketSetOption(g_client, ConcatenateCallbacks, AP_RECV_BUFFER_SIZE - 1);
    SocketSetReceiveCallback(g_client, OnSocketReceive);
    SocketSetDisconnectCallback(g_client, OnSocketDisconnect);
    SocketSetErrorCallback(g_client, OnSocketError);
    g_lastReportedState[0] = '\0';

    Bridge_Send("READY", "{}");
    Route_ReportGameState();
}

public void OnSocketReceive(Handle socket, const char[] data, const int size, any arg)
{
    if (socket != g_client || size <= 0)
    {
        return;
    }
    if (size >= sizeof(g_receiveBuffer) - g_receiveLength)
    {
        LogError("[AP] bridge input exceeded the receive buffer. Disconnecting.");
        Bridge_CloseSocket(socket);
        return;
    }

    for (int i = 0; i < size; i++)
    {
        g_receiveBuffer[g_receiveLength++] = data[i];
    }
    g_receiveBuffer[g_receiveLength] = '\0';
    Bridge_ProcessInput();
}

void Bridge_ProcessInput()
{
    int start = 0;
    int newline;
    while ((newline = FindCharInString(g_receiveBuffer[start], '\n')) != -1)
    {
        int end = start + newline;
        g_receiveBuffer[end] = '\0';
        if (end > start && g_receiveBuffer[end - 1] == '\r')
        {
            g_receiveBuffer[end - 1] = '\0';
        }

        Protocol_HandleMessage(g_receiveBuffer[start]);
        if (g_client == null)
        {
            return;
        }
        start = end + 1;
    }

    if (start == 0)
    {
        return;
    }
    int remaining = g_receiveLength - start;
    for (int i = 0; i < remaining; i++)
    {
        g_receiveBuffer[i] = g_receiveBuffer[start + i];
    }
    g_receiveBuffer[remaining] = '\0';
    g_receiveLength = remaining;
}

public void OnSocketError(Handle socket, const int errorType, const int errorNum, any data)
{
    LogError("[AP] socket error type=%d errno=%d", errorType, errorNum);
    Bridge_CloseSocket(socket);
}

public void OnSocketDisconnect(Handle socket, any data)
{
    LogMessage("[AP] bridge client disconnected.");
    Bridge_CloseSocket(socket);
}

// payload must already be JSON
void Bridge_Send(const char[] messageType, const char[] payload)
{
    if (g_client == null)
    {
        return;
    }

    static char message[AP_SEND_BUFFER_SIZE];
    int length = FormatEx(message, sizeof(message), "{\"version\":%d,\"type\":\"%s\",\"payload\":%s}\n",
        AP_PROTOCOL_VERSION, messageType, payload);
    if (length == 0 || message[length - 1] != '\n')
    {
        LogError("[AP] dropping %s message: payload does not fit the send buffer.", messageType);
        return;
    }
    SocketSend(g_client, message, length);
}

// ---------------------------------------------------------------------------
// JSON reader
// ---------------------------------------------------------------------------

int JSON_SkipWhitespace(const char[] json, int position)
{
    while (json[position] == ' ' || json[position] == '\t'
        || json[position] == '\r' || json[position] == '\n')
    {
        position++;
    }
    return position;
}

int JSON_StringEnd(const char[] json, int start)
{
    if (json[start] != '"')
    {
        return -1;
    }
    for (int position = start + 1; json[position] != '\0'; position++)
    {
        if (json[position] == '\\')
        {
            if (json[position + 1] == '\0')
            {
                return -1;
            }
            position++;
        }
        else if (json[position] == '"')
        {
            return position + 1;
        }
    }
    return -1;
}

int JSON_HexValue(char value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

int JSON_ParseHex4(const char[] json, int start)
{
    int value = 0;
    for (int i = 0; i < 4; i++)
    {
        int digit = JSON_HexValue(json[start + i]);
        if (digit < 0)
        {
            return -1;
        }
        value = (value << 4) | digit;
    }
    return value;
}

bool JSON_AppendCodePoint(int codePoint, char[] output, int &writePosition, int maxlen)
{
    if (codePoint == 0 || (codePoint >= 0xD800 && codePoint <= 0xDFFF))
    {
        codePoint = '?';
    }

    if (codePoint <= 0x7F)
    {
        if (writePosition + 1 >= maxlen) return false;
        output[writePosition++] = codePoint;
    }
    else if (codePoint <= 0x7FF)
    {
        if (writePosition + 2 >= maxlen) return false;
        output[writePosition++] = 0xC0 | (codePoint >> 6);
        output[writePosition++] = 0x80 | (codePoint & 0x3F);
    }
    else if (codePoint <= 0xFFFF)
    {
        if (writePosition + 3 >= maxlen) return false;
        output[writePosition++] = 0xE0 | (codePoint >> 12);
        output[writePosition++] = 0x80 | ((codePoint >> 6) & 0x3F);
        output[writePosition++] = 0x80 | (codePoint & 0x3F);
    }
    else
    {
        if (writePosition + 4 >= maxlen) return false;
        output[writePosition++] = 0xF0 | (codePoint >> 18);
        output[writePosition++] = 0x80 | ((codePoint >> 12) & 0x3F);
        output[writePosition++] = 0x80 | ((codePoint >> 6) & 0x3F);
        output[writePosition++] = 0x80 | (codePoint & 0x3F);
    }
    return true;
}

bool JSON_DecodeString(const char[] json, int start, int end, char[] output, int maxlen)
{
    if (start < 0 || end <= start + 1 || json[start] != '"' || json[end - 1] != '"' || maxlen <= 0)
    {
        return false;
    }

    int writePosition = 0;
    for (int position = start + 1; position < end - 1; position++)
    {
        if (json[position] != '\\')
        {
            if (writePosition + 1 >= maxlen)
            {
                return false;
            }
            output[writePosition++] = json[position];
            continue;
        }

        position++;
        char escaped = json[position];
        if (escaped == 'u')
        {
            int codePoint = JSON_ParseHex4(json, position + 1);
            if (codePoint < 0)
            {
                return false;
            }
            position += 4;
            if (codePoint >= 0xD800 && codePoint <= 0xDBFF
                && json[position + 1] == '\\' && json[position + 2] == 'u')
            {
                int low = JSON_ParseHex4(json, position + 3);
                if (low >= 0xDC00 && low <= 0xDFFF)
                {
                    codePoint = 0x10000 + ((codePoint - 0xD800) << 10) + (low - 0xDC00);
                    position += 6;
                }
            }
            if (!JSON_AppendCodePoint(codePoint, output, writePosition, maxlen))
            {
                return false;
            }
            continue;
        }

        char decoded;
        switch (escaped)
        {
            case '"', '\\', '/': decoded = escaped;
            case 'b': decoded = '\b';
            case 'f': decoded = '\f';
            case 'n': decoded = '\n';
            case 'r': decoded = '\r';
            case 't': decoded = '\t';
            default: return false;
        }
        if (writePosition + 1 >= maxlen)
        {
            return false;
        }
        output[writePosition++] = decoded;
    }
    output[writePosition] = '\0';
    return true;
}

int JSON_ValueEnd(const char[] json, int start)
{
    if (json[start] == '"')
    {
        return JSON_StringEnd(json, start);
    }

    if (json[start] != '{' && json[start] != '[')
    {
        int position = start;
        while (json[position] != '\0' && json[position] != ','
            && json[position] != '}' && json[position] != ']'
            && json[position] != ' ' && json[position] != '\t'
            && json[position] != '\r' && json[position] != '\n')
        {
            position++;
        }
        return position > start ? position : -1;
    }

    int depth = 0;
    bool inString = false;
    for (int position = start; json[position] != '\0'; position++)
    {
        char value = json[position];
        if (inString)
        {
            if (value == '\\')
            {
                if (json[position + 1] == '\0') return -1;
                position++;
            }
            else if (value == '"')
            {
                inString = false;
            }
            continue;
        }
        if (value == '"')
        {
            inString = true;
        }
        else if (value == '{' || value == '[')
        {
            depth++;
        }
        else if (value == '}' || value == ']')
        {
            depth--;
            if (depth == 0)
            {
                return position + 1;
            }
            if (depth < 0)
            {
                return -1;
            }
        }
    }
    return -1;
}

void JSON_CopyRange(const char[] source, int start, int end, char[] output)
{
    int length = end - start;
    for (int i = 0; i < length; i++)
    {
        output[i] = source[start + i];
    }
    output[length] = '\0';
}

bool JSON_GetField(const char[] json, const char[] key, char[] outValue, int maxlen)
{
    if (maxlen <= 0)
    {
        return false;
    }
    outValue[0] = '\0';

    int position = JSON_SkipWhitespace(json, 0);
    if (json[position] != '{')
    {
        return false;
    }
    position++;

    while (json[position] != '\0')
    {
        position = JSON_SkipWhitespace(json, position);
        if (json[position] == ',')
        {
            position = JSON_SkipWhitespace(json, position + 1);
        }
        if (json[position] == '}' || json[position] == '\0')
        {
            return false;
        }

        int keyEnd = JSON_StringEnd(json, position);
        if (keyEnd < 0)
        {
            return false;
        }
        char foundKey[128];
        if (!JSON_DecodeString(json, position, keyEnd, foundKey, sizeof(foundKey)))
        {
            return false;
        }
        position = JSON_SkipWhitespace(json, keyEnd);
        if (json[position] != ':')
        {
            return false;
        }
        int valueStart = JSON_SkipWhitespace(json, position + 1);
        int valueEnd = JSON_ValueEnd(json, valueStart);
        if (valueEnd < 0)
        {
            return false;
        }

        if (StrEqual(foundKey, key, false))
        {
            if (json[valueStart] == '"')
            {
                return JSON_DecodeString(json, valueStart, valueEnd, outValue, maxlen);
            }
            int length = valueEnd - valueStart;
            if (length >= maxlen)
            {
                return false;
            }
            JSON_CopyRange(json, valueStart, valueEnd, outValue);
            return true;
        }

        position = JSON_SkipWhitespace(json, valueEnd);
        if (json[position] == ',')
        {
            position++;
        }
        else
        {
            return false;
        }
    }
    return false;
}

void JSON_SplitArrayObjects(const char[] arrayText, ArrayList results)
{
    int position = JSON_SkipWhitespace(arrayText, 0);
    if (arrayText[position] != '[')
    {
        return;
    }
    position++;

    while (arrayText[position] != '\0')
    {
        position = JSON_SkipWhitespace(arrayText, position);
        if (arrayText[position] == ',')
        {
            position = JSON_SkipWhitespace(arrayText, position + 1);
        }
        if (arrayText[position] == ']')
        {
            return;
        }
        if (arrayText[position] != '{')
        {
            LogError("[AP] expected an object in a bridge array.");
            return;
        }

        int end = JSON_ValueEnd(arrayText, position);
        if (end < 0)
        {
            LogError("[AP] malformed object in a bridge array.");
            return;
        }
        int length = end - position;
        char objectText[2048];
        if (length >= sizeof(objectText))
        {
            LogError("[AP] bridge array object exceeds the supported size.");
            return;
        }
        JSON_CopyRange(arrayText, position, end, objectText);
        results.PushString(objectText);
        position = end;
    }
}

void JSON_SplitArrayStrings(const char[] arrayText, ArrayList results)
{
    int position = JSON_SkipWhitespace(arrayText, 0);
    if (arrayText[position] != '[')
    {
        return;
    }
    position++;

    while (arrayText[position] != '\0')
    {
        position = JSON_SkipWhitespace(arrayText, position);
        if (arrayText[position] == ']')
        {
            return;
        }
        if (arrayText[position] != '"')
        {
            LogError("[AP] expected a string in a bridge array.");
            return;
        }

        int end = JSON_StringEnd(arrayText, position);
        if (end < 0)
        {
            LogError("[AP] bad string in a bridge array.");
            return;
        }

        char value[AP_NAME_LEN];
        if (!JSON_DecodeString(arrayText, position, end, value, sizeof(value)))
        {
            LogError("[AP] bridge array string exceeds the supported size.");
            return;
        }
        results.PushString(value);

        position = JSON_SkipWhitespace(arrayText, end);
        if (arrayText[position] == ',')
        {
            position++;
        }
        else if (arrayText[position] != ']')
        {
            LogError("[AP] bad bridge string array.");
            return;
        }
    }
}

// ---------------------------------------------------------------------------
// JSON writer
// ---------------------------------------------------------------------------

// Writes text as the contents of a JSON string
bool JSON_Escape(const char[] text, char[] output, int maxlen)
{
    static const char hexDigits[] = "0123456789abcdef";

    int written = 0;
    for (int i = 0; text[i] != '\0'; i++)
    {
        int character = text[i] & 0xFF;
        bool quoted = character == '"' || character == '\\';
        bool control = character < 0x20;
        int needed = quoted ? 2 : (control ? 6 : 1);
        if (written + needed >= maxlen)
        {
            output[written] = '\0';
            return false;
        }

        if (quoted)
        {
            output[written++] = '\\';
            output[written++] = text[i];
        }
        else if (control)
        {
            output[written++] = '\\';
            output[written++] = 'u';
            output[written++] = '0';
            output[written++] = '0';
            output[written++] = hexDigits[character >> 4];
            output[written++] = hexDigits[character & 0xF];
        }
        else
        {
            output[written++] = text[i];
        }
    }
    output[written] = '\0';
    return true;
}

// ---------------------------------------------------------------------------
// Container helpers
// ---------------------------------------------------------------------------

void StringMap_ClearArrayValues(StringMap map)
{
    StringMapSnapshot snapshot = map.Snapshot();
    for (int i = 0; i < snapshot.Length; i++)
    {
        char key[256];
        snapshot.GetKey(i, key, sizeof(key));
        ArrayList list;
        if (map.GetValue(key, list))
        {
            delete list;
        }
    }
    delete snapshot;
    map.Clear();
}

void StringMap_SetArrayValue(StringMap map, const char[] key, ArrayList list)
{
    ArrayList previous;
    if (map.GetValue(key, previous) && previous != list)
    {
        delete previous;
    }
    map.SetValue(key, list);
}

void ArrayList_MoveToFront(ArrayList list, const char[] value)
{
    int index = list.FindString(value);
    if (index != -1)
    {
        list.Erase(index);
    }
    if (list.Length == 0)
    {
        list.PushString(value);
        return;
    }
    list.ShiftUp(0);
    list.SetString(0, value);
}
