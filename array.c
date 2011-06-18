// Defines NewLang's array type.
#include <stddef.h>
#include <stdbool.h>

struct Array
{
    size_t dynamic_length;
    size_t dynamic_capacity;
    Value* dynamic_elems;

    size_t static_length;
    size_t static_capacity;
    Value static_elems[];
};

// If the array is normalized, all the elements lie in dynamic_elems
static bool is_normalized(Array* a)
{
    return a->static_length == 0;
}

// Moves all the elements into contiguous memory.
static void normalize(Array* a)
{
    resize_dynamic(a, dynamic_length + static_length);

    memcpy(a->dynamic_elems + a->dynamic_length,
           a->static_elems,
           a->static_length * sizeof(Value));

    a->dynamic_length += a->static_length;
    a->static_length = 0;
}

static void resize_dynamic(Array* a, size_t newlen)
{
    assert(newlen <= a->dynamic_length);

    // BUG: No OOM checking.
    a->dynamic_elems = realloc(a->dynamic_elems, newlen * sizeof(Value));
    a->dynamic_capacity = newlen;
}

size_t length(Array* a)
{
    return a->static_length + a->dynamic_length;
}

void reserve(Array* a, size_t cap)
{
    if(length(a) >= cap)
        return;

    if(cap <= a->static_capacity)
        return;

    resize_dynamic(a, cap - static_capacity);
}

// `a' MUST be allocated in the parent function with the following stub:
//
//    #define len(requested_size) (5*sizeof(size_t) + requested_size*sizeof(Value))
//      sub rsp, len
//      mov [rsp+(4*sizeof(size_t))], len // set up static_capacity
//      mov rdi, rsp // pass the newly allocated struct to Array.init
//      call Array.init
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
        a->dynamic_elems = realloc(a->dynamic_elems, a->dynamic_capacity);
    }
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
        if(a->dynamic_capacity == 0)  reserve_dynamic(a, 16);
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
