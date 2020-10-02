/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Lattice.h
 *
 * Defines the class for ram lattices 
 ***********************************************************************/

#pragma once

#include "ram/Node.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class RamLattice
 * @brief A RAM Lattice in the RAM intermediate representation.
 */
class RamLattice : public RamNode {
public:
    RamLattice(std::string name, std::string base, std::string leq, std::string lub, std::string glb, std::string bot, std::string top) :
            name(std::move(name)), base(std::move(base)), leq(std::move(leq)), lub(std::move(lub)), glb(std::move(glb)), bot(std::move(bot)), top(std::move(top)) {}

    /** @brief Get name */
    const std::string& getName() const {
        return name;
    }

    /** @brief Get base */
    const std::string& getBase() const {
        return base;
    }

    /** @brief Get leq functor */
    const std::string& getLeq() const {
        return leq;
    }

    /** @brief Get lub functor */
    const std::string& getLub() const {
        return lub;
    }

    /** @brief Get glb functor */
    const std::string& getGlb() const {
        return glb;
    }

    /** @brief Get bot functor */
    const std::string& getBot() const {
        return bot;
    }

    /** @brief Get top */
    const std::string& getTop() const {
        return top;
    }

    RamLattice* clone() const override {
        return new RamLattice(name, base, leq, lub, glb, bot, top);
    }

protected:
    void print(std::ostream& out) const override {
        out << name << " <" << base << ", " << leq << ", " << lub << ", " << glb << ", " << bot << ", " << top << ">";
    }

    bool equal(const RamNode& node) const override {
        assert(nullptr != dynamic_cast<const RamLattice*>(&node));
        const auto& other = static_cast<const RamLattice&>(node);
        return name == other.name && base == other.base && leq == other.leq && lub == other.lub && glb == other.glb && bot == other.bot && top == other.top;
    }

protected:
    /** Name of relation */
    const std::string name;

    /** Base type of relation */
    const std::string base;

    /** Name of leq functor */
    const std::string leq;

    /** Name of lub functor */
    const std::string lub;

    /** Name of glb functor */
    const std::string glb;

    /** Name of bot functor */
    const std::string bot;

    /** Name of top functor */
    const std::string top;
};

}  // end of namespace souffle
