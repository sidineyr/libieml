#pragma once

#include "ast/interfaces/AST.h"

namespace ieml::AST {
class IReferenceValue : virtual public AST {
public:
    IEML_DECLARE_PTR_TYPE_AST(IReferenceValue)

    IReferenceValue() {}
};

}