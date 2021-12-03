#pragma once

#include <memory>
#include <string>
#include <sstream>

#include "ast/interfaces/AST.h"
#include "ast/Constants.h"
#include "ast/Identifier.h"


namespace ieml::AST {

class LanguageString: virtual public AST {
public:
    LanguageString(std::unique_ptr<CharRange>&& char_range, 
                   std::unique_ptr<Identifier>&& language_type, 
                   std::unique_ptr<Identifier>&& identifier) : 
        AST(std::move(char_range)), 
        language_type_(std::move(language_type)), 
        identifier_(std::move(identifier)) {}

    std::string to_string() const override {
        return language_type_->getName() + "\"" + identifier_->to_string() + "\"";
    }

    const Identifier& identifier() const {return *identifier_;};
    const Identifier& language() const {return *language_type_;};

    std::shared_ptr<structure::LanguageString> check_language_string(parser::ParserContext& ctx) const {    
        auto language = structure::LanguageType::_from_string_nocase_nothrow(language_type_->getName().c_str());
        if (!language) {
            ctx.getErrorManager().visitorError(
                language_type_->getCharRange(), 
                "Invalid language identifier for language string, got: '" + language_type_->getName() + "'."
            );
        }

        return std::make_shared<structure::LanguageString>(*language, identifier_->getName());
    }

private:
    std::unique_ptr<Identifier> language_type_;
    std::unique_ptr<Identifier> identifier_;
};

}