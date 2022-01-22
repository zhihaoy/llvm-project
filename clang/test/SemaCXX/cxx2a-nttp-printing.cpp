// RUN: %clang_cc1 %s -std=c++20 -fsyntax-only -verify

template <int N> struct Str {
  constexpr Str(char const (&s)[N]) { __builtin_memcpy(value, s, N); }
  char value[N];
};

template <Str V> class ASCII {};

void Foo(ASCII<"this nontype template argument is too long to print">); // expected-note {{no known conversion from 'ASCII<{"this nontype template argument is too long"}>' to 'ASCII<{"this nontype template argument is too long to print"}>'}}
void Bar(ASCII<"this nttp argument is too short">);                     // expected-note {{no known conversion from 'ASCII<{{119, 97, 105, 116, 32, 97, 32, 115, 27, 99, 111, 110, 100, 0}}>' to 'ASCII<{"this nttp argument is too short"}>'}}

void test_ascii() {
  ASCII<"this nontype template argument"
        " is too long">
      a;
  Foo(a);                 // expected-error {{no matching function}}
  decltype(a)::display(); // expected-error {{no member named 'display' in 'ASCII<{"this nontype template argument is [...]"}>'}}
}

void test_non_ascii() {
  ASCII<"wait a s\033cond"> a;
  Bar(a);                 // expected-error {{no matching function}}
  decltype(a)::display(); // expected-error {{no member named 'display' in 'ASCII<{{119, 97, 105, 116, 32, 97, 32, 115, 27, 99, ...}}>'}}
}
