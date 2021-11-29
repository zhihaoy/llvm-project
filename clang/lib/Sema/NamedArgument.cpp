#include "clang/Sema/NamedArgument.h"
#include "clang/AST/Expr.h"
#include "clang/Sema/Sema.h"
#include "clang/Sema/SemaDiagnostic.h"

namespace clang {

void NamedArgumentContext::enterKey(StringLiteral *Tag) {
  auto Inserted = SeenSoFar.try_emplace(Tag->getString(), Tag);
  if (!Inserted.second) {
    S.Diag(Tag->getBeginLoc(), diag::err_param_namedarg_redefinition) << Tag;
    S.Diag(Inserted.first->second->getBeginLoc(),
           diag::note_previous_declaration);
  }
}

} // namespace clang
