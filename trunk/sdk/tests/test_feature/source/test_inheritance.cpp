#include "utils.h"

namespace TestInheritance
{

bool Test2();

bool Test()
{
	bool fail = false;
	int r;

	asIScriptModule *mod = 0;
	COutStream out;
	CBufferedOutStream bout;
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	// Test that it is possible to declare a class that inherits from another
	// Test that the inherited properties are available in the derived class
	// Test that the inherited methods are available in the derived class
	// Test that it is possible to override the inherited methods
	const char *script =
		"class Base                             \n"
		"{                                      \n"
		"  int a;                               \n"
		"  void f1() { a = 1; }                 \n"
		"  void f2() { a = 0; }                 \n"
		"}                                      \n"
		"class Derived : Base                   \n"
		"{                                      \n"
		   // overload f2()
		"  void f2() { a = 2; }                 \n"
		"  void func()                          \n"
		"  {                                    \n"
		     // call Base::f1()
		"    f1();                              \n"
		"    assert(a == 1);                    \n"
		     // call Derived::f2()
		"    f2();                              \n"
		"    assert(a == 2);                    \n"
		"  }                                    \n"
		"}                                      \n";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script);
	r = mod->Build();
	if( r < 0 )
	{
		fail = true;
	}

	asIScriptStruct *obj = (asIScriptStruct*)engine->CreateScriptObject(mod->GetTypeIdByDecl("Derived"));
	asIScriptContext *ctx = engine->CreateContext();
	ctx->Prepare(obj->GetObjectType()->GetMethodIdByDecl("void func()"));
	ctx->SetObject(obj);
	r = ctx->Execute();
	if( r != asEXECUTION_FINISHED )
	{
		fail = true;
	}
	ctx->Release();
	obj->Release();

	// Test that implicit cast from derived to base is working
	r = engine->ExecuteString(0, "Derived d; Base @b = @d; assert( b !is null );");
	if( r != asEXECUTION_FINISHED )
	{
		fail = true;
	}

	// Test that cast from base to derived require explicit cast
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	bout.buffer = "";
	r = engine->ExecuteString(0, "Base b; Derived @d = @b;");
	if( r >= 0 )
	{
		fail = true;
	}
	if( bout.buffer != "ExecuteString (1, 22) : Error   : Can't implicitly convert from 'Base@' to 'Derived@&'.\n" )
	{
		fail = true;
		printf(bout.buffer.c_str());
	}

	// Test that it is possible to explicitly cast to derived class
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	r = engine->ExecuteString(0, "Derived d; Base @b = @d; assert( cast<Derived>(b) !is null );");
	if( r != asEXECUTION_FINISHED )
	{
		fail = true;
	}

	// Test polymorphing
	r = engine->ExecuteString(0, "Derived d; Base @b = @d; b.a = 3; b.f2(); assert( b.a == 2 );");
	if( r != asEXECUTION_FINISHED )
	{
		fail = true;
	}

	// Test polymorphing with pre-compiled bytecode
	// Must make sure that the inheritance path is stored/restored with the saved byte code
	{
		CBytecodeStream stream;
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
		{
			fail = true;
		}

		asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&stream);
		if( r < 0 )
		{
			fail = true;
		}

		r = engine->ExecuteString("2", "Derived d; Base @b = @d; b.a = 3; b.f2(); assert( b.a == 2 );");
		if( r != asEXECUTION_FINISHED )
		{
			fail = true;
		}

		// Both modules should have the same number of functions
		if( mod->GetFunctionCount() != mod2->GetFunctionCount() )
		{
			fail = true;

			asUINT n;
			printf("First module's functions\n");
			for( n = 0; n < (asUINT)mod->GetFunctionCount(); n++ )
			{
				asIScriptFunction *f = mod->GetFunctionDescriptorByIndex(n);
				printf("%s\n", f->GetDeclaration());
			}
			printf("\nSecond module's functions\n");
			for( n = 0; n < (asUINT)mod2->GetFunctionCount(); n++ )
			{
				asIScriptFunction *f = mod2->GetFunctionDescriptorByIndex(n);
				printf("%s\n", f->GetDeclaration());
			}
		}

		engine->DiscardModule("2");
	}

	// TODO: Base class' destructor must be called when object is destroyed

	// TODO: Test that it is possible to call the base class' constructor
	//       The constructor can be called with the keyword super(args);

	// TODO: Test that it is possible to call base class methods from within overridden methods in derived class 
	//       Requires scope operator

	// TODO: If the derived class doesn't declare a default constructor, should the base class' default constructor be called?

	// TODO: Implement support for initialization list for the constructor, where the base class' constructor can be invoked from the initialization list

	// TODO: If the base class is garbage collected, then the derived class must also be garbage collected

	// TODO: Can a derived class introduce new reference cycles involving the base class? I.e. are there any 
	//       situations where the base class wouldn't be garbage collected, unless the derived class is implemented?

	// TODO: not related to inheritance, but it should be possible to call another constructor from within a constructor. We can follow D's design of using this(args) to call the constructor

	engine->Release();

	fail = Test2() || fail;

	// Success
	return fail;
}

bool Test2()
{
	bool fail = false;
	CBufferedOutStream bout;
	int r;
	asIScriptModule *mod;
	asIScriptEngine *engine;
	const char *script;

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
	RegisterScriptString(engine);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

	// Test that it is not possible to inherit from application registered type
	script = "class A : string {} \n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		fail = true;
	if( bout.buffer != "script (1, 11) : Error   : Can't inherit from 'string'\n" )
	{
		fail = true;
		printf(bout.buffer.c_str());
	}

	// Test that it is not possible to inherit from multiple script classes
	script = "class B {} class C {} class D {} class A : B, C, D {} \n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		fail = true;
	if( bout.buffer != "script (1, 47) : Error   : Can't inherit from multiple classes\n" )
	{
		fail = true;
		printf(bout.buffer.c_str());
	}

	// Test that it is not possible to inherit from a class that in turn inherits from this class
	script = "class A : C {} class B : A {} class C : B {}\n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		fail = true;
	if( bout.buffer != "script (1, 41) : Error   : Can't inherit from itself, or another class that inherits from this class\n" )
	{
		fail = true;
		printf(bout.buffer.c_str());
	}

	// Test that derived classes can't overload properties
	// TODO: In C++ it is possible to overload properties, in which case the base class property is hidden. Should we adopt this for AngelScript too?
	script = "class A { int a; } class B : A { double a; }\n";

	mod->AddScriptSection("script", script);
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		fail = true;
	// TODO: The error should explain that the original property is from the base class
	if( bout.buffer != "script (1, 41) : Error   : Name conflict. 'a' is an object property.\n" )
	{
		fail = true;
		printf(bout.buffer.c_str());
	}

	engine->Release();

	return fail;
}

} // namespace

