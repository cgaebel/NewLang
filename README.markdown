Points of Contention
-----------------------

Generics should almost never be used. Consider their elimination from the
language, or a serious refactoring.

Consider making only one kind of comment.

"void" is often used simply as a compensation for a mediocre type system. Can
this be eliminated?

Operator-overloading is almost always a bad idea. It should perhaps be
disallowed.

reinterpret\_cast is almost always a bad idea. Its useful points should be
refactored into a safer, better system.

Destructors are often misused, but arguably necessary - can they be improved?

Language Specification
-------------------------

Objects are defined as POD.

Any function taking an Object\* as its first parameter can be syntactically
used as a member function of that typed. This can be extended to accomodate
a multiple-dispatch syntax.

Any function returning an instance of an object can be used as a constructor
for that object (if it takes a const object as a parameter, it's a copy
constructor).

Move constructors do not exist; move constructors should be simple moving
of data.

The \ character will replace the -> operator.

There are no references (as in "transparent" pointers).

No header files, only modules.

Static constructors from D still exist (likely renamed to init(), or
something similar).

Strings are vectors of chars.

Anonymous functions can be created with the function keyword.

The "infer" keyword will replace "auto".

All built-in types will have default initialization values; if default
initialization is not wanted, there will be a keyword to prevent
initialization.

Calling conventions will be undefined from function to function; the
compiler/optimizer may choose. This can be overriden if necessary.

There is a "pure" keyword.

There is a "const" or "immutable" keyword.

Anonymous structs are a thing; members can be either named, or accessed with
array operators (tuples).

n-conditionals are allowed (e.g. x < y < z = 0).

Embedded C and assembly are allowed.

Inheritance (and, by extension, polymorphism) is not a thing.

Const-casting is not a thing.

Unit-testing resembles that of D.

Only three build types: Debug (simple optimizations, focus on quick
compilation), Release (focus on fastest code), -O0 (no optimization
whatsoever).

