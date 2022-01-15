#pragma once

#include "ast/interfaces/AST.h"
#include "ast/interfaces/IInflectionList.h"
#include "ast/Identifier.h"

#include <memory>
#include <sstream>


namespace ieml::AST {

class InflectionList: virtual public AST, public IInflectionList {
public:
    InflectionList(std::shared_ptr<CharRange>&& char_range,
                  std::vector<std::shared_ptr<Identifier>>&& inflections) : 
        AST(std::move(char_range)),
        inflections_(std::move(inflections)) {}

    virtual std::string to_string() const override {
        std::ostringstream os;
        
        bool first = true;
        for (auto && inflection: inflections_) {
            if (first) first = false;
            else os << " ";

            os << "~" << inflection->to_string();
        }

        return os.str();
    }

    virtual structure::PathNode::Set check_inflection_list(parser::ParserContextManager& ctx, structure::RoleType role_type) const override {
        std::set<std::shared_ptr<structure::InflectionWord>> inflections;

        bool valid = true;
        for (auto&& inflection_id: inflections_) {
            auto inflection = ctx.getWordRegister().resolve_inflection(structure::LanguageString(ctx.getLanguage(), inflection_id->getName()));

            if (!inflection) {
                ctx.getErrorManager().visitorError(
                    inflection_id->getCharRange(),
                    "Undefined inflection identifier '" + inflection_id->getName() + "'."
                );
                valid = false;
                continue;
            }

            if (!inflection->accept_role(role_type)) {
                ctx.getErrorManager().visitorError(
                    inflection_id->getCharRange(),
                    "Invalid inflection for role " + std::string(role_type._to_string()) + "."
                );
                valid = false;
                continue;
            }

            inflections.insert(inflection);
        }
        
        if (!valid)
            return {nullptr};

        return {std::make_shared<structure::InflectionPathNode>(inflections)};
    }

private:
    const std::vector<std::shared_ptr<Identifier>> inflections_;

};

}