/*
 * Souffle - A Datalog Compiler
 * Copyright (c) 2015, Oracle and/or its affiliates. All rights reserved
 * Licensed under the Universal Permissive License v 1.0 as shown at:
 * - https://opensource.org/licenses/UPL
 * - <souffle root>/licenses/SOUFFLE-UPL.txt
 */

/************************************************************************
 *
 * @file ParserDriver.cpp
 *
 * Defines the parser driver.
 *
 ***********************************************************************/

#include "parser/ParserDriver.h"
#include "Global.h"
#include "ast/Clause.h"
#include "ast/Component.h"
#include "ast/ComponentInit.h"
#include "ast/Directive.h"
#include "ast/FunctorDeclaration.h"
#include "ast/Lattice.h"
#include "ast/Pragma.h"
#include "ast/Program.h"
#include "ast/QualifiedName.h"
#include "ast/Relation.h"
#include "ast/SubsetType.h"
#include "ast/TranslationUnit.h"
#include "ast/Type.h"
#include "ast/utility/Utils.h"
#include "reports/ErrorReport.h"
#include "souffle/utility/ContainerUtil.h"
#include "souffle/utility/FunctionalUtil.h"
#include "souffle/utility/StreamUtil.h"
#include "souffle/utility/StringUtil.h"
#include "souffle/utility/tinyformat.h"
#include <algorithm>
#include <memory>
#include <utility>
#include <vector>

using YY_BUFFER_STATE = struct yy_buffer_state*;
extern YY_BUFFER_STATE yy_scan_string(const char*, yyscan_t scanner);
extern int yylex_destroy(yyscan_t scanner);
extern int yylex_init_extra(scanner_data* data, yyscan_t* scanner);
extern void yyset_in(FILE* in_str, yyscan_t scanner);

