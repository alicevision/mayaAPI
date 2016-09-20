#pragma once
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


// Common functions for the MayaCmdCOM project.

SOCKET	ConnectToMayaCommandPortByAddress(LPADDRINFO pAddrInfo);
SOCKET	ConnectToMayaCommandPortByName(LPCSTR pszName);
void	DisconnectFromMayaCommandPort(SOCKET socket);
bool	GetMayaCommandPortAddress(LPCSTR pszName, ADDRINFO** ppAddrInfo);
bool	GetPortForMayaUnixSocketPath(LPCSTR pszPath, PUINT16 pnPort);
