// RUN: %clang_cc1 -std=c++2b -Wno-decltype-auto-cast -verify %s

// p2.3 allows only T = auto in T(x).
// As a Clang extension, we also allow T = decltype(auto) to match p2.2 (new T(x)).

void test_decay() {
  int v[3];
  static_assert(__is_same(decltype(auto(v)), int *));
  static_assert(__is_same(decltype(auto{v}), int *));
  static_assert(__is_same(decltype(auto("lit")), char const *));
  static_assert(__is_same(decltype(auto{"lit"}), char const *));

  void(decltype(auto)(v)); // expected-error {{functional-style cast}}
  void(decltype(auto){v}); // expected-error {{cannot initialize an array element}}
  static_assert(__is_same(decltype(decltype(auto)("lit")), char const(&)[4]));
  static_assert(__is_same(decltype(decltype(auto){"lit"}), char const(&)[4]));

  int fn(char *);
  static_assert(__is_same(decltype(auto(fn)), int (*)(char *)));
  static_assert(__is_same(decltype(auto{fn}), int (*)(char *)));

  void(decltype(auto)(fn)); // expected-error{{functional-style cast}}
  void(decltype(auto){fn}); // expected-error{{cannot create object of function type}}

  constexpr long i = 1;
  static_assert(__is_same(decltype(i), long const));
  static_assert(__is_same(decltype(auto(1L)), long));
  static_assert(__is_same(decltype(auto{1L}), long));
  static_assert(__is_same(decltype(auto(i)), long));
  static_assert(__is_same(decltype(auto{i}), long));

  // scalar prvalue is not cv-qualified
  static_assert(__is_same(decltype(decltype(auto)(1L)), long));
  static_assert(__is_same(decltype(decltype(auto){1L}), long));
  static_assert(__is_same(decltype(decltype(auto)(i)), long));
  static_assert(__is_same(decltype(decltype(auto){i}), long));

  class A {
  } a;
  A const ac;

  static_assert(__is_same(decltype(auto(a)), A));
  static_assert(__is_same(decltype(auto(ac)), A));

  static_assert(__is_same(decltype(decltype(auto)(a)), A));
  static_assert(__is_same(decltype(decltype(auto)(ac)), A const));

  static_assert(__is_same(decltype(decltype(auto)((a))), A &));
  static_assert(__is_same(decltype(decltype(auto)((ac))), A const &));

  static_assert(__is_same(decltype(decltype(auto){a}), A));
  static_assert(__is_same(decltype(decltype(auto){ac}), A const));

  static_assert(__is_same(decltype(decltype(auto){(a)}), A &));
  static_assert(__is_same(decltype(decltype(auto){(ac)}), A const &));

  A &lr = a;
  A const &lrc = a;
  A &&rr = static_cast<A &&>(a);
  A const &&rrc = static_cast<A &&>(a);

  static_assert(__is_same(decltype(auto(lr)), A));
  static_assert(__is_same(decltype(auto(lrc)), A));
  static_assert(__is_same(decltype(auto(rr)), A));
  static_assert(__is_same(decltype(auto(rrc)), A));

  static_assert(__is_same(decltype(decltype(auto)(lr)), A &));
  static_assert(__is_same(decltype(decltype(auto)(lrc)), A const &));
  static_assert(__is_same(decltype(decltype(auto)(rr)), A &&));
  static_assert(__is_same(decltype(decltype(auto)(rrc)), A const &&));
}

class cmdline_parser {
public:
  cmdline_parser(char const *);
  auto add_option(char const *, char const *) && -> cmdline_parser &&;
};

void test_rvalue_fluent_interface() {
  auto cmdline = cmdline_parser("driver");
  auto internal = auto{cmdline}.add_option("--dump-full", "do not minimize dump");
}

template <class T> constexpr auto decay_copy(T &&v) { return static_cast<T &&>(v); } // expected-error {{calling a protected constructor}}

class A {
  int x;
  friend void f(A &&);

public:
  A();

  auto test_access() {
    static_assert(__is_same(decltype(auto(*this)), A));
    static_assert(__is_same(decltype(auto(this)), A *));

    f(A(*this));          // ok
    f(auto(*this));       // ok in P0849
    f(decay_copy(*this)); // expected-note {{in instantiation of function template specialization}}
  }

  auto test_access() const {
    static_assert(__is_same(decltype(auto(*this)), A)); // ditto
    static_assert(__is_same(decltype(auto(this)), A const *));
  }

protected:
  A(const A &); // expected-note {{declared protected here}}
};

// post-C++17 semantics
namespace auto_x {
constexpr struct Uncopyable {
  constexpr explicit Uncopyable(int) {}
  constexpr Uncopyable(Uncopyable &&) = delete;
} u = auto(Uncopyable(auto(Uncopyable(42))));
} // namespace auto_x

// decltype(auto) is no-op to prvalues
namespace decltype_auto_x {
constexpr struct Uncopyable {
  constexpr explicit Uncopyable(int) {}
  constexpr Uncopyable(Uncopyable &&) = delete;
} u = decltype(auto)(Uncopyable(decltype(auto)(Uncopyable(42))));
} // namespace decltype_auto_x

// Forward with decltype(auto)
constexpr auto invoke1 = [](auto &&x, auto &&y) {
  return decltype(auto)(x)(decltype(auto)(y));
};

struct MoveOnly {
  MoveOnly() = default;
  MoveOnly(MoveOnly &&) = default;
  MoveOnly(MoveOnly const &) = delete;
};

constexpr MoveOnly getMoveOnly() { return {}; }

struct Fn {
  constexpr int operator()(MoveOnly &) & { return 0; }
  constexpr int operator()(MoveOnly &&) & { return 1; }

  constexpr int operator()(MoveOnly &) && { return 2; }
  constexpr int operator()(MoveOnly &&) && { return 3; }

  constexpr int operator()(MoveOnly &) const & { return 4; }
  constexpr int operator()(MoveOnly &&) const & { return 5; }

  constexpr int operator()(MoveOnly &) const && { return 6; }
  constexpr int operator()(MoveOnly &&) const && { return 7; }
};

constexpr void FwdWithDecltypeAuto() {
  MoveOnly lv;
  Fn f;
  constexpr Fn cf;

  static_assert(invoke1(f, lv) == 0);
  static_assert(invoke1(f, getMoveOnly()) == 1);

  static_assert(invoke1(Fn{}, lv) == 2);
  static_assert(invoke1(Fn{}, getMoveOnly()) == 3);

  static_assert(invoke1(cf, lv) == 4);
  static_assert(invoke1(cf, getMoveOnly()) == 5);

  static_assert(invoke1((Fn const){}, lv) == 6);
  static_assert(invoke1((Fn const){}, getMoveOnly()) == 7);
}

struct FnArray {
  template <class T, int N>
  constexpr int operator()(T (&)[N]) const { return 0; }

  template <class T, int N>
  constexpr int operator()(T (&&)[N]) const { return 1; }
};

constexpr void FwdArrayWithDecltypeAuto() {
  FnArray f;

  static_assert(invoke1(f, "foo") == 0);
  static_assert(invoke1(f, (int[]){1, 2, 3}) == 1);
}
