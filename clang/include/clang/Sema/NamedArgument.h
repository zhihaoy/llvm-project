//===--- NamedArgument.h - Named Arguments ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines semantics to apply while parsing C++ name argument
// declarations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_NAMEDARGUMENT_H
#define LLVM_CLANG_SEMA_NAMEDARGUMENT_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"

namespace clang {

class Decl;
class Declarator;
class Sema;
class SourceLocation;
class StringLiteral;

class NamedArgumentContext {
  llvm::SmallDenseMap<StringRef, StringLiteral *, 16> SeenSoFar;
  Sema &S;
  SourceLocation &EllipsisLoc;
  StringLiteral const *LastKey = nullptr;

public:
  explicit NamedArgumentContext(Sema &S, SourceLocation &EllipsisLoc)
      : S(S), EllipsisLoc(EllipsisLoc) {}

  explicit operator bool() const { return !SeenSoFar.empty(); }
  void enterKey(StringLiteral *Tag);
  void enterValue(Declarator &D, Decl *Param);

  ~NamedArgumentContext();
};

} // namespace clang

#endif
