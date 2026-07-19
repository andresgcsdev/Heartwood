# Heartwood

A statically typed, procedural programming language designed for people learning to program.

---

## About

Heartwood is a language built from scratch in C++17, designed with one goal: making programming approachable without hiding what's actually happening. There's no magic, no implicit behavior, and no runtime surprises — just clean, readable, procedural code.

The name comes from the heartwood of a tree — the dense, foundational core that holds everything else up. That's the intent: a language that teaches you the fundamentals for real.

File extension: `.hw`

---

## Design Philosophy

**Statically typed.** Every variable has a declared type. Type errors are caught before execution, not at runtime.

**Procedural only.** No classes, no inheritance, no closures. Functions are functions, data is data.

**Explicit over implicit.** Variables are immutable by default. Mutation requires `mut`. Nothing changes unless you say so.

**Simplified Referencing.** References are only created through function parameters. Ownership is only be borrowed or transferred.

---

## Language Overview

### Variables

```hw
var constant:int = 42;      # immutable
var mut count:int = 0;      # mutable
```

### Primitive Types

| Type    | Description              |
|---------|--------------------------|
| `int`   | Integer number           |
| `float` | Floating point number    |
| `str`   | Text string              |
| `bool`  | true or false            |

### Arrays

```hw
var scores:int[10] = [1, 2, 3, ..., 10];    # fixed size, initialized at declaration.
var mut groceries:str[2] = ["Milk"];        # fixed size, initialized any time.   
var mut log:int[];                          # grows on access, initialized any time.
var mut matrix:int[][] = [[0, 1],[]];       # multi-dimensional arrays, initialized any time. 
```

### Functions

```hw
fn add(a:int, b:int) -> int
{
    return a + b;
}

fn greet(name:str)
{
    # no return type needed for void functions
}

fn greet(age:int, name:str)
{
   # functions may be overloaded
}

fn addTimesTwo(mut a:int, mut b:int) -> int
{
   a = a*2;
   b = b*2;
   #arguments become mutable variables
   return add(a, b);
}

fn dontCopy(ref x:int[10]) -> bool
{
   # x is passed by reference, no copy is made
   # cheaper memory cost if needed
}

fn changeState(mut ref x:int[10]) -> bool
{
   # x is passed by reference, no copy is made
   # any alterations done to x inside of the function have effects outside of it
   # the reference is borrowed in the function, and cannot be deleted or point to other data
   # all data only has a single reference at all times, it is only borrowed from function to function
}
```

### Control Flow

```hw
if (x > 0)
{
    # ...
}
else if (x == 0)
{
    # ...
}
else
{
    # ...
}

while (running)
{
    # ...
}

# The iterator variable is mutable by default.
for (i:int = 0; i < 10; i++)
{
    # ...
}
```

### Structs

```hw
struct Point
{
    x:int;
    y:int;
}

var p:Point = {0, 1}; # automatically casts the given struct expression into the variable type at initialization
p = Point{1, 2}; # struct expression must contain the struct name at any other assign
p = Point{3, 4};
p = Point{4}; #default y = 0
p = Point{}; #default x = 0, y = 0;
```

Structs hold data only — no methods. No circular dependencies allowed.

### Enums

```hw
enum Direction
{
    NORTH, SOUTH, EAST, WEST
}

var a:Direction = Direction::NORTH;

if(a == Direction::NORTH)
{
   print("going north!");
}
```

### Global Scope

```hw
global
{
    var mut score:int = 0;
}

fn function_a() 
{
   #can access score
}

fn function_b()
{
   #can access score
}
```

### Comments

```hw
# This is a comment
```

---

## Implementation

Heartwood is an interpreted language implemented in C++17. The pipeline:

```
source file (.hw)
    → Lexer       (tokenization)
    → Parser      (AST construction)      [in progress]
    → Semantic    (type checking)         [planned]
    → Evaluator   (execution)             [planned]
```

### Building

```
git clone https://github.com/andresgcsdev/Heartwood.git
cd Heartwood
mkdir build
cmake -B build
cmake --build build
```

### Running

```
./heartwood yourscript.hw
```

Your heartwood file must be in the same directory as of the executable!

---

## Status

| Component         | Status          |
|-------------------|-----------------|
| Lexer             | ✅ Complete      |
| Parser            | 🔄 In progress  |
| Semantic Analysis | 📋 Planned      |
| Evaluator         | 📋 Planned      |

---

## About the Author

I'm André, a Computer Science student at UFLA (Universidade Federal de Lavras), with a technical degree in Systems Development from CEFET-MG Timóteo.

Heartwood is a project I'm building to understand how languages actually work — no tutorials, no guided resources, just building and breaking things until they make sense.

---

**Built to understand the fundamentals. Named after the core of a tree.**

---

*Pragmatism over purism*