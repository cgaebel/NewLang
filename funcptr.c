struct FuncPtr
{
    RetVal (*f)(void*, Args..);
    void* aux;
};

// To call a FuncPtr...
RetVal call(const FuncPtr* F, other_args)
{
    return F->f(aux, other_args);
}
