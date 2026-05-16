#pragma once

// XIII Engine — Core platform types and utilities
// Adapted from SurrealEngine, simplified for UE2/XIII

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <array>
#include <algorithm>
#include <stdexcept>

// Platform detection
#ifdef XIII_PLATFORM_R36S
#define XIII_ARM64 1
#endif

// Array alias (matches SurrealEngine convention)
template<typename T>
using Array = std::vector<T>;

// Exception helper
namespace Xiii {
inline void Throw(const std::string& msg) {
    fprintf(stderr, "[XIII FATAL] %s\n", msg.c_str());
    throw std::runtime_error(msg);
}
}
