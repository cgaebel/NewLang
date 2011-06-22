# Meta

* Wrap 80 characters/line.
* Have code samples for every language feature.
* Indentation with spaces - not tabs.
* Every member of the language spec has four parts:
    * Brief description
    * Code sample (as short as possible, illustrating the feature in isolation)
    * Rationale
    * Limitations (optional)

# Viability

In order for this language to be worth its salt, the following libraries and
programs _must_ have an elegant, idiomatic implementation. If it is impossible
to write any of these cleanly, the language is sorely lacking.

* GUI Toolkit
    * Tests the languge's ability to handle common OOP patterns.
* Arbitrary-sized arithmetic
    * Performance
    * Operator overloading
    * Inline assembly?
* Security Library
    * Performance
    * Correctness testing
    * Numerical processing
    * Repetitive code
* Asynchronous HTTP Server
    * Concurrency
    * Performance
    * Memory management

# Language Specification

## Points of Contention

### Indentation should be used instead of braces for indicating scope.

* Pros
    * Enforces nice readability.
    * More intuitive.
* Cons
    * Less intuitive to C/C++-style programmers.
    * Less flexible.

### Function header syntax should be more readable.

Current: `int foo(int bar, int foobar)`

Proposed: `foo(int bar, int foobar) = int:`

This way, information is presented nicely from left-to-right, going from most
general information, to most specific:

 * It is a function named foo.
 * It takes two parameters: `int bar` and `int foobar`.
 * It returns an `int`, as shown by function definition on subsequent lines.

