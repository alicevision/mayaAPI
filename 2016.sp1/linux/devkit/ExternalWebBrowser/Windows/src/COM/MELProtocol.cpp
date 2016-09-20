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
 * Parses the lpCmdLine argument as a mel: URL and attempts to send a command
 * to default Maya command port (named "commandportDefault").
 *
 * The format of the URL is mel:[//[<portname>]]<command>[/]
 *
 * Currently, there is no attempt to launch Maya if it isn't already running,
 * nor is there any kind of error reporting.
 */
extern "C" void CALLBACK OpenURLW(HWND hwnd, HINSTANCE hinst, LPCWSTR lpCmdLine, int nCmdShow)
{
	CAtlString strUrl(lpCmdLine);

	// for example, convert "%20" sequences back to spaces
	HRESULT hr = ::UrlUnescape(strUrl.GetBuffer(), NULL, NULL, URL_UNESCAPE_INPLACE);
	if (FAILED(hr))
		return;
	strUrl.ReleaseBuffer();

	CAtlRegExp<> re;
	REParseError status = re.Parse(L"[Mm][Ee][Ll]:(//({[^/]*?}/)?)?{.+}");
	if (status != REPARSE_ERROR_OK)
		return;

	// specified URL must match regular expression
	CAtlREMatchContext<> mc;
	if (!re.Match(strUrl, &mc) || mc.m_uNumGroups != 2)
		return;

	// extract command and optional port name
	CAtlREMatchContext<CAtlRECharTraitsW>::MatchGroup mg;
	mc.GetMatch(0, &mg);
	CAtlString strPortName(mg.szStart, (int)(ptrdiff_t)(mg.szEnd - mg.szStart));
	mc.GetMatch(1, &mg);
	CAtlString strCommand(mg.szStart, (int)(ptrdiff_t)(mg.szEnd - mg.szStart));

	// use default port name if not specified
	if (strPortName.IsEmpty())
		strPortName = L"commandportDefault";
	// trim trailing slash character(s)
	strCommand.TrimRight(L"/");

	// convert Unicode strings to ANSI
	CW2A pszPortName(strPortName);
	CW2A pszCommand(strCommand);

	// initialize Windows Sockets 2
	WSADATA wsaData;
	int err = WSAStartup(MAKEWORD(2, 0), &wsaData);
	if (err != 0)
		return;

	ADDRINFO *pAddrInfo;
	if (!GetMayaCommandPortAddress(pszPortName, &pAddrInfo))
	{
		WSACleanup();
		return;
	}

	SOCKET sock = ConnectToMayaCommandPortByAddress(pAddrInfo);
	freeaddrinfo(pAddrInfo);

	if (sock == INVALID_SOCKET)
	{
		WSACleanup();
		return;
	}

	// send command to Maya command port
	err = send(sock, pszCommand, (int)strlen(pszCommand), 0);
	if (err == SOCKET_ERROR)
	{
		DisconnectFromMayaCommandPort(sock);
		WSACleanup();
		return;
	}

	// receive reply, if any, but then just discard it...
	char* recvBuf = new char[4096+1];
	int numBytes = recv(sock, recvBuf, 4096, 0);
	delete[] recvBuf;

	DisconnectFromMayaCommandPort(sock);
	WSACleanup();
}

extern "C" void CALLBACK OpenURLA(HWND hwnd, HINSTANCE hinst, LPCSTR lpCmdLine, int nCmdShow)
{
	// convert ANSI command line to Unicode and call wide-character function
	OpenURLW(hwnd, hinst, CA2W(lpCmdLine), nCmdShow);
}

