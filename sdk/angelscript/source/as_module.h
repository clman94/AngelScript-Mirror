/*
   AngelCode Scripting Library
   Copyright (c) 2003-2008 Andreas Jonsson

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
// as_module.h
//
// A class that holds a script module
//

#ifndef AS_MODULE_H
#define AS_MODULE_H

#include "as_config.h"
#include "as_atomic.h"
#include "as_string.h"
#include "as_array.h"
#include "as_datatype.h"
#include "as_scriptfunction.h"
#include "as_property.h"

BEGIN_AS_NAMESPACE

const int asFUNC_INIT   = 0xFFFF;
const int asFUNC_STRING = 0xFFFE;

const int FUNC_IMPORTED = 0x40000000;

class asCScriptEngine;
class asCCompiler;
class asCBuilder;
class asCContext;
class asCConfigGroup;

struct sBindInfo
{
	asDWORD importFrom;
	int importedFunction;
};

struct sObjectTypePair
{
	asCObjectType *a;
	asCObjectType *b;
};

// TODO: addref/release for the module. The Module should have addref/release methods so that 
// the application can keep its own references. If DiscardModule is called on the module from
// the engine the module is not discarded immediately if the application holds it's own reference.
// Only when the application releases its references is the module discarded. Discard will however
// remove the reference from the engine's list of valid modules, so GetModule won't return it any
// more.

// TODO: global: The module represents the current scope. Global variables may be added/removed
// from the scope through DeclareGlobalVar, UndeclareGlobalVar. Undeclaring a global variable
// doesn't destroy it, it just means the variable is no longer visible from the module, e.g. for
// new function compilations. Only when no more functions are accessing the global variables is
// the variable removed.

// TODO: dynamic functions: It must be possible to compile new functions dynamically within the 
// scope of a module. The new functions can be added to the scope of the module, or it can be 
// left outside, thus only accessible through the function id that is returned. This can be used
// by scripts to dynamically compile new functions. It will also be possible to undeclare functions,
// in which case the function is removed from the scope of the module. When no one else is accessing
// the function anymore, will it be removed.

class asCModule : public asIScriptModule
{
//-------------------------------------------
// Public interface
//--------------------------------------------
public:
	virtual asIScriptEngine *GetEngine();
	virtual void             SetName(const char *name);
	virtual const char      *GetName(int *length);

	// Compilation
	virtual int  AddScriptSection(const char *name, const char *code, size_t codeLength, int lineOffset);
	virtual int  Build();

	// Script functions
	virtual int                GetFunctionCount();
	virtual int                GetFunctionIdByIndex(int index);
	virtual int                GetFunctionIdByName(const char *name);
	virtual int                GetFunctionIdByDecl(const char *decl);
	virtual asIScriptFunction *GetFunctionDescriptorByIndex(int index);
	virtual asIScriptFunction *GetFunctionDescriptorById(int funcId);

	// Script global variables
	virtual int         ResetGlobalVars();
	virtual int         GetGlobalVarCount();
	virtual int         GetGlobalVarIndexByName(const char *name);
	virtual int         GetGlobalVarIndexByDecl(const char *decl);
	virtual const char *GetGlobalVarDeclaration(int index, int *length);
	virtual const char *GetGlobalVarName(int index, int *length);
	virtual int         GetGlobalVarTypeId(int index);
	virtual void       *GetAddressOfGlobalVar(int index);

	// Type identification
	virtual int            GetObjectTypeCount();
	virtual asIObjectType *GetObjectTypeByIndex(asUINT index);
	virtual int            GetTypeIdByDecl(const char *decl);

	// Dynamic binding between modules
	virtual int         GetImportedFunctionCount();
	virtual int         GetImportedFunctionIndexByDecl(const char *decl);
	virtual const char *GetImportedFunctionDeclaration(int importIndex, int *length = 0);
	virtual const char *GetImportedFunctionSourceModule(int importIndex, int *length = 0);
	virtual int         BindImportedFunction(int index, int sourceID);
	virtual int         UnbindImportedFunction(int importIndex);
	virtual int         BindAllImportedFunctions();
	virtual int         UnbindAllImportedFunctions();

	// Bytecode Saving/Loading
	virtual int SaveByteCode(asIBinaryStream *out);
	virtual int LoadByteCode(asIBinaryStream *in);

//-----------------------------------------------
// Internal
//-----------------------------------------------
	asCModule(const char *name, int id, asCScriptEngine *engine);
	~asCModule();

//protected:
	friend class asCScriptEngine;
	friend class asCBuilder;
	friend class asCCompiler;
	friend class asCContext;
	friend class asCRestore;

	void Discard();
	void InternalReset();

	int  AddContextRef();
	int  ReleaseContextRef();
	asCAtomic contextCount;

	int  AddModuleRef();
	int  ReleaseModuleRef();
	asCAtomic moduleCount;

	void CallInit();
	void CallExit();
	bool isGlobalVarInitialized;

	bool IsUsed();

	int  AddConstantString(const char *str, size_t length);
	const asCString &GetConstantString(int id);

	int  AllocGlobalMemory(int size);

	int  GetNextFunctionId();
	int  AddScriptFunction(int sectionIdx, int id, const char *name, const asCDataType &returnType, asCDataType *params, int *inOutFlags, int paramCount, bool isInterface, asCObjectType *objType = 0, bool isConstMethod = false);
	int  AddImportedFunction(int id, const char *name, const asCDataType &returnType, asCDataType *params, int *inOutFlags, int paramCount, int moduleNameStringID);

	bool CanDeleteAllReferences(asCArray<asCModule*> &modules);

	void UpdateGlobalVarPointer(void *pold, void *pnew);

	int  GetNextImportedFunctionId();

	void ResolveInterfaceIds();
	bool AreInterfacesEqual(asCObjectType *a, asCObjectType *b, asCArray<sObjectTypePair> &equals);
	bool AreTypesEqual(const asCDataType &a, const asCDataType &b, asCArray<sObjectTypePair> &equals);

	asCScriptFunction *GetImportedFunction(int funcId);
	asCScriptFunction *GetScriptFunction(int funcId);
	asCScriptFunction *GetSpecialFunction(int funcId);

	asCObjectType *GetObjectType(const char *type);

	int  GetScriptSectionIndex(const char *name);
	bool CanDelete();

	int GetGlobalVarIndex(int propIdx);

	asCString name;

	asCScriptEngine *engine;
	asCBuilder      *builder;
	bool             isBuildWithoutErrors;

	int  moduleId;
	bool isDiscarded;

	asCScriptFunction             *initFunction;
	asCArray<asCString *>          scriptSections;
	asCArray<asCScriptFunction *>  scriptFunctions;
	asCArray<asCScriptFunction *>  importedFunctions;
	asCArray<sBindInfo>            bindInformations;

	// TODO: global: the memory for the global variables must be allocated individually, 
	// so that they can be managed individually. It must be possible to add/remove 
	// globals to an already compiled module. Functions that reference a global
	// variable should protect the global so that it isn't removed too early 
	// (or possibly it should be through weak pointers, which would cause script 
	//  exception if a removed variable is accessed)
	// Later on, all globals should be managed by the engine so that a module
	// can be discarded without having to remove the global attribute itself.
	asCArray<asCProperty *>        scriptGlobals;
	asCArray<size_t>               globalMem;
	asCArray<void*>                globalVarPointers;

	asCArray<asCString*>           stringConstants;
	asCArray<asCObjectType*>       classTypes;
};

END_AS_NAMESPACE

#endif
