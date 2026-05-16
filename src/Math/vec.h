#pragma once
#include <cmath>

struct Vec3 {
    float x, y, z;
    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    float Length() const { return std::sqrt(x*x + y*y + z*z); }
    Vec3 Normalized() const { float l = Length(); return l > 0 ? Vec3(x/l, y/l, z/l) : Vec3(); }
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    float Dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    Vec3 Cross(const Vec3& o) const { return {y*o.z-z*o.y, z*o.x-x*o.z, x*o.y-y*o.x}; }
};
