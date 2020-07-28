/* MIT License
 *
 * Copyright (c) 2020 Tyge Løvset, NORCE, www.norceresearch.no
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef CSTR__H__
#define CSTR__H__

#include <stdlib.h> /* alloca, malloc */
#include <malloc.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h> /* vsnprintf */

#include "cdefs.h"

typedef struct cstr {
    char* str;
} cstr;

#define _cstr_rep(self)   (((size_t *) (self)->str) - 2)
#define _cstr_size(s)     ((size_t *) (s).str)[-2]
#define _cstr_mem(cap)    (sizeof(size_t) * (3 + (cap)/sizeof(size_t)))
static size_t _cstr_nullrep[] = {0, 0, 0};

static  cstr cstr_init =  {(char* ) &_cstr_nullrep[2]};
#define cstr_size(s)      ((const size_t *) (s).str)[-2]
#define cstr_capacity(s)  ((const size_t *) (s).str)[-1]
#define cstr_npos         ((size_t) (-1))

STC_API cstr
cstr_make_n(const char* str, size_t len);
STC_API cstr
cstr_from(const char* fmt, ...);
STC_API void
cstr_reserve(cstr* self, size_t cap);
STC_API void
cstr_resize(cstr* self, size_t len, char fill);
STC_API cstr
cstr_make_reserved(size_t cap);
STC_API cstr*
cstr_assign_n(cstr* self, const char* str, size_t len);
STC_API cstr*
cstr_append_n(cstr* self, const char* str, size_t len);
STC_API void
cstr_insert_n(cstr* self, size_t pos, const char* str, size_t n);
STC_API size_t
cstr_replace_n(cstr* self, size_t pos, const char* str1, size_t n1, const char* str2, size_t n2);
STC_API void
cstr_erase(cstr* self, size_t pos, size_t n);
STC_API char*
cstr_strnstr(cstr s, size_t pos, const char* needle, size_t n);

STC_INLINE void
cstr_destroy(cstr* self) {
    if (cstr_capacity(*self)) {
        free(_cstr_rep(self));
    }
}

STC_INLINE cstr
cstr_make_filled(size_t len, char fill) {
    cstr s = cstr_init;
    if (len) cstr_resize(&s, len, fill);
    return s;
}

STC_INLINE cstr
cstr_make(const char* str) {
    return cstr_make_n(str, strlen(str));
}

STC_INLINE cstr
cstr_make_copy(cstr s) {
    return cstr_make_n(s.str, cstr_size(s));
}

STC_INLINE void
cstr_clear(cstr* self) {
    cstr_destroy(self);
    *self = cstr_init;
}

STC_INLINE cstr*
cstr_assign(cstr* self, const char* str) {
    return cstr_assign_n(self, str, strlen(str));
}

STC_INLINE cstr*
cstr_copy(cstr* self, cstr s) {
    return cstr_assign_n(self, s.str, cstr_size(s));
}

STC_INLINE cstr*
cstr_take(cstr* self, cstr s) {
    if (self->str != s.str && cstr_capacity(*self))
        free(_cstr_rep(self));
    self->str = s.str;
    return self;
}

STC_INLINE cstr
cstr_move(cstr* self) {
    cstr tmp = *self;
    *self = cstr_init;
    return tmp;
}

STC_INLINE cstr*
cstr_append(cstr* self, const char* str) {
    return cstr_append_n(self, str, strlen(str));
}
STC_INLINE cstr*
cstr_appendS(cstr* self, cstr s) {
    return cstr_append_n(self, s.str, cstr_size(s));
}
STC_INLINE cstr*
cstr_push_back(cstr* self, char value) {
    return cstr_append_n(self, &value, 1);
}
STC_INLINE void
cstr_pop_back(cstr* self) {
    --_cstr_size(*self);
}
STC_INLINE char
cstr_back(cstr s) {
    return s.str[cstr_size(s) - 1];
}

STC_INLINE void
cstr_insert(cstr* self, size_t pos, const char* str) {
    cstr_insert_n(self, pos, str, strlen(str));
}

STC_INLINE size_t
cstr_replace(cstr* self, size_t pos, const char* str1, const char* str2) {
    return cstr_replace_n(self, pos, str1, strlen(str1), str2, strlen(str2));
}

/* readonly */

STC_INLINE bool
cstr_empty(cstr s) {
    return cstr_size(s) == 0;
}
STC_INLINE bool
cstr_equals(cstr s1, const char* str) {
    return strcmp(s1.str, str) == 0;
}
STC_INLINE bool
cstr_equalsS(cstr s1, cstr s2) {
    return strcmp(s1.str, s2.str) == 0;
}
STC_INLINE int
cstr_compare(const void* s1, const void* s2) {
    return strcmp(((const cstr*)s1)->str, ((const cstr*)s2)->str);
}
STC_INLINE size_t
cstr_findN(cstr s, size_t pos, const char* needle, size_t n) {
    char* res = cstr_strnstr(s, pos, needle, n);
    return res ? res - s.str : cstr_npos;
}
STC_INLINE size_t
cstr_find(cstr s, size_t pos, const char* needle) {
    char* res = strstr(s.str + pos, needle);
    return res ? res - s.str : cstr_npos;
}

/* CVec / cmap API functions: */

