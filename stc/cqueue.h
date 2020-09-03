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

/*  Priority Queue using heap, with adapter class (normally cvec).

    #include <stc/crandom.h>
    #include <stc/cqueue.h>
    declare_clist(i, int);
    declare_cqueue(i, clist_i); // min-heap (increasing values)

    int main() {
        int n = 10000000;
        crand_rng32_t gen = crand_rng32_init(1234);
        crand_uniform_i32_t dist = crand_uniform_i32_init(gen, 0, n);

        cqueue_i queue = cqueue_i_init();

        // Push ten million random numbers onto the queue.
        for (int i=0; i<n; ++i)
            cqueue_i_push(&queue, crand_uniform_i32(&dist));

        // Push or pop on the queue ten million times
        printf("%d\n", n);
        for (int i=n; i>0; --i) {
            int r = crand_uniform_i32(&dist);
            if (r & 1)
                ++n, cqueue_i_push(&queue, r);
            else
                --n, cqueue_i_pop(&queue);
        }
        printf("%d\n", n);
        cqueue_i_destroy(&queue);
    }
*/

#ifndef CQUEUE__H__
#define CQUEUE__H__

#include "clist.h"

#define declare_cqueue(tag, ctype) \
 \
typedef struct ctype cqueue_##tag; \
typedef ctype##_value_t cqueue_##tag##_value_t; \
typedef ctype##_rawvalue_t cqueue_##tag##_rawvalue_t; \
typedef ctype##_input_t cqueue_##tag##_input_t; \
STC_INLINE cqueue_##tag \
cqueue_##tag##_init() {return ctype##_init();} \
STC_INLINE void \
cqueue_##tag##_destroy(cqueue_##tag* self) {ctype##_destroy(self);} \
STC_INLINE size_t \
cqueue_##tag##_size(cqueue_##tag pq) {return ctype##_size(pq);} \
STC_INLINE bool \
cqueue_##tag##_empty(cqueue_##tag pq) {return ctype##_empty(pq);} \
STC_INLINE cqueue_##tag##_value_t* \
cqueue_##tag##_front(cqueue_##tag* self) {return ctype##_front(self);} \
STC_INLINE cqueue_##tag##_value_t* \
cqueue_##tag##_back(cqueue_##tag* self) {return ctype##_back(self);} \
STC_INLINE void \
cqueue_##tag##_pop(cqueue_##tag* self) {ctype##_pop_front(self);} \
STC_API void \
cqueue_##tag##_push_v(cqueue_##tag* self, ctype##_value_t value) { \
    ctype##_push_back_v(self, value); \
} \
STC_INLINE void \
cqueue_##tag##_push(cqueue_##tag* self, cqueue_##tag##_rawvalue_t rawValue) { \
    ctype##_push_back(self, rawValue); \
} \
 \
STC_API void \
cqueue_##tag##_push_n(cqueue_##tag *self, const cqueue_##tag##_input_t in[], size_t size) { \
    ctype##_push_n(self, in, size); \
} \
typedef ctype##_iter_t cqueue_##tag##_iter_t; \
STC_INLINE cqueue_##tag##_iter_t \
cqueue_##tag##_begin(cqueue_##tag* self) {return ctype##_begin(self);} \
STC_INLINE void \
cqueue_##tag##_next(cqueue_##tag##_iter_t* it) {ctype##_next(it);} \
STC_INLINE cqueue_##tag##_value_t* \
cqueue_##tag##_itval(cqueue_##tag##_iter_t it) {return ctype##_itval(it);} \
 \
typedef int cqueue_##tag##_dud

#endif