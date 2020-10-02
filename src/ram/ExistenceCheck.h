/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ExistenceCheck.h
 *
 * Defines a class for evaluating conditions in the Relational Algebra
 * Machine.
 *
 ***********************************************************************/

#pragma once

#include "ram/AbstractExistenceCheck.h"
#include "ram/Expression.h"
#include "ram/Relation.h"
#include "souffle/utility/MiscUtil.h"
#include <memory>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class RamExistenceCheck
 * @brief Existence check for a tuple(-pattern) in a relation
 *
 * Returns true if the tuple is in the relation
 *
 * The following condition is evaluated to true if the
 * tuple element t0.1 is in the relation A:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * t0.1 IN A
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class RamExistenceCheck : public RamAbstractExistenceCheck {
public:
    RamExistenceCheck(
            std::unique_ptr<RamRelationReference> relRef, std::vector<std::unique_ptr<RamExpression>> concreteVals, std::vector<std::unique_ptr<RamExpression>> latticeVals)
            : RamAbstractExistenceCheck(std::move(relRef), std::move(concreteVals), std::move(latticeVals)) {}

    RamExistenceCheck* clone() const override {
        std::vector<std::unique_ptr<RamExpression>> newConcreteValues;
        for (auto& cur : concreteValues) {
            newConcreteValues.emplace_back(cur->clone());
        }
        std::vector<std::unique_ptr<RamExpression>> newLatticeValues;
        for (auto& cur : latticeValues) {
            newLatticeValues.emplace_back(cur->clone());
        }
        return new RamExistenceCheck(souffle::clone(relationRef), std::move(newConcreteValues), std::move(newLatticeValues));
    }
};

}  // end of namespace souffle
