/******************************************************************************
Copyright (c) 2017, Farbod Farshidian. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
******************************************************************************/

#pragma once

#include <ocs2_core/cost/CostFunctionBase.h>
#include <ocs2_core/cost/QuadraticCostFunction.h>
#include <ocs2_core/dynamics/LinearSystemDynamics.h>
#include <ocs2_core/dynamics/SystemDynamicsBase.h>

#include <ocs2_oc/oc_solver/ModeScheduleManager.h>

namespace ocs2 {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP1_Sys1 : public SystemDynamicsBase {
 public:
  EXP1_Sys1() = default;
  ~EXP1_Sys1() override = default;

  vector_t computeFlowMap(scalar_t t, const vector_t& x, const vector_t& u) final {
    vector_t dxdt(2);
    dxdt(0) = x(0) + u(0) * sin(x(0));
    dxdt(1) = -x(1) - u(0) * cos(x(1));
    return dxdt;
  }

  VectorFunctionLinearApproximation linearApproximation(scalar_t t, const vector_t& x, const vector_t& u) {
    VectorFunctionLinearApproximation dynamics;
    dynamics.f = computeFlowMap(t, x, u);
    dynamics.dfdx.resize(2, 2);
    dynamics.dfdx << u(0) * cos(x(0)) + 1, 0, 0, u(0) * sin(x(1)) - 1;
    dynamics.dfdu.resize(2, 1);
    dynamics.dfdu << sin(x(0)), -cos(x(1));
    return dynamics;
  }

