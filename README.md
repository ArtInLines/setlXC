# SetlXC

SetlXC is a Compiler for the Programming Language [SetlX](https://randoom.org/Software/SetlX/) written in SetlX itself.

## Disclaimer

**This Project is a Work-In-Progress. As of now, only Tokenization and large parts of parsing are actually implemented.**

## Bootstrapping the Compiler

**This is not possible yet, as the compiler is not capable of compiling itself yet. The information in this section will only be relevant in the future.**

Since the Compiler is self-hosted (i.e. written in the language it compiles), the question arises of how to compile the compiler. More succintly, how does one bootstrap the compiler?

There are two ways to bootstrap the compiler:

1. Using the SetlX Interpreter
2. Using LLVM

### Using the SetlX Interpreter

The SetlX Interpreter is the original program, that was used to run SetlX programs. The Interpreter is written in Java an provided via a JAR-File. This means, that you need to have java installed on your machine to run the interpreter.

If those prerequesites are fulfilled, you can run

```console
> setlX setlXC.stlx -p setlXC.stlx
```

After running that command, the compiler will have compiled its own source code into a llvm-intermediate representation

### Using LLVM

The Compiler uses [LLVM](https://www.llvm.org/) as its backend. This allows us to provide the compiler source code not just in SetlX (`SetlXC.stlx`) but also in LLVM's Intermediate Representation Language (`SetlXC.ll`). This file can be compiled to an executable using LLVM. To do that, run

```console
> clang setlXC.ll -o setlXC.exe
```

## FAQ

### WhY?

1. Why not?
2. When making a program of more than a couple dozen lines in SetlX (for example [this project](https://github.com/ArtInLines/differentiator)), debugging became very difficult, as the interpreter spams the console full with mostly unnecessary information. A key motiviation behind this compiler was thus to provide better and usable error messages.

### Differences to the Interpreter

The Compiler aims to mimic the SetlX's interpreter's behaviour as much as possible.

However, there are certain cases, where the compiler behaves intentionally differently. These cases are listed below:

#### Classes inside Procedures

The Interpreter lets you declare classes inside of procedures. However, the Interpreter crashes when returning said class from the procedure.

The Compiler instead fails and returns a compilation-error when you declare a class in a procedure.
