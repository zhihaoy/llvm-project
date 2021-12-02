#include "clang/Sema/NamedArgument.h"
#include "clang/AST/Expr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"

namespace clang {

void NonpositionalParameterContext::enterKey(StringLiteral *Key) {
  auto Inserted = SeenSoFar.try_emplace(Key->getString(), Key);
  Last = Inserted.first;
  if (!Inserted.second) {
    S.Diag(Key->getBeginLoc(), diag::err_param_nonpositional_redefinition)
        << Key;
    StringLiteral *OtherKey = Last->getSecond();
    S.Diag(OtherKey->getBeginLoc(), diag::note_previous_declaration);
  }
}

void NonpositionalParameterContext::enterParameter(Declarator &D, Decl *Param) {
  assert(*this && "we must have seen some nonpositional keys");

  if (Last == SeenSoFar.end()) {
    S.Diag(Param->getLocation(), diag::err_param_nonpositional_not_pairwise)
        << Param->isParameterPack() << D.getSourceRange();
    return;
  }

  Last = SeenSoFar.end();

  if (Param->isParameterPack())
    S.Diag(D.getEllipsisLoc(), diag::err_param_nonpositional_is_parameter_pack)
        << D.getSourceRange();
}

void NonpositionalParameterContext::invalidateParameter() {
  if (Last != SeenSoFar.end()) {
    SeenSoFar.erase(Last);
    Last = SeenSoFar.end();
  }
}

NonpositionalParameterContext::~NonpositionalParameterContext() {
  if (*this && EllipsisLoc.isValid())
    S.Diag(EllipsisLoc, diag::err_param_nonpositional_with_varargs);
}

void NamedArgumentContext ::enterName(IdentifierInfo *Name,
                                      SourceLocation Loc) {
  auto Inserted = SeenSoFar.try_emplace(Name, nullptr);
  Last = Inserted.first;
  if (!Inserted.second) {
    S.Diag(Loc, diag::err_namedarg_duplicate);
    if (Expr *OtherInit = Last->getSecond())
      S.Diag(OtherInit->getBeginLoc(), diag::note_previous_namedarg_init)
          << Name << OtherInit->getSourceRange();
  }
}

void NamedArgumentContext::enterArgument(Expr *InitClause) {
  assert(*this && "we must have seen some designator");

  if (Last == SeenSoFar.end()) {
    S.Diag(InitClause->getBeginLoc(), diag::err_namedarg_not_pairwise)
        << isa<PackExpansionExpr>(InitClause) << InitClause->getSourceRange();
    return;
  }

  Last->getSecond() = InitClause;
  Last = SeenSoFar.end();

  if (auto *InitPack = dyn_cast<PackExpansionExpr>(InitClause))
    S.Diag(InitPack->getEllipsisLoc(), diag::err_namedarg_is_pack_expansion)
        << InitPack->getSourceRange();
}

void NamedArgumentContext::invalidateArgument() {
  if (Last != SeenSoFar.end()) {
    SeenSoFar.erase(Last);
    Last = SeenSoFar.end();
  }
}

} // namespace clang
