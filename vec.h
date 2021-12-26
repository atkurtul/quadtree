#include <stdint.h>
#include <iosfwd>

using f32 = float;
using f64 = double;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using i8 = int8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using u8 = uint8_t;

template <class Scalar, int>
struct vec;

template <class Scalar, int... n>
struct swizz;

template <class Scalar, int r, int c>
struct mat;

template <class V, class Scalar, int n>
concept veccy = requires(V v) {
  {vec<Scalar, n>(v)};
};

template <int h, int... n>
static constexpr int max = h > max<n...> ? h : max<n...>;

template <int n>
static constexpr int max<n> = n;

template <int...>
struct is_unique;

template <int...>
struct is_unique2;

template <int a, int b, int... t>
struct is_unique2<a, b, t...> {
  static constexpr bool val = a != b && is_unique2<a, t...>::val;
};

template <int a>
struct is_unique2<a> {
  static constexpr bool val = true;
};

template <int a, int b, int... t>
struct is_unique<a, b, t...> {
  static constexpr bool val =
      is_unique2<a, b, t...>::val && is_unique2<b, t...>::val;
};

template <int... t>
static constexpr bool is_unique_v = is_unique<t...>::val;

template <int h, int... t>
struct Seq0 : Seq0<h - 1, h, t...> {};

template <class Scalar, int n>
using Seq = typename Seq0<n - 1>::template Funcs<Scalar>;

template <int... t>
struct Seq0<-1, t...> {
  template <class Scalar>
  struct Funcs {
    static constexpr int n = sizeof...(t);
    using vt = vec<Scalar, n>;

    template <veccy<Scalar, n> V, int... i>
    static constexpr void assign(swizz<Scalar, i...>& l, V r) {
      ((l[t] = r[t]), ...);
    }

    static constexpr vt pow(Scalar x) {
      Scalar p = 1;
      vt v;
      (((v[n - t - 1] = p), p *= x), ...);
      return v;
    }

    static constexpr vt mul(vt l, vt r) { return vt{l[t] * r[t]...}; }
    static constexpr vt div(vt l, vt r) { return vt{l[t] / r[t]...}; }
    static constexpr vt add(vt l, vt r) { return vt{l[t] + r[t]...}; }
    static constexpr vt sub(vt l, vt r) { return vt{l[t] - r[t]...}; }
    static constexpr vt neg(vt v) { return vt{-v[t]...}; }

    static constexpr vt mul(Scalar l, vt r) { return vt{l * r[t]...}; }
    static constexpr vt div(Scalar l, vt r) { return vt{l / r[t]...}; }
    static constexpr vt mul(vt l, Scalar r) { return vt{l[t] * r...}; }
    static constexpr vt div(vt l, Scalar r) { return vt{l[t] / r...}; }

    static constexpr mat<Scalar, n, n> outer(vt l, vt r) {
      return mat<Scalar, n, n>{l[t] * r...};
    }

    static constexpr Scalar dot(vt l, vt r) { return hadd(l * r); }
    static constexpr Scalar hadd(vt v) { return (v[t] + ...); }
    static std::ostream& stream(std::ostream& s, vt v) {
      ((v[t] = (fabs(v[t]) < 0.00001f ? 0.f : v[t])), ...);
      return ((s << v[t] << ' '), ...) << "\b\n";
    }

    template <int a>
    static std::ostream& stream(std::ostream& s, mat<Scalar, a, n> m) {
      return ((s << m[t]), ...) << "\b\n";
    }

    template <int r>
    static constexpr vec<Scalar, r> mul(mat<Scalar, r, n> m, vt v) {
      return ((m[t] * v[t]) + ...);
    }

    template <int c>
    static constexpr vt mul(vec<Scalar, c> v, mat<Scalar, c, n> m) {
      return vt{dot(m[t], v)...};
    }

    template <int a, int b>
    static constexpr mat<Scalar, a, n> mul(mat<Scalar, a, b> l,
                                           mat<Scalar, b, n> r) {
      return mat<Scalar, a, n>{(l * r[t])...};
    }

    template <int a>
    static constexpr mat<Scalar, a, n> mul(mat<Scalar, a, n> l, Scalar r) {
      return mat<Scalar, a, n>{l[t] * r...};
    }

    template <int a>
    static constexpr mat<Scalar, a, n> mul(Scalar l, mat<Scalar, a, n> r) {
      return mat<Scalar, a, n>{l * r[t]...};
    }

