/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2018 The Souffle Developers. All Rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Lattice.h
 *
 * Defines the lattice class
 *
 ***********************************************************************/

#pragma once

#include "ast/Node.h"
#include "parser/SrcLocation.h"
#include "souffle/RamTypes.h"
#include "souffle/TypeAttribute.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/MiscUtil.h"
#include "souffle/utility/StreamUtil.h"
#include "souffle/utility/tinyformat.h"
#include <cassert>
#include <cstdlib>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class AstLattice
 * @brief Ast representation of lattice 
 *
 * Example:
 *    .lattice L <base, leq, lub, glb, bot, top>
 */

class AstLattice : public AstNode {
public:
    AstLattice(AstQualifiedName name, AstQualifiedName base, AstQualifiedName leq, 
            AstQualifiedName lub, AstQualifiedName glb, AstQualifiedName bot, 
            AstQualifiedName top, SrcLocation loc = {})
            : AstNode(std::move(loc)), name(std::move(name)), base(std::move(base)),
              leq(std::move(leq)), lub(std::move(lub)), glb(std::move(glb)),
              bot(std::move(bot)), top(std::move(top)) {}

    /** Return name */
    const AstQualifiedName& getName() const {
        return name;
    }

    /** Return base */
    const AstQualifiedName& getBase() const {
        return base;
    }

    /** Return leq */
    const AstQualifiedName& getLeq() const {
        return leq;
    }

    /** Return lub */
    const AstQualifiedName& getLub() const {
        return lub;
    }

    /** Return glb */
    const AstQualifiedName& getGlb() const {
        return glb;
    }

    /** Return bot */
    const AstQualifiedName& getBot() const {
        return bot;
    }

    /** Return top */
    const AstQualifiedName& getTop() const {
        return top;
    }

    AstLattice* clone() const override {
        return new AstLattice(name, base, leq, lub, glb, bot, top, getSrcLoc());
    }

protected:
    void print(std::ostream& out) const override {
        tfm::format(
                out, ".lattice %s <%s, %s, %s, %s, %s, %s>", name, base, leq, lub, glb, bot, top);
    }

    bool equal(const AstNode& node) const override {
        const auto& other = static_cast<const AstLattice&>(node);
        return name == other.name && base == other.base && leq == other.leq && lub == other.lub && glb == other.glb && bot == other.bot && top == other.top;
    }

    /** Name of lattice */
    const AstQualifiedName name;

    /** Name of base type */
    const AstQualifiedName base;

    /** Name of leq function */
    const AstQualifiedName leq;

    /** Name of lub function */
    const AstQualifiedName lub;

    /** Name of glb function */
    const AstQualifiedName glb;

    /** Name of bot function */
    const AstQualifiedName bot;

    /** Name of top function */
    const AstQualifiedName top;

};

}  // end of namespace souffle
