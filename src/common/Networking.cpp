/////////////////////////////////////////
//
//             OpenLieroX
//
// code under LGPL, based on JasonBs work,
// enhanced by Dark Charlie and Albert Zeyer
//
//
/////////////////////////////////////////


// Common networking routines to help us
// Created 18/12/02
// Jason Boettcher

#include "LieroX.h"
#include "Error.h"
#include "Networking.h"
#include "Utils.h"
#include "StringUtils.h"
#include <SDL_syswm.h>
#include <SDL_thread.h>


#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <map>

#include <nl.h>
// workaraound for bad named makros by nl.h
// macros are bad, esp the names (reserved/used by CBytestream)
// TODO: they seem to not work correctly!
// all use of it in CBytestream was removed
inline void nl_writeShort(char* x, int& y, NLushort z)		{ writeShort(x, y, z); }
inline void nl_writeLong(char* x, int& y, NLulong z)		{ writeLong(x, y, z); }
inline void nl_writeFloat(char* x, int& y, NLfloat z)		{ writeFloat(x, y, z); }
inline void nl_writeDouble(char* x, int& y, NLdouble z)		{ writeDouble(x, y, z); }
inline void nl_readShort(char* x, int& y, NLushort z)		{ readShort(x, y, z); }
inline void nl_readLong(char* x, int& y, NLulong z)			{ readLong(x, y, z); }
inline void nl_readFloat(char* x, int& y, NLfloat z)		{ readFloat(x, y, z); }
inline void nl_readDouble(char* x, int& y, NLdouble z)		{ readDouble(x, y, z); }
#undef writeByte
#undef writeShort
#undef writeFloat
#undef writeString
#undef readByte
#undef readShort
#undef readFloat
#undef readString


class NetworkAddrIntern {
public:
	NLaddress addr;
	
	NetworkAddrIntern() {
		memset(&addr, 0, sizeof(addr));
	}
};

class NetworkSocketIntern {
private:	
	NLsocket* sock;
	int* refCount;
	
	void init() {
		if(!sock) {
			sock = new NLsocket;
			memset(sock, 0, sizeof(NLsocket));
			refCount = new int;
			*refCount = 1;
		}
	}

	void uninitSocket() {
		//nlClose(*sock);
		delete sock; delete refCount;
		sock = NULL; refCount = NULL;		
	}

	void reset() {
		if(sock) {
			(*refCount)--;
			if(*refCount == 0) uninitSocket();
		}
	}
	
public:
	NetworkSocketIntern() : sock(NULL) { init(); }
	~NetworkSocketIntern() { reset(); }
	
	NetworkSocketIntern(const NetworkSocketIntern& nsock) { operator=(nsock); }
	NetworkSocketIntern& operator=(const NetworkSocketIntern& nsock) {
		reset();
		sock = nsock.sock; refCount = nsock.refCount;
		(*refCount)++;
		return *this;
	}
	
	NLsocket* getSocket() { init(); return sock; }
};


DECLARE_INTERNDATA_CLASS(NetworkAddr, NetworkAddrIntern);
DECLARE_INTERNDATA_CLASS(NetworkSocket, NetworkSocketIntern);


static NLsocket* getNLsocket(NetworkSocket* socket) {
	return NetworkSocketData(socket)->getSocket();
}

static NLaddress* getNLaddr(NetworkAddr* addr) {
	return &NetworkAddrData(addr)->addr;
}

static const NLaddress* getNLaddr(const NetworkAddr* addr) {
	return &NetworkAddrData(addr)->addr;
}


// --------------------------------------------------------------------------
// ------------- Net events

enum	{ SDL_USEREVENT_NET_ACTIVITY = SDL_USEREVENT + 1 };

bool SdlNetEvent_Inited = false;
bool SdlNetEventThreadExit = false;
SDL_Thread * SdlNetEventThread = NULL;
NLint SdlNetEventGroup = 0;

static int SdlNetEventThreadMain( void * param )
{
	NLsocket sock_out;
	SDL_Event ev;
	ev.type = SDL_USEREVENT_NET_ACTIVITY;
	ev.user.code = 0;
	ev.user.data1 = NULL;
	ev.user.data2 = NULL;
	while( ! SdlNetEventThreadExit )
	{
		if( nlPollGroup( SdlNetEventGroup, NL_READ_STATUS, &sock_out, 1, 1000 ) > 0 )	// Wait 1 second
		{
			//printf("SdlNetEventThreadMain(): SDL_PushEvent()\n");
			SDL_PushEvent( &ev );
		}
	};
	return 0;
};

static bool SdlNetEvent_Init()
{
	if( SdlNetEvent_Inited )
		return false;
	SdlNetEvent_Inited = true;
	
	SdlNetEventGroup = nlGroupCreate();
	SdlNetEventThread = SDL_CreateThread( &SdlNetEventThreadMain, NULL );
	
	return true;
};

