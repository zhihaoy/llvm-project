//===--- NamedArgument.h - Named Arguments ----------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file defines semantics to apply while parsing C++ non-positional
// parameters and named arguments.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CLANG_SEMA_NAMEDARGUMENT_H
#define LLVM_CLANG_SEMA_NAMEDARGUMENT_H

#include "clang/Basic/SourceLocation.h"
#include "llvm/ADT/DenseMap.h"

namespace clang {

class Decl;
class Declarator;
class Expr;
class IdentifierInfo;
class Sema;
class StringLiteral;

class NonpositionalParameterContext {
  llvm::SmallDenseMap<StringRef, StringLiteral *, 16> SeenSoFar;
  Sema &S;
  SourceLocation &EllipsisLoc;
  decltype(SeenSoFar)::iterator Last = SeenSoFar.end();

public:
  explicit NonpositionalParameterContext(Sema &S, SourceLocation &EllipsisLoc)
      : S(S), EllipsisLoc(EllipsisLoc) {}

  explicit operator bool() const { return !SeenSoFar.empty(); }
  void enterKey(StringLiteral *Key);
  void enterParameter(Declarator &D, Decl *Param);
  void invalidateParameter();

  ~NonpositionalParameterContext();
};

class NamedArgumentContext {
  llvm::SmallDenseMap<IdentifierInfo *, Expr *, 8> SeenSoFar;
  Sema &S;
  decltype(SeenSoFar)::iterator Last = SeenSoFar.end();

public:
  explicit NamedArgumentContext(Sema &S) : S(S) {}

  explicit operator bool() const { return !SeenSoFar.empty(); }
  void enterName(IdentifierInfo *Name, SourceLocation Loc);
  void enterArgument(Expr *InitClause);
  void invalidateArgument();
};

} // namespace clang

#endif
