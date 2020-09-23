/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ast_transformers_test.cpp
 *
 * Tests souffle's AST transformers.
 *
 ***********************************************************************/

#include "tests/test.h"

#include "ast/Clause.h"
#include "ast/Node.h"
#include "ast/Program.h"
#include "ast/TranslationUnit.h"
#include "ast/analysis/ClauseNormalisation.h"
#include "ast/transform/MagicSet.h"
#include "ast/transform/MinimiseProgram.h"
#include "ast/transform/RemoveRedundantRelations.h"
#include "ast/transform/RemoveRelationCopies.h"
#include "ast/transform/ResolveAliases.h"
#include "ast/utility/Utils.h"
#include "parser/ParserDriver.h"
#include "reports/DebugReport.h"
#include "reports/ErrorReport.h"
#include "souffle/utility/StringUtil.h"
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace souffle {

namespace test {

/**
 * Test the equivalence (or lack of equivalence) of clauses using the MinimiseProgramTransfomer.
 */
TEST(AstTransformersLattice, CheckClausalEquivalence) {
    ErrorReport errorReport;
    DebugReport debugReport;

    std::unique_ptr<AstTranslationUnit> tu = ParserDriver::parseTranslationUnit(
            R"(
  
                .lattice Lattice <number, leq, lub, glb, bot, top>

                .decl A(x:number, y:number; l <- Lattice)
                .decl B(x:number)
                .decl C(x:number)

                A(0,0;0).
                A(0,0;0).
                A(0,0;1).
                A(0,1;0).

                B(1).

                C(z) :- A(z,y;l), A(z,x;l), x != 3, x < y, !B(x), y > 3, B(y).
                C(r) :- A(r,y;l), A(r,x;l), x != 3, x < y, !B(y), y > 3, B(y), B(x), x < y.
                C(z) :- A(z,y;l), A(z,x;k), x != 3, x < y, !B(x), y > 3, B(y).
                C(x) :- A(x,a;k), a != 3, !B(a), A(x,b;k), b > 3, B(c), a < b, c = b.
            )",
            errorReport, debugReport);

    const auto& program = *tu->getProgram();

    // Resolve aliases to remove trivial equalities
    std::make_unique<ResolveAliasesTransformer>()->apply(*tu);
    auto aClauses = getClauses(program, "A");
    auto bClauses = getClauses(program, "B");
    auto cClauses = getClauses(program, "C");

    EXPECT_EQ(4, aClauses.size());
    EXPECT_EQ("A(0,0;0).", toString(*aClauses[0]));
    EXPECT_EQ("A(0,0;0).", toString(*aClauses[1]));
    EXPECT_EQ("A(0,0;1).", toString(*aClauses[2]));
    EXPECT_EQ("A(0,1;0).", toString(*aClauses[3]));

    EXPECT_EQ(1, bClauses.size());
    EXPECT_EQ("B(1).", toString(*bClauses[0]));

    EXPECT_EQ(4, cClauses.size());
    EXPECT_EQ("C(z) :- \n   A(z,y;l),\n   A(z,x;l),\n   x != 3,\n   x < y,\n   !B(x),\n   y > 3,\n   B(y).",
            toString(*cClauses[0]));
    EXPECT_EQ(
            "C(r) :- \n   A(r,y;l),\n   A(r,x;l),\n   x != 3,\n   x < y,\n   !B(y),\n   y > 3,\n   B(y),\n   "
            "B(x).",
            toString(*cClauses[1]));
    EXPECT_EQ("C(z) :- \n   A(z,y;l),\n   A(z,x;k),\n   x != 3,\n   x < y,\n   !B(x),\n   y > 3,\n   B(y).",
            toString(*cClauses[2]));
    EXPECT_EQ("C(x) :- \n   A(x,a;k),\n   a != 3,\n   !B(a),\n   A(x,b;k),\n   b > 3,\n   B(b),\n   a < b.",
            toString(*cClauses[3]));

    // Check equivalence of these clauses
    // -- A
    const auto normA0 = NormalisedClause(aClauses[0]);
    const auto normA1 = NormalisedClause(aClauses[1]);
    const auto normA2 = NormalisedClause(aClauses[2]);
    const auto normA3 = NormalisedClause(aClauses[3]);
    EXPECT_TRUE(MinimiseProgramTransformer::areBijectivelyEquivalent(normA0, normA1));
    EXPECT_TRUE(MinimiseProgramTransformer::areBijectivelyEquivalent(normA1, normA0));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normA0, normA2));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normA1, normA2));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normA0, normA3));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normA1, normA3));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normA2, normA3));

    // -- C
    const auto normC0 = NormalisedClause(cClauses[0]);
    const auto normC1 = NormalisedClause(cClauses[1]);
    const auto normC2 = NormalisedClause(cClauses[2]);
    const auto normC3 = NormalisedClause(cClauses[3]);
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normC0, normC1));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normC0, normC2));
    EXPECT_TRUE(MinimiseProgramTransformer::areBijectivelyEquivalent(normC0, normC3));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normC1, normC2));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normC1, normC3));
    EXPECT_FALSE(MinimiseProgramTransformer::areBijectivelyEquivalent(normC2, normC3));

    // Make sure equivalent (and only equivalent) clauses are removed by the minimiser
    std::make_unique<MinimiseProgramTransformer>()->apply(*tu);
    auto aMinClauses = getClauses(program, "A");
    auto bMinClauses = getClauses(program, "B");
    auto cMinClauses = getClauses(program, "C");

    EXPECT_EQ(3, aMinClauses.size());
    EXPECT_EQ("A(0,0;0).", toString(*aMinClauses[0]));
    EXPECT_EQ("A(0,0;1).", toString(*aMinClauses[1]));
    EXPECT_EQ("A(0,1;0).", toString(*aMinClauses[2]));

    EXPECT_EQ(1, bMinClauses.size());
    EXPECT_EQ("B(1).", toString(*bMinClauses[0]));

    EXPECT_EQ(3, cMinClauses.size());
    EXPECT_EQ("C(z) :- \n   A(z,y;l),\n   A(z,x;l),\n   x != 3,\n   x < y,\n   !B(x),\n   y > 3,\n   B(y).",
            toString(*cMinClauses[0]));
    EXPECT_EQ(
            "C(r) :- \n   A(r,y;l),\n   A(r,x;l),\n   x != 3,\n   x < y,\n   !B(y),\n   y > 3,\n   B(y),\n   "
            "B(x).",
            toString(*cMinClauses[1]));
    EXPECT_EQ("C(z) :- \n   A(z,y;l),\n   A(z,x;k),\n   x != 3,\n   x < y,\n   !B(x),\n   y > 3,\n   B(y).",
            toString(*cMinClauses[2]));
}

