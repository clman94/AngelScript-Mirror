#include "utils.h"
#include "../../../add_on/scriptdictionary/scriptdictionary.h"

namespace TestRegisterType
{

void DummyFunc(asIScriptGeneric *) {}

bool TestHandleType();

bool TestIrrTypes();

bool TestRefScoped();

bool TestHelper();

int g_widget;
int *CreateWidget()
{
	return &g_widget;
}

// Use asSFuncPtr in structure initialized with a list
// http://www.gamedev.net/topic/649653-angelscript-2280-is-out/
typedef struct asBehavior_s
{
	asEBehaviours behavior;
	const char * declaration;
	asSFuncPtr funcPointer;
	asECallConvTypes callConv;
} asBehavior_t;

static const asBehavior_t astrace_ObjectBehaviors[] =
{
	{ asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(DummyFunc), asCALL_CDECL_OBJLAST },
	{ asBEHAVE_CONSTRUCT, "void f(const cTrace &in)", asFUNCTION(DummyFunc), asCALL_CDECL_OBJLAST },

	{ } // this was failing due to missing default constr in asSFuncPtr
};

bool Test()
{
	bool fail = TestHelper();
	fail = TestHandleType() || fail;
	fail = TestIrrTypes() || fail;
	int r = 0;
	CBufferedOutStream bout;
	COutStream out;
 	asIScriptEngine *engine;
	const char *script;

	// Test potential memory leak with template types
	// http://www.gamedev.net/topic/653872-memory-leak-when-registering-implicit-reference-cast-for-template/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetEngineProperty(asEP_ALLOW_UNSAFE_REFERENCES, true);

		engine->RegisterObjectType("String", 1, asOBJ_VALUE|asOBJ_POD);
		engine->RegisterInterface("IArchive");
		engine->RegisterObjectType("Array<T>", 1, asOBJ_REF|asOBJ_TEMPLATE|asOBJ_NOCOUNT);

		r = engine->RegisterObjectBehaviour("Array<T>", asBEHAVE_IMPLICIT_REF_CAST, "IArchive@ _beh_11_()", asFUNCTION(0), asCALL_GENERIC);
		if( r < 0 )
			TEST_FAILED;

		asIObjectType *type = engine->GetObjectTypeById(engine->GetTypeIdByDecl("Array<String>"));
		if( type == 0 )
			TEST_FAILED;

		engine->Release();
	}



	// Test registering a float[3] typedef
	// http://www.gamedev.net/topic/653085-dont-think-im-handling-objects-properly/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		typedef float vec3_t[3];
		struct gentity_t {
		};

		struct helper
		{
			static bool CalcMuzzlePoint( gentity_t *const ent, vec3_t wpFwd, vec3_t right, vec3_t wpUp, vec3_t muzzlePoint, float lead_in ) 
			{
				bool ok = true;
				ok &= wpFwd[0] == 1;
				ok &= right[0] == 4;
				ok &= wpUp[1] == 8;
				ok &= muzzlePoint[2] == 2;
				ok &= lead_in == 0;
				return ok;
			}
		};

#ifdef AS_CAN_USE_CPP11
		asUINT appFlags = GetTypeTraits<vec3_t>();
#else
		asUINT appFlags = asOBJ_APP_CLASS;
#endif

		engine->RegisterObjectType("vec3_t", sizeof(vec3_t), asOBJ_VALUE | asOBJ_APP_CLASS | asOBJ_POD);
		engine->RegisterObjectProperty("vec3_t", "float x", 0);
		engine->RegisterObjectProperty("vec3_t", "float y", 4);
		engine->RegisterObjectProperty("vec3_t", "float z", 8);

		engine->RegisterObjectType("gentity_t", sizeof(gentity_t), asOBJ_REF | asOBJ_NOCOUNT);

		// TODO: The vec3_t is passed by reference implicitly as it is an array 
		engine->RegisterGlobalFunction("bool CalcMuzzlePoint(gentity_t@ ent, const vec3_t &in fwd, const vec3_t &in right, const vec3_t &in up, const vec3_t &in muzzle, float lead_in)", asFUNCTIONPR(helper::CalcMuzzlePoint, ( gentity_t *const , vec3_t , vec3_t , vec3_t , vec3_t , float ), bool), asCALL_CDECL);

		vec3_t wpFwd = {1,2,3}, wpVright = {4,5,6}, wpUp = {7,8,9}, wpMuzzle = {0,1,2};
		engine->RegisterGlobalProperty("vec3_t wpFwd", &wpFwd);
		engine->RegisterGlobalProperty("vec3_t wpVright", &wpVright);
		engine->RegisterGlobalProperty("vec3_t wpUp", &wpUp);
		engine->RegisterGlobalProperty("vec3_t wpMuzzle", &wpMuzzle);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void test(gentity_t @ent) { assert( CalcMuzzlePoint(ent, wpFwd, wpVright, wpUp, wpMuzzle, 0) == true ); } \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		gentity_t *ent = 0;

		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(mod->GetFunctionByName("test"));
		ctx->SetArgAddress(0, &ent);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		ctx->Release();