    template <int a>
    static constexpr mat<Scalar, a, n> add(mat<Scalar, a, n> l,
                                           mat<Scalar, a, n> r) {
      return mat<Scalar, a, n>{l[t] + r[t]...};
    }

    template <int a>
    static constexpr mat<Scalar, a, n> sub(mat<Scalar, a, n> l,
                                           mat<Scalar, a, n> r) {
      return mat<Scalar, a, n>{l[t] - r[t]...};
    }

    template <int a, int i>
    static constexpr void write(mat<Scalar, a, n>& m, vt v) {
      ((m[t][i] = v[t]), ...);
    }

    template <int a>
    static constexpr mat<Scalar, n, a> tpos(mat<Scalar, a, n> m) {
      mat<Scalar, n, a> re;
      ((Seq<Scalar, a>::template write<n, t>(re, m[t])), ...);
      return re;
    }

    static constexpr mat<Scalar, n, n> fill(Scalar x) {
      mat<Scalar, n, n> m = {};
      ((m[t][t] = x), ...);
      return m;
    }
  };
};

template <class Scalar, int... n>
struct swizz {
  static constexpr int mx = max<n...> + 1;
  static constexpr int ns = sizeof...(n);
  static constexpr int idx[] = {n...};
  Scalar dat[mx];

  template <int... t>
  swizz& operator=(swizz<Scalar, t...> v) requires(ns == sizeof...(t) &&
                                                   is_unique_v<n...>) {
    return Seq<Scalar, ns>::assign(*this, v), *this;
  }

  swizz& operator=(swizz v) requires(is_unique_v<n...>) {
    return Seq<Scalar, ns>::assign(*this, v), *this;
  }

  template <veccy<Scalar, ns> V>
  swizz& operator=(V v) requires(is_unique_v<n...>) {
    Seq<Scalar, ns>::assign(*this, v);
    return *this;
  }

  using vt = vec<Scalar, sizeof...(n)>;
  constexpr operator vt() const { return vt{dat[n]...}; }

  Scalar& operator[](size_t i) { return dat[idx[i]]; };
  Scalar operator[](size_t i) const { return dat[idx[i]]; };

  vt operator*(vt r) const { return vt(*this) * r; }
  vt operator/(vt r) const { return vt(*this) / r; }
  vt operator+(vt r) const { return vt(*this) + r; }
  vt operator-(vt r) const { return vt(*this) - r; }

  vt operator*(Scalar r) const { return vt(*this) * r; }
  vt operator/(Scalar r) const { return vt(*this) / r; }
  friend vt operator*(Scalar l, swizz r) { return l * vt(r); }
  friend vt operator/(Scalar l, swizz r) { return l / vt(r); }

  friend std::ostream& operator<<(std::ostream& s, swizz v) {
    return s << vt(v);
  }
};

template <class Scalar>
struct vec<Scalar, 2> {
  union {
    Scalar dat[2];
    struct {
      Scalar x, y;
    };
#define SWIZZ_LEVEL 2
#include "swizz.inl"
  };

  vec& operator=(vec const& v) { return memcpy(this, &v, sizeof v), *this; };
  Scalar& operator[](size_t i) { return dat[i]; };
  Scalar operator[](size_t i) const { return dat[i]; };
  Scalar* begin() { return dat; }
  Scalar* end() { return dat + 2; }
  const Scalar* begin() const { return dat; }
  const Scalar* end() const { return dat + 2; }
};

template <class Scalar>
struct vec<Scalar, 3> {
  union {
    Scalar dat[3];
    struct {
      Scalar x, y, z;
    };
#define SWIZZ_LEVEL 3
#include "swizz.inl"
  };

  vec& operator=(vec const& v) { return memcpy(this, &v, sizeof v), *this; };
  Scalar& operator[](size_t i) { return dat[i]; };
  Scalar operator[](size_t i) const { return dat[i]; };
  Scalar* begin() { return dat; }
  Scalar* end() { return dat + 3; }
  const Scalar* begin() const { return dat; }
  const Scalar* end() const { return dat + 3; }
};

template <class Scalar>
struct vec<Scalar, 4> {
  union {
    Scalar dat[4];
    struct {
      Scalar x, y, z, w;
    };
#define SWIZZ_LEVEL 4
#include "swizz.inl"
  };

