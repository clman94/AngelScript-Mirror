/*
   AngelCode Scripting Library
   Copyright (c) 2003-2005 Andreas J�nsson

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

   Andreas J�nsson
   andreas@angelcode.com
*/


//
// as_compiler.h
//
// The class that does the actual compilation of the functions
//



#ifndef AS_COMPILER_H
#define AS_COMPILER_H

#include "as_builder.h"
#include "as_scriptfunction.h"
#include "as_variablescope.h"
#include "as_bytecode.h"
#include "as_array.h"
#include "as_datatype.h"
#include "as_types.h"
#include "as_typeinfo.h"

struct asSDeferredOutParam
{
	asCScriptNode *argNode;
	asCTypeInfo    argType;
	int            argInOutFlags;
};

struct asSExprContext
{
	asCByteCode bc;
	asCTypeInfo type;
	asCArray<asSDeferredOutParam> deferredOutParams;

	~asSExprContext() { assert( deferredOutParams.GetLength() == 0 ); }
};

class asCCompiler
{
public:
	asCCompiler();
	~asCCompiler();

	int CompileFunction(asCBuilder *builder, asCScriptCode *script, asCScriptNode *func);
	int CompileGlobalVariable(asCBuilder *builder, asCScriptCode *script, asCScriptNode *expr, sGlobalVariableDescription *gvar);

	asCByteCode byteCode;
	asCArray<asCObjectType*> objVariableTypes;
	asCArray<int> objVariablePos;

protected:
	friend class asCBuilder;

	void Reset(asCBuilder *builder, asCScriptCode *script);

	// Statements
	void CompileStatementBlock(asCScriptNode *block, bool ownVariableScope, bool *hasReturn, asCByteCode *bc);
	void CompileDeclaration(asCScriptNode *decl, asCByteCode *bc);
	void CompileStatement(asCScriptNode *statement, bool *hasReturn, asCByteCode *bc);
	void CompileIfStatement(asCScriptNode *node, bool *hasReturn, asCByteCode *bc);
	void CompileSwitchStatement(asCScriptNode *node, bool *hasReturn, asCByteCode *bc);
	void CompileCase(asCScriptNode *node, asCByteCode *bc);
	void CompileForStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileWhileStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileDoWhileStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileBreakStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileContinueStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileReturnStatement(asCScriptNode *node, asCByteCode *bc);
	void CompileExpressionStatement(asCScriptNode *node, asCByteCode *bc);

	// Expressions
	void CompileAssignment(asCScriptNode *expr, asSExprContext *out);
	void CompileCondition(asCScriptNode *expr, asSExprContext *out);
	void CompileExpression(asCScriptNode *expr, asSExprContext *out);
	void CompilePostFixExpression(asCArray<asCScriptNode *> *postfix, asSExprContext *out);
	void CompileExpressionTerm(asCScriptNode *node, asSExprContext *out);
	void CompileExpressionPreOp(asCScriptNode *node, asSExprContext *out);
	void CompileExpressionPostOp(asCScriptNode *node, asSExprContext *out);
	void CompileExpressionValue(asCScriptNode *node, asSExprContext *out);
	void CompileFunctionCall(asCScriptNode *node, asSExprContext *out, asCObjectType *objectType, bool objIsConst);
	void CompileConversion(asCScriptNode *node, asSExprContext *out);
	void CompileOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileOperatorOnHandles(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileMathOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileBitwiseOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileComparisonOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	void CompileBooleanOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);
	bool CompileOverloadedOperator(asCScriptNode *node, asSExprContext *l, asSExprContext *r, asSExprContext *out);

	void DefaultConstructor(asCByteCode *bc, asCDataType &dt);
	void CompileConstructor(asCDataType &type, int offset, asCByteCode *bc);
	void CompileDestructor(asCDataType &type, int offset, asCByteCode *bc);
	void CompileArgumentList(asCScriptNode *node, asCArray<asSExprContext *> &args, asCDataType *type = 0);
	void MatchFunctions(asCArray<int> &funcs, asCArray<asCTypeInfo> &argTypes, asCScriptNode *node, const char *name, bool isConstMethod = false);