/**
 * Test the equivalence (or lack of equivalence) of aggregators using the MinimiseProgramTransfomer.
 */
TEST(AstTransformersLattice, CheckAggregatorEquivalence) {
    ErrorReport errorReport;
    DebugReport debugReport;

    std::unique_ptr<AstTranslationUnit> tu = ParserDriver::parseTranslationUnit(
            R"(
                .lattice Lattice <number, leq, lub, glb, bot, top>
                .decl A(X:number; l <- Lattice)
                .decl B,C,D(X:number) input
                // first and second are equivalent
                D(X) :-
                    B(X),
                    X < max Y : { C(Y), B(Y), Y < 2 },
                    A(Z; l),
                    Z = sum A : { C(A), B(A), A > count : { A(M; l), C(M) } }.

                D(V) :-
                    B(V),
                    A(W; k),
                    W = sum test1 : { C(test1), B(test1), test1 > count : { C(X), A(X; k) } },
                    V < max test2 : { C(test2), B(test2), test2 < 2 }.

                // third not equivalent
                D(V) :-
                    B(V),
                    A(W; l),
                    W = min test1 : { C(test1), B(test1), test1 > count : { C(X), A(X; l) } },
                    V < max test2 : { C(test2), B(test2), test2 < 2 }.

                // Fourth not equivalent
                D(X) :-
                    B(X),
                    X < max Y : { C(Y), B(Y), Y < 2 },
                    A(Z; l),
                    Z = sum A : { C(A), B(A), A > count : { A(M; k), C(M) } }.
                .output D()
            )",
            errorReport, debugReport);

    const auto& program = *tu->getProgram();
    std::make_unique<MinimiseProgramTransformer>()->apply(*tu);

    // A, B, C, D should still be the relations
    EXPECT_EQ(4, program.getRelations().size());
    EXPECT_NE(nullptr, getRelation(program, "A"));
    EXPECT_NE(nullptr, getRelation(program, "B"));
    EXPECT_NE(nullptr, getRelation(program, "C"));
    EXPECT_NE(nullptr, getRelation(program, "D"));

    // D should now only have the two clauses non-equivalent clauses
    const auto& dClauses = getClauses(program, "D");
    EXPECT_EQ(3, dClauses.size());
    EXPECT_EQ(
            "D(X) :- \n   B(X),\n   X < max Y : { C(Y),B(Y),Y < 2 },\n   A(Z;l),\n   Z = sum A : { C(A),B(A),A "
            "> count : { A(M;l),C(M) } }.",
            toString(*dClauses[0]));
    EXPECT_EQ(
            "D(V) :- \n   B(V),\n   A(W;l),\n   W = min test1 : { C(test1),B(test1),test1 > count : { "
            "C(X),A(X;l) } },\n   V < max test2 : { C(test2),B(test2),test2 < 2 }.",
            toString(*dClauses[1]));
    EXPECT_EQ(
            "D(X) :- \n   B(X),\n   X < max Y : { C(Y),B(Y),Y < 2 },\n   A(Z;l),\n   Z = sum A : { C(A),B(A),A "
            "> count : { A(M;k),C(M) } }.",
            toString(*dClauses[2]));
}

