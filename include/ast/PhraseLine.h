#pragma once

#include <memory>
#include <sstream>
#include <set>

#include "ast/Constants.h"
#include "ast/AuxiliarySubPhraseLine.h"

#include "ast/interfaces/AST.h"
#include "ast/interfaces/IJunction.h"

#include "structure/Path.h"
#include "ParserContext.h"


namespace ieml::AST {

class PhraseLine : virtual public AST {
public:
    PhraseLine(int role_type,
               bool accentuation) : 
        role_type_(role_type),
        accentuation_(accentuation) {}

    int getRoleType() const {return role_type_;}
    bool getAccentuation() const {return accentuation_;}

    std::shared_ptr<structure::PathTree> check_phrase_line(parser::ParserContext& ctx) const {
        auto type = structure::RoleType::_from_integral_nothrow(role_type_);

        if (!type) {
            ctx.getErrorManager().visitorError(
                getCharRange(), "Invalid role number, got '" + std::to_string(role_type_) + "'"
            );
            return nullptr;
        }
        std::shared_ptr<structure::PathTree> child = _check_phrase_line(ctx, *type);

        if (!child || !type) {
            return nullptr;
        }

        return ctx.getPathTreeRegister().get_or_create(
            std::make_shared<structure::RoleNumberPathNode>(*type), 
            structure::PathTree::Children{child}
        );
    };
    

protected:
    virtual std::shared_ptr<structure::PathTree> _check_phrase_line(parser::ParserContext& ctx, structure::RoleType role_type) const = 0;


    std::string phrase_line_to_string() const {
        std::ostringstream os;

        os << std::to_string(role_type_) << " ";
        if (accentuation_)
            os << "! ";

        return os.str();
    }

private:
    const int role_type_;
    const bool accentuation_;
};

class SimplePhraseLine : public PhraseLine {
public:
    SimplePhraseLine(std::shared_ptr<CharRange>&& char_range,
                     int role_type,
                     bool accentuation,
                     std::shared_ptr<AuxiliarySubPhraseLine>&& auxiliary_subline) : 
        AST(std::move(char_range)),
        PhraseLine(role_type, accentuation),
        auxiliary_subline_(std::move(auxiliary_subline)) {}

    std::string to_string() const override {
        return phrase_line_to_string() + auxiliary_subline_->to_string();
    }
protected:
    std::shared_ptr<structure::PathTree> _check_phrase_line(parser::ParserContext& ctx, structure::RoleType role_type) const override {
        return auxiliary_subline_->check_auxiliary_subline(ctx, role_type);
    };

private:
    std::shared_ptr<AuxiliarySubPhraseLine> auxiliary_subline_;

};

class JunctionPhraseLine : public PhraseLine, public IJunction<AuxiliarySubPhraseLine, structure::AuxiliaryJunctionIndexPathNode, structure::AuxiliaryJunctionPathNode, structure::RoleType> {
public:
    JunctionPhraseLine(std::shared_ptr<CharRange>&& char_range,
                       std::vector<std::shared_ptr<AuxiliarySubPhraseLine>>&& sub_phrases,
                       std::shared_ptr<Identifier>&& junction_identifier,
                       int role_type,
                       bool accentuation) : 
        AST(std::move(char_range)),
        PhraseLine(role_type, accentuation),
        IJunction<AuxiliarySubPhraseLine, structure::AuxiliaryJunctionIndexPathNode, structure::AuxiliaryJunctionPathNode, structure::RoleType>(std::move(sub_phrases), std::move(junction_identifier)) {}

    std::string to_string() const override {
        return phrase_line_to_string() + junction_to_string();
    }

protected:
    virtual std::shared_ptr<structure::PathTree> check_junction_item(parser::ParserContext& ctx, size_t i, structure::RoleType role_type) const override {
        return items_[i]->check_auxiliary_subline(ctx, role_type);
    };

    virtual std::shared_ptr<structure::PathTree> _check_phrase_line(parser::ParserContext& ctx, structure::RoleType role_type) const override {
        return check_junction(ctx, role_type);
    }
};

}