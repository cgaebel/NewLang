// array.c: Defines NewLang's array type.
// 
// Eventually, this entire file will have to be written in assembly and the
// majority of the functions will be inlined directly (except maybe
// resize_dynamic, since it should be called infrequently). This little library
// will replace raw contiguous memory as de-facto storage, and is only possible
// as a language built-in or as an assembly hack.
//
// Arrays consist of two parts: the static half, and the dynamic half. The
// static half is constructed at initialization directly on the stack and has
// a fixed size throughout the array's lifetime. Note that the static_capacity // may be zero if the initial size is unknown at initialization. The static
// half is analogous to a C99 array.
//
// The dynamic half is constructed as the array grows. If more elements are in
// the list than the capacity of the static half, the excess overflow into the
// dynamic half. The dynamic half is analogous to a C++ vector, and is
// constructed on the heap and can be treated like an "unlimited" buffer.
//
// The benefits of this design include:
//   - O(1) random access
//   - No dynamic allocation unless it's truly needed
//   - O(n) iteration
//   - Small codesize
//   - Of the possible branches in the code, many (if not, all) of them may be
//     lifted into the parent function and the dead paths, removed.
//   - Amortized O(1) insertion
//   - Cache coherency, since many of the elements will reside in the cache-hot
//     stack, and the rest lie in contiguous memory.
#include <stddef.h>
#include <stdbool.h>

// The minimum size of the dynamic half after the initial allocation.
// This should always be a power of two for performance reasons.
#define DYNAMIC_SIZE_MIN    16

struct Array
{
    size_t dynamic_length;
    size_t dynamic_capacity;
    Value* dynamic_elems;

    size_t static_length;
    size_t static_capacity;
    Value static_elems[];
};

static void resize_dynamic(Array* a, size_t newlen)
{
    assert(a->dynamic_length <= newlen);

    // BUG: No OOM checking.
    a->dynamic_elems = realloc(a->dynamic_elems, newlen * sizeof(Value));
    a->dynamic_capacity = newlen;
}

size_t length(Array* a)
{
    return a->static_length + a->dynamic_length;
}

void reserve(Array* a, size_t capacity)
{
    if(capacity <= length(a))
        return;

    if(capacity <= a->static_capacity)
        return;

    resize_dynamic(a, capacity - a->static_capacity);
}

// `a' MUST be allocated in the parent function with the following stub:
//
//   #define len(requested_size) (5*sizeof(size_t) + requested_size*sizeof(Value))
//     sub rsp, len
//     mov [rsp+(4*sizeof(size_t))], len // set up static_capacity
//     mov rdi, rsp // pass the newly allocated struct to Array.init
//     call Array.init
//
// And when the function's scope is exited, `a' must be deallocated with:
//
//   #define len(requested_size) (5*sizeof(size_t) + requested_size*sizeof(Value))
//     mov rdi, rsp // assumes the array is at the top of the stack.
//     call Array.destroy
//     add rsp, len
void init(Array* a)
{
    a->dynamic_length = 0;
    a->dynamic_capacity = 0;
    a->dynamic_elems = NULL;

    a->static_length = 0;
    // a->static_capacity was set in assembly.
    // a->static_elems can be left undefined.
}

void destroy(Array* a)
{
    // free() checks this condition too, but by pulling it out of the library,
    // we give the compiler a chance to statically prove that the call is not
    // needed.
    if(a->dynamic_elems)
        free(a->dynamic_elems);
}

void pcopy(Array* a)
{
    if(a->dynamic_elems)
    {
        // BUG: No OOM checking.
        size_t dynamic_bytes = a->dynamic_capacity * sizeof(Value);
        Value* new_mem = malloc(dynamic_bytes);
        memcpy(new_mem, a->dynamic_elems, dynamic_bytes);
        a->dynamic_elems = new_mem;
    }

    // Recursively call pcopy on every element of the array.
    for(size_t i = 0; i < a->static_length; ++i)
        pcopy(&a->static_elems[i]);
    for(size_t i = 0; i < a->dynamic_length; ++i)
        pcopy(&a->dynamic_elems[i]);
}

// Note: the first two checks should be lifted into the parent function by the
// optimizer.
void append(Array* a, const Value* v)
{
    // check for the easy fastpath. Hopefully, the compiler will be able to
    // easily prove that this is the case in the majority of instances.
    if(a->static_length < a->static_capacity)
        a->static_elems[a->static_length++] = *v;

    // we overflowed the static buffer, but a resize is still unneeded.
    else if(a->dynamic_length < a->dynamic_capacity)
        a->dynamic_elems[a->dynamic_length++] = *v;

    // both buffers are full. resize needed.
    else
    {
        // if the dynamic buffer is empty, create a small initial reservation.
        // otherwise, double the size.
        if(a->dynamic_capacity == 0)  reserve_dynamic(a, DYNAMIC_SIZE_MIN);
        else                          reserve_dynamic(a, a->dynamic_capacity * 2);

        a->dynamic_elems[a->dynamic_length++] = *v;
    }
}

void foreach(Array* a, void (*iter)(Value*, void*), void* aux)
{
    for(size_t i = 0; i < a->static_length; ++i)
        iter(a->static_elems + i, aux);

    for(size_t i = 0; i < a->dynamic_length; ++i)
        iter(a->dynamic_elems + i, aux);
}

// Returns a pointer to the value at index `i'.
// This entire function should be inlined by the compiler.
Value* index(Array* a, size_t i)
{
    assert(i < length(a));

    if(i < a->static_length)
        return &a->static_elems[i];
    else
        return &a->dynamic_elems[i - a->static_length];
}

// Note: None of the removal functions call pcopy or any destructors.
// There is no need, since the elements are being returned. If necessary, these
// functions will be called in the parent scope.

Value remove_last(Array* a)
{
    // FASTPATH
    if(a->dynamic_length == 0)
        return a->static_elems[--a->static_length];

    Value ret = a->dynamic_elems[--a->dynamic_length];

    if(a->dynamic_length == 0)
    {
        resize_dynamic(a, 0);
    }
    else if(a->dynamic_length <= a->dynamic_capacity >> 2
         && a->dynamic_length >= DYNAMIC_SIZE_MIN)
    {
        resize_dynamic(a, a->dynamic_capacity >> 1);
    }

    return ret;
}

// Removes an element from the array without preserving the order of the
// elements.
Value unordered_remove(Array* a, size_t index)
{
    // assumes swap(Value*, Value*) has been defined. Hopefully, swapping will
    // be a compiler builtin.
 
    swap(index(a, i), index(a, length(a) - 1));
    return remove_last(a);
}
