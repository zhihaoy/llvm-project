//===----------------------------------------------------------------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//

#ifndef _LIBCPP___FUNCTIONAL_FUNCTION_REF_COMMON_H
#define _LIBCPP___FUNCTIONAL_FUNCTION_REF_COMMON_H

#include <__config>
#include <__type_traits/invoke.h>
#include <__type_traits/is_const.h>
#include <__type_traits/is_function.h>
#include <__type_traits/is_object.h>
#include <__type_traits/remove_pointer.h>
#include <__utility/constant_arg.h>

#if !defined(_LIBCPP_HAS_NO_PRAGMA_SYSTEM_HEADER)
#  pragma GCC system_header
#endif

_LIBCPP_BEGIN_NAMESPACE_STD

#if _LIBCPP_STD_VER >= 26

template <class...>
class function_ref;

template <class _Fn, bool _NoExcept1, class _Rp, class... _ArgTypes>
struct __is_convertible_from_specialization : false_type {};

  // use a union instead of a plain `void*` to avoid dropping const qualifiers and casting function pointers to data
  // pointers
  // todo: libstdc++ does not support volatile objects. shall we support it? the standard does not say it should not be
  // supported
  template <class _Rp, class... _ArgTypes>
  union __storage_func_ref_t {
    void* __obj_ptr_;
    void const* __obj_const_ptr_;
    _Rp (*__fn_ptr_)(_ArgTypes...);
    _Rp (*__fn_ptr_noexcept_)(_ArgTypes...) noexcept;
    void(* __fn_ptr_type_erased_)();

    _LIBCPP_HIDE_FROM_ABI constexpr explicit __storage_func_ref_t() noexcept : __obj_ptr_(nullptr) {}

    template <class _Tp>
    _LIBCPP_HIDE_FROM_ABI constexpr explicit __storage_func_ref_t(_Tp* __ptr) noexcept {
      if constexpr (is_object_v<_Tp>) {
        if constexpr (is_const_v<_Tp>) {
          __obj_const_ptr_ = __ptr;
        } else {
          __obj_ptr_ = __ptr;
        }
      } else {
        static_assert(is_function_v<_Tp>);
        if constexpr (is_nothrow_invocable_v<_Tp, _ArgTypes...>) {
          if constexpr (requires { __fn_ptr_noexcept_ = __ptr; }) {
            __fn_ptr_noexcept_ = __ptr;
          } else {
            __fn_ptr_type_erased_ = reinterpret_cast<void(*)()>(__ptr);
          }
        } else {
          if constexpr (requires { __fn_ptr_ = __ptr; }) {
            __fn_ptr_ = __ptr;
          } else {
            __fn_ptr_type_erased_ = reinterpret_cast<void(*)()>(__ptr);
          }
        }
      }
    }

  template <class _Tp>
  _LIBCPP_HIDE_FROM_ABI static constexpr auto __get(__storage_func_ref_t __storage) {
    if constexpr (is_object_v<_Tp>) {
      if constexpr (is_const_v<_Tp>) {
        return static_cast<_Tp*>(__storage.__obj_const_ptr_);
      } else {
        return static_cast<_Tp*>(__storage.__obj_ptr_);
      }
    } else {
      static_assert(is_function_v<_Tp>);
      if constexpr (is_nothrow_invocable_v<_Tp, _ArgTypes...>) {
        if constexpr (requires(_Tp* __ptr) { __storage.__fn_ptr_noexcept_ = __ptr; }) {
          return __storage.__fn_ptr_noexcept_;
        } else {
          return reinterpret_cast<_Tp*>(__storage.__fn_ptr_type_erased_);
        }
      } else {
        if constexpr (requires(_Tp* __ptr) { __storage.__fn_ptr_ = __ptr; }) {
          return __storage.__fn_ptr_;
        } else {
          return reinterpret_cast<_Tp*>(__storage.__fn_ptr_type_erased_);
        }
      }
    }
  }
  };



template <class _Fp, class _Tp>
struct __function_ref_bind {};

template <class _Tp, class _Rp, class _Gp, class... _ArgTypes>
struct __function_ref_bind<_Rp (*)(_Gp, _ArgTypes...), _Tp> {
  using type _LIBCPP_NODEBUG = _Rp(_ArgTypes...);
};

template <class _Tp, class _Rp, class _Gp, class... _ArgTypes>
struct __function_ref_bind<_Rp (*)(_Gp, _ArgTypes...) noexcept, _Tp> {
  using type _LIBCPP_NODEBUG = _Rp(_ArgTypes...) noexcept;
};

template <class _Tp, class _Mp, class _Gp>
  requires is_object_v<_Mp>
struct __function_ref_bind<_Mp _Gp::*, _Tp> {
  using type _LIBCPP_NODEBUG = invoke_result_t<_Mp _Gp::*, _Tp&>();
};

template <class _Fp, class _Tp>
using __function_ref_bind_t _LIBCPP_NODEBUG = __function_ref_bind<_Fp, _Tp>::type;

template <class _Fp>
  requires is_function_v<_Fp>
function_ref(_Fp*) -> function_ref<_Fp>;

template <auto _Fn>
  requires is_function_v<remove_pointer_t<decltype(_Fn)>>
function_ref(constant_arg_t<_Fn>) -> function_ref<remove_pointer_t<decltype(_Fn)>>;

template <auto _Fn, class _Tp>
function_ref(constant_arg_t<_Fn>, _Tp&&) -> function_ref<__function_ref_bind_t<decltype(_Fn), _Tp&>>;

#endif // _LIBCPP_STD_VER >= 26

_LIBCPP_END_NAMESPACE_STD

#endif // _LIBCPP___FUNCTIONAL_FUNCTION_REF_COMMON_H
