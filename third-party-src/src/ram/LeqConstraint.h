/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file LeqConstraint.h
 *
 * Defines a class for imposing leq constraints between lattice values 
 *
 ***********************************************************************/

#pragma once

#include "ram/Condition.h"
#include "ram/Expression.h"
#include "ram/Node.h"
#include "ram/NodeMapper.h"
#include "souffle/BinaryConstraintOps.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/MiscUtil.h"
#include <cassert>
#include <memory>
#include <sstream>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class RamLeqConstraint
 * @brief Imposes a leq constraint between two lattice values
 *
 * Condition is true if the constraint does not force the first
 * lattice value to become bot.
 *
 * Example:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * l1.1 <= l0.0
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class RamLeqConstraint : public RamCondition {
public:
    RamLeqConstraint(std::unique_ptr<RamExpression> l, std::unique_ptr<RamExpression> r)
            : lhs(std::move(l)), rhs(std::move(r)) {
        assert(lhs != nullptr && "left-hand side of constraint is a null-pointer");
        assert(rhs != nullptr && "right-hand side of constraint is a null-pointer");
    }

    /** @brief Get left-hand side */
    const RamExpression& getLHS() const {
        return *lhs;
    }

    /** @brief Get right-hand side */
    const RamExpression& getRHS() const {
        return *rhs;
    }

    std::vector<const RamNode*> getChildNodes() const override {
        return {lhs.get(), rhs.get()};
    }

    RamLeqConstraint* clone() const override {
        return new RamLeqConstraint(souffle::clone(lhs), souffle::clone(rhs));
    }

    void apply(const RamNodeMapper& map) override {
        lhs = map(std::move(lhs));
        rhs = map(std::move(rhs));
    }

protected:
    void print(std::ostream& os) const override {
        os << "(" << *lhs << " <= " << *rhs << ")";
    }

    bool equal(const RamNode& node) const override {
        const auto& other = static_cast<const RamLeqConstraint&>(node);
        return equal_ptr(lhs, other.lhs) && equal_ptr(rhs, other.rhs);
    }

    /** Left-hand side of constraint*/
    std::unique_ptr<RamExpression> lhs;

    /** Right-hand side of constraint */
    std::unique_ptr<RamExpression> rhs;
};

}  // end of namespace souffle
