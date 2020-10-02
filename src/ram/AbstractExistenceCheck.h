/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file AbstractExistenceCheck.h
 *
 * Defines a class for evaluating conditions in the Relational Algebra
 * Machine.
 *
 ***********************************************************************/

#pragma once

#include "ram/Condition.h"
#include "ram/Expression.h"
#include "ram/Node.h"
#include "ram/NodeMapper.h"
#include "ram/Relation.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/StreamUtil.h"
#include <cassert>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class RamAbstractExistenceCheck
 * @brief Abstract existence check for a tuple in a relation
 */
class RamAbstractExistenceCheck : public RamCondition {
public:
    RamAbstractExistenceCheck(
            std::unique_ptr<RamRelationReference> relRef, std::vector<std::unique_ptr<RamExpression>> concreteVals, std::vector<std::unique_ptr<RamExpression>> latticeVals)
            : relationRef(std::move(relRef)), concreteValues(std::move(concreteVals)), latticeValues(std::move(latticeVals)) {
        assert(relationRef != nullptr && "Relation reference is a nullptr");
        for (const auto& v : concreteValues) {
            assert(v != nullptr && "value is a nullptr");
        }
        for (const auto& v : latticeValues) {
            assert(v != nullptr && "value is a nullptr");
        }
    }

    /** @brief Get relation */
    const RamRelation& getRelation() const {
        return *relationRef->get();
    }

    /**
     *  @brief Get concrete arguments of the tuple/pattern
     *  A null pointer element in the vector denotes an unspecified
     *  pattern for a tuple element.
     */
    const std::vector<RamExpression*> getConcreteValues() const {
        return toPtrVector(concreteValues);
    }

    /**
     *  @brief Get lattice arguments of the tuple/pattern
     *  A null pointer element in the vector denotes an unspecified
     *  pattern for a tuple element.
     */
    const std::vector<RamExpression*> getLatticeValues() const {
        return toPtrVector(latticeValues);
    }

    std::vector<const RamNode*> getChildNodes() const override {
        std::vector<const RamNode*> res = {relationRef.get()};
        for (const auto& cur : concreteValues) {
            res.push_back(cur.get());
        }
        for (const auto& cur : latticeValues) {
            res.push_back(cur.get());
        }
        return res;
    }

    void apply(const RamNodeMapper& map) override {
        relationRef = map(std::move(relationRef));
        for (auto& val : concreteValues) {
            val = map(std::move(val));
        }
        for (auto& val : latticeValues) {
            val = map(std::move(val));
        }
    }

protected:
    void print(std::ostream& os) const override {
        os << "(";
        if (getRelation().getConcreteArity() > 0) {
            os << join(concreteValues, ",",
                       [](std::ostream& out, const std::unique_ptr<RamExpression>& value) {
                           if (!value) {
                               out << "_";
                           } else {
                               out << *value;
                           }
                       });
        }
        if (getRelation().getLatticeArity() > 0) {
            os << "; ";
            os << join(latticeValues, ",",
                       [](std::ostream& out, const std::unique_ptr<RamExpression>& value) {
                           if (!value) {
                               out << "_";
                           } else {
                               out << *value;
                           }
                       });
        }
        os << ") ∈ " << getRelation().getName();
    }

    bool equal(const RamNode& node) const override {
        const auto& other = static_cast<const RamAbstractExistenceCheck&>(node);
        return equal_ptr(relationRef, other.relationRef) && equal_targets(concreteValues, other.concreteValues) && equal_targets(latticeValues, other.latticeValues);
    }

    /** Relation */
    std::unique_ptr<RamRelationReference> relationRef;

    /** Concrete pattern -- nullptr if undefined */
    std::vector<std::unique_ptr<RamExpression>> concreteValues;

    /** Lattice pattern -- nullptr if undefined */
    std::vector<std::unique_ptr<RamExpression>> latticeValues;
};

}  // end of namespace souffle