namespace souffle {

std::unique_ptr<AstTranslationUnit> ParserDriver::parse(
        const std::string& filename, FILE* in, ErrorReport& errorReport, DebugReport& debugReport) {
    translationUnit =
            std::make_unique<AstTranslationUnit>(std::make_unique<AstProgram>(), errorReport, debugReport);
    yyscan_t scanner;
    scanner_data data;
    data.yyfilename = filename;
    yylex_init_extra(&data, &scanner);
    yyset_in(in, scanner);

    yy::parser parser(*this, scanner);
    parser.parse();

    yylex_destroy(scanner);

    return std::move(translationUnit);
}

std::unique_ptr<AstTranslationUnit> ParserDriver::parse(
        const std::string& code, ErrorReport& errorReport, DebugReport& debugReport) {
    translationUnit =
            std::make_unique<AstTranslationUnit>(std::make_unique<AstProgram>(), errorReport, debugReport);

    scanner_data data;
    data.yyfilename = "<in-memory>";
    yyscan_t scanner;
    yylex_init_extra(&data, &scanner);
    yy_scan_string(code.c_str(), scanner);
    yy::parser parser(*this, scanner);
    parser.parse();

    yylex_destroy(scanner);

    return std::move(translationUnit);
}

std::unique_ptr<AstTranslationUnit> ParserDriver::parseTranslationUnit(
        const std::string& filename, FILE* in, ErrorReport& errorReport, DebugReport& debugReport) {
    ParserDriver parser;
    return parser.parse(filename, in, errorReport, debugReport);
}

std::unique_ptr<AstTranslationUnit> ParserDriver::parseTranslationUnit(
        const std::string& code, ErrorReport& errorReport, DebugReport& debugReport) {
    ParserDriver parser;
    return parser.parse(code, errorReport, debugReport);
}

void ParserDriver::addPragma(std::unique_ptr<AstPragma> p) {
    translationUnit->getProgram()->addPragma(std::move(p));
}

void ParserDriver::addFunctorDeclaration(std::unique_ptr<AstFunctorDeclaration> f) {
    const std::string& name = f->getName();
    const AstFunctorDeclaration* existingFunctorDecl =
            getIf(translationUnit->getProgram()->getFunctorDeclarations(),
                    [&](const AstFunctorDeclaration* current) { return current->getName() == name; });
    if (existingFunctorDecl != nullptr) {
        Diagnostic err(Diagnostic::Type::ERROR,
                DiagnosticMessage("Redefinition of functor " + toString(name), f->getSrcLoc()),
                {DiagnosticMessage("Previous definition", existingFunctorDecl->getSrcLoc())});
        translationUnit->getErrorReport().addDiagnostic(err);
    } else {
        translationUnit->getProgram()->addFunctorDeclaration(std::move(f));
    }
}

void ParserDriver::addLattice(std::unique_ptr<AstLattice> lattice) {
    const AstQualifiedName& name = lattice->getName();
    const AstLattice* existingLattice =
            getIf(translationUnit->getProgram()->getLattices(),
                    [&](const AstLattice* current) { return current->getName() == name; });
    if (existingLattice != nullptr) {
        Diagnostic err(Diagnostic::Type::ERROR,
                DiagnosticMessage("Redefinition of lattice " + toString(name), lattice->getSrcLoc()),
                {DiagnosticMessage("Previous definition", existingLattice->getSrcLoc())});
        translationUnit->getErrorReport().addDiagnostic(err);
    } else {
        translationUnit->getProgram()->addLattice(std::move(lattice));
    }
}

void ParserDriver::addRelation(std::unique_ptr<AstRelation> r) {
    const auto& name = r->getQualifiedName();
    if (AstRelation* prev = getRelation(*translationUnit->getProgram(), name)) {
        Diagnostic err(Diagnostic::Type::ERROR,
                DiagnosticMessage("Redefinition of relation " + toString(name), r->getSrcLoc()),
                {DiagnosticMessage("Previous definition", prev->getSrcLoc())});
        translationUnit->getErrorReport().addDiagnostic(err);
    } else {
        translationUnit->getProgram()->addRelation(std::move(r));
    }
}

void ParserDriver::addDirective(std::unique_ptr<AstDirective> directive) {
    if (directive->getType() == AstDirectiveType::printsize) {
        for (const auto& cur : translationUnit->getProgram()->getDirectives()) {
            if (cur->getQualifiedName() == directive->getQualifiedName() &&
                    cur->getType() == AstDirectiveType::printsize) {
                Diagnostic err(Diagnostic::Type::ERROR,
                        DiagnosticMessage("Redefinition of printsize directives for relation " +
                                                  toString(directive->getQualifiedName()),
                                directive->getSrcLoc()),
                        {DiagnosticMessage("Previous definition", cur->getSrcLoc())});
                translationUnit->getErrorReport().addDiagnostic(err);
                return;
            }
        }
    } else if (directive->getType() == AstDirectiveType::limitsize) {
        for (const auto& cur : translationUnit->getProgram()->getDirectives()) {
            if (cur->getQualifiedName() == directive->getQualifiedName() &&
                    cur->getType() == AstDirectiveType::limitsize) {
                Diagnostic err(Diagnostic::Type::ERROR,
                        DiagnosticMessage("Redefinition of limitsize directives for relation " +
                                                  toString(directive->getQualifiedName()),
                                directive->getSrcLoc()),
                        {DiagnosticMessage("Previous definition", cur->getSrcLoc())});
                translationUnit->getErrorReport().addDiagnostic(err);
                return;
            }
        }
    }
    translationUnit->getProgram()->addDirective(std::move(directive));
}

void ParserDriver::addType(std::unique_ptr<AstType> type) {
    const auto& name = type->getQualifiedName();
    auto* existingType = getIf(translationUnit->getProgram()->getTypes(),
            [&](const AstType* current) { return current->getQualifiedName() == name; });
    if (existingType != nullptr) {
        Diagnostic err(Diagnostic::Type::ERROR,
                DiagnosticMessage("Redefinition of type " + toString(name), type->getSrcLoc()),
                {DiagnosticMessage("Previous definition", existingType->getSrcLoc())});
        translationUnit->getErrorReport().addDiagnostic(err);
    } else {
        translationUnit->getProgram()->addType(std::move(type));
    }
}

void ParserDriver::addClause(std::unique_ptr<AstClause> c) {
    translationUnit->getProgram()->addClause(std::move(c));
}
void ParserDriver::addComponent(std::unique_ptr<AstComponent> c) {
    translationUnit->getProgram()->addComponent(std::move(c));
}
void ParserDriver::addInstantiation(std::unique_ptr<AstComponentInit> ci) {
    translationUnit->getProgram()->addInstantiation(std::move(ci));
}

void ParserDriver::addIoFromDeprecatedTag(AstRelation& rel) {
    if (rel.hasQualifier(RelationQualifier::INPUT)) {
        addDirective(mk<AstDirective>(AstDirectiveType::input, rel.getQualifiedName(), rel.getSrcLoc()));
    }

    if (rel.hasQualifier(RelationQualifier::OUTPUT)) {
        addDirective(mk<AstDirective>(AstDirectiveType::output, rel.getQualifiedName(), rel.getSrcLoc()));
    }

    if (rel.hasQualifier(RelationQualifier::PRINTSIZE)) {
        addDirective(mk<AstDirective>(AstDirectiveType::printsize, rel.getQualifiedName(), rel.getSrcLoc()));
    }
}

std::set<RelationTag> ParserDriver::addDeprecatedTag(
        RelationTag tag, SrcLocation tagLoc, std::set<RelationTag> tags) {
    if (!Global::config().has("legacy")) {
        warning(tagLoc, tfm::format("Deprecated %s qualifier was used", tag));
    }
    return addTag(tag, std::move(tagLoc), std::move(tags));
}

std::set<RelationTag> ParserDriver::addReprTag(
        RelationTag tag, SrcLocation tagLoc, std::set<RelationTag> tags) {
    return addTag(tag, {RelationTag::BTREE, RelationTag::BRIE, RelationTag::EQREL}, std::move(tagLoc),
            std::move(tags));
}

std::set<RelationTag> ParserDriver::addTag(RelationTag tag, SrcLocation tagLoc, std::set<RelationTag> tags) {
    return addTag(tag, {tag}, std::move(tagLoc), std::move(tags));
}

std::set<RelationTag> ParserDriver::addTag(RelationTag tag, std::vector<RelationTag> incompatible,
        SrcLocation tagLoc, std::set<RelationTag> tags) {
    if (any_of(incompatible, [&](auto&& x) { return contains(tags, x); })) {
        error(tagLoc, tfm::format("%s qualifier already set", join(incompatible, "/")));
    }

    tags.insert(tag);
    return tags;
}

Own<AstSubsetType> ParserDriver::mkDeprecatedSubType(
        AstQualifiedName name, AstQualifiedName baseTypeName, SrcLocation loc) {
    if (!Global::config().has("legacy")) {
        warning(loc, "Deprecated type declaration used");
    }
    return mk<AstSubsetType>(std::move(name), std::move(baseTypeName), std::move(loc));
}

void ParserDriver::warning(const SrcLocation& loc, const std::string& msg) {
    translationUnit->getErrorReport().addWarning(msg, loc);
}
void ParserDriver::error(const SrcLocation& loc, const std::string& msg) {
    translationUnit->getErrorReport().addError(msg, loc);
}
void ParserDriver::error(const std::string& msg) {
    translationUnit->getErrorReport().addDiagnostic(
            Diagnostic(Diagnostic::Type::ERROR, DiagnosticMessage(msg)));
}

}  // end of namespace souffle
