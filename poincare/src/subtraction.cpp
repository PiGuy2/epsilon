#include <poincare/subtraction.h>
#include <poincare/serialization_helper.h>
#include <poincare/addition.h>
#include <poincare/multiplication.h>
#include <poincare/opposite.h>
#include <poincare/rational.h>
#include <assert.h>

namespace Poincare {

int SubtractionNode::polynomialDegree(char symbolName) const {
  int degree = 0;
  for (int i = 0; i < numberOfChildren(); i++) {
    int d = childAtIndex(i)->polynomialDegree(symbolName);
    if (d < 0) {
      return -1;
    }
    degree = d > degree ? d : degree;
  }
  return degree;
}

// Private

bool SubtractionNode::needsParenthesesWithParent(const SerializationHelperInterface * parent) const {
  Type types[] = {Type::Subtraction, Type::Opposite, Type::Multiplication, Type::Division, Type::Power, Type::Factorial};
  return static_cast<const ExpressionNode *>(parent)->isOfType(types, 6);
}

LayoutRef SubtractionNode::createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
  return LayoutHelper::Infix(Subtraction(this), floatDisplayMode, numberOfSignificantDigits, name());
}

int SubtractionNode::serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const {
    return SerializationHelper::Infix(this, buffer, bufferSize, floatDisplayMode, numberOfSignificantDigits, name());
}

template<typename T> MatrixComplex<T> SubtractionNode::computeOnComplexAndMatrix(const std::complex<T> c, const MatrixComplex<T> m) {
  MatrixComplex<T> opposite = computeOnMatrixAndComplex(m, c);
  MatrixComplex<T> result;
  for (int i = 0; i < opposite.numberOfChildren(); i++) {
    result.addChildAtIndexInPlace(OppositeNode::compute(opposite.complexAtIndex(i)), i, i);
  }
  result.setDimensions(opposite.numberOfRows(), opposite.numberOfColumns());
  return result;
}

Expression SubtractionNode::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) {
  return Subtraction(this).shallowReduce(context, angleUnit);
}

Expression Subtraction::shallowReduce(Context & context, Preferences::AngleUnit angleUnit) {
  Expression e = Expression::defaultShallowReduce(context, angleUnit);
  if (e.isUndefined()) {
    return e;
  }
  Expression m = Multiplication(Rational(-1), childAtIndex(1));
  Addition a(childAtIndex(0), m);
  m = m.shallowReduce(context, angleUnit);
  replaceWithInPlace(a);
  return a.shallowReduce(context, angleUnit);
}

}