		engine->Release();
	}

	// Test registering a type that require a rectangular list factory
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		engine->RegisterObjectType("rect", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectBehaviour("rect", asBEHAVE_FACTORY, "rect @f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("rect", asBEHAVE_LIST_FACTORY, "rect @f(int&in) {repeat {repeat_same int}}", asFUNCTION(0), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void main() \n"
			"{ \n"
			"  rect r1 = {{1,2,3},{4,5,6}}; \n" // OK 2x3 rect
			"  rect r2 = {{1,2},{3,4,5}}; \n" // Not OK. The second row is not the same length as the first
			"  rect r3 = {{1,2,3},{4,5}}; \n" // Not OK. The second row is not the same length as the first
			"} \n");

		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 1) : Info    : Compiling void main()\n"
						   "test (4, 21) : Error   : Too many values to match pattern\n"
						   "test (5, 23) : Error   : Not enough values to match pattern\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test registering a method that takes an array as argument
	// http://www.gamedev.net/topic/652723-segfault-when-binding-function-which-takes-script-array-param/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptArray(engine, false);

		engine->SetDefaultNamespace("gfx");

		engine->RegisterObjectType("GfxSprite", 0, 1);
		engine->RegisterObjectMethod("GfxSprite", "array<GfxSprite@>@ GetChildren() const", asFUNCTION(0), asCALL_GENERIC);

		r = engine->RegisterObjectMethod("GfxSprite", "void Foo(const array<float> &in)", asFUNCTION(0), asCALL_CDECL_OBJLAST);

		engine->Release();
	}

	// Test registering opAssign twice
	// http://www.gamedev.net/topic/649718-validatenousage/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		engine->RegisterObjectType("Test", 0, asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectMethod("Test", "Test &opAssign(const Test &in)", asFUNCTION(0), asCALL_GENERIC);
		r = engine->RegisterObjectMethod("Test", "Test &opAssign(const Test &)", asFUNCTION(0), asCALL_GENERIC);
		if( r >= 0 )
			TEST_FAILED;

		engine->Release();

		if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterObjectMethod' with 'Test' and 'Test &opAssign(const Test &)' (Code: -13)\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Register circular reference between types
	// http://www.gamedev.net/topic/649718-validatenousage/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		// Enums
		engine->RegisterEnum("SortType");
		engine->RegisterEnumValue("SortType", "cAsc", 0);

		// Types
		engine->RegisterObjectType("String", 1, asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectType("Array<T>", 0, asOBJ_REF | asOBJ_TEMPLATE | asOBJ_NOCOUNT);
		engine->RegisterObjectType("Matrix", 0, asOBJ_REF | asOBJ_NOCOUNT);

		// Type members
		engine->RegisterObjectMethod("Array<T>", "void Sort(SortType = SortType :: cAsc)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Array<T>", "String TraceString() const", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("Matrix", "Array<Array<double>>@ ToArray() const", asFUNCTION(0), asCALL_GENERIC);

		engine->Release();

		if( bout.buffer != "" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test calling RegisterObjectMethod with incorrect object name
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterObjectType("obj", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectMethod("obj@", "void func()", asFUNCTION(0), asCALL_GENERIC);
		if( r >= 0 )
			TEST_FAILED;

		engine->Release();
	}

#ifdef AS_CAN_USE_CPP11
	// Test the automatic determination of flags for registering value types
	if( GetTypeTraits<std::string>() != asOBJ_APP_CLASS_CDAK )
		TEST_FAILED;
	if( GetTypeTraits<void*>() != asOBJ_APP_PRIMITIVE )
		TEST_FAILED;
	if( GetTypeTraits<float>() != asOBJ_APP_FLOAT )
		TEST_FAILED;
	if( GetTypeTraits<double>() != asOBJ_APP_FLOAT )
		TEST_FAILED;
	if( GetTypeTraits<bool>() != asOBJ_APP_PRIMITIVE )
		TEST_FAILED;
	struct T {bool a;};
	if( GetTypeTraits<T>() != asOBJ_APP_CLASS )
		TEST_FAILED;
#endif

#ifndef AS_MAX_PORTABILITY
	// It should be possible to use asCALL_THISCALL_ASGLOBAL for global obj behaviours and string factory
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterObjectType("T", 0, asOBJ_REF);
		r = engine->RegisterObjectBehaviour("T", asBEHAVE_FACTORY, "T @f()", asFUNCTION(0), asCALL_THISCALL_ASGLOBAL, (void*)1);
		if( r < 0 )
			TEST_FAILED;
		r = engine->RegisterStringFactory("T@", asFUNCTION(0), asCALL_THISCALL_ASGLOBAL, (void*)1);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}
#endif

	// Test registering a specialized template instance 
	// http://www.gamedev.net/topic/647678-bug-multiple-registration-of-template-type/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		RegisterScriptArray(engine, false);
		r = engine->RegisterObjectType("array<int32>", 0, asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectType("array<int>", 0, asOBJ_REF);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterObjectType' with 'array<int>' (Code: -13)\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();

	}

	// Test registering a function with autohandles for a type that is not yet registered (must give proper error)
	// http://www.gamedev.net/topic/647707-determining-autohandle-support/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		r = engine->RegisterGlobalFunction("void func(type @+)", asFUNCTION(0), asCALL_CDECL);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "System function (1, 11) : Error   : Identifier 'type' is not a data type\n"
						   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void func(type @+)' (Code: -10)\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Don't accept registering object properties with offsets larger than signed 16 bit
	// TODO: Support 32bit offsets, but that requires changes in VM, compiler, and bytecode serialization
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		r = engine->RegisterObjectType("Object", 1<<24, asOBJ_VALUE | asOBJ_POD);
		r = engine->RegisterObjectProperty("Object", "int f", 1<<24);
		if( r != asINVALID_ARG )
			TEST_FAILED;

		engine->Release();
	}

	// Problem reported by Paril101
	// http://www.gamedev.net/topic/636336-member-function-chaining/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		// This should fail, because asOBJ_NOHANDLE is exclusive for asOBJ_REF
		r = engine->RegisterObjectType("Box", sizeof(4), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CK | asOBJ_NOHANDLE);
		if( r >= 0 )
			TEST_FAILED;

		engine->Release();
	}


	// Problem reported by FDsagizi
	// http://www.gamedev.net/topic/632067-asassert-call-release-method-from-object-with-out-ref-counting/
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);

		r = engine->RegisterObjectType( "Widget", 0,  asOBJ_REF | asOBJ_NOCOUNT ); assert( r >= 0 );
#ifndef AS_MAX_PORTABILITY
		r = engine->RegisterGlobalFunction("Widget @CreateWidget()", asFUNCTION(CreateWidget), asCALL_CDECL); assert( r >= 0 );
#else
		r = engine->RegisterGlobalFunction("Widget @CreateWidget()", WRAP_FN(CreateWidget), asCALL_GENERIC); assert( r >= 0 );
