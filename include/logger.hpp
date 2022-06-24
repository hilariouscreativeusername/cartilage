#pragma once

#include <cstdio>

#ifndef NDEBUG
#define CR_LOG_INFO(...)  printf("[INFO] ");  printf(__VA_ARGS__);
#else
#define CR_LOG_INFO(...)
#endif

#define CR_LOG_WARN(...)  printf("[WARN] ");  fprintf(stderr, __VA_ARGS__);
#define CR_LOG_ERROR(...) printf("[ERROR] "); fprintf(stderr, __VA_ARGS__);
