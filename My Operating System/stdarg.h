#pragma once

#define va_start(V, L) V.x = ((char*) &L) + sizeof(L)
#define va_arg(V, t) (*((t*)_va_arg(&V, sizeof(t))))
#define va_end(x)

typedef struct va_list_{
    char* x;
} va_list;

static void * _va_arg(va_list * v, unsigned size) {
    char* temp = v->x;
    v->x += size;
    return temp;
}