  EXP1_Sys1* clone() const final { return new EXP1_Sys1(*this); }
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP1_Sys2 : public SystemDynamicsBase {
 public:
  EXP1_Sys2() = default;
  ~EXP1_Sys2() = default;

  vector_t computeFlowMap(scalar_t t, const vector_t& x, const vector_t& u) final {
    vector_t dxdt(2);
    dxdt(0) = x(1) + u(0) * sin(x(1));
    dxdt(1) = -x(0) - u(0) * cos(x(0));
    return dxdt;
  }

  VectorFunctionLinearApproximation linearApproximation(scalar_t t, const vector_t& x, const vector_t& u) {
    VectorFunctionLinearApproximation dynamics;
    dynamics.f = computeFlowMap(t, x, u);
    dynamics.dfdx.resize(2, 2);
    dynamics.dfdx << 0, u(0) * cos(x(1)) + 1, u(0) * sin(x(0)) - 1, 0;
    dynamics.dfdu.resize(2, 1);
    dynamics.dfdu << sin(x(1)), -cos(x(0));
    return dynamics;
  }

  EXP1_Sys2* clone() const final { return new EXP1_Sys2(*this); }
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP1_Sys3 : public SystemDynamicsBase {
 public:
  EXP1_Sys3() = default;
  ~EXP1_Sys3() = default;

  vector_t computeFlowMap(scalar_t t, const vector_t& x, const vector_t& u) final {
    vector_t dxdt(2);
    dxdt(0) = -x(0) - u(0) * sin(x(0));
    dxdt(1) = x(1) + u(0) * cos(x(1));
    return dxdt;
  }

  VectorFunctionLinearApproximation linearApproximation(scalar_t t, const vector_t& x, const vector_t& u) {
    VectorFunctionLinearApproximation dynamics;
    dynamics.f = computeFlowMap(t, x, u);
    dynamics.dfdx.resize(2, 2);
    dynamics.dfdx << -u(0) * cos(x(0)) - 1, 0, 0, 1 - u(0) * sin(x(1));
    dynamics.dfdu.resize(2, 1);
    dynamics.dfdu << -sin(x(0)), cos(x(1));
    return dynamics;
  }

  EXP1_Sys3* clone() const final { return new EXP1_Sys3(*this); }
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP1_System : public SystemDynamicsBase {
 public:
  EXP1_System(std::shared_ptr<ModeScheduleManager> modeScheduleManagerPtr) : modeScheduleManagerPtr_(std::move(modeScheduleManagerPtr)) {
    subsystemDynamicsPtr_[0].reset(new EXP1_Sys1());
    subsystemDynamicsPtr_[1].reset(new EXP1_Sys2());
    subsystemDynamicsPtr_[2].reset(new EXP1_Sys3());
  }

  ~EXP1_System() = default;

  EXP1_System(const EXP1_System& other) : EXP1_System(other.modeScheduleManagerPtr_) {}

  EXP1_System* clone() const final { return new EXP1_System(*this); }

  vector_t computeFlowMap(scalar_t t, const vector_t& x, const vector_t& u) final {
    const auto activeMode = modeScheduleManagerPtr_->getModeSchedule().modeAtTime(t);
    return subsystemDynamicsPtr_[activeMode]->computeFlowMap(t, x, u);
  }

  VectorFunctionLinearApproximation linearApproximation(scalar_t t, const vector_t& x, const vector_t& u) final {
    const auto activeMode = modeScheduleManagerPtr_->getModeSchedule().modeAtTime(t);
    return subsystemDynamicsPtr_[activeMode]->linearApproximation(t, x, u);
  }

 private:
  std::shared_ptr<ModeScheduleManager> modeScheduleManagerPtr_;
  std::vector<std::shared_ptr<SystemDynamicsBase>> subsystemDynamicsPtr_{3};
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
class EXP1_CostFunction : public CostFunctionBase {
 public:
  explicit EXP1_CostFunction(std::shared_ptr<ModeScheduleManager> modeScheduleManagerPtr)
      : modeScheduleManagerPtr_(std::move(modeScheduleManagerPtr)) {
    matrix_t Q(2, 2);
    matrix_t R(1, 1);
    matrix_t Qf(2, 2);
    Q << 1.0, 0.0, 0.0, 1.0;
    R << 1.0;
    Qf << 1.0, 0.0, 0.0, 1.0;
    subsystemCostsPtr_[0].reset(new QuadraticCostFunction(Q, R, matrix_t::Zero(2, 2)));
    subsystemCostsPtr_[1].reset(new QuadraticCostFunction(Q, R, matrix_t::Zero(2, 2)));
    subsystemCostsPtr_[2].reset(new QuadraticCostFunction(Q, R, Qf));

    vector_t x(2);
    vector_t u(1);
    x << 1.0, -1.0;
    u << 0.0;
    costDesiredTrajectories_ = CostDesiredTrajectories({0.0}, {x}, {u});
    subsystemCostsPtr_[0]->setCostDesiredTrajectoriesPtr(&costDesiredTrajectories_);
    subsystemCostsPtr_[1]->setCostDesiredTrajectoriesPtr(&costDesiredTrajectories_);
    subsystemCostsPtr_[2]->setCostDesiredTrajectoriesPtr(&costDesiredTrajectories_);
  }

  ~EXP1_CostFunction() = default;

  EXP1_CostFunction(const EXP1_CostFunction& other) : EXP1_CostFunction(other.modeScheduleManagerPtr_) {}

  EXP1_CostFunction* clone() const final { return new EXP1_CostFunction(*this); }

  scalar_t cost(scalar_t t, const vector_t& x, const vector_t& u) final {
    const auto activeMode = modeScheduleManagerPtr_->getModeSchedule().modeAtTime(t);
    return subsystemCostsPtr_[activeMode]->cost(t, x, u);
  }

  scalar_t finalCost(scalar_t t, const vector_t& x) final {
    const auto activeMode = modeScheduleManagerPtr_->getModeSchedule().modeAtTime(t);
    return subsystemCostsPtr_[activeMode]->finalCost(t, x);
  }

  ScalarFunctionQuadraticApproximation costQuadraticApproximation(scalar_t t, const vector_t& x, const vector_t& u) final {
    const auto activeMode = modeScheduleManagerPtr_->getModeSchedule().modeAtTime(t);
    return subsystemCostsPtr_[activeMode]->costQuadraticApproximation(t, x, u);
  }

  ScalarFunctionQuadraticApproximation finalCostQuadraticApproximation(scalar_t t, const vector_t& x) final {
    const auto activeMode = modeScheduleManagerPtr_->getModeSchedule().modeAtTime(t);
    return subsystemCostsPtr_[activeMode]->finalCostQuadraticApproximation(t, x);
  }

 public:
  std::shared_ptr<ModeScheduleManager> modeScheduleManagerPtr_;
  std::vector<std::shared_ptr<CostFunctionBase>> subsystemCostsPtr_{3};
  CostDesiredTrajectories costDesiredTrajectories_;
};

}  // namespace ocs2