static void SdlNetEvent_UnInit() {
	if( ! SdlNetEvent_Inited )
		return;
		
	SdlNetEventThreadExit = true;
	int status = 0;
	SDL_WaitThread( SdlNetEventThread, &status );
	nlGroupDestroy(SdlNetEventGroup);
};

static bool isSocketInGroup(NLint group, NetworkSocket sock) {
	NLsocket sockets[NL_MAX_GROUP_SOCKETS];
	NLint len = NL_MAX_GROUP_SOCKETS;
	nlGroupGetSockets( group, sockets, &len );
	for( int f = 0; f < len; f++ )
		if( sockets[f] == *getNLsocket(&sock) )
			return false;
	
	return true;
}

static void AddSocketToNotifierGroup( NetworkSocket sock )
{
	SdlNetEvent_Init();
	
	if( !isSocketInGroup(SdlNetEventGroup, sock) )
		nlGroupAddSocket( SdlNetEventGroup, *getNLsocket(&sock) );
};

static void RemoveSocketFromNotifierGroup( NetworkSocket sock )
{
	SdlNetEvent_Init();
	
	nlGroupDeleteSocket( SdlNetEventGroup, *getNLsocket(&sock) );
};

// ------------------------------------------------------------------------





/*
 *
 * HawkNL Network wrapper
 *
 */
bool bNetworkInited = false;

/////////////////////
// Initializes network
bool InitNetworkSystem() {
	bNetworkInited = false;

    if(!nlInit()) {
    	SystemError("nlInit failed");
    	return false;
    }
    
    if(!nlSelectNetwork(NL_IP)) {
        SystemError("could not select IP-based network");
		return false;
    }
	
	bNetworkInited = true;
	
	if(!SdlNetEvent_Init()) {
		SystemError("SdlNetEvent_Init failed");
		return false;
	}
	
	return true;
}

//////////////////
// Shutdowns the network system
bool QuitNetworkSystem() {
	SdlNetEvent_UnInit();
	nlShutdown();
	bNetworkInited = false;
	return true;
}

NetworkSocket OpenReliableSocket(unsigned short port) {
	NetworkSocket ret;
	*getNLsocket(&ret) = nlOpen(port, NL_RELIABLE);
	AddSocketToNotifierGroup(ret);
	return ret;
}

NetworkSocket OpenUnreliableSocket(unsigned short port) {
	NetworkSocket ret;
	*getNLsocket(&ret) = nlOpen(port, NL_UNRELIABLE);
	AddSocketToNotifierGroup(ret);
	return ret;
}

NetworkSocket OpenBroadcastSocket(unsigned short port) {
	NetworkSocket ret;
	*getNLsocket(&ret) = nlOpen(port, NL_BROADCAST);
	AddSocketToNotifierGroup(ret);
	return ret;
}