_ben: It makes more sense linguistically, but the use of = unsettles me; I'd
prefer something resembling the lunate epsilon. We could use {, but that would
be incredibly unintuitive to C++-style programmers._

### Generics should be implemented only for classes.

### How should class generics be implemented?

_ben: My current way would be:

    Type vector(typename)
    {
        typename[] data;
        uint length;
    }

Essentially function-definition syntax, but with `Type` as the return type._

### Equality and assignment should be the same operator.

* Pros
    * More intuitive.
* Cons
    * Makes return values of operators context-sensitive.

### Return values should be part of a function's signature.

* Pros
    * Allows overloading by only return type.
* Cons
    * Forces the compiler to have good type inferrence.

### Backslash should be used as the pointer operator.

`int i = 5;
\int p = \i
int j = p\`

The second line is read intuitively as "A pointer to an integer, `p`, is a
pointer to `i`"

The third line is read "An integer, `j`, is the thing `p` points at."

### What keyword should replace `auto`?

`infer` was the original idea, but that leads to confusing-looking code:

`infer foo = 5`

This seems to read "infer that foo is 5", which sounds like a replacement for
`assert`, not type inferrence.

`typeinfer` is too long to be used.

_ben: `create` would be nice in the context above, since `create bar = 5` reads
"create bar, which equals 5", clearly demonstrating that we're allocating a new
variable, but not specifying the type. It seems out-of-place, though, because
most variable declarations are `noun name`, not `verb name`._

### Operator overloading should be a language feature.

* Pros
    * A bignum library (or any numerical processing, really) would be elegant.
* Cons
    * Extremely hard to get right.
    * Breaks uniform function syntax, adding yet another exception to the rule.
    * Has different semantics from functions entirely! Infix expressions are
      just so alien.

_ben: Overloaded operators should have a guarantee of purity - that if the same
object is invoked with the same operator and the same parameter(s), then the
result is guaranteed._

_clark: What about if when an operator is overloaded, it MUST have the same
mathematial properties as the thing it's emulating. Therefore, the (+) operator
must be associative and commutative. This will ensure that only math-emulators
overload these operators. We still need to think of a syntax though. And don't
forget that there are unary AND binary operators, as well as the possibilty
(or lack) of user-defined operators._

What about the following implementation?

    S uop(string op, const S* a);             // unary ops
    S bop(string op, const S* a, const S* b); // binary ops

Fatal flaw: how do we handle `a += b`?

_ben: += and similar operators can be inferred from + and = (and inlined), and
then the optimizer can optimize the extraneous copying and whatnot._

### Users should be allowed to create their own operators

* Pros:
    * Operations like dot-product and cross-product can have operators defined
    for them, rather than using member functions.
* Cons:
    * Can lead to unreadable and unintuitive code.
    * It's a lexical nightmare, depending on what restrictions we place on it.

_ben: All operators should have a guarantee of purity, even user-created ones.
This helps keep the code sane._

### Casting between arbitrary types should be allowed.

* Pros
    * Allows for systems programming tasks such as reversing the bytes in a
      number. It can be argued that this can be done with bitshifts, but I'd
      rather have the compiler do it.
* Cons
    * Makes the compiler's job harder. Less assumptions can be made.
    * It's evil. Almost everywhere. Is there anywhere it's necessary?

_ben: Allow it, but with much greater restrictions than those of C++, like size
conformity and only allowing casting from pointers TO integer types (not back)._
_clark: I really like that rule._

### Destructors. They should exist. If you agree, in what form?

* Pros
    * Resource cleanup is sane. A File struct makes sense.
* Cons
    * Destructors have a hidden control path, and they definitely don't play
      nice with exceptions or garbage collection. If neither of those become
      language features, however, then destructors seem okay.
    * How will they interact with our inheritance model?

### Inline assembly.

* Pros
    * Makes the language a true systems programming language.
    * Performance concerns can be alleviated with "just write it in assembly if
      you care!"
* Cons
    * Complicates things. What could have once been undefined behavior now
      needs precise, well-documented semantics.

### Unit tests for separate modules should be run in parallel.

* Pros
    * Testing becomes significantly faster.
    * Thread-unsafe code will break.
* Cons
    * Every module's public API will need to be reentrant.

_clark: This MAY be a good thing, since there's no legacy code that needs
porting. And since it's baked into the language, it essentially REQUIRES people
to write thread-safe APIs._

## Built-in types

* int\[8, 16, 32, 64, inf\] (unsigned types begin with `u`)
* native (unsigned. memsize)
* ptrdiff (signed, memsize)
* bool (Only two valid assignments - true/false. No other assumptions about its
  representation are made.)
* tinyfloat (equivalent to Cs `float`)
* float (equivalent to Cs `double`)

## Object Model

Objects are POD (plain old data).

    struct SomeObject
    {
        int a;
        SomeOtherObject b;
        int c;
    }

Any function taking an `Object*` as its first parameter can be syntactically
used as a member function of that type. This can be extended to accomodate
a multiple-dispatch syntax.

Therefore,

    int foo(SomeObject* this, int x, int y)
    {
        ++this.c;
        return x + y + this.a;
    }

    SomeObject bar;
    int z = bar.foo(2, 3);

    SomeObject* pbar = &bar;
    z = pbar.foo(2, 3);

is entirely valid syntax.

There is only one special constructor - the default constructor. This is
defined as a function with the same name as the structure is is
constructing. All built-in structures have a default constructor, and if
one is not provided by the code, the compiler shall create one which does
nothing.

Before an object's default constructor is called, the default constructor for
all its elements will be called.

    struct S
    {
        int a;
        int b;
        int c;
    }

    S s; // all elements are set to zero since the default constructor
         // generated by the compiler sets all ints to zero.

    ///////////////////////////////////////////////////////////////////////////

    S S()
    {
        S ret; // TODO: This is recursive. Fix. Do we need a different syntax?
               //       What if we had an implicit `this` parameter, which is
               //       a pointer to the object to be constructed (after its
               //       children have been)?

        ret.a = 1;
        ret.b = 2;
        ret.c = 3;

        return ret;
    }

    S s; // in this case, we have a default constructor. s will be { 1, 2, 3 }.

To define a constructor which takes arguments, you can just use an ordinary
function!

    S S(int x, int y)

        S ret; // calls the default constructor first...

        ret.a = x;
        ret.b = y;
        
        assert(ret.c == 3); // From the default constructor.

        return ret;
    }

    S s = S(9, 10);
    assert(s == { 9, 10, 3 });

### Copying and Moving Data

Copying is done automatically by the compiler when necessary, such as assigning
from an lvalue, or passing-by-value. It is done in two steps:

1. Each element of the structure is copied recursively.
2. `pcopy()` is run on the new structure.

`pcopy()` is a user-defined function defined as `void pcopy(S*)` where S is the
type of the struct you want `pcopy` to be defined for. It stands for *p*ost
*copy* since the function is run after the structure's elements have been
copied. If `pcopy()` is not defined for a structure, a blank one is generated
by the compiler.

    struct S { int a; }

    void pcopy(S* s)
    {
        s.a += 1; // increments s.a every time a copy is made.
    }

    S s;
    assert(s.a == 0); // thanks to int's default constructor

    S x = s;
    assert(x.a == 1);

    // Even though S is returned, pcopy isn't run. This is because it is
    // entirely transparent, and would just be wasted cycles.
    S returns1()
    {
        S ret;
        ret.a = 1;
        return ret;
    }

    S y = returns1();
    assert(y.a == 1);

Move constructors are not necessary, since it can be emulated by the compiler
refusing to call `pcopy()`.

### Inheritance/Polymorphism

Inheritance (and, by extension, polymorphism) is not a language built-in. A
vtable library will be provided by the standard library to assist in explicit
construction.

## Functions

Variadic functions will take a tuple of all the variadic arguments as a
parameter, which can then be iterated over, used with RTTI, etc.

## Pointers

In NewLang, the . operator will work for both objects and object pointers.

    s->x    ===>    s.x

C++'s references (as a replacement for pointers) do not exist.

`restrict` pointers are allowed, with some heavy static checking to ensure
coder sanity.

## Testing

Tests are run as the final step in compilation. A failed test is equal to a
failed build. There is no such thing as disabling all tests, but specific tests
may be disabled with an attribute on the unittest block. Possibly something
like:

    unittest(disable)
    {
        assert(2 + 2 == 5); // WHY DOESN'T THIS WORK!?!
    }

All tests are attempted. If a test fails, it is marked and testing of the rest
continues.

## To Be Organized

Generics and RTTI may be either runtime or compile-time determined; the
optimizer may choose which is preferable.

Only single-line comments are allowed, with \#.

`assert(expr)` will be a compiler built-in. If the expression can be resolved
at compile time, it will be. In release/fast builds, the expression becomes an
assumption for the optimizer. This provides performance incentives to defensive
coding.

No header files, only modules. We can probably rip off D's module system in its
entirety.

Static constructors from D still exist (likely renamed to init(), or
something similar).

Strings are vectors of chars. Conversion to C-strings will be necessary to
talk to C. Much of stdlib's string.h will have to be rewritten.

Anonymous functions can be created with the function keyword.

All built-in types will have default initialization values; if default
initialization is not wanted, there will be a keyword to prevent
initialization.

Calling conventions will be undefined by default. This ensures the optimizer
gets the best possible angle of attack on your code.

There is a `pure` keyword. If a function is pure and unannotated, emit a
diagnostic. If a function is annotated pure and is not, terminate compilation.

`const` stays.

Anonymous structs are a thing; members can be either named, or accessed with
array operators (tuples). A syntax still needs to be decided upon. It should
probably resemble lambdas.

n-conditionals are allowed (e.g. x < y < z = 0).

Nice interfacing with C.

Casting away `const` is illegal - not undefined behavior.

Unit-testing resembles that of D, but has an API accessible from `main()` which
handles test reporting, running, etc. Possibly run tests before `main()` iff
`test_ext` has not been imported. Otherwise, don't run any tests except those
explicitly run by `main()`.

## Compiler Options

* Build types (--build)
    * Debug (--build=debug)
        * No optimizations, all asserts on.
        * Focus on code making sense in a debugger.
    * Dev (default) (--build=dev)
        * Simple optimizations, all asserts on.
        * Focus on lowering the build->test cycle. Compilation should be
          blazing fast, but without sacrificing too much run-time speed. There
          is a delicate middle ground that the Dev build tries to find.
    * Release (--build=release)
        * Full optimization. All asserts on (custom hook enabled).
        * Focus on production-quality code. Build time is not important, and is
          sacrificed to improve quality of shipping code. Asserts are also on,
          but can be hooked by the program to do proper error-reporting.
    * Fast (--build=fast)
        * Full optimization. All asserts off.
        * Focus on fast code. That's it. Asserts will be off, and build time
          will be sacrificed for final runtime speed.

Compiler Internals
-------------------

The compiler should be designed modularly, and as a library. The binary will
just be a litle driver (no more than absolutely necessary, such as argument
parsing) to the main compilation library. Then, the compiler library will be
available in stdlib.

Compiler Development
---------------------

An optimization may only be added to the dev build if and only if it lowers the
bootstrapping time.

An optimization may only be added to the compiler if and only if the run time
of the compiler's test suite does not increase with the optimiation enabled.
