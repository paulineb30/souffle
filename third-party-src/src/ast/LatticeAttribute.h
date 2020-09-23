/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Attribute.h
 *
 * Defines the attribute class
 *
 ***********************************************************************/

#pragma once

#include "ast/Node.h"
#include "ast/QualifiedName.h"
#include "parser/SrcLocation.h"
#include <ostream>
#include <string>
#include <utility>

namespace souffle {

/**
 * @class AstLatticeAttribute
 * @brief Lattice attribute class
 *
 * Example:
 *    i <- Intervals
 * An attribute consists of a name and its associated lattice.
 */
class AstLatticeAttribute : public AstNode {
public:
    AstLatticeAttribute(std::string n, AstQualifiedName l, SrcLocation loc = {})
            : AstNode(std::move(loc)), name(std::move(n)), latticeName(std::move(l)) {}

    /** Return attribute name */
    const std::string& getName() const {
        return name;
    }

    /** Return type name */
    const AstQualifiedName& getLatticeName() const {
        return latticeName;
    }

    /** Set type name */
    void setLatticeName(AstQualifiedName name) {
        latticeName = std::move(name);
    }

    AstLatticeAttribute* clone() const override {
        return new AstLatticeAttribute(name, latticeName, getSrcLoc());
    }

protected:
    void print(std::ostream& os) const override {
        os << name << "<-" << latticeName;
    }

    bool equal(const AstNode& node) const override {
        const auto& other = static_cast<const AstLatticeAttribute&>(node);
        return name == other.name && latticeName == other.latticeName;
    }

private:
    /** Attribute name */
    std::string name;

    /** Lattice name */
    AstQualifiedName latticeName;
};

}  // end of namespace souffle
