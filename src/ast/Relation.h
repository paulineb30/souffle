/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Relation.h
 *
 * Defines the relation class and helper its classes
 *
 ***********************************************************************/

#pragma once

#include "RelationTag.h"
#include "ast/Attribute.h"
#include "ast/LatticeAttribute.h"
#include "ast/Node.h"
#include "ast/QualifiedName.h"
#include "ast/utility/NodeMapper.h"
#include "parser/SrcLocation.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/StreamUtil.h"
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class AstRelation
 * @brief Defines a relation with a name, attributes, qualifiers, and internal representation.
 */
class AstRelation : public AstNode {
public:
    AstRelation() = default;
    AstRelation(AstQualifiedName name, SrcLocation loc = {}) : name(std::move(name)) {
        setSrcLoc(std::move(loc));
    }

    /** Get qualified relation name */
    const AstQualifiedName& getQualifiedName() const {
        return name;
    }

    /** Set name for this relation */
    void setQualifiedName(AstQualifiedName n) {
        name = std::move(n);
    }

    /** Add a new concrete type to this relation */
    void addConcreteAttribute(Own<AstAttribute> attr) {
        assert(attr && "Undefined attribute");
        concreteAttributes.push_back(std::move(attr));
    }

    /** Return the concrete arity of this relation */
    size_t getConcreteArity() const {
        return concreteAttributes.size();
    }

    /** Set concrete relation attributes */
    void setConcreteAttributes(VecOwn<AstAttribute> attrs) {
        concreteAttributes = std::move(attrs);
    }

    /** Get concrete relation attributes */
    std::vector<AstAttribute*> getConcreteAttributes() const {
        return toPtrVector(concreteAttributes);
    }

    /** Add a new lattice type to this relation */
    void addLatticeAttribute(Own<AstLatticeAttribute> attr) {
        assert(attr && "Undefined attribute");
        latticeAttributes.push_back(std::move(attr));
    }

    /** Return the lattice arity of this relation */
    size_t getLatticeArity() const {
        return latticeAttributes.size();
    }

    /** Set lattice relation attributes */
    void setLatticeAttributes(VecOwn<AstLatticeAttribute> attrs) {
        latticeAttributes = std::move(attrs);
    }

    /** Get lattice relation attributes */
    std::vector<AstLatticeAttribute*> getLatticeAttributes() const {
        return toPtrVector(latticeAttributes);
    }

    /** Get relation qualifiers */
    const std::set<RelationQualifier>& getQualifiers() const {
        return qualifiers;
    }

    /** Add qualifier to this relation */
    void addQualifier(RelationQualifier q) {
        qualifiers.insert(q);
    }

    /** Remove qualifier from this relation */
    void removeQualifier(RelationQualifier q) {
        qualifiers.erase(q);
    }

    /** Get relation representation */
    RelationRepresentation getRepresentation() const {
        return representation;
    }

    /** Set relation representation */
    void setRepresentation(RelationRepresentation representation) {
        this->representation = representation;
    }

    /** Check for a relation qualifier */
    bool hasQualifier(RelationQualifier q) const {
        return qualifiers.find(q) != qualifiers.end();
    }

    AstRelation* clone() const override {
        auto res = new AstRelation(name, getSrcLoc());
        res->concreteAttributes = souffle::clone(concreteAttributes);
        res->latticeAttributes = souffle::clone(latticeAttributes);
        res->qualifiers = qualifiers;
        res->representation = representation;
        return res;
    }

    void apply(const AstNodeMapper& map) override {
        for (auto& cur : concreteAttributes) {
            cur = map(std::move(cur));
        }
        for (auto& cur : latticeAttributes) {
            cur = map(std::move(cur));
        }
    }

    std::vector<const AstNode*> getChildNodes() const override {
        std::vector<const AstNode*> res;
        for (const auto& cur : concreteAttributes) {
            res.push_back(cur.get());
        }
        for (const auto& cur : latticeAttributes) {
            res.push_back(cur.get());
        }
        return res;
    }

protected:
    void print(std::ostream& os) const override {
        os << ".decl " << getQualifiedName() << "(";
        os << join(concreteAttributes, ", ");
        if (!latticeAttributes.empty()) {
            os << "; " << join(latticeAttributes, ", ");
        }
        os << ")" << join(qualifiers, " ");
        os << " " << representation;
    }

    bool equal(const AstNode& node) const override {
        const auto& other = static_cast<const AstRelation&>(node);
        return name == other.name && equal_targets(concreteAttributes, other.concreteAttributes) && equal_targets(latticeAttributes, other.latticeAttributes);
    }

    /** Name of relation */
    AstQualifiedName name;

    /** Concrete attributes of the relation */
    VecOwn<AstAttribute> concreteAttributes;

    /** Lattice attributes of the relation */
    VecOwn<AstLatticeAttribute> latticeAttributes;

    /** Qualifiers of relation */
    std::set<RelationQualifier> qualifiers;

    /** Datastructure to use for this relation */
    RelationRepresentation representation{RelationRepresentation::DEFAULT};
};

/**
 * @class AstNameComparison
 * @brief Comparator for relations
 *
 * Lexicographical order for AstRelation
 * using the qualified name as an ordering criteria.
 */
struct AstNameComparison {
    bool operator()(const AstRelation* x, const AstRelation* y) const {
        if (x != nullptr && y != nullptr) {
            return x->getQualifiedName() < y->getQualifiedName();
        }
        return y != nullptr;
    }
};

/** Relation set */
using AstRelationSet = std::set<const AstRelation*, AstNameComparison>;

}  // end of namespace souffle
