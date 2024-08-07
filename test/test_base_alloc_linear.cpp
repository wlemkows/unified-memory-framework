/*
 * Copyright (C) 2024 Intel Corporation
 *
 * Under the Apache License v2.0 with LLVM Exceptions. See LICENSE.TXT.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
*/

#include <cstdio>
#include <cstdlib>
#include <thread>

#include "base_alloc_linear.h"

#include "base.hpp"
#include "test_helpers.h"

using umf_test::test;

TEST_F(test, baseAllocLinearMultiThreadedAllocMemset) {
    static constexpr int NTHREADS = 10;
    static constexpr int ITERATIONS = 1000;
    static constexpr int MAX_ALLOCATION_SIZE = 1024;

    srand(0);

    auto pool = std::shared_ptr<umf_ba_linear_pool_t>(
        umf_ba_linear_create(NTHREADS * ITERATIONS * MAX_ALLOCATION_SIZE),
        umf_ba_linear_destroy);

    auto poolAlloc = [](int TID, umf_ba_linear_pool_t *pool) {
        struct buffer_t {
            unsigned char *ptr;
            size_t size;
        } buffer[ITERATIONS];

        for (int i = 0; i < ITERATIONS; i++) {
            buffer[i].size =
                (size_t)((rand() / (double)RAND_MAX) * MAX_ALLOCATION_SIZE);
            buffer[i].ptr =
                (unsigned char *)umf_ba_linear_alloc(pool, buffer[i].size);
            UT_ASSERTne(buffer[i].ptr, NULL);
            memset(buffer[i].ptr, (i + TID) & 0xFF, buffer[i].size);
        }

        for (int i = 0; i < ITERATIONS; i++) {
            UT_ASSERTne(buffer[i].ptr, NULL);
            for (size_t k = 0; k < buffer[i].size; k++) {
                UT_ASSERTeq(*(buffer[i].ptr + k), (i + TID) & 0xFF);
            }
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < NTHREADS; i++) {
        threads.emplace_back(poolAlloc, i, pool.get());
    }

    for (auto &thread : threads) {
        thread.join();
    }
}
