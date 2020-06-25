//
// Created by ruben on 14.09.18.
//

#pragma once

#include <ocs2_core/loopshaping/dynamics/LoopshapingDynamics.h>

namespace ocs2 {

class LoopshapingDynamicsInputPattern final : public LoopshapingDynamics {
 public:
  using BASE = LoopshapingDynamics;

  LoopshapingDynamicsInputPattern(const SystemDynamicsBase& controlledSystem, std::shared_ptr<LoopshapingDefinition> loopshapingDefinition)
      : BASE(controlledSystem, std::move(loopshapingDefinition)) {}

  ~LoopshapingDynamicsInputPattern() override = default;

  LoopshapingDynamicsInputPattern(const LoopshapingDynamicsInputPattern& obj) = default;

  LoopshapingDynamicsInputPattern* clone() const override { return new LoopshapingDynamicsInputPattern(*this); };

  VectorFunctionLinearApproximation linearApproximation(scalar_t t, const vector_t& x, const vector_t& u) override;
  VectorFunctionLinearApproximation jumpMapLinearApproximation(scalar_t t, const vector_t& x, const vector_t& u) override;

 protected:
  using BASE::loopshapingDefinition_;

 private:
  vector_t filterFlowmap(const vector_t& x_filter, const vector_t& u_filter, const vector_t& u_system) override;
};

}  // namespace ocs2
