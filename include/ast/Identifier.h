#pragma once

#include <memory>
#include <string>

#include "ast/interfaces/AST.h"
#include "ast/interfaces/ICategory.h"
#include "ast/interfaces/IReferenceValue.h"
#include "ast/PartialPathTree.h"

#include "structure/LanguageString.h"


namespace ieml::AST {
class Identifier : virtual public AST, public ICategory, public IReferenceValue {
public:
    IEML_DECLARE_PTR_TYPE_AST(Identifier)

    Identifier(CharRange::Ptr&& char_range, 
               std::string name) : 
        AST(std::move(char_range)),
        ICategory(),
        IReferenceValue(), 
        name_(name) {};
    
    std::string to_string() const override {
        return name_;
    }; 

    const std::string& getName() const {
        return name_;
    }

    virtual PartialPathTree::Optional check_category(parser::ParserContextManager& ctx) const override {        
        std::shared_ptr<structure::PathTree> phrase = ctx.getCategoryRegister().resolve_category(structure::LanguageString(ctx.getLanguage(), name_));

        if (phrase == nullptr) {
            ctx.getErrorManager().visitorError(
                getCharRange(),
                "Undefined category identifier '" + name_ + "'."
            );

            return {};
        }

        return PartialPathTree(structure::PathTree::singular_sequences(phrase), {});
    };

    std::optional<ieml::structure::PathTree::Ptr> check_paradigm(parser::ParserContextManager& ctx) const {
        auto phrase = ctx.getCategoryRegister().resolve_category(structure::LanguageString(ctx.getLanguage(), name_));

        if (phrase == nullptr) {
            ctx.getErrorManager().visitorError(
                getCharRange(),
                "Undefined paradigm identifier '" + name_ + "'."
            );

            return {};
        }

        if (!phrase->is_paradigm()) {
            ctx.getErrorManager().visitorError(
                getCharRange(),
                "Not a paradigm identifier '" + name_ + "'."
            );

            return {};
        }

        return phrase;
    };

private:
    const std::string name_;

};

}