#ifndef POINCARE_REEL_PART_H
#define POINCARE_REEL_PART_H

#include <poincare/expression.h>
#include <poincare/approximation_helper.h>

namespace Poincare {

class RealPartNode final : public ExpressionNode  {
public:

  // TreeNode
  size_t size() const override { return sizeof(RealPartNode); }
  int numberOfChildren() const override;
#if POINCARE_TREE_LOG
  virtual void logNodeName(std::ostream & stream) const override {
    stream << "RealPart";
  }
#endif

  // Properties
  Type type() const override { return Type::RealPart; }

  // Complex
  bool isReal(Context & context) const override { return true; }

private:
  // Layout
  Layout createLayout(Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  int serialize(char * buffer, int bufferSize, Preferences::PrintFloatMode floatDisplayMode, int numberOfSignificantDigits) const override;
  // Simplification
  Expression shallowReduce(Context & context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit, ReductionTarget target) override;
  // Evaluation
  template<typename T> static Complex<T> computeOnComplex(const std::complex<T> c, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) {
    return Complex<T>::Builder(std::real(c));
  }
  Evaluation<float> approximate(SinglePrecision p, Context& context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const override {
    return ApproximationHelper::Map<float>(this, context, complexFormat, angleUnit,computeOnComplex<float>);
  }
  Evaluation<double> approximate(DoublePrecision p, Context& context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit) const override {
    return ApproximationHelper::Map<double>(this, context, complexFormat, angleUnit, computeOnComplex<double>);
  }
};

class RealPart final : public Expression {
public:
  RealPart(const RealPartNode * n) : Expression(n) {}
  static RealPart Builder(Expression child) { return TreeHandle::FixedArityBuilder<RealPart, RealPartNode>(&child, 1); }

  static constexpr Expression::FunctionHelper s_functionHelper = Expression::FunctionHelper("re", 1, &UntypedBuilderOneChild<RealPart>);

  Expression shallowReduce(Context & context, Preferences::ComplexFormat complexFormat, Preferences::AngleUnit angleUnit, ExpressionNode::ReductionTarget target);
};

}

#endif
