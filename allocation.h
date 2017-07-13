// Copyright (c) 2014, the Scal Project Authors.  All rights reserved.
// Please see the AUTHORS file for details.  Use of this source code is governed
// by a BSD license that can be found in the LICENSE file.

#ifndef ALLOCATION_H_
#define ALLOCATION_H_

#include <assert.h>
#include <pthread.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


namespace memalloc
{

const uint64_t  kCachePrefetch = 64;



void* MallocAligned(size_t size, size_t alignment) {
    void* mem;
    if (posix_memalign(reinterpret_cast<void**>(&mem),
                       alignment, size)) {
        perror("posix_memalign");
        abort();
    }
    return mem;
}


void* CallocAligned(size_t num, size_t size, size_t alignment) {
    const size_t sz = num * size;
    void* mem = MallocAligned(sz, alignment);
    memset(mem, 0, sz);
    return mem;
}

template<typename T>
T* get(uint64_t alignment) {
        void *mem;
        mem = MallocAligned(sizeof(T), alignment);
        memset(mem, 0, sizeof(T));
        T *obj = new(mem) T();
        return obj;
}
    
}
#endif  // SCAL_UTIL_ALLOCATION_H_
