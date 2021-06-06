#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__
#include <cmath>
#include <vector>
#include <cassert>
#include <iostream>

template<size_t DimCols,size_t DimRows,typename T> class mat;

template <size_t DIM, typename T> struct vec {
    vec() { for (size_t i=DIM; i--; data_[i] = T()); }
          T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
    const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }
private:
    T data_[DIM];
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<2,T> {
    vec() : x(T()), y(T()) {}
    vec(T X, T Y) : x(X), y(Y) {}
    template <class U> vec<2,T>(const vec<2,U> &v);
          T& operator[](const size_t i)       { assert(i<2); return i<=0 ? x : y; }
    const T& operator[](const size_t i) const { assert(i<2); return i<=0 ? x : y; }

    T x,y;
};

/////////////////////////////////////////////////////////////////////////////////

template <typename T> struct vec<3,T> {
    vec() : x(T()), y(T()), z(T()) {}
    vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
    template <class U> vec<3,T>(const vec<3,U> &v);
          T& operator[](const size_t i)       { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    const T& operator[](const size_t i) const { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
    float norm() { return std::sqrt(x*x+y*y+z*z); }
    vec<3,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }

    T x,y,z;
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DIM,typename T> T operator*(const vec<DIM,T>& lhs, const vec<DIM,T>& rhs) {
    T ret = T();
    for (size_t i=DIM; i--; ret+=lhs[i]*rhs[i]);
    return ret;
}


template<size_t DIM,typename T>vec<DIM,T> operator+(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
    for (size_t i=DIM; i--; lhs[i]+=rhs[i]);
    return lhs;
}

template<size_t DIM,typename T>vec<DIM,T> operator-(vec<DIM,T> lhs, const vec<DIM,T>& rhs) {
    for (size_t i=DIM; i--; lhs[i]-=rhs[i]);
    return lhs;
}

template<size_t DIM,typename T,typename U> vec<DIM,T> operator*(vec<DIM,T> lhs, const U& rhs) {
    for (size_t i=DIM; i--; lhs[i]*=rhs);
    return lhs;
}

template<size_t DIM,typename T,typename U> vec<DIM,T> operator/(vec<DIM,T> lhs, const U& rhs) {
    for (size_t i=DIM; i--; lhs[i]/=rhs);
    return lhs;
}

template<size_t LEN,size_t DIM,typename T> vec<LEN,T> embed(const vec<DIM,T> &v, T fill=1) {
    vec<LEN,T> ret;
    for (size_t i=LEN; i--; ret[i]=(i<DIM?v[i]:fill));
    return ret;
}

template<size_t LEN,size_t DIM, typename T> vec<LEN,T> proj(const vec<DIM,T> &v) {
    vec<LEN,T> ret;
    for (size_t i=LEN; i--; ret[i]=v[i]);
    return ret;
}

template <typename T> vec<3,T> cross(vec<3,T> v1, vec<3,T> v2) {
    return vec<3,T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
}

template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, vec<DIM,T>& v) {
    for(unsigned int i=0; i<DIM; i++) {
        out << v[i] << " " ;
    }
    return out ;
}

/////////////////////////////////////////////////////////////////////////////////

template<size_t DIM,typename T> struct dt {
    static T det(const mat<DIM,DIM,T>& src) {
        T ret=0;
        for (size_t i=DIM; i--; ret += src[0][i]*src.cofactor(0,i));
        return ret;
    }
};

template<typename T> struct dt<1,T> {
    static T det(const mat<1,1,T>& src) {
        return src[0][0];
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DimRows,size_t DimCols,typename T> class mat {
    vec<DimCols,T> rows[DimRows];
public:
    mat() {}

    vec<DimCols,T>& operator[] (const size_t idx) {
        assert(idx<DimRows);
        return rows[idx];
    }

    const vec<DimCols,T>& operator[] (const size_t idx) const {
        assert(idx<DimRows);
        return rows[idx];
    }

    vec<DimRows,T> col(const size_t idx) const {
        assert(idx<DimCols);
        vec<DimRows,T> ret;
        for (size_t i=DimRows; i--; ret[i]=rows[i][idx]);
        return ret;
    }

    void set_col(size_t idx, vec<DimRows,T> v) {
        assert(idx<DimCols);
        for (size_t i=DimRows; i--; rows[i][idx]=v[i]);
    }

    static mat<DimRows,DimCols,T> identity() {
        mat<DimRows,DimCols,T> ret;
        for (size_t i=DimRows; i--; )
            for (size_t j=DimCols;j--; ret[i][j]=(i==j));
        return ret;
    }

    T det() const {
        return dt<DimCols,T>::det(*this);
    }

    mat<DimRows-1,DimCols-1,T> get_minor(size_t row, size_t col) const {
        mat<DimRows-1,DimCols-1,T> ret;
        for (size_t i=DimRows-1; i--; )
            for (size_t j=DimCols-1;j--; ret[i][j]=rows[i<row?i:i+1][j<col?j:j+1]);
        return ret;
    }

    T cofactor(size_t row, size_t col) const {
        return get_minor(row,col).det()*((row+col)%2 ? -1 : 1);
    }

    mat<DimRows,DimCols,T> adjugate() const {
        mat<DimRows,DimCols,T> ret;
        for (size_t i=DimRows; i--; )
            for (size_t j=DimCols; j--; ret[i][j]=cofactor(i,j));
        return ret;
    }

    mat<DimRows,DimCols,T> invert_transpose() {
        mat<DimRows,DimCols,T> ret = adjugate();
        T tmp = ret[0]*rows[0];
        return ret/tmp;
    }

    mat<DimRows,DimCols,T> invert() {
        return invert_transpose().transpose();
    }

    mat<DimCols,DimRows,T> transpose() {
        mat<DimCols,DimRows,T> ret;
        for (size_t i=DimCols; i--; ret[i]=this->col(i));
        return ret;
    }
};

/////////////////////////////////////////////////////////////////////////////////

template<size_t DimRows,size_t DimCols,typename T> vec<DimRows,T> operator*(const mat<DimRows,DimCols,T>& lhs, const vec<DimCols,T>& rhs) {
    vec<DimRows,T> ret;
    for (size_t i=DimRows; i--; ret[i]=lhs[i]*rhs);
    return ret;
}

template<size_t R1,size_t C1,size_t C2,typename T>mat<R1,C2,T> operator*(const mat<R1,C1,T>& lhs, const mat<C1,C2,T>& rhs) {
    mat<R1,C2,T> result;
    for (size_t i=R1; i--; )
        for (size_t j=C2; j--; result[i][j]=lhs[i]*rhs.col(j));
    return result;
}

template<size_t DimRows,size_t DimCols,typename T>mat<DimCols,DimRows,T> operator/(mat<DimRows,DimCols,T> lhs, const T& rhs) {
    for (size_t i=DimRows; i--; lhs[i]=lhs[i]/rhs);
    return lhs;
}

template <size_t DimRows,size_t DimCols,class T> std::ostream& operator<<(std::ostream& out, mat<DimRows,DimCols,T>& m) {
    for (size_t i=0; i<DimRows; i++) out << m[i] << std::endl;
    return out;
}

/////////////////////////////////////////////////////////////////////////////////

typedef vec<2,  float> Vec2f;
typedef vec<2,  int>   Vec2i;
typedef vec<3,  float> Vec3f;
typedef vec<3,  int>   Vec3i;
typedef vec<4,  float> Vec4f;
typedef mat<4,4,float> Matrix;
#endif //__GEOMETRY_H__


// #ifndef __GEOMETRY_H__
// #define __GEOMETRY_H__
// #include <iostream>
// #include <cmath>
// template<int n> struct vec {
//     vec() = default;
//     float & operator[](const int i)       { (i>=0 && i<n); return data[i]; }
//     float   operator[](const int i) const { (i>=0 && i<n); return data[i]; }
//     float norm2() const { return (*this)*(*this) ; }
//     float norm()  const { return std::sqrt(norm2()); }
//     float data[n] = {0};
// };
// template<int n> float operator*(const vec<n>& lhs, const vec<n>& rhs) {
//     float ret = 0;
//     for (int i=n; i--; ret+=lhs[i]*rhs[i]);
//     return ret;
// }

// template<int n> vec<n> operator+(const vec<n>& lhs, const vec<n>& rhs) {
//     vec<n> ret = lhs;
//     for (int i=n; i--; ret[i]+=rhs[i]);
//     return ret;
// }

// template<int n> vec<n> operator-(const vec<n>& lhs, const vec<n>& rhs) {
//     vec<n> ret = lhs;
//     for (int i=n; i--; ret[i]-=rhs[i]);
//     return ret;
// }

// template<int n> vec<n> operator*(const float& rhs, const vec<n> &lhs) {
//     vec<n> ret = lhs;
//     for (int i=n; i--; ret[i]*=rhs);
//     return ret;
// }

// template<int n> vec<n> operator*(const vec<n>& lhs, const float& rhs) {
//     vec<n> ret = lhs;
//     for (int i=n; i--; ret[i]*=rhs);
//     return ret;
// }

// template<int n> vec<n> operator/(const vec<n>& lhs, const float& rhs) {
//     vec<n> ret = lhs;
//     for (int i=n; i--; ret[i]/=rhs);
//     return ret;
// }

// template<int n1,int n2> vec<n1> embed(const vec<n2> &v, float fill=1) {
//     vec<n1> ret;
//     for (int i=n1; i--; ret[i]=(i<n2?v[i]:fill));
//     return ret;
// }

// template<int n1,int n2> vec<n1> proj(const vec<n2> &v) {
//     vec<n1> ret;
//     for (int i=n1; i--; ret[i]=v[i]);
//     return ret;
// }


// template<int n> std::ostream& operator<<(std::ostream& out, const vec<n>& v) {
//     for (int i=0; i<n; i++) out << v[i] << " ";
//     return out;
// }
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// template <class t> struct Vec2 {
// 	union {
// 		struct {t u, v;};
// 		struct {t x, y;};
// 		t raw[2];
// 	};
// 	Vec2() : u(0), v(0) {}
// 	Vec2(t _u, t _v) : u(_u),v(_v) {}
// 	inline Vec2<t> operator +(const Vec2<t> &V) const { return Vec2<t>(u+V.u, v+V.v); }
// 	inline Vec2<t> operator -(const Vec2<t> &V) const { return Vec2<t>(u-V.u, v-V.v); }
// 	inline Vec2<t> operator *(float f)          const { return Vec2<t>(u*f, v*f); }
// 	inline t&  operator [](int i) { return raw[i]; }
// 	template <class > friend std::ostream& operator<<(std::ostream& s, Vec2<t>& v);
// };

// template <class t> struct Vec3 {
// 	union {
// 		struct {t x, y, z;};
// 		struct { t ivert, iuv, inorm; };
// 		t raw[3];
// 	};
// 	Vec3() : x(0), y(0), z(0) {}
// 	Vec3(t _x, t _y, t _z) : x(_x),y(_y),z(_z) {}
// 	inline Vec3<t> operator ^(const Vec3<t> &v) const { return Vec3<t>(y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x); }
// 	inline Vec3<t> operator +(const Vec3<t> &v) const { return Vec3<t>(x+v.x, y+v.y, z+v.z); }
// 	inline Vec3<t> operator -(const Vec3<t> &v) const { return Vec3<t>(x-v.x, y-v.y, z-v.z); }
// 	inline Vec3<t> operator *(float f)          const { return Vec3<t>(x*f, y*f, z*f); }
// 	inline t       operator *(const Vec3<t> &v) const { return x*v.x + y*v.y + z*v.z; }
// 	t&  operator [](int i) { return raw[i]; }
// 	float norm () const { return std::sqrt(x*x+y*y+z*z); }
// 	Vec3<t> & normalize(t l=1) { *this = (*this)*(l/norm()); return *this; }
// 	template <class > friend std::ostream& operator<<(std::ostream& s, Vec3<t>& v);
// };

// typedef Vec2<float> Vec2f;
// typedef Vec2<int>   Vec2i;
// typedef Vec3<float> Vec3f;
// typedef Vec3<int>   Vec3i;

// template <class t> std::ostream& operator<<(std::ostream& s, Vec2<t>& v) {
// 	s << "(" << v.x << ", " << v.y << ")\n";
// 	return s;
// }

// template <class t> std::ostream& operator<<(std::ostream& s, Vec3<t>& v) {
// 	s << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
// 	return s;
// }

// template<int nrows,int ncols> struct mat {
//     vec<ncols> rows[nrows];
//     mat() = default;
//           vec<ncols>& operator[] (const int idx)       { return rows[idx]; }
//     const vec<ncols>& operator[] (const int idx) const { return rows[idx]; }
//     vec<nrows> col(const int idx) const {
//         vec<nrows> ret;
//         for (int i=nrows; i--; ret[i]=rows[i][idx]);
//         return ret;
//     }

//     void set_col(const int idx, const vec<nrows> &v) {
//         for (int i= 0;i<nrows;i++){
//             rows[i][idx]=v[i];
//         }
//     }
//     void set_row(const int idx, const vec<nrows> &v) {
//         for(int i=0;i<nrows;i++){
//             rows[idx][i]=v[i];
//         }
//     }

//     static mat<nrows,ncols> identity() {
//         mat<nrows,ncols> ret;
//         for(int i=0;i<nrows;i++){
//             for(int j=0;j<ncols;j++){
//                  ret[i][j]=(i==j);
//             }
//         }
//         return ret;
//     }

//     mat<nrows-1,ncols-1> get_minor(const int row, const int col) const {
//         mat<nrows-1,ncols-1> ret;
//         for (int i=nrows-1; i--; )
//             for (int j=ncols-1;j--; ret[i][j]=rows[i<row?i:i+1][j<col?j:j+1]);
//         return ret;
//     }

// };
// template<int nrows,int ncols> vec<nrows> operator*(const mat<nrows,ncols>& lhs, const vec<ncols>& rhs) {
//     vec<nrows> ret;
//     for (int i=nrows; i--; ret[i]=lhs[i]*rhs);
//     return ret;
// }

// template<int R1,int C1,int C2>mat<R1,C2> operator*(const mat<R1,C1>& lhs, const mat<C1,C2>& rhs) {
//     mat<R1,C2> result;
//     for (int i=R1; i--; )
//         for (int j=C2; j--; result[i][j]=lhs[i]*rhs.col(j));
//     return result;
// }

// template<int nrows,int ncols>mat<nrows,ncols> operator*(const mat<nrows,ncols>& lhs, const float& val) {
//     mat<nrows,ncols> result;
//     for (int i=nrows; i--; result[i] = lhs[i]*val);
//     return result;
// }

// template<int nrows,int ncols>mat<nrows,ncols> operator/(const mat<nrows,ncols>& lhs, const float& val) {
//     mat<nrows,ncols> result;
//     for (int i=nrows; i--; result[i] = lhs[i]/val);
//     return result;
// }

// template<int nrows,int ncols>mat<nrows,ncols> operator+(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
//     mat<nrows,ncols> result;
//     for (int i=nrows; i--; )
//         for (int j=ncols; j--; result[i][j]=lhs[i][j]+rhs[i][j]);
//     return result;
// }

// template<int nrows,int ncols>mat<nrows,ncols> operator-(const mat<nrows,ncols>& lhs, const mat<nrows,ncols>& rhs) {
//     mat<nrows,ncols> result;
//     for (int i=nrows; i--; )
//         for (int j=ncols; j--; result[i][j]=lhs[i][j]-rhs[i][j]);
//     return result;
// }

// template<int nrows,int ncols> std::ostream& operator<<(std::ostream& out, const mat<nrows,ncols>& m) {
//     for (int i=0; i<nrows; i++) out << m[i] << std::endl;
//     return out;
// }

// /////////////////////////////////////////////////////////////////////////////////

// typedef vec<2> vec2;
// typedef vec<3> vec3;
// typedef vec<4> vec4;
// vec3 cross(const vec3 &v1, const vec3 &v2);
// #endif //__GEOMETRY_H__
