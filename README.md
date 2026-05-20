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

**Readable operators.** Boolean operators can be written as words (`and`, `or`, `not`, `xor`) or symbols (`&&`, `||`, `!`, `^`), whichever is clearer to you.

**Scoped by design.** Variables live and die within their scope. A `global {}` block exists for data that needs to outlive a function.

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
var scores:int array[10] = {1, 2, 3, ..., 10};    # fixed size, initialized at declaration.
var mut log:int mut array = {};                   # grows on access, initialized any time.
var mut groceries:str array[2] = {"Milk"};        # fixed size, initialized any time.   
var mut matrix:int mut array[2][2] = {{0, 1},{}}; # multi-dimensional arrays. 
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
for (var i:int = 0; i < 10; i = i + 1)
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
```

Structs hold data only — no methods. No circular dependencies allowed.

### Enums

```hw
enum Direction
{
    NORTH, SOUTH, EAST, WEST
}
```

### Global Scope

```hw
global
{
    var mut score:int = 0;
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

| Component        | Status         |
|------------------|----------------|
| Lexer            | ✅ Complete     |
| Parser           | 🔄 In progress  |
| Semantic Analysis| 📋 Planned      |
| Evaluator        | 📋 Planned      |

---

## About the Author

I'm André, a Computer Science student at UFLA (Universidade Federal de Lavras), with a technical degree in Systems Development from CEFET-MG Timóteo.

Heartwood is a project I'm building to understand how languages actually work — no tutorials, no guided resources, just building and breaking things until they make sense.

---

**Built to understand the fundamentals. Named after the core of a tree.**

---

*Pragmatism over purism*