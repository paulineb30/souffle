/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ast_program_lattice_test.cpp
 *
 * Tests lattice support for souffle's AST program.
 *
 ***********************************************************************/

#include "tests/test.h"

#include "AggregateOp.h"
#include "ast/Argument.h"
#include "ast/Atom.h"
#include "ast/Attribute.h"
#include "ast/Clause.h"
#include "ast/Literal.h"
#include "ast/Node.h"
#include "ast/Program.h"
#include "ast/Relation.h"
#include "ast/TranslationUnit.h"
#include "ast/Variable.h"
#include "ast/utility/Utils.h"
#include "parser/ParserDriver.h"
#include "reports/DebugReport.h"
#include "reports/ErrorReport.h"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace souffle::test {

inline std::unique_ptr<AstTranslationUnit> makeATU(std::string program) {
    ErrorReport e;
    DebugReport d;
    return ParserDriver::parseTranslationUnit(program, e, d);
}

TEST(AstProgramLattice, ParseLattice) {
    std::unique_ptr<AstTranslationUnit> tu = makeATU(
            R"(
                   .type LatticeType1 <: unsigned 
                   .functor leq1(unsigned, unsigned): unsigned
                   .functor lub1(unsigned, unsigned): unsigned
                   .functor glb1(unsigned, unsigned): unsigned
                   .functor bot1(unsigned): number
                   .functor top1(unsigned): number

                   .type LatticeType2 <: unsigned 
                   .functor leq2(unsigned, unsigned): unsigned
                   .functor lub2(unsigned, unsigned): unsigned
                   .functor glb2(unsigned, unsigned): unsigned
                   .functor bot2(unsigned): number
                   .functor top2(unsigned): number

                   .lattice Lattice1 <LatticeType1, leq1, lub1, glb1, bot1, top1>
                   .lattice Lattice2 <LatticeType2, leq2, lub2, glb2, bot2, top2>
            )");

    auto* prog = tu->getProgram();
    std::cout << *prog << "\n";

    EXPECT_EQ(2, prog->getLattices().size());

    EXPECT_TRUE(getLattice(*prog, "Lattice1"));
    EXPECT_TRUE(getLattice(*prog, "Lattice2"));
    EXPECT_FALSE(getLattice(*prog, "Lattice3"));

}

TEST(AstProgramLattice, ParseRelation) {
    std::unique_ptr<AstTranslationUnit> tu = makeATU(
            R"(
                   .decl rel1(x: number, y: symbol)
                   .decl rel2(x: number, y: symbol ; z1: L1, z2: L2)
                   .decl rel3( ; z2: L2)

                   rel3(; Z2) :- rel1(X, Y), rel2(X, Y; Z1, Z2).

            )");
    std::cout << tu->getErrorReport() << "\n";
    auto* prog = tu->getProgram();
    std::cout << *prog << "\n";

    auto* rel1 = getRelation(*prog, "rel1");
    EXPECT_EQ(rel1->getConcreteArity(), 2);
    EXPECT_EQ(rel1->getLatticeArity(), 0);

    auto* rel2 = getRelation(*prog, "rel2");
    EXPECT_EQ(rel1->getConcreteArity(), 2);
    EXPECT_EQ(rel2->getLatticeArity(), 2);

    auto* rel3 = getRelation(*prog, "rel3");
    EXPECT_EQ(rel3->getConcreteArity(), 0);
    EXPECT_EQ(rel3->getLatticeArity(), 1);

    const auto& clauses = getClauses(*prog, *rel3);
    EXPECT_EQ(clauses.size(), 1);

    const auto* clause = clauses[0];
    const auto* head = clause->getHead();
    EXPECT_EQ(head->getConcreteArity(), 0);
    EXPECT_EQ(head->getLatticeArity(), 1);

    const auto& bodyLiterals = clause->getBodyLiterals();
    EXPECT_EQ(bodyLiterals.size(), 2);

    const auto* literal1 = static_cast<AstAtom*>(bodyLiterals[0]);
    EXPECT_EQ(literal1->getConcreteArity(), 2);
    EXPECT_EQ(literal1->getLatticeArity(), 0);
    
    const auto* literal2 = static_cast<AstAtom*>(bodyLiterals[1]);
    EXPECT_EQ(literal2->getConcreteArity(), 2);
    EXPECT_EQ(literal2->getLatticeArity(), 2);
}


}  // end namespace souffle::test
