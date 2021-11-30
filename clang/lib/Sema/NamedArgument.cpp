#include "clang/Sema/NamedArgument.h"
#include "clang/AST/Expr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"

namespace clang {

void NamedArgumentContext::enterKey(StringLiteral *Tag) {
  auto Inserted = SeenSoFar.try_emplace(Tag->getString(), Tag);
  auto *Equivalent = Inserted.first->second;
  if (Inserted.second) {
    LastKey = Equivalent;
  } else {
    S.Diag(Tag->getBeginLoc(), diag::err_param_namedarg_redefinition) << Tag;
    S.Diag(Equivalent->getBeginLoc(), diag::note_previous_declaration);
  }
}

void NamedArgumentContext::enterValue(Declarator &D, Decl *Param) {
  assert(*this && "we must have seen some namedarg keys");

  if (!std::exchange(LastKey, nullptr)) {
    S.Diag(Param->getLocation(), diag::err_param_namedarg_not_pairwise)
        << Param->isParameterPack() << D.getSourceRange();
    return;
  }

  if (Param->isParameterPack())
    S.Diag(D.getEllipsisLoc(), diag::err_param_namedarg_is_parameter_pack)
        << D.getSourceRange();
}

NamedArgumentContext::~NamedArgumentContext() {
  if (*this && EllipsisLoc.isValid())
    S.Diag(EllipsisLoc, diag::err_param_namedarg_with_varargs);
}

} // namespace clang
