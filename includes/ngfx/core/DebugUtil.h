/*
 * Copyright 2020 GoPro Inc.
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
#pragma once
#include <cstdio>
#include <cstdlib>
#include <execinfo.h>
#ifndef __PRETTY_FUNCTION__
#define __PRETTY_FUNCTION__ __FUNCTION__
#endif
#define NGFX_LOG(fmt, ...) fprintf(stderr, fmt, "\n", ##__VA_ARGS__)
#define NGFX_LOG_TRACE(fmt, ...)                                               \
  NGFX_LOG("[%s][%s][%d] " fmt, __FILE__, __PRETTY_FUNCTION__, __LINE__,       \
           ##__VA_ARGS__)
#define NGFX_TODO(fmt, ...)                                                    \
  NGFX_LOG("[%s][%s][%d] TODO: " fmt, __FILE__, __FUNCTION__, __LINE__,        \
           ##__VA_ARGS__)
#include <cstdint>
struct DebugUtil {
  static inline void Exit(uint32_t code) { exit(code); };
};
#define NGFX_ERR(fmt, ...)                                                     \
  {                                                                            \
    fprintf(stderr, "ERROR: [%s][%s][%d] " fmt "\n", __FILE__,                 \
            __PRETTY_FUNCTION__, __LINE__, ##__VA_ARGS__, "\n");               \
    /*TODO Add Linux Preprocessor Directive*/                                  \
    void *array[10];                                                           \
    size_t size;                                                               \
    char **strings;                                                            \
    size = backtrace(array, 10);                                               \
    strings = backtrace_symbols(array, size);                                  \
    if (strings == NULL) {                                                     \
        perror("backtrace_symbols");                                           \
        exit(EXIT_FAILURE);                                                    \
    }                                                                          \
    for (int j = 0; j < size; j++)                                             \
        printf("%s\n", strings[j]);                                            \
    free(strings);                                                             \
    DebugUtil::Exit(1);                                                        \
  }
