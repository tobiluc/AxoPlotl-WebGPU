#pragma once
#include <array>
#include <cmath>
#include <ostream>

namespace AxoPlotl
{

template<typename T, int DIM>
struct Vec
{
    std::array<T,DIM> e;

    inline T& operator[](int i) {return e[i];}
    inline const T& operator[](int i) const {return e[i];}
    inline T dot(const Vec& _rhs) const {
        T res(0);
        for (int i = 0; i < DIM; ++i) {
            res += e[i]*_rhs.e[i];
        }
        return res;
    }
    inline Vec operator-() const {
        std::array<T,DIM> res;
        for (int i = 0; i < DIM; ++i) {
            res[i] = -e[i];
        }
        return {res};
    }
    inline Vec operator+(const Vec& _v) const {
        std::array<T,DIM> res;
        for (int i = 0; i < DIM; ++i) {
            res[i] = e[i] + _v.e[i];
        }
        return {res};
    }
    inline Vec operator-(const Vec& _v) const {
        return *this + (-_v);
    }
    inline Vec operator*(const T& _s) const {
        std::array<T,DIM> res;
        for (int i = 0; i < DIM; ++i) {
            res[i] = e[i] * _s;
        }
        return {res};
    }
    inline Vec operator/(const T& _s) const {
        return *this * (1.0/_s);
    }
    inline T squared_norm() const {
        return dot(*this);
    }
    inline T norm() const {
        return std::sqrt(this->dot(*this));
    }
    inline Vec normalized() const {
        return *this / norm();
    }
    inline static constexpr Vec zero() {
        std::array<T,DIM> z;
        for (int i = 0; i < DIM; ++i) {z[i] = 0;}
        return {z};
    }
};

template<typename T, int DIM>
inline std::ostream& operator<<(std::ostream& _os, const Vec<T,DIM>& _v)
{
    for (int i = 0; i < DIM; ++i) {
        _os << _v[i] << " ";
    }
    return _os;
}

}
