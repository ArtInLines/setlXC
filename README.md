# SetlXC

SetlXC is a Compiler for the Programming Language [SetlX](https://randoom.org/Software/SetlX/) written in SetlX itself.

## Disclaimer

**This Project is a Work-In-Progress. Most things will not properly compile yet.**

## Quickstart

(Note: The following command only works on Windows and if setlX and gcc are in your path. You can easily run both setlX and then any C compiler of your choosing without the batch-script too though)

```
> comp.bat <path_to_setlX_file_without_file_ending> -d
```

### What does the Script do?

The Compiler is actually more of a transpiler than a compiler, as it outputs C code instead of an executable. I might consider making my own compiler backend, that directly outputs an executable someday, but I wanted this project to be cross-platform and I didn't want to spend an eternity on this admittedly rather useless project.

Therefore, you need both [SetlX](https://randoom.org/Software/SetlX/) and some C compiler installed on your machine. The `comp.bat` script then first runs the setlXC compiler via the setlX interpreter, before running the C compiler on the output C file.

If SetlX would support running console commands, this could be done without a separate batch-script. It is thus also a goal of this project, to eventually extend the SetlX language enough to allow the compiler the compile itself without the help of any outside script (except a C compiler). If a custom backend would be created, even the C compiler could be ditched.

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

#### Reassigning builtin Functions

The interpreter allows you to reassign any variable, including builtin functions. Doing so is certainly bad style and makes the code rather unreadable.

There is only one case where I can imagine this being useful: A function that logs some info to stdout with print, could be made to log the info to a file, by redefining the print function.

However, since this is bad style (and due to design decisions made when writing the compiler, that made supporting this a bit more difficult), the compiler forbids reassigning builtin functions.

#### Importing seperate files

To import code from separate files in SetlX, you need to use the `load(file_name)` function. When using the interpreter, this function acts as a function like any other. Among other things, this means that you can overwrite the `load` function and can also dynamically load any file at any time.

The Compiler does not allow this. If it were to allow it, then every compiled binary would need to include the necessary code to read, parse and dynamically load code from another source file. In almost all cases, it is completely sufficient to only support static imports.

The Compiler thus forbids assigning any value to the variable `load` and requires the file_name to be a string literal. Lastly, calls to the `load` function are only allowed in the outermost scope.

Furthermore, the Compiler also makes sure to import any file no more than once, unlike the Interpreter. You are allowed to import the same file several times, yet it will only be loaded and included the first time.

#### Builtin Execute

The compiler does not provide the builtin function `execute`, which executes and evaluates an arbitrary SetlX expression. This kind of function cannot be practically implemented in a compiled language and is thus simply ignored here.

#### loadLibrary

The Interpreter offers the builtin `loadLibrary` function, which loads function from a specific library. This library is not made up of Java-Code that is interoperable with the Interpreter. Since I did not feel like re-implementing each library in C, I decided to just leave out all libraries.

#### Plotting

For the same reason as presented in [loadLibrary](#loadlibrary), I decided to leave out plotting for this compiler.