  vec& operator=(vec const& v) { return memcpy(this, &v, sizeof v), *this; };
  Scalar& operator[](size_t i) { return dat[i]; };
  Scalar operator[](size_t i) const { return dat[i]; };
  Scalar* begin() { return dat; }
  Scalar* end() { return dat + 4; }
  const Scalar* begin() const { return dat; }
  const Scalar* end() const { return dat + 4; }
};

template <class Scalar, int n>
struct vec {
  Scalar dat[n];
  Scalar& operator[](size_t i) { return dat[i]; };
  Scalar operator[](size_t i) const { return dat[i]; };

  Scalar* begin() { return dat; }
  Scalar* end() { return dat + n; }
  const Scalar* begin() const { return dat; }
  const Scalar* end() const { return dat + n; }
};

using vec2 = vec<f32, 2>;
using vec3 = vec<f32, 3>;
using vec4 = vec<f32, 4>;

using ivec2 = vec<i32, 2>;
using ivec3 = vec<i32, 3>;
using ivec4 = vec<i32, 4>;

using uvec2 = vec<u32, 2>;
using uvec3 = vec<u32, 3>;
using uvec4 = vec<u32, 4>;

using mat2 = mat<f32, 2, 2>;
using mat3 = mat<f32, 3, 3>;
using mat4 = mat<f32, 4, 4>;

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n>& operator*=(vec<Scalar, n>& l, V r) {
  return l = l * r;
}

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n>& operator/=(vec<Scalar, n>& l, V r) {
  return l = l / r;
}

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n>& operator+=(vec<Scalar, n>& l, V r) {
  return l = l + r;
}

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n>& operator-=(vec<Scalar, n>& l, V r) {
  return l = l - r;
}

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n> operator*(vec<Scalar, n> l, V r) {
  return Seq<Scalar, n>::mul(l, r);
}

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n> operator/(vec<Scalar, n> l, V r) {
  return Seq<Scalar, n>::div(l, r);
}

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n> operator+(vec<Scalar, n> l, V r) {
  return Seq<Scalar, n>::add(l, r);
}

template <class Scalar, int n, veccy<Scalar, n> V>
vec<Scalar, n> operator-(vec<Scalar, n> l, V r) {
  return Seq<Scalar, n>::sub(l, r);
}

template <class Scalar, int n>
vec<Scalar, n> operator-(vec<Scalar, n> v) {
  return Seq<Scalar, n>::neg(v);
}

template <class Scalar, int n>
vec<Scalar, n> operator*(vec<Scalar, n> l, Scalar r) {
  return Seq<Scalar, n>::mul(l, r);
}
template <class Scalar, int n>
vec<Scalar, n> operator/(vec<Scalar, n> l, Scalar r) {
  return Seq<Scalar, n>::div(l, r);
}

template <class Scalar, int n>
vec<Scalar, n> operator*(Scalar l, vec<Scalar, n> r) {
  return Seq<Scalar, n>::mul(l, r);
}

template <class Scalar, int n>
vec<Scalar, n> operator/(Scalar l, vec<Scalar, n> r) {
  return Seq<Scalar, n>::div(l, r);
}

#undef op0
#undef op

template <class Scalar, int n>
Scalar dot(vec<Scalar, n> l, vec<Scalar, n> r) {
  return Seq<Scalar, n>::dot(l, r);
}

template <class Scalar, int n>
mat<Scalar, n, n> outer(vec<Scalar, n> l, vec<Scalar, n> r) {
  return Seq<Scalar, n>::outer(l, r);
}

template <class Scalar, int n>
Scalar len(vec<Scalar, n> v) {
  return sqrt(Seq<Scalar, n>::dot(v, v));
}

template <class Scalar, int n>
Scalar hadd(vec<Scalar, n> v) {
  return Seq<Scalar, n>::hadd(v);
}

template <class Scalar, int n>
vec<Scalar, n> norm(vec<Scalar, n> v) {
  return v / sqrtf(dot(v, v));
}

template <class Scalar>
inline vec<Scalar, 3> cross(vec<Scalar, 3> l, vec<Scalar, 3> r) {
  return l.yzx * r.zxy - r.yzx * l.zxy;
}

template <class Scalar, int n>
std::ostream& operator<<(std::ostream& s, vec<Scalar, n> v) {
  return Seq<Scalar, n>::stream(s, v);
}

