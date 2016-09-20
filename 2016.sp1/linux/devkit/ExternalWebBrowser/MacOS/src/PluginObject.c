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

#include "mcp.h"
#include "PluginObject.h"

#include <string.h>

NPObject *pluginAllocate (NPP npp, NPClass *aClass);
void pluginDeallocate (NPObject *obj);
void pluginInvalidate (NPObject *obj);
bool pluginHasMethod (NPObject *obj, NPIdentifier name);
bool pluginInvoke (NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool pluginInvokeDefault (NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result);
bool pluginHasProperty (NPObject *obj, NPIdentifier name);
bool pluginGetProperty (NPObject *obj, NPIdentifier name, NPVariant *result);
bool pluginSetProperty (NPObject *obj, NPIdentifier name, const NPVariant *value);
bool pluginRemoveProperty (NPObject *npobj, NPIdentifier name);

static NPClass _pluginFunctionPtrs = { 
	NP_CLASS_STRUCT_VERSION,
	pluginAllocate, 
    pluginDeallocate, 
    pluginInvalidate,
    pluginHasMethod,
    pluginInvoke,
    pluginInvokeDefault,
    pluginHasProperty,
    pluginGetProperty,
    pluginSetProperty,
	pluginRemoveProperty
};
 
NPClass *getPluginClass(void)
{
    return &_pluginFunctionPtrs;
}

#define kBuffSize 5000
static bool identifiersInitialized = false;
static char gBuff[kBuffSize];

#define ID_PORT_PROPERTY               0
#define	NUM_PROPERTY_IDENTIFIERS	1

static NPIdentifier pluginPropertyIdentifiers[NUM_PROPERTY_IDENTIFIERS];
static const NPUTF8 *pluginPropertyIdentifierNames[NUM_PROPERTY_IDENTIFIERS] = {
	"port"
};

#define ID_EXECUTE_METHOD		        0
#define NUM_METHOD_IDENTIFIERS		        1

static NPIdentifier pluginMethodIdentifiers[NUM_METHOD_IDENTIFIERS];
static const NPUTF8 *pluginMethodIdentifierNames[NUM_METHOD_IDENTIFIERS] = {
	"execute"
};

static void initializeIdentifiers()
{
    browser->getstringidentifiers (pluginPropertyIdentifierNames, NUM_PROPERTY_IDENTIFIERS, pluginPropertyIdentifiers);
    browser->getstringidentifiers (pluginMethodIdentifierNames, NUM_METHOD_IDENTIFIERS, pluginMethodIdentifiers);
};

bool pluginHasProperty(NPObject *obj, NPIdentifier name)
{	
    int i;
    for (i = 0; i < NUM_PROPERTY_IDENTIFIERS; i++) {
        if (name == pluginPropertyIdentifiers[i]){
            return true;
        }
    }
    return false;
}

bool pluginHasMethod (NPObject *obj, NPIdentifier name)
{
    int i;
    for (i = 0; i < NUM_METHOD_IDENTIFIERS; i++) {
        if (name == pluginMethodIdentifiers[i]){
            return true;
        }
    }
    return false;
}

void getResultAsVariant(NPVariant *result)
{
	NPUTF8 *res = (NPUTF8 *)malloc(strlen(gBuff)+1);
	memcpy(res, gBuff, strlen(gBuff)+1);
	// Get rid of leading white spaces
	while(isspace(res[0])) {
		if(strlen(res) > 1) {
			memmove(res,res+1,strlen(res)-1);
		} else {
			res[0] = '\0';
			break;
		}
	}
	// Get rid of trailing white spaces
	while(strlen(res) > 0 && isspace(res[strlen(res)-1])) {
		res[strlen(res)-1] = '\0';
	}

	if(strlen(res) > 0) {
		// Try to convert to integer
		char *endPtr;
		int32_t intResult = strtol(res, &endPtr, 10);
		if(res != NULL && *endPtr == '\0') {
			INT32_TO_NPVARIANT(intResult, *result);
			return;
		}
		// Try to convert to double
		double doubleResult = strtod(res, &endPtr);
		if(res != NULL && *endPtr == '\0') {
			DOUBLE_TO_NPVARIANT(doubleResult, *result);
			return;
		}
	}

	// Return string
	STRINGN_TO_NPVARIANT(res, (uint32_t)strlen(res), *result);
	return;
}

bool pluginGetProperty (NPObject *npobj, NPIdentifier name, NPVariant *result)
{
	PluginObject *obj = (PluginObject *)npobj;
	result->type = NPVariantType_Void;

    if (name == pluginPropertyIdentifiers[ID_PORT_PROPERTY]) {
		if(obj->socket >= 0) {
			result->type = NPVariantType_String;
			NPUTF8 *s = (NPUTF8 *)malloc(obj->port.UTF8Length);
			strncpy(s, obj->port.UTF8Characters, obj->port.UTF8Length);
			result->value.stringValue.UTF8Characters = s;
			result->value.stringValue.UTF8Length = obj->port.UTF8Length;
		}
		return true;
	}
	
	return false;
}

bool pluginSetProperty (NPObject *npobj, NPIdentifier name, const NPVariant *value)
{
	PluginObject *obj = (PluginObject *)npobj;
    if (name == pluginPropertyIdentifiers[ID_PORT_PROPERTY]) {
		if(NPVARIANT_IS_STRING(*value)) {
			if(obj->port.UTF8Length > 0) {
				free((void *)(obj->port.UTF8Characters));
				obj->port.UTF8Length = 0;
			}
			NPUTF8 *s = (NPUTF8 *)malloc((value->value.stringValue.UTF8Length)+1);
			memcpy(s, value->value.stringValue.UTF8Characters, value->value.stringValue.UTF8Length);
			s[value->value.stringValue.UTF8Length] = '\0';
			obj->port.UTF8Characters = s;
			obj->port.UTF8Length = value->value.stringValue.UTF8Length;

			// when user sets port, connect to it
			if(obj->socket >= 0) {
				mcpClose(obj->socket);
				obj->socket = -1;
			}
			if(obj->port.UTF8Length > 0) {
				obj->socket = mcpOpen((char *)(obj->port.UTF8Characters));
			}
			return true;
		}
    }
	return false;	
}

bool pluginRemoveProperty (NPObject *npobj, NPIdentifier name)
{
	return false;
}

bool pluginInvoke (NPObject *npobj, NPIdentifier name, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
	PluginObject *obj = (PluginObject *)npobj;
	if (obj->socket >= 0) {
		if(name == pluginMethodIdentifiers[ID_EXECUTE_METHOD]) {
			if (argCount == 1){
				if (NPVARIANT_IS_STRING(args[0])) {
					// Send command to socket
					NPString string = NPVARIANT_TO_STRING(args[0]);
					mcpWrite(obj->socket, (char *)string.UTF8Characters, string.UTF8Length);
					// Read the result back
					int bytes = mcpRead(obj->socket, gBuff, kBuffSize);
					gBuff[bytes] = '\0';
					getResultAsVariant(result);
					return true;
				}
			}
		}
    }
	
	result->type = NPVariantType_Void;
	return false;
}

bool pluginInvokeDefault(NPObject *npobj, const NPVariant *args, uint32_t argCount, NPVariant *result)
{
    result->type = NPVariantType_Void;
	return false;
}

void pluginInvalidate (NPObject *obj)
{
    // Make sure we've released any remainging references to JavaScript
    // objects.
}

NPObject *pluginAllocate (NPP npp, NPClass *aClass)
{
    PluginObject *newInstance = (PluginObject *)malloc (sizeof(PluginObject));
    
    if (!identifiersInitialized) {
        identifiersInitialized = true;
        initializeIdentifiers();
    }

    newInstance->npp = npp;
	newInstance->referenceCount = 1;
	newInstance->port.UTF8Length = 0;
	newInstance->port.UTF8Characters = NULL;
	newInstance->socket = -1;

    return (NPObject *)newInstance;
}

void pluginDeallocate (NPObject *obj) 
{
	free ((void *)obj);
}

