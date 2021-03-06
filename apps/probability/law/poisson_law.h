#ifndef PROBABILITE_POISSON_LAW_H
#define PROBABILITE_POISSON_LAW_H

#include "one_parameter_law.h"

namespace Probability {

class PoissonLaw final : public OneParameterLaw {
public:
  PoissonLaw() : OneParameterLaw(4.0f) {}
  I18n::Message title() override { return I18n::Message::PoissonLaw; }
  Type type() const override { return Type::Poisson; }
  bool isContinuous() const override { return false; }
  float xMin() override;
  float yMin() override;
  float xMax() override;
  float yMax() override;
  I18n::Message parameterNameAtIndex(int index) override {
    assert(index == 0);
    return I18n::Message::Lambda;
  }
  I18n::Message parameterDefinitionAtIndex(int index) override  {
    assert(index == 0);
    return I18n::Message::LambdaPoissonDefinition;
  }
  float evaluateAtAbscissa(float x) const override {
    return templatedApproximateAtAbscissa(x);
  }
  bool authorizedValueAtIndex(float x, int index) const override;
private:
  double evaluateAtDiscreteAbscissa(int k) const override {
    return templatedApproximateAtAbscissa((double)k);
  }
  template<typename T> T templatedApproximateAtAbscissa(T x) const;
};

}

#endif
