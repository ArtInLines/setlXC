# Overview

1. Tokenize the Source File (String -> Token[])
2. Parse the Token-Stream (Token[] -> AST)
3. Include imported files (AST -> AST)
4. Create an IR (AST -> IR)
5. Transform LLVM to IR (IR -> LLVM)

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
	string  id;
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
	Assign,   // Args: Iden/Index/Property, Expr for value
	BinOp,    // Data: operator; Args: left Expr, right Expr
	UnaryOp,  // Data: operaor; Args: operand
	In,       // Data: String; Args: collection @TODO not implemented in code yet
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
	Om;       // Nothing @TODO not implemented in code yet
} nodeType;

typedef struct baseType { // See chapter 4 for an overview of these base types
	Int,
	Float,
	Rational,
	Bool,
	String,
	Om,
	List,
	Set,
	Proc,
	Any;
} baseType;

typedef struct astNode {
	nodeType  id;
	loc       start;
	loc       end;
	baseType  types{}; // Set of types, that this node could have
	astNode  *data;    // Single data point, usually an Identifier/Index/Property - can be om
	astNode   args[];  // List of arguments to the astNode. Depends on the exprType of the node (see nodeType definition above)
} astNode;
```

## 3. Imports

<!-- @Study Can included files reference variables set in including file? -->
<!-- @Study How does the interpreter handle inputs that aren't string-literals? Because for the compiler it would be easiest to only allow string-literals as arguments to load() -->
<!-- @Study Can load be overwritten? -->

SetlX allows users to include other files via the `load(string file_name)` function. In the Interpreter, this function executes all statements made in the provided file. For the compiler, this means that the file-contents can more or less simply be copy-pasted in and parsed as though they were in a single file.

The idea here is to check for any function calls of `load`, then tokenize and parse the inputs of the given file and simply insert the AST from there into this AST.

## 4. Intermediate Representation

-   Currently being worked on

Transforms the ASTNode produced by parsing into an intermediate representation, that can easily be transformed into LLVM code.

The intermediate representation uses primarily two different structures, which are more thoroughly specified below:

-   `IR`: Contains all information required to create LLVM code
-   `Inst`: Represents an individual instruction in the intermediate representation.

<!-- @Incomplete classes, objects and lambdas are ignored here still -->

For generating the code, we need certain information. Firstly we need to know the list of functions defined. SetlX lets the user bind procedures to variables the same as any other object and importantly, a variable can be reassigned. It is thus not guaranteed that a name will always point to a function. Thus we treat all functions in SetlX to be unnamed and all variables assigned to those procedures are treated like function pointers in C.

The Instructions in the IR are not recursively defined. Instead, they use a stack with implicit definitions on how they operate on said stack. Each procedure has its own internal stack, on which the instructions operate.

```c
typedef struct instType { // Represented as strings in SetlX
	// Binary Ops
	And,        // [op1, op2] -> [res]
	Or,         // [op1, op2] -> [res]
	Eq,         // [op1, op2] -> [res]
	Ge,         // [op1, op2] -> [res]
	Le,         // [op1, op2] -> [res]
	Gt,         // [op1, op2] -> [res]
	Lt,         // [op1, op2] -> [res]
	In,         // [op1, op2] -> [res]
	Add,        // [op1, op2] -> [res]
	Sub,        // [op1, op2] -> [res]
	Mul,        // [op1, op2] -> [res]
	Div,        // [op1, op2] -> [res]
	Mod,        // [op1, op2] -> [res]
	IntDiv,     // [op1, op2] -> [res]
	Exp,        // [op1, op2] -> [res]
	CartProd,   // [op1, op2] -> [res]
	AddCollBin, // [op1, op2] -> [res]
	MulCollBin, // [op1, op2] -> [res]
	// Unary Ops
	AddColl,    // [op] -> [res]
	MulColl,    // [op] -> [res]
	Not,        // [op] -> [res]
	Neg,        // [op] -> [res]
	Len,        // [op] -> [res]
	// Other Ops
	Assign,     // [val, var] -> []
	Ret,        // [...] -> []
	Range,      // [coll1, lo, hi] -> [coll2]
	Index,      // [coll, idx] -> [val]
	Property,   // [val, str] -> [val]
	Call,       // [...args, arglen, var] -> [val]
	NativeCall, // [...args, arglen, var, type] -> [val]
	Label,      // [] -> [str] | Labels for jumps
	CondJmp,    // [] -> [] | Jumps to label (given in instData) if last expression computes "false"
	Jmp,        // [] -> [] | Unconditional Jump to label (given in instData)
	Set,        // [...args, arglen] -> [set]
	List,       // [...args, arglen] -> [list]
	Str,        // [] -> [str]
	Int,        // [] -> [num]
	Float,      // [] -> [num]
	Om,         // [] -> [om]
	Proc,		// [] -> [proc]
	Var;        // [] -> [str]
	Block;      // [] -> []
} instType;

typedef union instData {
	inst[],
	String,
	Int,
	Float
} instData;

typedef struct inst {
	instType id;
	instData data;  // Only needed when a value is pushed without being computable from the stack's current contents
	loc      start;
	loc      end;
} inst;

typedef struct procedure {
	string   name;     // Potentially useful for debugging
	inst     args[][]; // List of arguments, each of which is a list of 'Var' and 'Assign' insts
	string   vars[];   // Variables (aside from arguments) that are used in the function
	inst     code[];   // Actual operations in the procedure
	loc      start;
	loc      end;
} proc;

typedef struct intermediateRep {
	proc   procs[];
	inst   code[];
} intermediateRep;
```

## 5. Intermediate Representation

Taking the ScopeData produced in the last phase, we create the intermediate representation for each statement in the provided AST. To realize jumps, we should use Labels.

## 6. LLVM IR

This should hopefully be relatively straight forward, as we simply need to translate our internal IR into the LLVM IR and then write it as a text.

<!-- @Study Can we easily insert debug information into the llvm IR? It would be very nice, if we could for example tell the user the location of failure as well as a stack-trace on an error -->
