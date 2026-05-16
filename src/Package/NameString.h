#pragma once

#include "xiii.h"

// NameString — interned string with case-insensitive comparison
// Adapted from SurrealEngine's NameString

class NameString
{
public:
    NameString() {}
    NameString(const char* str) { GetIndex(str); }
    NameString(const std::string& str) { GetIndex(str); }
    NameString(const NameString& other) = default;
    NameString& operator=(const NameString&) = default;

    bool IsNone() const { return CompareIndex == 0; }
    const std::string& ToString() const { return Names[SpelledIndex]; }

    bool operator==(const char* other) const { return *this == NameString(other); }
    bool operator==(const std::string& other) const { return *this == NameString(other); }
    bool operator!=(const char* other) const { return *this != NameString(other); }
    bool operator!=(const std::string& other) const { return *this != NameString(other); }

    bool operator==(const NameString& other) const { return CompareIndex == other.CompareIndex; }
    bool operator!=(const NameString& other) const { return CompareIndex != other.CompareIndex; }
    bool operator<(const NameString& other) const { return CompareIndex < other.CompareIndex; }
    bool operator>(const NameString& other) const { return CompareIndex > other.CompareIndex; }

    int GetCompareIndex() const { return CompareIndex; }

private:
    int CompareIndex = 0;
    int SpelledIndex = 0;

    void GetIndex(const std::string& value);

    static Array<std::string> Names;
    static std::unordered_map<std::string, int> CompareStringToIndex;
    static std::unordered_map<std::string, std::pair<int, int>> SpellStringToIndex;
};
