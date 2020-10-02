/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Project.h
 *
 ***********************************************************************/

#pragma once

#include "ram/Expression.h"
#include "ram/Node.h"
#include "ram/NodeMapper.h"
#include "ram/Operation.h"
#include "ram/Relation.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/MiscUtil.h"
#include "souffle/utility/StreamUtil.h"
#include <cassert>
#include <iosfwd>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class RamProject
 * @brief Project a result into the target relation.
 *
 * For example:
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * FOR t0 IN A
 *   ...
 *     PROJECT (t0.a, t0.b, t0.c) INTO @new_X
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
class RamProject : public RamOperation {
public:
    RamProject(std::unique_ptr<RamRelationReference> relRef,
            std::vector<std::unique_ptr<RamExpression>> concreteExpressions, std::vector<std::unique_ptr<RamExpression>> latticeExpressions)
            : relationRef(std::move(relRef)), concreteExpressions(std::move(concreteExpressions)), latticeExpressions(std::move(latticeExpressions)) {
        assert(relationRef != nullptr && "Relation reference is a null-pointer");
        for (auto const& expr : concreteExpressions) {
            assert(expr != nullptr && "Expression is a null-pointer");
        }
        for (auto const& expr : latticeExpressions) {
            assert(expr != nullptr && "Expression is a null-pointer");
        }
    }

    /** @brief Get relation */
    const RamRelation& getRelation() const {
        return *relationRef->get();
    }

    /** @brief Get concrete expressions */
    std::vector<RamExpression*> getConcreteValues() const {
        return toPtrVector(concreteExpressions);
    }

    /** @brief Get lattice expressions */
    std::vector<RamExpression*> getLatticeValues() const {
        return toPtrVector(latticeExpressions);
    }

    std::vector<const RamNode*> getChildNodes() const override {
        std::vector<const RamNode*> res;
        res.push_back(relationRef.get());
        for (const auto& expr : concreteExpressions) {
            res.push_back(expr.get());
        }
        for (const auto& expr : latticeExpressions) {
            res.push_back(expr.get());
        }
        return res;
    }

    RamProject* clone() const override {
        std::vector<std::unique_ptr<RamExpression>> newConcreteValues;
        for (auto& expr : concreteExpressions) {
            newConcreteValues.emplace_back(expr->clone());
        }
        std::vector<std::unique_ptr<RamExpression>> newLatticeValues;
        for (auto& expr : latticeExpressions) {
            newLatticeValues.emplace_back(expr->clone());
        }
        return new RamProject(souffle::clone(relationRef), std::move(newConcreteValues), std::move(newLatticeValues));
    }

    void apply(const RamNodeMapper& map) override {
        relationRef = map(std::move(relationRef));
        for (auto& expr : concreteExpressions) {
            expr = map(std::move(expr));
        }
        for (auto& expr : latticeExpressions) {
            expr = map(std::move(expr));
        }
    }

protected:
    void print(std::ostream& os, int tabpos) const override {
        os << times(" ", tabpos);
        os << "PROJECT (";
        os << join(concreteExpressions, ", ", print_deref<std::unique_ptr<RamExpression>>());
        if (getRelation().getLatticeArity() > 0) {
        os << "; " << join(latticeExpressions, ", ", print_deref<std::unique_ptr<RamExpression>>());
        }
        os << ") INTO " << getRelation().getName() << std::endl;
    }

    bool equal(const RamNode& node) const override {
        const auto& other = static_cast<const RamProject&>(node);
        return equal_ptr(relationRef, other.relationRef) && equal_targets(concreteExpressions, other.concreteExpressions) && equal_targets(latticeExpressions, other.latticeExpressions);
    }

    /** Relation that values are projected into */
    std::unique_ptr<RamRelationReference> relationRef;

    /* Concrete values (expressions) for projection */
    std::vector<std::unique_ptr<RamExpression>> concreteExpressions;

    /* Lattice values (expressions) for projection */
    std::vector<std::unique_ptr<RamExpression>> latticeExpressions;
};

}  // namespace souffle
