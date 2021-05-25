/******************************************************************************
Copyright (c) 2020, Farbod Farshidian. All rights reserved.

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

#include <iostream>

#include <ocs2_core/misc/LinearAlgebra.h>
#include <ocs2_oc/approximate_model/LinearQuadraticApproximator.h>

namespace ocs2 {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void LinearQuadraticApproximator::approximateLQProblem(const scalar_t& time, const vector_t& state, const vector_t& input,
                                                       ModelData& modelData) const {
  const auto flags = Request::Cost | Request::SoftConstraint | Request::Constraint | Request::Dynamics | Request::Approximation;
  problem_.preComputationPtr->request(flags, time, state, input);

  // dynamics
  approximateDynamics(time, state, input, modelData);

  // constraints
  approximateConstraints(time, state, input, modelData);

  // cost
  approximateCost(time, state, input, modelData);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void LinearQuadraticApproximator::approximateUnconstrainedLQProblem(const scalar_t& time, const vector_t& state, const vector_t& input,
                                                                    ModelData& modelData) const {
  const auto flags = Request::Cost | Request::SoftConstraint | Request::Dynamics | Request::Approximation;
  problem_.preComputationPtr->request(flags, time, state, input);

  // dynamics
  approximateDynamics(time, state, input, modelData);

  // cost
  approximateCost(time, state, input, modelData);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void LinearQuadraticApproximator::approximateLQProblemAtEventTime(const scalar_t& time, const vector_t& state, ModelData& modelData) const {
  auto& preComputation = *problem_.preComputationPtr;
  const auto flags = Request::Cost | Request::SoftConstraint | Request::Constraint | Request::Dynamics | Request::Approximation;
  preComputation.requestPreJump(flags, time, state);

  // Jump map
  modelData.dynamics_ = problem_.dynamicsPtr->jumpMapLinearApproximation(time, state, preComputation);

  // Pre-jump constraints
  // state-only equality constraint
  modelData.stateEqConstr_ = problem_.preJumpEqualityConstraint.getLinearApproximation(time, state, preComputation);

  // Pre-jump cost
  modelData.cost_ = problem_.preJumpCost.getQuadraticApproximation(time, state, *problem_.costDesiredTrajectories, preComputation);
  modelData.cost_ +=
      problem_.preJumpSoftConstraint.getQuadraticApproximation(time, state, *problem_.costDesiredTrajectories, preComputation);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void LinearQuadraticApproximator::approximateLQProblemAtFinalTime(const scalar_t& time, const vector_t& state, ModelData& modelData) const {
  auto& preComputation = *problem_.preComputationPtr;
  const auto flags = Request::Cost | Request::SoftConstraint | Request::Constraint | Request::Approximation;
  preComputation.requestFinal(flags, time, state);

  // state-only equality constraint
  modelData.stateEqConstr_ = problem_.finalEqualityConstraint.getLinearApproximation(time, state, preComputation);

  // Final cost
  modelData.cost_ = problem_.finalCost.getQuadraticApproximation(time, state, *problem_.costDesiredTrajectories, preComputation);
  modelData.cost_ += problem_.finalSoftConstraint.getQuadraticApproximation(time, state, *problem_.costDesiredTrajectories, preComputation);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void LinearQuadraticApproximator::approximateDynamics(const scalar_t& time, const vector_t& state, const vector_t& input,
                                                      ModelData& modelData) const {
  // get results
  modelData.dynamics_ = problem_.dynamicsPtr->linearApproximation(time, state, input, *problem_.preComputationPtr);
  modelData.dynamicsCovariance_ = problem_.dynamicsPtr->dynamicsCovariance(time, state, input);

  // checking the numerical stability
  if (checkNumericalCharacteristics_) {
    std::string err = modelData.checkDynamicsDerivativsProperties();
    if (!err.empty()) {
      std::cerr << "what(): " << err << " at time " << time << " [sec]." << std::endl;
      std::cerr << "x: " << state.transpose() << '\n';
      std::cerr << "u: " << input.transpose() << '\n';
      std::cerr << "Am: \n" << modelData.dynamics_.dfdx << std::endl;
      std::cerr << "Bm: \n" << modelData.dynamics_.dfdu << std::endl;
      throw std::runtime_error(err);
    }
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void LinearQuadraticApproximator::approximateConstraints(const scalar_t& time, const vector_t& state, const vector_t& input,
                                                         ModelData& modelData) const {
  // State-input equality constraint
  modelData.stateInputEqConstr_ = problem_.equalityConstraint.getLinearApproximation(time, state, input, *problem_.preComputationPtr);
  if (modelData.stateInputEqConstr_.f.rows() > input.rows()) {
    throw std::runtime_error("Number of active state-input equality constraints should be less-equal to the input dimension.");
  }

  // State-only equality constraint
  modelData.stateEqConstr_ = problem_.stateEqualityConstraint.getLinearApproximation(time, state, *problem_.preComputationPtr);
  if (modelData.stateEqConstr_.f.rows() > input.rows()) {
    throw std::runtime_error("Number of active state-only equality constraints should be less-equal to the input dimension.");
  }

  // Inequality constraint
  modelData.ineqConstr_ = problem_.inequalityConstraint.getQuadraticApproximation(time, state, input, *problem_.preComputationPtr);

  if (checkNumericalCharacteristics_) {
    std::string err = modelData.checkConstraintProperties();
    if (!err.empty()) {
      std::cerr << "what(): " << err << " at time " << time << " [sec]." << std::endl;
      std::cerr << "x: " << state.transpose() << '\n';
      std::cerr << "u: " << input.transpose() << '\n';
      std::cerr << "Ev: " << modelData.stateInputEqConstr_.f.transpose() << std::endl;
      std::cerr << "Cm: \n" << modelData.stateInputEqConstr_.dfdx << std::endl;
      std::cerr << "Dm: \n" << modelData.stateInputEqConstr_.dfdu << std::endl;
      std::cerr << "Hv: " << modelData.stateEqConstr_.f.transpose() << std::endl;
      std::cerr << "Fm: \n" << modelData.stateEqConstr_.dfdx << std::endl;
      throw std::runtime_error(err);
    }
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void LinearQuadraticApproximator::approximateCost(const scalar_t& time, const vector_t& state, const vector_t& input,
                                                  ModelData& modelData) const {
  const auto& desiredTrajectory = *problem_.costDesiredTrajectories;
  const auto& preComputation = *problem_.preComputationPtr;

  // get results
  modelData.cost_ = problem_.cost.getQuadraticApproximation(time, state, input, desiredTrajectory, preComputation);
  modelData.cost_ += problem_.softConstraint.getQuadraticApproximation(time, state, input, desiredTrajectory, preComputation);

  // add soft constraint penalty
  auto stateCost = problem_.stateCost.getQuadraticApproximation(time, state, desiredTrajectory, preComputation);
  stateCost += problem_.stateSoftConstraint.getQuadraticApproximation(time, state, desiredTrajectory, preComputation);
  modelData.cost_.f += stateCost.f;
  modelData.cost_.dfdx += stateCost.dfdx;
  modelData.cost_.dfdxx += stateCost.dfdxx;

  // checking the numerical stability
  if (checkNumericalCharacteristics_) {
    std::string err = modelData.checkCostProperties();
    if (!err.empty()) {
      std::cerr << "what(): " << err << " at time " << time << " [sec]." << '\n';
      std::cerr << "x: " << state.transpose() << '\n';
      std::cerr << "u: " << input.transpose() << '\n';
      std::cerr << "q: " << modelData.cost_.f << '\n';
      std::cerr << "Qv: " << modelData.cost_.dfdx.transpose() << '\n';
      std::cerr << "Qm: \n" << modelData.cost_.dfdxx << '\n';
      std::cerr << "Qm eigenvalues : " << LinearAlgebra::eigenvalues(modelData.cost_.dfdxx).transpose() << '\n';
      std::cerr << "Rv: " << modelData.cost_.dfdu.transpose() << '\n';
      std::cerr << "Rm: \n" << modelData.cost_.dfduu << '\n';
      std::cerr << "Rm eigenvalues : " << LinearAlgebra::eigenvalues(modelData.cost_.dfduu).transpose() << '\n';
      std::cerr << "Pm: \n" << modelData.cost_.dfdux << '\n';
      throw std::runtime_error(err);
    }
  }
}

}  // namespace ocs2