template <class Scalar, int a, int b>
std::ostream& operator<<(std::ostream& s, mat<Scalar, a, b> m) {
  return Seq<Scalar, b>::stream(s, m);
}

template <class Scalar, int r, int... n>
struct mat_swizz {
  using mt = mat<Scalar, r, sizeof...(n)>;

  static constexpr int mx = max<n...> + 1;
  static constexpr int idx[] = {n...};

  vec<Scalar, r> dat[mx];

  constexpr operator mt() const { return mt{dat[n]...}; }

  vec<Scalar, r>& operator[](size_t i) { return dat[idx[i]]; };
  vec<Scalar, r> operator[](size_t i) const { return dat[idx[i]]; };
};

template <class Scalar, int r>
struct mat<Scalar, r, 2> {
  template <class S, int... n>
  using swizz = mat_swizz<S, r, n...>;
  union {
    vec<Scalar, r> dat[2];
    struct {
      vec<Scalar, r> x, y;
    };
#define SWIZZ_LEVEL 2
#include "swizz.inl"
  };
  mat& operator=(mat const& m) {
    memcpy(dat, &m, sizeof m);
    return *this;
  }
  vec<Scalar, r>& operator[](size_t i) { return dat[i]; };
  vec<Scalar, r> operator[](size_t i) const { return dat[i]; };
};

template <class Scalar, int r>
struct mat<Scalar, r, 3> {
  template <class S, int... n>
  using swizz = mat_swizz<S, r, n...>;
  union {
    vec<Scalar, r> dat[3];
    struct {
      vec<Scalar, r> x, y, z;
    };
#define SWIZZ_LEVEL 3
#include "swizz.inl"
  };

  mat& operator=(mat const& m) {
    memcpy(dat, &m, sizeof m);
    return *this;
  }
  vec<Scalar, r>& operator[](size_t i) { return dat[i]; };
  vec<Scalar, r> operator[](size_t i) const { return dat[i]; };
};

template <class Scalar, int r>
struct mat<Scalar, r, 4> {
  template <class S, int... n>
  using swizz = mat_swizz<S, r, n...>;
  union {
    vec<Scalar, r> dat[4];
    struct {
      vec<Scalar, r> x, y, z, w;
    };
#define SWIZZ_LEVEL 4
#include "swizz.inl"
  };

  mat& operator=(mat const& m) {
    memcpy(dat, &m, sizeof m);
    return *this;
  }
  vec<Scalar, r>& operator[](size_t i) { return dat[i]; };
  vec<Scalar, r> operator[](size_t i) const { return dat[i]; };
};

template <int n, class Scalar>
mat<Scalar, n, n> identity(Scalar x) {
  return Seq<Scalar, n>::fill(x);
}

template <class Scalar, int r, int n, veccy<Scalar, n> V>
vec<Scalar, r> operator*(mat<Scalar, r, n> m, V v) {
  return Seq<Scalar, n>::mul(m, v);
}

template <class Scalar, int c, int n, veccy<Scalar, c> V>
vec<Scalar, n> operator*(V v, mat<Scalar, c, n> m) {
  return Seq<Scalar, n>::mul(vec<Scalar, c>(v), m);
}

template <class Scalar, int a, int b, int n>
mat<Scalar, a, n> operator*(mat<Scalar, a, b> l, mat<Scalar, b, n> r) {
  return Seq<Scalar, n>::mul(l, r);
}

template <class Scalar, int a, int n>
mat<Scalar, a, n> operator*(mat<Scalar, a, n> l, Scalar r) {
  return Seq<Scalar, n>::mul(l, r);
}

template <class Scalar, int a, int n>
mat<Scalar, a, n> operator*(Scalar l, mat<Scalar, a, n> r) {
  return Seq<Scalar, n>::mul(l, r);
}

template <class Scalar, int a, int b>
mat<Scalar, a, b> operator+(mat<Scalar, a, b> l, mat<Scalar, a, b> r) {
  return Seq<Scalar, b>::add(l, r);
}

template <class Scalar, int a, int b>
mat<Scalar, a, b> operator-(mat<Scalar, a, b> l, mat<Scalar, a, b> r) {
  return Seq<Scalar, b>::sub(l, r);
}

template <class Scalar, int a, int b>
mat<Scalar, b, a> tpos(mat<Scalar, a, b> m) {
  return Seq<Scalar, b>::tpos(m);
}