#endif

		const char *script = 
			"class NoRef\n"
			"{\n"
			"         Widget @no_ref_count;\n"
			"}\n"
			"void startGame()\n"
			"{\n"
			"         NoRef nr;\n"
			"         @nr.no_ref_count = CreateWidget();\n"
			"         NoRef nr2 = nr;\n"
			"}\n";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "startGame()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// A type registered with asOBJ_REF must not register destructor
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 4, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
	if( r != asILLEGAL_BEHAVIOUR_FOR_TYPE )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : The behaviour is not compatible with the type\n"
		               " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'ref' and 'void f()' (Code: -23)\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// It must be possible to register a string factory that returns a const ref
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("string", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterStringFactory("const string &", asFUNCTION(0), asCALL_GENERIC);
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// A type registered with asOBJ_GC must register all gc behaviours
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("gc", 4, asOBJ_REF | asOBJ_GC); assert( r >= 0 );
	r = ExecuteString(engine, "");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : Type 'gc' is missing behaviours\n"
		               " (0, 0) : Info    : A garbage collected type must have the addref, release, and all gc behaviours\n"
		               " (0, 0) : Error   : Invalid configuration. Verify the registered application interface.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// A type registered with asOBJ_VALUE must not register addref, release, and gc behaviours
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("val", 4, asOBJ_VALUE | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("val", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
	if( r != asILLEGAL_BEHAVIOUR_FOR_TYPE )
		TEST_FAILED;
	r = engine->RegisterObjectBehaviour("val", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);
	if( r != asILLEGAL_BEHAVIOUR_FOR_TYPE )
		TEST_FAILED;
	r = engine->RegisterObjectBehaviour("val", asBEHAVE_GETREFCOUNT, "int f()", asFUNCTION(0), asCALL_GENERIC);
	if( r != asILLEGAL_BEHAVIOUR_FOR_TYPE )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : The behaviour is not compatible with the type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'val' and 'void f()' (Code: -23)\n"
		               " (0, 0) : Error   : The behaviour is not compatible with the type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'val' and 'void f()' (Code: -23)\n"
					   " (0, 0) : Error   : The behaviour is not compatible with the type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'val' and 'int f()' (Code: -23)\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Object types registered as ref must not be allowed to be
	// passed by value to registered functions, nor returned by value
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 4, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void f(ref)", asFUNCTION(0), asCALL_GENERIC);
	if( r >= 0 )
		TEST_FAILED;
	r = engine->RegisterGlobalFunction("ref f()", asFUNCTION(0), asCALL_GENERIC);
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void f(ref)' (Code: -10)\n"
	                   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'ref f()' (Code: -10)\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Ref type without registered assignment behaviour won't allow the assignment
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_FACTORY, "ref@ f()", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
	r = ExecuteString(engine, "ref r1, r2; r1 = r2;");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "ExecuteString (1, 16) : Error   : There is no copy operator for the type 'ref' available.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Ref type must register addref and release
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF); assert( r >= 0 );
	r = ExecuteString(engine, "ref r");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : Type 'ref' is missing behaviours\n"
		               " (0, 0) : Info    : A reference type must have the addref and release behaviours\n"
		               " (0, 0) : Error   : Invalid configuration. Verify the registered application interface.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Ref type with asOBJ_NOHANDLE must not register addref, release, and factory
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC);
	if( r != asILLEGAL_BEHAVIOUR_FOR_TYPE )
		TEST_FAILED;
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC);
	if( r != asILLEGAL_BEHAVIOUR_FOR_TYPE )
		TEST_FAILED;
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_FACTORY, "ref @f()", asFUNCTION(0), asCALL_GENERIC);
	if( bout.buffer != " (0, 0) : Error   : The behaviour is not compatible with the type\n"
		               " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'ref' and 'void f()' (Code: -23)\n"
		               " (0, 0) : Error   : The behaviour is not compatible with the type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'ref' and 'void f()' (Code: -23)\n"
					   "System function (1, 5) : Error   : Object handle is not supported for this type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'ref' and 'ref @f()' (Code: -10)\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Value type with asOBJ_POD without registered assignment behaviour should allow bitwise copy
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("val", 4, asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = ExecuteString(engine, "val v1, v2; v1 = v2;");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Value type without asOBJ_POD and assignment behaviour must not allow bitwise copy
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("val", 4, asOBJ_VALUE | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("val", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("val", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = ExecuteString(engine, "val v1, v2; v1 = v2;");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "ExecuteString (1, 16) : Error   : There is no copy operator for the type 'val' available.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

    // Value types without asOBJ_POD must have constructor and destructor registered
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("val", 4, asOBJ_VALUE | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->RegisterObjectType("val1", 4, asOBJ_VALUE | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("val1", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = engine->RegisterObjectType("val2", 4, asOBJ_VALUE | asOBJ_APP_PRIMITIVE); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("val2", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = ExecuteString(engine, "val v1, v2; v1 = v2;");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : Type 'val' is missing behaviours\n"
		               " (0, 0) : Info    : A non-pod value type must have the default constructor and destructor behaviours\n"
		               " (0, 0) : Error   : Type 'val1' is missing behaviours\n"
					   " (0, 0) : Info    : A non-pod value type must have the default constructor and destructor behaviours\n"
					   " (0, 0) : Error   : Type 'val2' is missing behaviours\n"
					   " (0, 0) : Info    : A non-pod value type must have the default constructor and destructor behaviours\n"
					   " (0, 0) : Error   : Invalid configuration. Verify the registered application interface.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Ref type must register ADDREF and RELEASE
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectType("ref1", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref1", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = engine->RegisterObjectType("ref2", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref2", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = ExecuteString(engine, "ref @r;");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : Type 'ref' is missing behaviours\n"
		               " (0, 0) : Info    : A reference type must have the addref and release behaviours\n"
		               " (0, 0) : Error   : Type 'ref1' is missing behaviours\n"
					   " (0, 0) : Info    : A reference type must have the addref and release behaviours\n"
					   " (0, 0) : Error   : Type 'ref2' is missing behaviours\n"
					   " (0, 0) : Info    : A reference type must have the addref and release behaviours\n"
					   " (0, 0) : Error   : Invalid configuration. Verify the registered application interface.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Ref type with asOBJ_NOCOUNT must not register ADDREF and RELEASE
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	if( r >= 0 ) TEST_FAILED;
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	if( r >= 0 ) TEST_FAILED;
	r = ExecuteString(engine, "ref @r;");
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != " (0, 0) : Error   : The behaviour is not compatible with the type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'ref' and 'void f()' (Code: -23)\n"
					   " (0, 0) : Error   : The behaviour is not compatible with the type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'ref' and 'void f()' (Code: -23)\n"
					   " (0, 0) : Error   : Invalid configuration. Verify the registered application interface.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release();

	// Problem reported by ThyReaper
	// It must not be possible to register multiple ref cast behaviours for the same type
	{
		bout.buffer = "";
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		r = engine->RegisterObjectType("A", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );
		r = engine->RegisterObjectType("B", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("B", asBEHAVE_IMPLICIT_REF_CAST, "A @f()", asFUNCTION(0), asCALL_GENERIC);
		if( r < 0 )
			TEST_FAILED;

		r = engine->RegisterObjectBehaviour("B", asBEHAVE_IMPLICIT_REF_CAST, "A @f()", asFUNCTION(0), asCALL_GENERIC);
		if( r >= 0 )
			TEST_FAILED;

		r = engine->RegisterObjectBehaviour("B", asBEHAVE_IMPLICIT_REF_CAST, "const A@ f() const", asFUNCTION(0), asCALL_GENERIC);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'B' and 'A @f()' (Code: -13)\n"
		                   " (0, 0) : Error   : Failed in call to function 'RegisterObjectBehaviour' with 'B' and 'const A@ f() const' (Code: -10)\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Ref types without default factory must not be allowed to be initialized, nor must it be allowed to be passed by value in parameters or returned by value
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	script = "ref func(ref r) { ref r2; return ref(); }";
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 1) : Info    : Compiling ref func(ref)\n"
		               "script (1, 1) : Error   : Return type can't be 'ref'\n"
					   "script (1, 1) : Error   : Parameter type can't be 'ref', because the type cannot be instanciated.\n"
					   "script (1, 23) : Error   : Data type can't be 'ref'\n"
					   "script (1, 34) : Error   : No matching signatures to 'ref()'\n"
					   "script (1, 34) : Error   : Can't implicitly convert from 'const int' to 'ref'.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->Release(); 

	// Ref types without default constructor must not be allowed to be passed by in/out reference, but must be allowed to be passed by inout reference
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyFunc), asCALL_GENERIC);
	script = "void func(ref &in r1, ref &out r2, ref &inout r3) { }";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (1, 1) : Info    : Compiling void func(ref&in, ref&out, ref&inout)\n"
		               "script (1, 1) : Error   : Parameter type can't be 'ref&in', because the type cannot be instanciated.\n"
					   "script (1, 1) : Error   : Parameter type can't be 'ref&out', because the type cannot be instanciated.\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	int t1 = engine->GetTypeIdByDecl("ref");
	int t2 = engine->GetTypeIdByDecl("ref@") & ~asTYPEID_OBJHANDLE; 
	if( t1 != t2 )
		TEST_FAILED;

	engine->Release();

	// It must not be possible to register functions that take handles of types with asOBJ_HANDLE
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("ref", 0, asOBJ_REF | asOBJ_NOHANDLE); assert( r >= 0 );
	r = ExecuteString(engine, "ref @r");
	if( r >= 0 )
		TEST_FAILED;
	r = engine->RegisterGlobalFunction("ref@ func()", asFUNCTION(0), asCALL_GENERIC);
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "ExecuteString (1, 5) : Error   : Object handle is not supported for this type\n"
	                   "ExecuteString (1, 6) : Error   : Data type can't be 'ref'\n"
	                   "System function (1, 4) : Error   : Object handle is not supported for this type\n"
					   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'ref@ func()' (Code: -10)\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Must be possible to register float types
	r = engine->RegisterObjectType("real", sizeof(float), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_FLOAT); assert( r >= 0 );

	// It should be allowed to register the type without specifying the application type,
	// if the engine won't use it (i.e. no native functions take or return the type by value)
	if( !strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		r = engine->RegisterObjectType("test1", 4, asOBJ_VALUE | asOBJ_POD);
		if( r < 0 ) TEST_FAILED;
		r = engine->RegisterGlobalFunction("test1 f()", asFUNCTION(0), asCALL_CDECL);
		if( r < 0 ) TEST_FAILED;
		r = engine->RegisterGlobalFunction("void f(test1)", asFUNCTION(0), asCALL_CDECL);
		if( r < 0 ) TEST_FAILED;
		r = ExecuteString(engine, "test1 t");
		if( r >= 0 ) TEST_FAILED;
		// TODO: These errors should really be returned immediately when registering the function
		if( bout.buffer != " (0, 0) : Info    : test1 f()\n"
			               " (0, 0) : Error   : Can't return type 'test1' by value unless the application type is informed in the registration\n"
                           " (0, 0) : Info    : void f(test1)\n"
		                   " (0, 0) : Error   : Can't pass type 'test1' by value unless the application type is informed in the registration\n"
						   " (0, 0) : Error   : Invalid configuration. Verify the registered application interface.\n" &&
			// The errors are slightly different on 64bit Linux
			bout.buffer != " (0, 0) : Info    : test1 f()\n"
			               " (0, 0) : Error   : Can't return type 'test1' by value unless the application type is informed in the registration\n"
                           " (0, 0) : Info    : void f(test1)\n"
		                   " (0, 0) : Error   : Can't pass type 'test1' by value unless the application type is informed in the registration\n"
						   " (0, 0) : Info    : void f(test1)\n"
						   " (0, 0) : Error   : Don't support passing type 'test1' by value to application in native calling convention on this platform\n"
						   " (0, 0) : Error   : Invalid configuration. Verify the registered application interface.\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	// It must not be possible to register a value type without defining the application type
	r = engine->RegisterObjectType("test2", 4, asOBJ_VALUE | asOBJ_APP_CLASS_CONSTRUCTOR);
	if( r >= 0 ) TEST_FAILED;

	engine->Release();

	// REF+SCOPED
	if( !fail ) fail = TestRefScoped();

	// Registering a global function as const shouldn't be allowed
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		r = engine->RegisterGlobalFunction("void func() const", asFUNCTION(0), asCALL_GENERIC);
		if( r >= 0 )
			TEST_FAILED;
		engine->Release();
	}

	// Registering an object property with invalid declaration
	// http://www.gamedev.net/topic/623515-reference-object-with-properties/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		r = engine->RegisterObjectType("Npc", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );
		r = engine->RegisterObjectProperty("Npc", "unsigned int hp",  0); assert( r < 0 );

		// TODO: The 'Failed in call' message should show the return code too (and if possible the symbolic name, i.e. asINVALID_DECL)
		if( bout.buffer != "Property (1, 10) : Error   : Expected identifier\n"
		                   " (0, 0) : Error   : Failed in call to function 'RegisterObjectProperty' with 'Npc' and 'unsigned int hp' (Code: -10)\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// TODO:
	// What about asOBJ_NOHANDLE and asEP_ALLOW_UNSAFE_REFERENCES? Should it allow &inout?

	// TODO:
    // Validate if the same behaviour is registered twice, e.g. if index
    // behaviour is registered twice with signature 'int f(int)' and error should be given

	// Success
 	return fail;
}

//////////////////////////////////////

int *Scoped_Factory()
{
	int *p = new int(42);

//	printf("new %p\n", p);

	return p;
}

void Scoped_Release(int *p)
{
//	printf("del %p\n", p);

	if( p ) delete p;
}

int *Scoped_Negate(int *p)
{
	if( p )
		return new int(-*p);
	return 0;
}

int &Scoped_Assignment(int &a, int *p)
{
//	printf("assign %p = %p\n", p, &a);

	*p = a;
	return *p;
}

int *Scoped_Add(int &a, int b)
{
	return new int(a + b);
}

void Scoped_InRef(int &)
{
}

bool TestRefScoped()
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		// Skipping this due to not supporting native calling conventions
		printf("Skipped due to AS_MAX_PORTABILITY\n");
		return false;
	}

	bool fail = false;
	int r = 0;
	CBufferedOutStream bout;
 	asIScriptEngine *engine;
	asIScriptModule *mod;

	// REF+SCOPED
	// This type requires a factory and a release behaviour. It cannot have the addref behaviour.
	// The intention of this type is to permit value types that have special needs for memory management,
	// for example must be aligned on 16 byte boundaries, or must use a memory pool. The type must not allow
	// object handles (except when returning a new value from registered functions).
	bout.buffer = "";
	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterStdString(engine);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	r = engine->RegisterObjectType("scoped", 0, asOBJ_REF | asOBJ_SCOPED); assert( r >= 0 );
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterObjectBehaviour("scoped", asBEHAVE_FACTORY, "scoped @f()", asFUNCTION(Scoped_Factory), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("scoped", asBEHAVE_RELEASE, "void f()", asFUNCTION(Scoped_Release), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("scoped", "scoped @opNeg()", asFUNCTION(Scoped_Negate), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("scoped", "scoped &opAssign(const scoped &in)", asFUNCTION(Scoped_Assignment), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("scoped", "scoped @opAdd(int)", asFUNCTION(Scoped_Add), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
#else
	r = engine->RegisterObjectBehaviour("scoped", asBEHAVE_FACTORY, "scoped @f()", WRAP_FN(Scoped_Factory), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("scoped", asBEHAVE_RELEASE, "void f()", WRAP_OBJ_LAST(Scoped_Release), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("scoped", "scoped @opNeg()", WRAP_OBJ_LAST(Scoped_Negate), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("scoped", "scoped &opAssign(const scoped &in)", WRAP_OBJ_LAST(Scoped_Assignment), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("scoped", "scoped @opAdd(int)", WRAP_OBJ_FIRST(Scoped_Add), asCALL_GENERIC); assert( r >= 0 );
#endif

	// Enumerate the objects behaviours
	asIObjectType *ot = engine->GetObjectTypeById(engine->GetTypeIdByDecl("scoped"));
	if( ot->GetBehaviourCount() != 1 )
		TEST_FAILED;
	asEBehaviours beh;
	ot->GetBehaviourByIndex(0, &beh);
	if( beh != asBEHAVE_RELEASE )
		TEST_FAILED;

	// Must be possible to determine type id for scoped types with handle
	asIScriptFunction *func = ot->GetFactoryByIndex(0);
	int typeId = func->GetReturnTypeId();
	if( typeId != engine->GetTypeIdByDecl("scoped@") )
		TEST_FAILED;

	// Don't permit handles to be taken
	r = ExecuteString(engine, "scoped @s = null");
	if( r >= 0 ) TEST_FAILED;
	// TODO: The second message is a consequence of the first error, and should ideally not be shown
	if( sizeof(void*) == 4 )
	{
		if( bout.buffer != "ExecuteString (1, 8) : Error   : Object handle is not supported for this type\n"
						   "ExecuteString (1, 13) : Error   : Can't implicitly convert from '<null handle>' to 'scoped&'.\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}
	else
	{
		if( bout.buffer != "ExecuteString (1, 8) : Error   : Object handle is not supported for this type\n"
						   "ExecuteString (1, 13) : Error   : Can't implicitly convert from '<null handle>' to 'scoped&'.\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test a legal actions
	r = ExecuteString(engine, "scoped a");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;

	bout.buffer = "";
	r = ExecuteString(engine, "scoped s; scoped t = s + 10;");
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if( bout.buffer != "" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Test a compiler assert failure reported by Jeff Slutter on 2009-04-02
	bout.buffer = "";
	const char *script =
		"SetObjectPosition( GetWorldPositionByName() ); \n";

#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterGlobalFunction("const scoped @GetWorldPositionByName()", asFUNCTION(Scoped_Factory), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void SetObjectPosition(scoped &in)", asFUNCTION(Scoped_InRef), asCALL_CDECL); assert( r >= 0 );
#else
	r = engine->RegisterGlobalFunction("const scoped @GetWorldPositionByName()", WRAP_FN(Scoped_Factory), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void SetObjectPosition(scoped &in)", WRAP_FN(Scoped_InRef), asCALL_GENERIC); assert( r >= 0 );
#endif

	r = ExecuteString(engine, script);
	if( r != asEXECUTION_FINISHED ) TEST_FAILED;
	if( bout.buffer != "" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// It must be possible to include the scoped type as member in script class
	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", "class A { scoped s; }");
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	r = ExecuteString(engine, "A a; scoped s; a.s = s;", mod);
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	// Test saving/loading bytecode with a global scoped variable
	bout.buffer = "";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("test", "scoped g;");
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	CBytecodeStream stream("test");
	r = mod->SaveByteCode(&stream);
	if( r < 0 )
		TEST_FAILED;
	mod = engine->GetModule("b", asGM_ALWAYS_CREATE);
	r = mod->LoadByteCode(&stream);
	if( r < 0 )
		TEST_FAILED;

	// Don't permit functions to be registered with handle for parameters
	bout.buffer = "";
	r = engine->RegisterGlobalFunction("void f(scoped@)", asFUNCTION(DummyFunc), asCALL_GENERIC);
	if( r >= 0 ) TEST_FAILED;
	if( bout.buffer != "System function (1, 14) : Error   : Object handle is not supported for this type\n"
	                   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void f(scoped@)' (Code: -10)\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Don't permit functions to be registered to take type by reference (since that require handles)
	bout.buffer = "";
	r = engine->RegisterGlobalFunction("void f(scoped&)", asFUNCTION(DummyFunc), asCALL_GENERIC);
	if( r >= 0 ) TEST_FAILED;
	if( bout.buffer != "System function (1, 14) : Error   : Only object types that support object handles can use &inout. Use &in or &out instead\n"
	                   " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void f(scoped&)' (Code: -10)\n" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Permit &in
	r = engine->RegisterGlobalFunction("void f(scoped&in)", asFUNCTION(DummyFunc), asCALL_GENERIC);
	if( r < 0 ) TEST_FAILED;


	engine->Release();

	return fail;
}

///////////////////////////////
//

// TODO: This code should eventually be moved to an standard add-on, e.g. ScriptRegistrator

// AngelScript is pretty good at validating what is registered by the application, however,
// there are somethings that AngelScript just can't be aware of. This is an attempt to remedy
// that by adding extra validations through the use of templates.

// Template for getting information on types
template<typename T>
struct CTypeInfo
{
	static const char *GetTypeName()
	{
	    // Unknown type
#ifdef _MSC_VER
        // GNUC won't let us compile at all if this is here
	    int ERROR_UnknownType[-1];
#endif
	    return 0;
    };
	static bool IsReference() { return false; }
};

// Mapping the C++ type 'int' to the AngelScript type 'int'
template<>
struct CTypeInfo<int>
{
	static const char *GetTypeName() { return "int"; }
	static bool IsReference() { return false; }
};

template<>
struct CTypeInfo<int&>
{
	static const char *GetTypeName() { return "int"; }
	static bool IsReference() { return true; }
};

template<>
struct CTypeInfo<int*>
{
	static const char *GetTypeName() { return "int"; }
	static bool IsReference() { return true; }
};

// Mapping the C++ type 'std::string' to the AngelScript type 'string'
template<>
struct CTypeInfo<std::string>
{
	static const char *GetTypeName() { return "string"; }
	static bool IsReference() { return false; }
};

template<>
struct CTypeInfo<std::string&>
{
	static const char *GetTypeName() { return "string"; }
	static bool IsReference() { return true; }
};

template<>
struct CTypeInfo<std::string*>
{
	static const char *GetTypeName() { return "string@"; }
	static bool IsReference() { return false; }
};

template<>
struct CTypeInfo<std::string**>
{
	static const char *GetTypeName() { return "string@"; }
	static bool IsReference() { return true; }
};

template<>
struct CTypeInfo<std::string*&>
{
	static const char *GetTypeName() { return "string@"; }
	static bool IsReference() { return true; }
};

// Template for verifying a parameter
template<typename A1>
struct CParamValidator
{
	static int Validate(asIScriptFunction *descr, int arg)
	{
		asDWORD flags;
		int t1 = descr->GetParamTypeId(arg, &flags);
		int t2 = descr->GetEngine()->GetTypeIdByDecl(CTypeInfo<A1>::GetTypeName());
		if( t1 != t2 )
			return -1;

		// Verify if the type is properly declared as reference / non-reference
		if( flags == asTM_NONE && CTypeInfo<A1>::IsReference() )
			return -1;

		return 0;
	}
};

// Template for registering a function
template<typename A1>
int RegisterGlobalFunction(asIScriptEngine *e, const char *decl, void (*f)(A1), asDWORD callConv)
{
	int r = e->RegisterGlobalFunction(decl, asFUNCTION(f), callConv);
	assert( r >= 0 );

	if( r >= 0 )
	{
		// TODO: Can write messages to the message callback in the engine instead of testing through asserts

		asIScriptFunction *descr = e->GetFunctionById(r);

		// Verify the parameter count
		assert( descr->GetParamCount() == 1 );

		// Verify the parameter types
		assert( CParamValidator<A1>::Validate(descr, 0) >= 0 );
	}

	return r;
}

template<typename A1, typename A2>
int RegisterGlobalFunction(asIScriptEngine *e, const char *decl, void (*f)(A1, A2), asDWORD callConv)
{
	int r = e->RegisterGlobalFunction(decl, asFUNCTION(f), callConv);
	assert( r >= 0 );

	if( r >= 0 )
	{
		asIScriptFunction *descr = e->GetFunctionById(r);

		// Verify the parameter count
		assert( descr->GetParamCount() == 2 );

		// Verify the parameter types
		assert( CParamValidator<A1>::Validate(descr, 0) >= 0 );
		assert( CParamValidator<A2>::Validate(descr, 1) >= 0 );
	}

	return r;
}

void func1(int) {}
void func2(std::string &) {}
void func3(std::string *) {}
void func4(int *) {}
void func5(int &) {}
void func6(std::string **) {}
void func7(std::string *&) {}
void func8(int, std::string &) {}
void func9(std::string &, int) {}

bool TestHelper()
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		// Skipping this due to not supporting native calling conventions
		printf("Skipped due to AS_MAX_PORTABILITY\n");
		return false;
	}

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	RegisterScriptString(engine);

	// TODO: Add validation of return type

	RegisterGlobalFunction(engine, "void func1(int)", func1, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func2(string &in)", func2, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func3(string @)", func3, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func4(int &in)", func4, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func5(int &out)", func5, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func6(string @&out)", func6, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func7(string @&out)", func7, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func8(int, string &)", func8, asCALL_CDECL);
	RegisterGlobalFunction(engine, "void func9(string &, int)", func9, asCALL_CDECL);

	engine->Release();

	return fail;
}


//============================================

class CHandleType
{
public:
	CHandleType(asIScriptEngine *engine)
	{
		m_ref = 0;
		m_typeId = 0;
		m_engine = engine;
	}
	CHandleType(const CHandleType &o)
	{
		m_ref = 0;
		m_typeId = 0;
		m_engine = o.m_engine;
		opAssign(o.m_ref, o.m_typeId);
	}
	CHandleType(void *ref, int typeId)
	{
		m_ref = 0;
		m_typeId = 0;
		m_engine = asGetActiveContext()->GetEngine();
		opAssign(ref, typeId);
	}
	~CHandleType()
	{
		ReleaseHandle();
	}
	CHandleType &operator=(const CHandleType &o)
	{
		opAssign(o.m_ref, o.m_typeId);
		return *this;
	}

	CHandleType &opAssign(void *ref, int typeId)
	{
		ReleaseHandle();

		// When receiving a null handle we just clear our memory
		if( typeId == 0 )
			ref = 0;
		// Dereference handles
		if( typeId & asTYPEID_OBJHANDLE )
		{
			// Store the actual reference
			ref = *(void**)ref;
			typeId &= ~asTYPEID_OBJHANDLE;
		}

		m_ref    = ref;
		m_typeId = typeId;

		AddRefHandle();

		return *this;
	}

	bool opEquals(const CHandleType &o) const
	{
		if( m_ref == o.m_ref &&
			m_typeId == o.m_typeId )
			return true;

		// TODO: If typeId is not the same, we should attempt to do a dynamic cast,
		//       which may change the pointer for application registered classes

		return false;
	}

	bool opEquals(void *ref, int typeId) const
	{
		// Null handles are received as reference to a null handle
		if( typeId == 0 )
			ref = 0;

		// Dereference handles
		if( typeId & asTYPEID_OBJHANDLE )
		{
			// Compare the actual reference
			ref = *(void**)ref;
			typeId &= ~asTYPEID_OBJHANDLE;
		}

		// TODO: If typeId is not the same, we should attempt to do a dynamic cast,
		//       which may change the pointer for application registered classes

		if( ref == m_ref ) return true;

		return false;
	}

	// AngelScript: used as '@obj = cast<obj>(ref);'
	void opCast(void **outRef, int typeId)
	{
		// It is expected that the outRef is always a handle
		assert( typeId & asTYPEID_OBJHANDLE );

		// Compare the type id of the actual object
		typeId &= ~asTYPEID_OBJHANDLE;

		if( typeId != m_typeId )
		{
			// TODO: Should attempt a dynamic cast of the stored handle to the requested handle

			*outRef = 0;
			return;
		}

		// Must increase the ref count
		AddRefHandle();

		*outRef = m_ref;
	}

	void ReleaseHandle()
	{
		if( m_ref )
		{
			m_engine->ReleaseScriptObject(m_ref, m_engine->GetObjectTypeById(m_typeId));

			m_ref = 0;
			m_typeId = 0;
		}
	}

	void AddRefHandle()
	{
		if( m_ref )
		{
			m_engine->AddRefScriptObject(m_ref, m_engine->GetObjectTypeById(m_typeId));
		}
	}

	static void Construct(CHandleType *self) { asIScriptEngine *engine = asGetActiveContext()->GetEngine(); new(self) CHandleType(engine); }
	static void Construct(CHandleType *self, const CHandleType &o) { new(self) CHandleType(o); }
	static void Construct(CHandleType *self, void *r, int t) { new(self) CHandleType(r, t); }
	static void Destruct(CHandleType *self) { self->~CHandleType(); }

	void *m_ref;
	int m_typeId;
	asIScriptEngine *m_engine;
};

void CHandleType_ConstructVar_Generic(asIScriptGeneric *gen)
{
	void *ref = gen->GetArgAddress(0);
	int typeId = gen->GetArgTypeId(0);
	CHandleType *self = reinterpret_cast<CHandleType*>(gen->GetObject());
	CHandleType::Construct(self, ref, typeId);
}

void CHandleType_AssignVar_Generic(asIScriptGeneric *gen)
{
	void *ref = gen->GetArgAddress(0);
	int typeId = gen->GetArgTypeId(0);
	CHandleType *self = reinterpret_cast<CHandleType*>(gen->GetObject());
	self->opAssign(ref, typeId);
	gen->SetReturnAddress(self);
}

void CHandleType_EqualsVar_Generic(asIScriptGeneric *gen)
{
	void *ref = gen->GetArgAddress(0);
	int typeId = gen->GetArgTypeId(0);
	CHandleType *self = reinterpret_cast<CHandleType*>(gen->GetObject());
	gen->SetReturnByte(self->opEquals(ref, typeId));
}

void CHandleType_Cast_Generic(asIScriptGeneric *gen)
{
	void **ref = reinterpret_cast<void**>(gen->GetArgAddress(0));
	int typeId = gen->GetArgTypeId(0);
	CHandleType *self = reinterpret_cast<CHandleType*>(gen->GetObject());
	self->opCast(ref, typeId);
}

bool TestHandleType()
{
	bool fail = false;
	asIScriptEngine *engine;
	int r;
	CBufferedOutStream bout;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

	r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectType("ref", sizeof(CHandleType), asOBJ_VALUE | asOBJ_ASHANDLE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(CHandleType::Construct, (CHandleType *), void), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ref &in)", asFUNCTIONPR(CHandleType::Construct, (CHandleType *, const CHandleType &), void), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ? &in)", asFUNCTIONPR(CHandleType::Construct, (CHandleType *, void *, int), void), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_DESTRUCT, "void f()", asFUNCTIONPR(CHandleType::Destruct, (CHandleType *), void), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "ref &opAssign(const ref &in)", asMETHOD(CHandleType, operator=), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "ref &opAssign(const ?&in)", asMETHOD(CHandleType, opAssign), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "bool opEquals(const ref &in) const", asMETHODPR(CHandleType, opEquals, (const CHandleType &) const, bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "bool opEquals(const ?&in) const", asMETHODPR(CHandleType, opEquals, (void*, int) const, bool), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_REF_CAST, "void f(?&out)", asMETHODPR(CHandleType, opCast, (void **, int), void), asCALL_THISCALL); assert( r >= 0 );
#else
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f()", WRAP_OBJ_FIRST_PR(CHandleType::Construct, (CHandleType *), void), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ref &in)", WRAP_OBJ_FIRST_PR(CHandleType::Construct, (CHandleType *, const CHandleType &), void), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_CONSTRUCT, "void f(const ? &in)", asFUNCTION(CHandleType_ConstructVar_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_DESTRUCT, "void f()", WRAP_OBJ_FIRST_PR(CHandleType::Destruct, (CHandleType *), void), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "ref &opAssign(const ref &in)", WRAP_MFN(CHandleType, operator=), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "ref &opAssign(const ?&in)", asFUNCTION(CHandleType_AssignVar_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "bool opEquals(const ref &in) const", WRAP_MFN_PR(CHandleType, opEquals, (const CHandleType &) const, bool), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("ref", "bool opEquals(const ?&in) const", asFUNCTION(CHandleType_EqualsVar_Generic), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("ref", asBEHAVE_REF_CAST, "void f(?&out)", asFUNCTION(CHandleType_Cast_Generic), asCALL_GENERIC); assert( r >= 0 );
#endif

	// It must be possible to use the is and !is operators on the handle type
	// It must be possible to use handle assignment on the handle type
	// It must be possible to do a cast on the handle type to get the real handle
	const char *script = "class A {} \n"
		                 "class B {} \n"
						 "void main() \n"
						 "{ \n"
						 "  ref@ ra, rb; \n"
						 "  A a; B b; \n"
						 // Assignment of reference
						 "  @ra = @a; \n"
						 "  assert( ra is a ); \n"
						 "  @rb = @b; \n"
						 // Casting to reference
						 "  A@ ha = cast<A>(ra); \n"
						 "  assert( ha !is null ); \n"
						 "  B@ hb = cast<B>(ra); \n"
						 "  assert( hb is null ); \n"
						 // Assigning null, and comparing with null
						 "  @ra = null; \n"
						 "  assert( ra is null ); \n"
						 "  func2(ra); \n"
						 // Handle assignment with explicit handle
						 "  @ra = @rb; \n"
						 "  assert( ra is b ); \n"
						 "  assert( rb is b ); \n"
						 "  assert( ra is rb ); \n"
						 // Handle assignment with implicit handle
						 "  @rb = rb; \n"
						 "  assert( rb is b ); \n"
						 "  assert( ra is rb ); \n"
						 // Function call and return value
						 "  @rb = func(rb); \n"
						 "  assert( rb is b ); \n"
						 "  assert( func(rb) is b ); \n"
						 // Global variable
						 "  @g = @b; \n"
						 "  assert( g is b ); \n"
						 // Assignment to reference
						 "  @func3() = @a; \n"
						 "  assert( g is a ); \n"
						 "  assert( func3() is a ); \n"
						 // Copying the reference
						 "  ref@ rc = rb; \n"
						 "  assert( rc is rb ); \n"
						 "} \n"
						 "ref@ func(ref@ r) { return r; } \n"
						 "void func2(ref@r) { assert( r is null ); } \n"
						 "ref@ g; \n"
						 "ref@ g2 = g; \n"
						 "ref@& func3() { return g; } \n";
	asIScriptModule *mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;
	if( bout.buffer != "" )
	{
		printf("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	CHandleType *ptr = (CHandleType*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByName("g"));
	if( !ptr || ptr->m_engine != engine )
		TEST_FAILED;
	ptr = (CHandleType*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByName("g2"));
	if( !ptr || ptr->m_engine != engine )
		TEST_FAILED;


	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "main()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		if( r == asEXECUTION_EXCEPTION )
			PrintException(ctx);
		TEST_FAILED;
	}
	ctx->Release();


	{
		// TODO: Should require the ASHANDLE to be declared with @
		//       Currently it doesn't matter whether it is declared with or without
		// This type must always be passed as if it is a handle to arguments, i.e. ref @func(ref @);
		script = "ref func() { return null; } \n" // ERROR
			     "void func2(ref a) {} \n"        // ERROR
  			 	 // It must also be declared as handle in variables, globals, and members.
				 "ref globl; \n"                  // ERROR
				 "void main() \n"
				 "{ \n"
				 "  ref a; \n"                    // ERROR
				 "} \n"
				 "class T { ref a; } \n"          // ERROR
				 // It must not be allowed to do a value assignment on the type
				 "class S {} \n"
				 "void test() \n"
				 "{ \n"
                 "  ref @r; \n"
				 "  S s; \n"
				 "  r = s; \n"                    // ERROR
				 "} \n";

		bout.buffer = "";
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		if( bout.buffer != //"script (3, 1) : Error   : Data type can't be 'ref'\n"
		                   //"script (8, 11) : Error   : Data type can't be 'ref'\n"
		                   //"script (1, 1) : Info    : Compiling ref func()\n"
		                   //"script (1, 1) : Error   : Data type can't be 'ref'\n"
		                   //"script (1, 21) : Error   : Can't implicitly convert from '<null handle>' to 'ref'.\n"
		                   //"script (2, 1) : Info    : Compiling void func2(ref)\n"
		                   //"script (2, 12) : Error   : Parameter type can't be 'ref'\n"
		                   //"script (4, 1) : Info    : Compiling void main()\n"
		                   //"script (6, 7) : Error   : Data type can't be 'ref'\n"
		                   "script (10, 1) : Info    : Compiling void test()\n"
		                   "script (14, 3) : Error   : Illegal operation on 'ref'\n" )
		{
			printf("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	engine->Release();


	return fail;
}

///////////////////////////////////

// This is the relevant part of the type from the Irrlicht engine.
// It is reproduced here to test the registration of the
// type with AngelScript.

template <class T>
class dimension2d
{
	public:
		dimension2d() : Width(0), Height(0) {}
		dimension2d(const T& width, const T& height) :
			Width(width), Height(height) {}

		template <class U>
		explicit dimension2d(const dimension2d<U>& other) :
			Width((T)other.Width), Height((T)other.Height) { }

		template <class U>
		dimension2d<T>& operator=(const dimension2d<U>& other)
		{
			Width = (T) other.Width;
			Height = (T) other.Height;
			return *this;
		}


		dimension2d<T> operator+(const dimension2d<T>& other) const
		{
			return dimension2d<T>(Width+other.Width, Height+other.Height);
		}

		T Width;
		T Height;
};

typedef dimension2d<float> dimension2df;

void Construct_dim2f(dimension2df *mem)
{
	new(mem) dimension2df();
}

void Construct_dim2f(const dimension2df &other, dimension2df *mem)
{
	new(mem) dimension2df(other);
}

void Construct_dim2f(float x, float y, dimension2df *mem)
{
	new(mem) dimension2df(x, y);
}

void ByValue(dimension2df val)
{
	assert( fabsf(val.Width - 1.0f) <= 0.0001f );
	assert( fabsf(val.Height - 2.0f) <= 0.0001f );
}

bool TestIrrTypes()
{
	bool fail = false;
	int r;
	COutStream out;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

	// The dimension2df type must be registered with asOBJ_APP_CLASS_C, 
	// despite it having an assignment and copy constructor. It must also 
	// be registered with asOBJ_APP_CLASS_FLOATS to work on Linux 64bit
	r = engine->RegisterObjectType("dim2f", sizeof(dimension2df), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_C | asOBJ_APP_CLASS_ALLFLOATS); assert( r >= 0 );
#ifndef AS_MAX_PORTABILITY
	r = engine->RegisterObjectBehaviour("dim2f",asBEHAVE_CONSTRUCT,"void f()", asFUNCTIONPR(Construct_dim2f, (dimension2df*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("dim2f",asBEHAVE_CONSTRUCT,"void f(const dim2f &in)", asFUNCTIONPR(Construct_dim2f, (const dimension2df &, dimension2df*), void),asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("dim2f",asBEHAVE_CONSTRUCT,"void f(float x, float y)", asFUNCTIONPR(Construct_dim2f, (float x, float y, dimension2df*), void),asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("dim2f", "dim2f &opAssign(const dim2f &in)",     asMETHODPR(dimension2df, operator =, (const dimension2df&), dimension2df&),      asCALL_THISCALL); assert(r >= 0);
	r = engine->RegisterObjectMethod("dim2f", "dim2f opAdd(const dim2f &in) const",   asMETHODPR(dimension2df, operator+,  (const dimension2df&) const, dimension2df), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void ByValue(dim2f)", asFUNCTION(ByValue), asCALL_CDECL); assert( r >= 0 );
#else
	r = engine->RegisterObjectBehaviour("dim2f",asBEHAVE_CONSTRUCT,"void f()", WRAP_OBJ_LAST_PR(Construct_dim2f, (dimension2df*), void), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("dim2f",asBEHAVE_CONSTRUCT,"void f(const dim2f &in)", WRAP_OBJ_LAST_PR(Construct_dim2f, (const dimension2df &, dimension2df*), void),asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("dim2f",asBEHAVE_CONSTRUCT,"void f(float x, float y)", WRAP_OBJ_LAST_PR(Construct_dim2f, (float x, float y, dimension2df*), void),asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterObjectMethod("dim2f", "dim2f &opAssign(const dim2f &in)",     WRAP_MFN_PR(dimension2df, operator =, (const dimension2df&), dimension2df&),      asCALL_GENERIC); assert(r >= 0);
	r = engine->RegisterObjectMethod("dim2f", "dim2f opAdd(const dim2f &in) const",   WRAP_MFN_PR(dimension2df, operator+,  (const dimension2df&) const, dimension2df), asCALL_GENERIC); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void ByValue(dim2f)", WRAP_FN(ByValue), asCALL_GENERIC); assert( r >= 0 );
#endif
	r = engine->RegisterObjectProperty("dim2f", "float x", asOFFSET(dimension2df, Width)); assert( r >= 0 );
	r = engine->RegisterObjectProperty("dim2f", "float y", asOFFSET(dimension2df, Height)); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

	r = ExecuteString(engine, "dim2f video_res(800,600);\n"
		                      "dim2f total_res;\n"
							  "total_res = video_res + video_res;\n"
							  "assert( total_res.x == 1600 && total_res.y == 1200 );\n"
							  "ByValue(dim2f(1,2)); \n");
	if( r != asEXECUTION_FINISHED )
	{
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

} // namespace

