// RUN: %clang_cc1 -std=c++2b -verify %s

template <class T>
void foo(T);

struct A {
  int m;
  char g(int);
  float g(double);
} a{1};

void diagnostics() {
  foo(auto());   // expected-error {{initializer for functional-style cast to 'auto' is empty}}
  foo(auto {});  // expected-error {{initializer for functional-style cast to 'auto' is empty}}
  foo(auto({})); // expected-error {{initializer for functional-style cast to 'auto' is empty}}

  foo(auto(a));
  foo(auto {a});
  foo(auto(a));

  foo(auto(&A::g)); // expected-error {{functional-style cast to 'auto' has incompatible initializer of type '<overloaded function type>'}}

  foo(auto(a, 3.14));   // expected-error {{initializer for functional-style cast to 'auto' contains multiple expressions}}
  foo(auto {a, 3.14});  // expected-error {{initializer for functional-style cast to 'auto' contains multiple expressions}}
  foo(auto({a, 3.14})); // expected-error {{initializer for functional-style cast to 'auto' contains multiple expressions}}
}
