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

/*  Priority queue using CVec as heap.

    #include <stc/cvecheap.h>
    #include <stc/crandom.h>
    declare_CVec(i, int);
    declare_CVec_heap(i, >); // min-heap (increasing values)
    int main() {
        pcg32_random_t pcg = pcg32_seed(1234, 0);
        CVec_i heap = cvec_init;
        for (int i=0; i<100; ++i) cvec_i_heapPush(&heap, pcg32_random(&pcg));
        for (int i=0; i<5; ++i) {
            printf("%d ", cvec_i_heapTop(&heap, pcg32_random(&pcg)));
            cvec_i_heapPop(&heap, pcg32_random(&pcg));
        }
    }
 */

#ifndef CVECHEAP__H__
#define CVECHEAP__H__

#include "cvec.h"

#define declare_CVec_heap(tag, cmpOpr) /* < or > */ \
 \
STC_API void \
cvec_##tag##_heapify(CVec_##tag* self); \
STC_API void \
cvec_##tag##_heapErase(CVec_##tag* self, size_t i); \
STC_INLINE CVecValue_##tag \
cvec_##tag##_heapTop(CVec_##tag* self) {return self->data[0];} \
STC_INLINE void \
cvec_##tag##_heapPop(CVec_##tag* self) {cvec_##tag##_heapErase(self, 0);} \
STC_API void \
cvec_##tag##_heapPush(CVec_##tag* self, CVecValue_##tag value); \
 \
implement_CVec_heap(tag, cmpOpr) \
typedef CVec_##tag CVecHeap_##tag
    
/* -------------------------- IMPLEMENTATION ------------------------- */

#if !defined(STC_HEADER) || defined(STC_IMPLEMENTATION)
#define implement_CVec_heap(tag, cmpOpr) \
 \
STC_INLINE void \
_cvec_##tag##_siftDown(CVecValue_##tag* arr, size_t i, size_t n) { \
    size_t r = i, c = i << 1; \
    while (c <= n) { \
        if (c < n && cvec_##tag##_sortCompare(&arr[c], &arr[c + 1]) cmpOpr 0) \
            ++c; \
        if (cvec_##tag##_sortCompare(&arr[r], &arr[c]) cmpOpr 0) { \
            CVecValue_##tag t = arr[r]; arr[r] = arr[c]; arr[r = c] = t; \
        } else \
            return; \
        c <<= 1; \
    } \
} \
 \
STC_API void \
cvec_##tag##_heapErase(CVec_##tag* self, size_t i) { \
    self->data[i] = cvec_##tag##_back(*self); \
    cvec_##tag##_popBack(self); \
    _cvec_##tag##_siftDown(self->data - 1, i + 1, cvec_size(*self)); \
} \
 \
STC_API void \
cvec_##tag##_heapPush(CVec_##tag* self, CVecValue_##tag value) { \
    cvec_##tag##_pushBack(self, value); \
    size_t n = cvec_size(*self), c = n; \
    CVecValue_##tag *arr = self->data - 1; \
    for (; c > 1 && cvec_##tag##_sortCompare(&arr[c >> 1], &value) cmpOpr 0; c >>= 1) \
        arr[c] = arr[c >> 1]; \
    arr[c] = value; \
} \
 \
STC_API void \
cvec_##tag##_heapify(CVec_##tag* self) { \
    size_t n = cvec_size(*self);  \
    CVecValue_##tag *arr = self->data - 1; \
    for (size_t k = n >> 1; k != 0; --k) \
        _cvec_##tag##_siftDown(arr, k, n); \
}

#else
#define implement_CVec_heap(tag, cmpOpr)
#endif

#endif
