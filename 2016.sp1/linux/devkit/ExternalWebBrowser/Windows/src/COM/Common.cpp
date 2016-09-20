//-
// ==========================================================================
// Copyright 1995,2006,2008 Autodesk, Inc. All rights reserved.
//
// Use of this software is subject to the terms of the Autodesk
// license agreement provided at the time of installation or download,
// or which otherwise accompanies this software in either electronic
// or hard copy form.
// ==========================================================================
//+

#include "stdafx.h"
#include "Common.h"

/*
 * Establishes a connection to the Maya command port at the specified address.
 * Returns the socket if successful, or INVALID_SOCKET otherwise.
 */
SOCKET ConnectToMayaCommandPortByAddress(LPADDRINFO pAddrInfo)
{
	SOCKET sock = socket(pAddrInfo->ai_family, SOCK_STREAM, 0);

	if (sock != INVALID_SOCKET)
	{
		int err = connect(sock, pAddrInfo->ai_addr, (int)pAddrInfo->ai_addrlen);
		if (err == SOCKET_ERROR)
		{
			closesocket(sock);
			sock = INVALID_SOCKET;
		}
		else
		{
			BOOL bNoDelay = TRUE;
			setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
				(const char*)&bNoDelay, sizeof(bNoDelay));
		}
	}

	return sock;
}

/*
 * Establishes a connection to the Maya command port with the specified name.
 * Returns the socket if successful, or INVALID_SOCKET otherwise.
 */
SOCKET ConnectToMayaCommandPortByName(LPCSTR pszName)
{
	LPADDRINFO pAddrInfo;

	if (!GetMayaCommandPortAddress(pszName, &pAddrInfo))
		return INVALID_SOCKET;

	SOCKET sock = ConnectToMayaCommandPortByAddress(pAddrInfo);

	freeaddrinfo(pAddrInfo);

	return sock;
}

/*
 * Forcefully closes the connection on the specified socket. This is necessary
 * because Maya will not otherwise close the connection on its end, which will
 * potentially leave a lot of sockets open in the CLOSE_WAIT state.
 */
void DisconnectFromMayaCommandPort(SOCKET sock)
{
	if (sock != INVALID_SOCKET)
	{
		LINGER l;
		l.l_linger = 0;
		l.l_onoff = 1;
		setsockopt(sock, SOL_SOCKET, SO_LINGER, (const char*)&l, sizeof(l));
		closesocket(sock);
	}
}

/*
 * Retrieves the address information (hostname, port number) for the
 * specified Maya command port name. If the port name contains a
 * colon (':') character, it is interpreted as "<hostname>:<port>".
 * Otherwise, it is assumed to specify a Maya Unix socket path.
 *
 * Returns true if successful, or false otherwise. If this function
 * returns true, the caller is responsible for freeing the address
 * information that is dynamically allocated, using freeaddrinfo().
 */
bool GetMayaCommandPortAddress(LPCSTR pszName, ADDRINFO** ppAddrInfo)
{
	CAtlStringA strName(pszName);
	CAtlStringA strHost("localhost");
	CAtlStringA strPath;
	CAtlStringA strPort;

	int nFindPos = strName.Find(':');
	if (nFindPos >= 0)
	{
		// split port name into hostname and port
		if (nFindPos > 0)
		{
			strHost = strName.Left(nFindPos);
		}
		strPort = strName.Mid(nFindPos + 1);
		if (strPort.IsEmpty())
			return false;
		if (getaddrinfo(strHost, strPort, NULL, ppAddrInfo) == 0)
			return true;
		strPath = strPort;
	}
	else
	{
		strPath = pszName;
	}

	// get port number for unix socket path
	UINT16 nPort;
	if (!GetPortForMayaUnixSocketPath(strPath, &nPort))
		return false;
	strPort.Format("%u", nPort);

	return (getaddrinfo(strHost, strPort, NULL, ppAddrInfo) == 0);
}

#define MAYA_UNIX_SOCKET_PATH_OFFSET		0
#define MAYA_UNIX_SOCKET_PATH_SIZE			108
#define MAYA_UNIX_SOCKET_PORT_OFFSET \
	(MAYA_UNIX_SOCKET_PATH_OFFSET + MAYA_UNIX_SOCKET_PATH_SIZE)
#define MAYA_UNIX_SOCKET_PORT_SIZE			sizeof(UINT16)
#define MAYA_UNIX_SOCKET_RECORD_SIZE \
	(MAYA_UNIX_SOCKET_PATH_SIZE + MAYA_UNIX_SOCKET_PORT_SIZE)

#define MAYA_UNIX_SOCKET_BITMASK_SIZE		sizeof(UINT32)
#define MAYA_UNIX_SOCKET_RECORD_COUNT		32

#define MAYA_UNIX_SOCKET_SHARE_SIZE (MAYA_UNIX_SOCKET_BITMASK_SIZE +\
	MAYA_UNIX_SOCKET_RECORD_COUNT * MAYA_UNIX_SOCKET_RECORD_SIZE)

#define MAYA_UNIX_SOCKET_SHARE_NAME			"Maya_Unix_Socket_Share"

/*
 * Performs a lookup on the specified Maya Unix socket path for
 * the corresponding port number. If successful, fills in the
 * port number parameter and returns true; otherwise returns false.
 */
bool GetPortForMayaUnixSocketPath(LPCSTR pszPath, PUINT16 pnPort)
{
	HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, 0,
		PAGE_READWRITE, 0, MAYA_UNIX_SOCKET_SHARE_SIZE,
		MAYA_UNIX_SOCKET_SHARE_NAME);
	if (!hMap)
		return false;
	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		CloseHandle(hMap);
		return false;
	}

	LPVOID pvMem = MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
	if (!pvMem)
	{
		CloseHandle(hMap);
		return false;
	}

	bool bFound = false;

	UINT32 nMask = *(PUINT32)pvMem;
	LPBYTE pRecord = (LPBYTE)pvMem + MAYA_UNIX_SOCKET_BITMASK_SIZE;

	for (int i = 0; i < 32; i++)
	{
		if (nMask & (1 << i))
		{
			LPCSTR pszRecordPath = (LPCSTR)(pRecord + MAYA_UNIX_SOCKET_PATH_OFFSET);
			if (strncmp(pszRecordPath, pszPath, MAYA_UNIX_SOCKET_PATH_SIZE) == 0)
			{
				*pnPort = ntohs(*(PUINT16)(pRecord + MAYA_UNIX_SOCKET_PORT_OFFSET));
				bFound = true;
				break;
			}
			pRecord += MAYA_UNIX_SOCKET_RECORD_SIZE;
		}
	}

	UnmapViewOfFile(pvMem);
	CloseHandle(hMap);

	return bFound;
}

