/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2013, 2014, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file Relation.h
 *
 * Defines the class for ram relations
 ***********************************************************************/

#pragma once

#include "RelationTag.h"
#include "ram/Node.h"
#include "souffle/utility/ContainerUtil.h"
#include <cassert>
#include <cstddef>
#include <memory>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

namespace souffle {

/**
 * @class RamRelation
 * @brief A RAM Relation in the RAM intermediate representation.
 */
class RamRelation : public RamNode {
public:
    RamRelation(std::string name, size_t concreteArity, size_t latticeArity, size_t auxiliaryArity,
            std::vector<std::string> concreteAttributeNames, std::vector<std::string> concreteAttributeTypes,
            std::vector<std::string> latticeAttributeNames, std::vector<std::string> latticeAttributeLattices, RelationRepresentation representation)
            : representation(representation), name(std::move(name)), concreteArity(concreteArity),
              latticeArity(latticeArity), auxiliaryArity(auxiliaryArity), concreteAttributeNames(std::move(concreteAttributeNames)),
              concreteAttributeTypes(std::move(concreteAttributeTypes)), latticeAttributeNames(std::move(latticeAttributeNames)), latticeAttributeLattices(std::move(latticeAttributeLattices)) {
        assert(this->concreteAttributeNames.size() == concreteArity && "concrete arity mismatch for attributes");
        assert(this->concreteAttributeTypes.size() == concreteArity && "concrete arity mismatch for types");
        assert(this->latticeAttributeNames.size() == latticeArity && "lattice arity mismatch for attributes");
        assert(this->latticeAttributeLattices.size() == latticeArity && "lattice arity mismatch for lattices");
        for (std::size_t i = 0; i < concreteArity; i++) {
            assert(!this->concreteAttributeNames[i].empty() && "no concrete attribute name specified");
            assert(!this->concreteAttributeTypes[i].empty() && "no concrete attribute type specified");
        }
        for (std::size_t i = 0; i < latticeArity; i++) {
            assert(!this->latticeAttributeNames[i].empty() && "no lattice attribute name specified");
            assert(!this->latticeAttributeLattices[i].empty() && "no lattice attribute lattice specified");
        }
    }

    /** @brief Get name */
    const std::string& getName() const {
        return name;
    }

    /** @brief Get concrete attribute types */
    const std::vector<std::string>& getConcreteAttributeTypes() const {
        return concreteAttributeTypes;
    }

    /** @brief Get concrete attribute names */
    const std::vector<std::string>& getConcreteAttributeNames() const {
        return concreteAttributeNames;
    }

    /** @brief Get lattice attribute types */
    const std::vector<std::string>& getLatticeAttributeLattices() const {
        return latticeAttributeLattices;
    }

    /** @brief Get lattice attribute names */
    const std::vector<std::string>& getLatticeAttributeNames() const {
        return latticeAttributeNames;
    }

    /** @brief Is nullary relation */
    bool isNullary() const {
        return concreteArity == 0;
    }

    /** @brief Relation representation type */
    RelationRepresentation getRepresentation() const {
        return representation;
    }

    /** @brief Is temporary relation (for semi-naive evaluation) */
    bool isTemp() const {
        return name.at(0) == '@';
    }

    /** @brief Get concrete arity of relation */
    unsigned getConcreteArity() const {
        return concreteArity;
    }

    /** @brief Get lattice arity of relation */
    unsigned getLatticeArity() const {
        return latticeArity;
    }

    /** @brief Get number of auxiliary attributes */
    unsigned getAuxiliaryArity() const {
        return auxiliaryArity;
    }

    /** @brief Compare two relations via their name */
    bool operator<(const RamRelation& other) const {
        return name < other.name;
    }

    RamRelation* clone() const override {
        return new RamRelation(name, concreteArity, latticeArity, auxiliaryArity, concreteAttributeNames, concreteAttributeTypes, latticeAttributeNames, latticeAttributeLattices, representation);
    }

protected:
    void print(std::ostream& out) const override {
        out << name;
        if (concreteArity > 0) {
            out << "(" << concreteAttributeNames[0] << ":" << concreteAttributeTypes[0];
            for (unsigned i = 1; i < concreteArity; i++) {
                out << ",";
                out << concreteAttributeNames[i] << ":" << concreteAttributeTypes[i];
                if (i >= concreteArity - auxiliaryArity) {
                    out << " auxiliary";
                }
            }
            if (latticeArity > 0) {
                out << "; " << latticeAttributeNames[0] << "<-" << latticeAttributeLattices[0];
                for (unsigned i = 1; i < latticeArity; i++) {
                    out << ",";
                    out << latticeAttributeNames[i] << "<-" << latticeAttributeLattices[i];
                }
            }
            out << ")";
            out << " " << representation;
        } else if (latticeArity > 0) {
            out << "(; " << latticeAttributeNames[0] << "<-" << latticeAttributeLattices[0];
            for (unsigned i = 1; i < latticeArity; i++) {
                out << ",";
                out << latticeAttributeNames[i] << "<-" << latticeAttributeLattices[i];
            }
            out << ")";
            out << " " << representation;
        } else {
            out << " nullary";
        }
    }

    bool equal(const RamNode& node) const override {
        assert(nullptr != dynamic_cast<const RamRelation*>(&node));
        const auto& other = static_cast<const RamRelation&>(node);
        return representation == other.representation && name == other.name && concreteArity == other.concreteArity &&
               latticeArity == other.latticeArity && auxiliaryArity == other.auxiliaryArity && concreteAttributeNames == other.concreteAttributeNames &&
               concreteAttributeTypes == other.concreteAttributeTypes && latticeAttributeNames == other.latticeAttributeNames && latticeAttributeLattices == other.latticeAttributeLattices;
    }

protected:
    /** Data-structure representation */
    const RelationRepresentation representation;

    /** Name of relation */
    const std::string name;

    /** Arity, i.e., number of concrete attributes */
    const size_t concreteArity;

    /** Arity, i.e., number of lattice attributes */
    const size_t latticeArity;

    /** Number of auxiliary attributes (e.g. provenance attributes etc) */
    const size_t auxiliaryArity;

    /** Name of concrete attributes */
    const std::vector<std::string> concreteAttributeNames;

    /** Type of concrete attributes */
    const std::vector<std::string> concreteAttributeTypes;

    /** Name of lattice attributes */
    const std::vector<std::string> latticeAttributeNames;

    /** Type of lattice attributes */
    const std::vector<std::string> latticeAttributeLattices;
};

/**
 * @class RamRelationReference
 * @brief A RAM Relation in the RAM intermediate representation.
 */
class RamRelationReference : public RamNode {
public:
    RamRelationReference(const RamRelation* relation) : relation(relation) {
        assert(relation != nullptr && "null relation");
    }

    /** @brief Get reference */
    const RamRelation* get() const {
        return relation;
    }

    RamRelationReference* clone() const override {
        return new RamRelationReference(relation);
    }

protected:
    void print(std::ostream& out) const override {
        out << relation->getName();
    }

    bool equal(const RamNode& node) const override {
        const auto& other = static_cast<const RamRelationReference&>(node);
        return equal_ptr(relation, other.relation);
    }

protected:
    /** Name of relation */
    const RamRelation* relation;
};

}  // end of namespace souffle
