/*
   AngelCode Scripting Library
   Copyright (c) 2003-2013 Andreas Jonsson

   This software is provided 'as-is', without any express or implied 
   warranty. In no event will the authors be held liable for any 
   damages arising from the use of this software.

   Permission is granted to anyone to use this software for any 
   purpose, including commercial applications, and to alter it and 
   redistribute it freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you 
      must not claim that you wrote the original software. If you use
      this software in a product, an acknowledgment in the product 
      documentation would be appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and 
      must not be misrepresented as being the original software.

   3. This notice may not be removed or altered from any source 
      distribution.

   The original version of this library can be located at:
   http://www.angelcode.com/angelscript/

   Andreas Jonsson
   andreas@angelcode.com
*/



//
// as_scriptobject.h
//
// A generic class for handling script declared structures
//



#ifndef AS_SCRIPTOBJECT_H
#define AS_SCRIPTOBJECT_H

#include "as_config.h"
#include "as_atomic.h"

BEGIN_AS_NAMESPACE

class asCObjectType;

// TODO: Add const overload for GetAddressOfProperty

// TODO: weak: Should move to its own file
class asCSharedBool : public asISharedBool
{
public:
	asCSharedBool();
	int AddRef() const;
	int Release() const;

	bool Get() const;
	void Set(bool);

protected:
	mutable asCAtomic refCount;
	bool      value;
};

class asCScriptObject : public asIScriptObject
{
public:
//===================================
// From asIScriptObject
//===================================
	asIScriptEngine *GetEngine() const;

	// Memory management
	int AddRef() const;
	int Release() const;

	// Type info
	int            GetTypeId() const;
	asIObjectType *GetObjectType() const;

	// Class properties
	asUINT      GetPropertyCount() const;
	int         GetPropertyTypeId(asUINT prop) const;
	const char *GetPropertyName(asUINT prop) const;
	void       *GetAddressOfProperty(asUINT prop);

	int         CopyFrom(asIScriptObject *other);

//====================================
// Internal
//====================================
	asCScriptObject(asCObjectType *objType, bool doInitialize = true);
	virtual ~asCScriptObject();

	asCScriptObject &operator=(const asCScriptObject &other);

	// GC methods
	void Destruct();
	int  GetRefCount();
	void SetFlag();
	bool GetFlag();
	void EnumReferences(asIScriptEngine *engine);
	void ReleaseAllHandles(asIScriptEngine *engine);

	// Weakref methods
	asISharedBool *GetWeakRefFlag() const;

	// Used for properties
	void *AllocateUninitializedObject(asCObjectType *objType, asCScriptEngine *engine);
	void FreeObject(void *ptr, asCObjectType *objType, asCScriptEngine *engine);
	void CopyObject(void *src, void *dst, asCObjectType *objType, asCScriptEngine *engine);
	void CopyHandle(asPWORD *src, asPWORD *dst, asCObjectType *objType, asCScriptEngine *engine);

	void CallDestructor();

	asCObjectType *objType;

protected:
	mutable asCAtomic refCount;
	mutable bool gcFlag;
	mutable asCSharedBool *weakRefFlag;
	bool isDestructCalled;
};

void ScriptObject_Construct(asCObjectType *objType, asCScriptObject *self);
asCScriptObject &ScriptObject_Assignment(asCScriptObject *other, asCScriptObject *self);

void ScriptObject_ConstructUnitialized(asCObjectType *objType, asCScriptObject *self);

void ScriptObject_Construct_Generic(asIScriptGeneric *gen);
void ScriptObject_Assignment_Generic(asIScriptGeneric *gen);

void RegisterScriptObject(asCScriptEngine *engine);

asIScriptObject *ScriptObjectFactory(const asCObjectType *objType, asCScriptEngine *engine);

END_AS_NAMESPACE

#endif
