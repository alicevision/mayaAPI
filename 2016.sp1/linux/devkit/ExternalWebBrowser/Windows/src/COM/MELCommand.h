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

// MELCommand.h : Declaration of the CMELCommand

#pragma once
#include "resource.h"       // main symbols

#include "MayaCmdCOM.h"


// CMELCommand

class ATL_NO_VTABLE CMELCommand : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CMELCommand, &CLSID_MELCommand>,
	public IDispatchImpl<IMELCommand, &IID_IMELCommand, &LIBID_MayaCommandEngine, /*wMajor =*/ 1, /*wMinor =*/ 0>,
	public ISupportErrorInfoImpl<&IID_IMELCommand>
{
private:
	CAtlString m_strPortName;
	CAtlString m_strResult;
	SOCKET m_socket;

	static CComVariant MakeVariant(const CAtlString& strText);
	static CComVariant MakeVariantNotArray(const CAtlString& strText);

public:
	CMELCommand()
	{
		m_strPortName = L"commandportDefault";
		m_socket = INVALID_SOCKET;
	}

DECLARE_REGISTRY_RESOURCEID(IDR_MELCOMMAND)


BEGIN_COM_MAP(CMELCommand)
	COM_INTERFACE_ENTRY(IMELCommand)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()


	DECLARE_PROTECT_FINAL_CONSTRUCT()

	HRESULT FinalConstruct()
	{
		return S_OK;
	}
	
	void FinalRelease() 
	{
		Disconnect();
	}

public:

	STDMETHOD(Execute)(BSTR Command);
	STDMETHOD(get_Result)(VARIANT* pVal);
	STDMETHOD(get_PortName)(BSTR* pVal);
	STDMETHOD(put_PortName)(BSTR newVal);
	STDMETHOD(Connect)(void);
	STDMETHOD(Disconnect)(void);
	STDMETHOD(get_Connected)(VARIANT_BOOL* pVal);
};

OBJECT_ENTRY_AUTO(__uuidof(MELCommand), CMELCommand)