bool ConnectSocket(NetworkSocket sock, const NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else  {
		AddSocketToNotifierGroup(sock);
		return (nlConnect(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
	}
}

bool ListenSocket(NetworkSocket sock) {
	AddSocketToNotifierGroup(sock);
	return (nlListen(*getNLsocket(&sock)) != NL_FALSE);
}

bool CloseSocket(NetworkSocket sock) {
	RemoveSocketFromNotifierGroup(sock);
	return (nlClose(*getNLsocket(&sock)) != NL_FALSE);
}

int WriteSocket(NetworkSocket sock, const void* buffer, int nbytes) {
	NLint ret = nlWrite(*getNLsocket(&sock), buffer, nbytes);

#ifdef DEBUG
	// Error checking
	if (ret == NL_INVALID)  {
		if (nlGetError() == NL_SYSTEM_ERROR)
			printf("WriteSocket: " + std::string(nlGetSystemErrorStr(nlGetSystemError())) + "\n");
		else
			printf("WriteSocket: " + std::string(nlGetErrorStr(nlGetError())) + "\n");

		return NL_INVALID;
	}

	if (ret == 0) {
		printf("WriteSocket: Could not send the packet, network buffers are full.\n");
	}
#endif // DEBUG

	return ret;
}

int	WriteSocket(NetworkSocket sock, const std::string& buffer) {
	return WriteSocket(sock, buffer.data(), buffer.size());
}

int ReadSocket(NetworkSocket sock, void* buffer, int nbytes) {
	NLint ret = nlRead(*getNLsocket(&sock), buffer, nbytes);

#ifdef DEBUG
	// Error checking
	if (ret == NL_INVALID)  {
		if (nlGetError() == NL_SYSTEM_ERROR)
			printf("ReadSocket: SYSTEM: " + std::string(nlGetSystemErrorStr(nlGetSystemError())) + "\n");
		else
			printf("ReadSocket: " + std::string(nlGetErrorStr(nlGetError())) + "\n");

		return NL_INVALID;
	}
#endif // DEBUG

	return ret;
}

bool IsSocketStateValid(NetworkSocket sock) {
	return (*getNLsocket(&sock) != NL_INVALID);
}

bool IsSocketReady(NetworkSocket sock)  {
	return nlWrite(*getNLsocket(&sock), (void *)"", 0) >= 0;
}

void InvalidateSocketState(NetworkSocket& sock) {
	*getNLsocket(&sock) = NL_INVALID;
}

int GetSocketErrorNr() {
	return nlGetError();
}

const std::string GetSocketErrorStr(int errnr) {
	return std::string(nlGetErrorStr(errnr));
}

bool IsMessageEndSocketErrorNr(int errnr) {
	return (errnr == NL_MESSAGE_END);
}

void ResetSocketError()  {
	if (!bNetworkInited)
		return;

	// TODO: make this correct
	// Init a new instance and then shut it down (a bit dirty but there's no other way to do it)
	nlInit();
	nlShutdown();
}

bool GetLocalNetAddr(NetworkSocket sock, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlGetLocalAddr(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
}

bool GetRemoteNetAddr(NetworkSocket sock, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlGetRemoteAddr(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
}

bool SetRemoteNetAddr(NetworkSocket sock, const NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else
		return (nlSetRemoteAddr(*getNLsocket(&sock), getNLaddr(addr)) != NL_FALSE);
}

bool IsNetAddrValid(NetworkAddr* addr) {
	if(addr)
		return (getNLaddr(addr)->valid != NL_FALSE);
	else
		return false;
}

bool SetNetAddrValid(NetworkAddr* addr, bool valid) {
	if(!addr) return false;
	getNLaddr(addr)->valid = valid ? NL_TRUE : NL_FALSE;
	return true;
}

void ResetNetAddr(NetworkAddr* addr) {
	if(!addr) return;
	// TODO: is this the best way?
	memset(getNLaddr(addr), 0, sizeof(NLaddress));
	SetNetAddrValid(addr, false);
}

bool StringToNetAddr(const std::string& string, NetworkAddr* addr) {
	if(addr == NULL) {
		return false;
	} else	
		return (nlStringToAddr(string.c_str(), getNLaddr(addr)) != NL_FALSE);
}

bool NetAddrToString(const NetworkAddr* addr, std::string& string) {
	static char buf[256];
	NLchar* res = nlAddrToString(getNLaddr(addr), buf);
	if(res) {
		fix_markend(buf);
		string = buf;
		return true;
	} else
		return false;
}

unsigned short GetNetAddrPort(NetworkAddr* addr) {
	if(addr == NULL)
		return 0;
	else
		return nlGetPortFromAddr(getNLaddr(addr));
}

bool SetNetAddrPort(NetworkAddr* addr, unsigned short port) {
	if(addr == NULL)
		return false;
	else
		return (nlSetAddrPort(getNLaddr(addr), port) != NL_FALSE);
}

bool AreNetAddrEqual(const NetworkAddr* addr1, const NetworkAddr* addr2) {
	if(addr1 == addr2)
		return true;
	else {
		if(addr1 == NULL || addr2 == NULL)
			return false;
		else
			return (nlAddrCompare(getNLaddr(addr1), getNLaddr(addr2)) != NL_FALSE);
	}
}

typedef std::map<std::string, NetworkAddr> dnsCacheT; 
dnsCacheT dnsCache;

void AddToDnsCache(const std::string& name, const NetworkAddr* addr) {
	dnsCache[name] = *addr;
}

bool GetFromDnsCache(const std::string& name, NetworkAddr* addr) {
	dnsCacheT::iterator it = dnsCache.find(name);
	if(it != dnsCache.end()) {
		*addr = it->second;
		return true;
	} else
		return false;
}

bool GetNetAddrFromNameAsync(const std::string& name, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else {
		if(GetFromDnsCache(name, addr)) {
			SetNetAddrValid(addr, true);
			return true;
		}
		return (nlGetAddrFromNameAsync(name.c_str(), getNLaddr(addr)) != NL_FALSE);
	}
}

bool GetNetAddrFromName(const std::string& name, NetworkAddr* addr) {
	if(addr == NULL)
		return false;
	else {
		if(GetFromDnsCache( name, addr )) {
			SetNetAddrValid( addr, true );
			return true;
		}
		if( nlGetAddrFromName(name.c_str(), getNLaddr(addr)) != NL_FALSE ) {
			AddToDnsCache( name, addr );
			return true;
		}
		return false;
	}
}

bool isDataAvailable(NetworkSocket sock) {
	NLint group = nlGroupCreate();
	nlGroupAddSocket( group, *getNLsocket(&sock) );
	NLsocket sock_out[2];
	int ret = nlPollGroup( group, NL_READ_STATUS, sock_out, 1, 0 );
	nlGroupDestroy(group);
	return ret > 0;
};



