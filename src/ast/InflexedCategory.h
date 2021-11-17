#pragma once

#include <memory>
#include <vector>
#include <string>
#include <sstream>

#include "ast/interfaces/AST.h"
#include "ast/Identifier.h"


namespace ieml::AST {

class InflexedCategory : virtual public AST {
public:
    InflexedCategory(std::unique_ptr<CharRange>&& char_range,
                     std::vector<std::unique_ptr<Identifier>>&& inflexions,
                     std::unique_ptr<ICategory>&& category) :
        AST(std::move(char_range)),
        inflexions_(std::move(inflexions)),
        category_(std::move(category)) {}

    std::string to_string() const override {
        std::ostringstream os;
        bool first = true;


        for (auto && inflexion: inflexions_) {
            if (first) first = false;
            else os << " ";

            os << "~" << inflexion->to_string();
        }

        if (!first) os << " ";
        
        os << "#" << category_->to_string();

        return os.str();
    }

private:
    std::vector<std::unique_ptr<Identifier>> inflexions_;
    std::unique_ptr<ICategory> category_;
};



}