/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ast_utils_test.cpp
 *
 * Tests souffle's AST utils.
 *
 ***********************************************************************/

#include "tests/test.h"

#include "ast/Argument.h"
#include "ast/Atom.h"
#include "ast/BinaryConstraint.h"
#include "ast/Clause.h"
#include "ast/Literal.h"
#include "ast/Negation.h"
#include "ast/Node.h"
#include "ast/Program.h"
#include "ast/QualifiedName.h"
#include "ast/TranslationUnit.h"
#include "ast/Variable.h"
#include "ast/analysis/Ground.h"
#include "ast/utility/Utils.h"
#include "parser/ParserDriver.h"
#include "reports/DebugReport.h"
#include "reports/ErrorReport.h"
#include "souffle/BinaryConstraintOps.h"
#include "souffle/utility/StringUtil.h"
#include <algorithm>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace souffle {
class AstRelation;

namespace test {

TEST(AstUtilsLattice, Grounded) {
    // create an example clause:
    auto clause = std::make_unique<AstClause>();

    // r(X,Y,Z;L1)
    auto* head = new AstAtom("r");
    head->addConcreteArgument(std::unique_ptr<AstArgument>(new AstVariable("X")));
    head->addConcreteArgument(std::unique_ptr<AstArgument>(new AstVariable("Y")));
    head->addConcreteArgument(std::unique_ptr<AstArgument>(new AstVariable("Z")));
    head->addLatticeArgument(std::unique_ptr<AstArgument>(new AstVariable("L1")));
    head->addLatticeArgument(std::unique_ptr<AstArgument>(new AstVariable("L2")));
    clause->setHead(std::unique_ptr<AstAtom>(head));

    // a(X;l)
    auto* a = new AstAtom("a");
    a->addConcreteArgument(std::unique_ptr<AstArgument>(new AstVariable("X")));
    a->addLatticeArgument(std::unique_ptr<AstArgument>(new AstVariable("L1")));
    clause->addToBody(std::unique_ptr<AstLiteral>(a));

    // X = Y
    AstLiteral* e1 = new AstBinaryConstraint(BinaryConstraintOp::EQ,
            std::unique_ptr<AstArgument>(new AstVariable("X")),
            std::unique_ptr<AstArgument>(new AstVariable("Y")));
    clause->addToBody(std::unique_ptr<AstLiteral>(e1));

    // !b(Z)
    auto* b = new AstAtom("b");
    b->addConcreteArgument(std::unique_ptr<AstArgument>(new AstVariable("Z")));
    auto* neg = new AstNegation(std::unique_ptr<AstAtom>(b));
    clause->addToBody(std::unique_ptr<AstLiteral>(neg));

    // check construction
    EXPECT_EQ("r(X,Y,Z;L1,L2) :- \n   a(X;L1),\n   X = Y,\n   !b(Z).", toString(*clause));

    auto program = std::make_unique<AstProgram>();
    program->addClause(std::move(clause));
    DebugReport dbgReport;
    ErrorReport errReport;
    AstTranslationUnit tu{std::move(program), errReport, dbgReport};

    // obtain groundness
    auto isGrounded = getGroundedTerms(tu, *tu.getProgram()->getClauses()[0]);

    auto concreteArgs = head->getConcreteArguments();
    auto latticeArgs = head->getLatticeArguments();
    // check selected sub-terms
    EXPECT_TRUE(isGrounded[concreteArgs[0]]);   // X
    EXPECT_TRUE(isGrounded[concreteArgs[1]]);   // Y
    EXPECT_FALSE(isGrounded[concreteArgs[2]]);  // Z
    EXPECT_TRUE(isGrounded[latticeArgs[0]]);  // L1
    EXPECT_FALSE(isGrounded[latticeArgs[1]]);  // L2
}

}  // namespace test
}  // namespace souffle
