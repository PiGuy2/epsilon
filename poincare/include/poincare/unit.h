#ifndef POINCARE_UNIT_H
#define POINCARE_UNIT_H

#include <poincare/expression.h>
#include <poincare/approximation_helper.h>

// based on constant.h

namespace poincare {

class UnitNode final : public SymbolAbstractNode {
  UnitNode(const char * newName, int length);  
}

}