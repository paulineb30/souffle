/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Atom.h
 *
 * Defines the atom class
 *
 ***********************************************************************/

#pragma once

#include "ast/Argument.h"
#include "ast/Literal.h"
#include "ast/Node.h"
#include "ast/QualifiedName.h"
#include "ast/utility/NodeMapper.h"
#include "parser/SrcLocation.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/StreamUtil.h"
#include <algorithm>
#include <cstddef>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class AstAtom
 * @brief An atom class
 *
 * An atom representing the use of a relation
 * either in the head or in the body of a clause,
 * e.g., parent(x,y), !parent(x,y), ...
 */
class AstAtom : public AstLiteral {
public:
    AstAtom(AstQualifiedName name = {}, VecOwn<AstArgument> concreteArgs = {}, VecOwn<AstArgument> latticeArgs = {},SrcLocation loc = {})
            : AstLiteral(std::move(loc)), name(std::move(name)), concreteArguments(std::move(concreteArgs)), latticeArguments(std::move(latticeArgs)) {}

    /** Return qualified name */
    const AstQualifiedName& getQualifiedName() const {
        return name;
    }

    /** Set qualified name */
    void setQualifiedName(AstQualifiedName n) {
        name = std::move(n);
    }

    /** Return concrete arity of the atom */
    size_t getConcreteArity() const {
        return concreteArguments.size();
    }

    /** Add concrete argument to the atom */
    void addConcreteArgument(Own<AstArgument> arg) {
        concreteArguments.push_back(std::move(arg));
    }

    /** Return concrete arguments */
    std::vector<AstArgument*> getConcreteArguments() const {
        return toPtrVector(concreteArguments);
    }

    /** Return lattice arity of the atom */
    size_t getLatticeArity() const {
        return latticeArguments.size();
    }

    /** Add lattice argument to the atom */
    void addLatticeArgument(Own<AstArgument> arg) {
        latticeArguments.push_back(std::move(arg));
    }

    /** Return lattice arguments */
    std::vector<AstArgument*> getLatticeArguments() const {
        return toPtrVector(latticeArguments);
    }

    AstAtom* clone() const override {
        return new AstAtom(name, souffle::clone(concreteArguments), souffle::clone(latticeArguments), getSrcLoc());
    }

    void apply(const AstNodeMapper& map) override {
        for (auto& arg : concreteArguments) {
            arg = map(std::move(arg));
        }
        for (auto& arg : latticeArguments) {
            arg = map(std::move(arg));
        }
    }

    std::vector<const AstNode*> getChildNodes() const override {
        std::vector<const AstNode*> res;
        for (auto& cur : concreteArguments) {
            res.push_back(cur.get());
        }
        for (auto& cur : latticeArguments) {
            res.push_back(cur.get());
        }
        return res;
    }

protected:
    void print(std::ostream& os) const override {
        os << getQualifiedName() << "(" << join(concreteArguments);
        if (!latticeArguments.empty()) {
            os << "; " << join(latticeArguments);
        }
        os << ")";
    }

    bool equal(const AstNode& node) const override {
        const auto& other = static_cast<const AstAtom&>(node);
        return name == other.name && equal_targets(concreteArguments, other.concreteArguments) && equal_targets(latticeArguments, other.latticeArguments);
    }

    /** Name of atom */
    AstQualifiedName name;

    /** Concrete arguments of atom */
    VecOwn<AstArgument> concreteArguments;

    /** Lattice arguments of atom */
    VecOwn<AstArgument> latticeArguments;
};

}  // end of namespace souffle