/**
 * Test the removal of redundancies within clauses using the MinimiseProgramTransformer.
 *
 * In particular, the removal of:
 *      - intraclausal literals equivalent to another literal in the body
 *          e.g. a(x) :- b(x), b(x), c(x). --> a(x) :- b(x), c(x).
 *      - clauses that are only trivially satisfiable
 *          e.g. a(x) :- a(x), x != 0. is only true if a(x) is already true
 */
TEST(AstTransformersLattice, RemoveClauseRedundancies) {
    ErrorReport errorReport;
    DebugReport debugReport;

    std::unique_ptr<AstTranslationUnit> tu = ParserDriver::parseTranslationUnit(
            R"(

                .lattice Lattice <number, leq, lub, glb, bot, top>

                .decl a(X:number)
                .decl b,c(X:number; l <- Lattice)
                a(0).
                b(1; 0).
                c(X; l) :- b(X; l).

                a(X) :- b(X; l), c(X; l).
                a(X) :- a(X).
                a(X) :- a(X), X != 1.

                q(X) :- a(X).

                .decl q(X:number)
                .output q()
            )",
            errorReport, debugReport);

    const auto& program = *tu->getProgram();

    // Invoking the `RemoveRelationCopiesTransformer` to create some extra redundancy
    // In particular: The relation `c` will be replaced with `b` throughout, creating
    // the clause b(x;l) :- b(x;l).
    std::make_unique<RemoveRelationCopiesTransformer>()->apply(*tu);
    EXPECT_EQ(nullptr, getRelation(program, "c"));
    auto bIntermediateClauses = getClauses(program, "b");
    EXPECT_EQ(2, bIntermediateClauses.size());
    EXPECT_EQ("b(1;0).", toString(*bIntermediateClauses[0]));
    EXPECT_EQ("b(X;l) :- \n   b(X;l).", toString(*bIntermediateClauses[1]));

    // Attempt to minimise the program
    std::make_unique<MinimiseProgramTransformer>()->apply(*tu);
    EXPECT_EQ(3, program.getRelations().size());

    auto aClauses = getClauses(program, "a");
    EXPECT_EQ(2, aClauses.size());
    EXPECT_EQ("a(0).", toString(*aClauses[0]));
    EXPECT_EQ("a(X) :- \n   b(X;l).", toString(*aClauses[1]));

    auto bClauses = getClauses(program, "b");
    EXPECT_EQ(1, bClauses.size());
    EXPECT_EQ("b(1;0).", toString(*bClauses[0]));

    auto qClauses = getClauses(program, "q");
    EXPECT_EQ(1, qClauses.size());
    EXPECT_EQ("q(X) :- \n   a(X).", toString(*qClauses[0]));
}

}  // namespace test

}  // namespace souffle
