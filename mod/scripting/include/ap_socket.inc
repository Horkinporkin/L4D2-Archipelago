#if defined _ap_socket_included
    #endinput
#endif
#define _ap_socket_included

#include <sourcemod>

enum SocketType
{
    SOCKET_TCP = 1
}

enum SocketOption
{
    ConcatenateCallbacks = 1,
    SocketReuseAddr = 5
}

typeset SocketIncomingCB
{
    function void (Handle socket, Handle newSocket, const char[] remoteIP, int remotePort, any arg);
}

typeset SocketReceiveCB
{
    function void (Handle socket, const char[] receiveData, const int dataSize, any arg);
}

typeset SocketDisconnectCB
{
    function void (Handle socket, any arg);
}

typeset SocketErrorCB
{
    function void (Handle socket, const int errorType, const int errorNum, any arg);
}

native Handle SocketCreate(SocketType protocol = SOCKET_TCP, SocketErrorCB efunc);
native bool SocketBind(Handle socket, const char[] hostname, int port);
native bool SocketListen(Handle socket, SocketIncomingCB ifunc);
native void SocketSend(Handle socket, const char[] data, int size = -1);
native int SocketSetOption(Handle socket, SocketOption option, int value);
native void SocketSetReceiveCallback(Handle socket, SocketReceiveCB rfunc);
native void SocketSetDisconnectCallback(Handle socket, SocketDisconnectCB dfunc);
native void SocketSetErrorCallback(Handle socket, SocketErrorCB efunc);

public Extension __ext_smsock =
{
    name = "Socket",
    file = "socket.ext",
#if defined AUTOLOAD_EXTENSIONS
    autoload = 1,
#else
    autoload = 0,
#endif
#if defined REQUIRE_EXTENSIONS
    required = 1,
#else
    required = 0,
#endif
};
