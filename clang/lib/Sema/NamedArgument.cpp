#include "clang/Sema/NamedArgument.h"
#include "clang/AST/Expr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"

namespace clang {

void NonpositionalParameterContext::enterKey(StringLiteral *Key) {
  auto Inserted = SeenSoFar.try_emplace(Key->getString(), Key);
  auto *Equivalent = Inserted.first->second;
  if (Inserted.second) {
    LastKey = Equivalent;
  } else {
    S.Diag(Key->getBeginLoc(), diag::err_param_nonpositional_redefinition)
        << Key;
    S.Diag(Equivalent->getBeginLoc(), diag::note_previous_declaration);
  }
}

void NonpositionalParameterContext::enterParameter(Declarator &D, Decl *Param) {
  assert(*this && "we must have seen some nonpositional keys");

  if (!std::exchange(LastKey, nullptr)) {
    S.Diag(Param->getLocation(), diag::err_param_nonpositional_not_pairwise)
        << Param->isParameterPack() << D.getSourceRange();
    return;
  }

  if (Param->isParameterPack())
    S.Diag(D.getEllipsisLoc(), diag::err_param_nonpositional_is_parameter_pack)
        << D.getSourceRange();
}

NonpositionalParameterContext::~NonpositionalParameterContext() {
  if (*this && EllipsisLoc.isValid())
    S.Diag(EllipsisLoc, diag::err_param_nonpositional_with_varargs);
}

} // namespace clang
