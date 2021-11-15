#pragma once

#include <vector>

#include "ast/Constants.h"
#include "ast/Reference.h"


namespace ieml {
namespace AST {

class IReferencable {
public:
    IReferencable(const std::vector<Reference>& references) : references_(references) {};

private:
    const std::vector<Reference> references_;

};

}
}