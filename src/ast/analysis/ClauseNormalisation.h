/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2020, The Souffle Developers. All rights reserved.
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ClauseNormalisation.h
 *
 * Defines a clause-normalisation analysis, providing a normal form for
 * each clause in the program.
 *
 ***********************************************************************/

#pragma once

#include "ast/QualifiedName.h"
#include "ast/analysis/Analysis.h"
#include <map>
#include <set>
#include <string>
#include <vector>

namespace souffle {

class AstArgument;
class AstAtom;
class AstClause;
class AstLiteral;
class AstTranslationUnit;

class NormalisedClause {
public:
    struct NormalisedClauseElement {
        AstQualifiedName name;
        std::vector<std::string> concreteParams;
        std::vector<std::string> latticeParams;
    };

    NormalisedClause() = default;

    NormalisedClause(const AstClause* clause);

    bool isFullyNormalised() const {
        return fullyNormalised;
    }

    const std::set<std::string>& getVariables() const {
        return variables;
    }

    const std::set<std::string>& getConstants() const {
        return constants;
    }

    const std::vector<NormalisedClauseElement>& getElements() const {
        return clauseElements;
    }

private:
    bool fullyNormalised{true};
    size_t aggrScopeCount{0};
    std::set<std::string> variables{};
    std::set<std::string> constants{};
    std::vector<NormalisedClauseElement> clauseElements{};

    /**
     * Parse an atom with a preset name qualifier into the element list.
     */
    void addClauseAtom(const std::string& qualifier, const std::string& scopeID, const AstAtom* atom);

    /**
     * Parse a body literal into the element list.
     */
    void addClauseBodyLiteral(const std::string& scopeID, const AstLiteral* lit);

    /**
     * Return a normalised string repr of an argument.
     */
    std::string normaliseArgument(const AstArgument* arg);
};

class ClauseNormalisationAnalysis : public AstAnalysis {
public:
    static constexpr const char* name = "clause-normalisation";

    ClauseNormalisationAnalysis() : AstAnalysis(name) {}

    void run(const AstTranslationUnit& translationUnit) override;

    void print(std::ostream& os) const override;

    const NormalisedClause& getNormalisation(const AstClause* clause) const;

private:
    std::map<const AstClause*, NormalisedClause> normalisations;
};

}  // namespace souffle
