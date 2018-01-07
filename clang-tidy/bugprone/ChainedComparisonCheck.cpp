//===--- ChainedComparisonCheck.cpp - clang-tidy---------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "ChainedComparisonCheck.h"
#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang::ast_matchers;

namespace clang {
namespace tidy {
namespace bugprone {

bool isUnparenthesizedComparison(Expr const *E) {
  if (const auto *Binary = llvm::dyn_cast<BinaryOperator>(E)) {
    switch (Binary->getOpcode()) {
    case BinaryOperatorKind::BO_EQ:
    case BinaryOperatorKind::BO_NE:
    case BinaryOperatorKind::BO_LT:
    case BinaryOperatorKind::BO_GT:
    case BinaryOperatorKind::BO_LE:
    case BinaryOperatorKind::BO_GE:
      return true;
    default:
      return false;
    }
  } else if (const auto *CXXOper = llvm::dyn_cast<CXXOperatorCallExpr>(E)) {
    switch (CXXOper->getOperator()) {
    case OverloadedOperatorKind::OO_EqualEqual:
    case OverloadedOperatorKind::OO_ExclaimEqual:
    case OverloadedOperatorKind::OO_Greater:
    case OverloadedOperatorKind::OO_GreaterEqual:
    case OverloadedOperatorKind::OO_Less:
    case OverloadedOperatorKind::OO_LessEqual:
      return true;
    default:
      return false;
    }
  } else {
    // Certain kinds of expressions we need to reject: parens indicate intent
    // and explicit casts are not transparent
    if (llvm::isa<ParenExpr>(E) || llvm::isa<ExplicitCastExpr>(E)) {
      return false;
    }

    // If this expression has exactly one child, recursive into it
    if (std::distance(E->child_begin(), E->child_end()) != 1) {
      return false;
    }

    auto Child = llvm::dyn_cast<Expr>(*E->child_begin());
    return Child && isUnparenthesizedComparison(Child);
  }
}

void ChainedComparisonCheck::registerMatchers(MatchFinder *Finder) {
  Finder->addMatcher(binaryOperator(anyOf(
    hasOperatorName("=="),
    hasOperatorName("!="),
    hasOperatorName("<"),
    hasOperatorName("<="),
    hasOperatorName(">"),
    hasOperatorName(">="))
  ).bind("binary"), this);

  Finder->addMatcher(cxxOperatorCallExpr(anyOf(
    hasOverloadedOperatorName("=="),
    hasOverloadedOperatorName("!="),
    hasOverloadedOperatorName("<"),
    hasOverloadedOperatorName("<="),
    hasOverloadedOperatorName(">"),
    hasOverloadedOperatorName(">="))
  ).bind("call"), this);
}

void ChainedComparisonCheck::check(const MatchFinder::MatchResult &Result) {
  auto Diagnose = [&](Expr const *E, Expr const *Lhs, Expr const *Rhs){
    if (isUnparenthesizedComparison(Lhs) || isUnparenthesizedComparison(Rhs)) {
      diag(E->getLocStart(), "chaining comparisons likely wrong (yielding %0)")
        << E->getType().getAsString(Result.Context->getPrintingPolicy());
    }
  };

  if (const auto *Binary = Result.Nodes.getNodeAs<BinaryOperator>("binary")) {
    Diagnose(Binary, Binary->getLHS(), Binary->getRHS());
  } else if (const auto *Call = Result.Nodes.getNodeAs<CXXOperatorCallExpr>("call")) {
    Diagnose(Call, Call->getArg(0), Call->getArg(1));
  }
}

} // namespace bugprone
} // namespace tidy
} // namespace clang
