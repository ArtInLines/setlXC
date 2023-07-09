# Overview

1. Tokenize the Source File (String -> Token[])
2. Parse the Token-Stream (Token[] -> AST)
3. Include imported files (AST -> AST)
4. Verify the AST (AST -> ScopeData)
5. Create IR (ScopeData -> IR)
6. Write IR to LLVM (IR -> String)

## 1. Tokenization

-   Completed for entire desired Syntax of SetlX: The Compiler can tokenize itself.
-   The list of supported tokens can be found as static lists in the `global` variable
-   Each token consists of the token itself as well as a start and end location in the file
-   Some tokens also contain data (an Int contains the associated integer for example)

```c
typedef struct loc {
	string file;
	uint   row;
	uint   col;
} loc;

typedef struct token {
	string  type;
	loc     start;
	loc     end;
	string *data;  // Can be om
} token;
```

## 2. Pasing

-   Partially completed

Linearly go through Tokens with a simple index. Decide what parsing function to call based on the current token(s) (e.g. paseStmt, parseExpr, etc.). These functions often call each other recursively.

Many functions make certain implicit assumptions (especially about the positon of the index), which are documented on a per-function basis.

The AST is defined as follows:

```c
typedef enum nodeType { // Represented as strings in SetlX
	Block,    // Args: List of Statements in block
	Ret,      // Args: Expr to return
	Assign,   // Data: Iden/Index/Property; Args: Expr for value
	BinOp,    // Data: operator; Args: left Expr, right Expr
	UnaryOp,  // Data: operaor; Args: operand
	Range,    // Args: Expr to range on, first & second range parameter (each could be om) @Note: Both can't be om, as SetlX doesn't accept that @Note: If the parameter is om, it can be subsituted without semantic change: for the first with 1, for the second with #(expr)
	Index,    // Args: Expr to index, parameter for indexing
	Call,     // Data: Iden/Index/Property to call; Args: List of arguments
	Property, // Data: Iden/Index/Property to access property of; Args: Iden of property to access
	ArgList,  // Args: List of Idens and Assignments
	Proc,     // Args: ArgList, Block
	If,       // Args: Condition, Block
	Else,     // Args: Condition, Block for else-if otherwise just Block
	Class,    // Data: Class name (Iden); Args: ArgList, Block
	Static,   // Args: Block
	For,      // Data: variable name (Iden); Args: Expr for collection, Block
	While,    // Args: Condition, Block
	DoWhile,  // Args: Condition, Block
	Set,      // Args: List of Expr
	List,     // Args: List of Expr
	Str,      // Data: String
	Int,      // Data: String of Int
	Float,    // Data: String of Float
	Iden,	  // Data: String
} nodeType;

typedef struct type {
	// @Incomplete
} type;

typedef struct astNode {
	nodeType  node;
	loc       start;
	loc       end;
	astNode  *data;   // Single data point, usually an Identifier/Index/Property - can be om
	astNode   args[]; // List of arguments to the astNode. Depends on the type of the node (see nodeType definition above)
	type     *t;      // Is om before verification phase
} astNode;
```

## 3. Imports

<!-- @Study Can included files reference variables set in including file? -->
<!-- @Study How does the interpreter handle inputs that aren't string-literals? Because for the compiler it would be easiest to only allow string-literals as arguments to load() -->

SetlX allows users to include other files via the `load(string file_name)` function. In the Interpreter, this function executes all statements made in the provided file. For the compiler, this means that the file-contents can more or less simply be copy-pasted in and parsed as though they were in a single file.

The idea here is to check for any function calls of `load`, then tokenize and parse the inputs of the given file and simply insert the AST from there into this AST.

## 4. Verification

The aim of this step is two-fold:

1. Infer types of variables and provide simple typechecking
2. Bring data into a structure, that can be directly used to create the intermediate representation

### Typechecking

For a given procedure, we can roughly infer input and output types. This inference must be based on builtin operations and functions, that have clearly defined signatures. These types must also be structured hierachically, with some top type, that will be infered if no better information is accessible.

<!-- @Incomplete What about recursive and mutually recursive procedures? Should output type simply be assumed to be the 'any' unless it can be reduced later on? Reducing it later on would require a second pass though, right? -->

The types of variables can be infered from their assignments. Since later assignments can increase the possible type of the variable, we can only check for type-errors in a second pass.

<!--
@Study We would like to create a new variable for assignments that increase the type of the variable. This is only possible if we know the exact order of operations that affect the provided variable. This might not always be the case. Take for example:

// This example uses mutual recursion, but that's probably not even necessary
a := 3; // a is an int
f := procedure()  { return "Hello " + a; } // Signature: void -> String
g := procedure(x) { a := h(x); return a; } // Signature: void -> any   (with side-effects)
h := procedure(x) {                        // Signature: any  -> String | any
	if (a == x) { return f();      }
	else        { return g(x - 1); }
}
g(5); // Especially impossible if 5 would come from user input
-->

The actual typechecking would then only happen in a second pass. Here we would simply make sure that the types of the input variables have some overlap with the expected types for function parameters and operators. We must accept the program as correct both if the variable's type is smaller and if it's bigger than the expected type. This is because SetlX is a very weakly typed language, where variables can change their types throughout the program.

### ScopeData

ScopeData is the current working title for the kind of structure expected by the next phase. The idea is that this structure is recursively defined with the outermost value defining the global scope of the program. The structure would also contain information about the functions and variables in the scope, as well as the statements, that are meant to be executed (which would be stored as astNodes probably).

## 5. Intermediate Representation

Taking the ScopeData produced in the last phase, we create the intermediate representation for each statement in the provided AST. To realize jumps, we should use Labels.

## 6. LLVM IR

This should hopefully be relatively straight forward, as we simply need to translate our internal IR into the LLVM IR and then write it as a text.

<!-- @Study Can we easily insert debug information into the llvm IR? It would be very nice, if we could for example tell the user the location of failure as well as a stack-trace on an error -->
