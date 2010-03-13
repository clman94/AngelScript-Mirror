//
// This test was designed to test the asOBJ_CLASS_A flag with cdecl
//
// Author: Andreas Jonsson
//

#include "utils.h"

static const char * const TESTNAME = "TestCDecl_ClassA";

class ClassA1
{
public:
	ClassA1 &operator=(ClassA1 &other) {a = other.a; return *this;}
	unsigned long a;
};

class ClassA2
{
public:
	ClassA2 &operator=(ClassA2 &other) {a = other.a; b = other.b; return *this;}
	unsigned long a;
	unsigned long b;
};

class ClassA3
{
public:
	ClassA3 &operator=(ClassA3 &other) {a = other.a; b = other.b; c = other.c; return *this;}
	unsigned long a;
	unsigned long b;
	unsigned long c;
};

static ClassA1 classA1()
{
	ClassA1 c = {0xDEADC0DE};
	return c;
}

static ClassA2 classA2()
{
	ClassA2 c = {0xDEADC0DE, 0x01234567};
	return c;
}

static ClassA3 classA3()
{
	ClassA3 c = {0xDEADC0DE, 0x01234567, 0x89ABCDEF};
	return c;
}

static ClassA1 c1;
static ClassA2 c2;
static ClassA3 c3;

static void class1ByVal(ClassA1 c)
{
	assert( c.a == 0xDEADC0DE );
}

static void class2ByVal(ClassA2 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 ); 
}

static void class3ByVal(ClassA3 c)
{
	assert( c.a == 0xDEADC0DE && c.b == 0x01234567 && c.c == 0x89ABCDEF );
}

bool TestCDecl_ClassA()
{
	if( strstr(asGetLibraryOptions(), "AS_MAX_PORTABILITY") )
	{
		printf("%s: Skipped due to AS_MAX_PORTABILITY\n", TESTNAME);
		return false;
	}

	bool fail = false;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	engine->RegisterObjectType("class1", sizeof(ClassA1), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_A);
	engine->RegisterObjectType("class2", sizeof(ClassA2), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_A);
	engine->RegisterObjectType("class3", sizeof(ClassA3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_A);
	
	engine->RegisterGlobalProperty("class1 c1", &c1);
	engine->RegisterGlobalProperty("class2 c2", &c2);
	engine->RegisterGlobalProperty("class3 c3", &c3);

	engine->RegisterGlobalFunction("class1 _class1()", asFUNCTION(classA1), asCALL_CDECL);
	engine->RegisterGlobalFunction("class2 _class2()", asFUNCTION(classA2), asCALL_CDECL);
	engine->RegisterGlobalFunction("class3 _class3()", asFUNCTION(classA3), asCALL_CDECL);

	COutStream out;

	c1.a = 0;

	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	int r = ExecuteString(engine, "c1 = _class1();");
	if( r < 0 )
	{
		printf("%s: ExecuteString() failed %d\n", TESTNAME, r);
		fail = true;
	}

	if( c1.a != 0xDEADC0DE )
	{
		printf("%s: Failed to assign object returned from function. c1.a = %X\n", TESTNAME, c1.a);
		fail = true;
	}


	c2.a = 0;
	c2.b = 0;

	r = ExecuteString(engine, "c2 = _class2();");
	if( r < 0 )
	{
		printf("%s: ExecuteString() failed %d\n", TESTNAME, r);
		fail = true;
	}

	if( c2.a != 0xDEADC0DE )
	{
		printf("%s: Failed to assign object returned from function. c2.a = %X\n", TESTNAME, c2.a);
		fail = true;
	}

	if( c2.b != 0x01234567 )
	{
		printf("%s: Failed to assign object returned from function. c2.b = %X\n", TESTNAME, c2.b);
		fail = true;
	}

	c3.a = 0;
	c3.b = 0;
	c3.c = 0;

	r = ExecuteString(engine, "c3 = _class3();");
	if( r < 0 )
	{
		printf("%s: ExecuteString() failed %d\n", TESTNAME, r);
		fail = true;
	}

	if( c3.a != 0xDEADC0DE )
	{
		printf("%s: Failed to assign object returned from function. c3.a = %X\n", TESTNAME, c3.a);
		fail = true;
	}

	if( c3.b != 0x01234567 )
	{
		printf("%s: Failed to assign object returned from function. c3.b = %X\n", TESTNAME, c3.b);
		fail = true;
	}

	if( c3.c != 0x89ABCDEF )
	{
		printf("%s: Failed to assign object returned from function. c3.c = %X\n", TESTNAME, c3.c);
		fail = true;
	}

	// Test passing the object types by value to a system function
	// TODO: Why isn't this available on 64bit Linux?
	if ( !strstr( asGetLibraryOptions(), "AS_X64_GCC" ) )
	{
		r = engine->RegisterGlobalFunction("void class1ByVal(class1)", asFUNCTION(class1ByVal), asCALL_CDECL); assert( r >= 0 );
		r = ExecuteString(engine, "class1 c = _class1(); class1ByVal(c)");
		if( r != asEXECUTION_FINISHED )
			fail = true;

		r = engine->RegisterGlobalFunction("void class2ByVal(class2)", asFUNCTION(class2ByVal), asCALL_CDECL); assert( r >= 0 );
		r = ExecuteString(engine, "class2 c = _class2(); class2ByVal(c)");
		if( r != asEXECUTION_FINISHED )
			fail = true;

		r = engine->RegisterGlobalFunction("void class3ByVal(class3)", asFUNCTION(class3ByVal), asCALL_CDECL); assert( r >= 0 );
		r = ExecuteString(engine, "class3 c = _class3(); class3ByVal(c)");
		if( r != asEXECUTION_FINISHED )
			fail = true;
	}

	engine->Release();

	return fail;
}
