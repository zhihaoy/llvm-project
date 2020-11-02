// RUN: %clang_cc1 -std=c++2b -verify %s
// expected-no-diagnostics

void test_decay() {
  int v[3];
  static_assert(__is_same(decltype(auto(v)), int *));
  static_assert(__is_same(decltype(auto("lit")), char const *));

  int fn(char *);
  static_assert(__is_same(decltype(auto(fn)), int (*)(char *)));

  constexpr long i = 1;
  static_assert(__is_same(decltype(auto(1L)), long));
  static_assert(__is_same(decltype(i), long const));
  static_assert(__is_same(decltype(auto(i)), long));

  class A {
  } a;

  A &lr = a;
  A const &lrc = a;
  A &&rr = static_cast<A &&>(a);
  A const &&rrc = static_cast<A &&>(a);

  static_assert(__is_same(decltype(auto(lr)), A));
  static_assert(__is_same(decltype(auto(lrc)), A));
  static_assert(__is_same(decltype(auto(rr)), A));
  static_assert(__is_same(decltype(auto(rrc)), A));
}

class cmdline_parser {
public:
  cmdline_parser(char const *);
  auto add_option(char const *, char const *) && -> cmdline_parser &&;
};

void test_rvalue_fluent_interface() {
  auto cmdline = cmdline_parser("driver");
  auto internal = auto {cmdline}.add_option("--dump-full", "do not minimize dump");
}

class A {
  int x;
  friend void f(A &&);

public:
  A();

  auto test_access() {
    f(A(*this));    // ok
    f(auto(*this)); // ok in P0849
  }

protected:
  A(const A &);
};