#define             cstr_getRaw(x) ((x)->str)
#define             cstr_compareRaw(x, y) strcmp(*(x), *(y))
#define             cstr_equalsRaw(x, y) (strcmp(*(x), *(y)) == 0)
STC_INLINE uint32_t cstr_hashRaw(const char* const* sPtr, size_t ignored) {
    uint32_t hash = 5381, c; /* djb2 */
    const char* tmp = *sPtr;
    while (c = *tmp++) hash = ((hash << 5) + hash) ^ c;
    return hash;    
}

/* -------------------------- IMPLEMENTATION ------------------------- */

#if !defined(STC_HEADER) || defined(STC_IMPLEMENTATION)

STC_API void
cstr_reserve(cstr* self, size_t cap) {
    size_t len = cstr_size(*self), oldcap = cstr_capacity(*self);
    if (cap > oldcap) {
        size_t* rep = (size_t *) realloc(oldcap ? _cstr_rep(self) : NULL, _cstr_mem(cap));
        self->str = (char *) (rep + 2);
        self->str[rep[0] = len] = '\0';
        rep[1] = cap;
    }
}

STC_API void
cstr_resize(cstr* self, size_t len, char fill) {
    size_t n = cstr_size(*self);
    cstr_reserve(self, len);
    if (len > n) memset(self->str + n, fill, len - n);
    self->str[_cstr_size(*self) = len] = '\0';
}

STC_API cstr
cstr_make_reserved(size_t cap) {
    if (cap == 0) return cstr_init;
    size_t *rep = (size_t *) malloc(_cstr_mem(cap));
    cstr s = {(char *) (rep + 2)};
    rep[0] = 0, rep[1] = cap, s.str[0] = '\0';
    return s;
}

STC_API cstr
cstr_make_n(const char* str, size_t len) {
    if (len == 0) return cstr_init;
    size_t *rep = (size_t *) malloc(_cstr_mem(len));
    cstr s = {(char *) (rep + 2)};
    memcpy(s.str, str, len);
    s.str[rep[0] = rep[1] = len] = '\0';
    return s;
}

STC_API cstr
cstr_from(const char* fmt, ...) {
    cstr tmp = cstr_init;
    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(NULL, (size_t)0, fmt, args);
    if (len > 0) {
        tmp = cstr_make_reserved(len);
        vsprintf(tmp.str, fmt, args);
        _cstr_size(tmp) = len;
    }
    va_end(args);
    return tmp;
}

STC_API cstr*
cstr_assign_n(cstr* self, const char* str, size_t len) {
    if (len || cstr_capacity(*self)) {
        cstr_reserve(self, len);
        memmove(self->str, str, len);
        self->str[_cstr_size(*self) = len] = '\0';
    }
    return self;
}

STC_API cstr*
cstr_append_n(cstr* self, const char* str, size_t len) {
    if (len) {
        size_t oldlen = cstr_size(*self), newlen = oldlen + len;
        if (newlen > cstr_capacity(*self))
            cstr_reserve(self, newlen * 5 / 3);
        memmove(&self->str[oldlen], str, len);
        self->str[_cstr_size(*self) = newlen] = '\0';
    }
    return self;
}

STC_INLINE void _cstr_internalMove(cstr* self, size_t pos1, size_t pos2) {
    if (pos1 == pos2)
        return;
    size_t len = cstr_size(*self), newlen = len + pos2 - pos1;
    if (newlen > cstr_capacity(*self))
        cstr_reserve(self, newlen * 5 / 3);
    memmove(&self->str[pos2], &self->str[pos1], len - pos1);
    self->str[_cstr_size(*self) = newlen] = '\0';
}

STC_API void
cstr_insert_n(cstr* self, size_t pos, const char* str, size_t n) {
    char* xstr = (char *) memcpy(n > c_max_alloca ? malloc(n) : alloca(n), str, n);
    _cstr_internalMove(self, pos, pos + n);
    memcpy(&self->str[pos], xstr, n);
    if (n > c_max_alloca) free(xstr); 
}

STC_API size_t
cstr_replace_n(cstr* self, size_t pos, const char* str1, size_t n1, const char* str2, size_t n2) {
    size_t pos2 = cstr_findN(*self, pos, str1, n1);
    if (pos2 == cstr_npos) return cstr_npos;
    char* xstr2 = (char *) memcpy(n2 > c_max_alloca ? malloc(n2) : alloca(n2), str2, n2);
    _cstr_internalMove(self, pos2 + n1, pos2 + n2);
    memcpy(&self->str[pos2], xstr2, n2);
    if (n2 > c_max_alloca) free(xstr2);
    return pos2;
}

STC_API void
cstr_erase(cstr* self, size_t pos, size_t n) {
    size_t len = cstr_size(*self);
    if (len) {
        memmove(&self->str[pos], &self->str[pos + n], len - (pos + n));
        self->str[_cstr_size(*self) -= n] = '\0';
    }
}

STC_API char*
cstr_strnstr(cstr s, size_t pos, const char* needle, size_t n) {
    char *x = s.str + pos, /* haystack */
         *z = s.str + cstr_size(s) - n + 1;
    if (x >= z)
        return NULL;
    ptrdiff_t sum = 0;
    const char *y = x, *p = needle, *q = needle + n;
    while (p != q)
        sum += *y++ - *p++;
    while (x != z) {
        if (sum == 0 && memcmp(x, needle, n) == 0)
            return x;
        sum += *y++ - *x++;
    }
    return NULL;
}

#endif

#endif