	// Helper functions
	void SwapPostFixOperands(asCArray<asCScriptNode *> &postfix, asCArray<asCScriptNode *> &target);
	void PrepareTemporaryObject(asCScriptNode *node, asSExprContext *ctx);
	void PrepareOperand(asSExprContext *ctx, asCScriptNode *node);
	void PrepareForAssignment(asCDataType *lvalue, asCTypeInfo *rvalue, asCByteCode *bc, asCScriptNode *node);
	void PerformAssignment(asCTypeInfo *lvalue, asCByteCode *bc, asCScriptNode *node);
	bool IsVariableInitialized(asCTypeInfo *type, asCScriptNode *node);
	void Dereference(asSExprContext *ctx, bool generateCode);
	void ImplicitConversion(asCByteCode *bc, const asCDataType &to, asCTypeInfo *from, asCScriptNode *node, bool isExplicit);
	void ImplicitConversionConstant(asCByteCode *bc, const asCDataType &to, asCTypeInfo *from, asCScriptNode *node, bool isExplicit);
	int  MatchArgument(asCArray<int> &funcs, asCArray<int> &matches, asCTypeInfo *argType, int paramNum);
	void PerformFunctionCall(int funcID, asSExprContext *out, bool isConstructor = false, asCArray<asCTypeInfo> *argTypes = 0, asCScriptNode *argListNode = 0);
	void MoveArgsToStack(int funcID, asCByteCode *bc, asCArray<asSExprContext *> &args, bool addOneToOffset);
	void PrepareFunctionCall(int funcID, asCScriptNode *argListNode, asCByteCode *bc, asCArray<asSExprContext *> &args);
	void AfterFunctionCall(int funcID, asCScriptNode *argListNode, asSExprContext *ctx, asCArray<asCTypeInfo> &argTypes);
	void ProcessDeferredOutParams(asSExprContext *ctx);
	void PrepareArgument(asCDataType *paramType, asSExprContext *ctx, asCScriptNode *node, bool isFunction = false, bool isInRef = false);
	bool IsLValue(asCTypeInfo &type);
	void DetermineDefaultArrayArgs(asCDataType &type, asDWORD *subType, asDWORD *size_arrayType);
	void DoAssignment(asSExprContext *out, asSExprContext *lctx, asSExprContext *rctx, asCScriptNode *lexpr, asCScriptNode *rexpr, int op, asCScriptNode *opNode);
	void MergeExprContexts(asSExprContext *before, asSExprContext *after);
	void FilterConst(asCArray<int> &funcs);

	void LineInstr(asCByteCode *bc, int pos);

	int  RegisterConstantBStr(const char *str, int len);
	int  GetPrecedence(asCScriptNode *op);

	void Error(const char *msg, asCScriptNode *node);
	void Warning(const char *msg, asCScriptNode *node);

	void AddVariableScope(bool isBreakScope = false, bool isContinueScope = false);
	void RemoveVariableScope();

	bool hasCompileErrors;

	int nextLabel;

	asCVariableScope *variables;
	asCBuilder *builder;
	asCScriptEngine *engine;
	asCScriptCode *script;

	asCArray<int> breakLabels;
	asCArray<int> continueLabels;

	int AllocateVariable(const asCDataType &type, bool isTemporary);
	int AllocateVariableNotIn(const asCDataType &type, bool isTemporary, asCArray<int> &vars);
	int GetVariableOffset(int varIndex);
	int GetVariableSlot(int varOffset);
	void DeallocateVariable(int pos);
	void ReleaseTemporaryVariable(asCTypeInfo &t, asCByteCode *bc);

	asCArray<asCDataType> variableAllocations;
	asCArray<int> freeVariables;
	asCArray<int> tempVariables;

	bool globalExpression;
	bool isProcessingDeferredOutParams;
	int noCodeOutput;
};

#endif
