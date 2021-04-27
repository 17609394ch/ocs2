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

#include <gtest/gtest.h>

#include <ocs2_qp_solver/test/testProblemsGeneration.h>
#include "ocs2_sqp/MultipleShootingSolver.h"

namespace ocs2 {
namespace {

PrimalSolution solveWithFeedbackSetting(bool feedback, bool emptyConstraint, const VectorFunctionLinearApproximation& dynamicsMatrices,
                                        const ScalarFunctionQuadraticApproximation& costMatrices) {
  int n = dynamicsMatrices.dfdu.rows();
  int m = dynamicsMatrices.dfdu.cols();

  // System
  auto systemPtr = ocs2::qp_solver::getOcs2Dynamics(dynamicsMatrices);

  // Cost
  auto costPtr = ocs2::qp_solver::getOcs2Cost(costMatrices, costMatrices);
  ocs2::CostDesiredTrajectories costDesiredTrajectories({0.0}, {ocs2::vector_t::Ones(n)}, {ocs2::vector_t::Ones(m)});
  costPtr->setCostDesiredTrajectoriesPtr(&costDesiredTrajectories);

  // Solver settings
  ocs2::multiple_shooting::Settings settings;
  settings.dt = 0.05;
  settings.n_state = n;
  settings.n_input = m;
  settings.sqpIteration = 20;
  settings.projectStateInputEqualityConstraints = true;
  settings.useFeedbackPolicy = feedback;
  settings.printSolverStatistics = false;
  settings.printSolverStatus = false;
  settings.printLinesearch = false;
  std::unique_ptr<ocs2::MultipleShootingSolver> solver;
  if (emptyConstraint) {
    ConstraintBase emptyBaseConstraints;
    solver.reset(new ocs2::MultipleShootingSolver(settings, systemPtr.get(), costPtr.get(), &emptyBaseConstraints));
  } else {
    solver.reset(new ocs2::MultipleShootingSolver(settings, systemPtr.get(), costPtr.get(), nullptr));
  }
  solver->setCostDesiredTrajectories(costDesiredTrajectories);

  // Additional problem definitions
  const ocs2::scalar_t startTime = 0.0;
  const ocs2::scalar_t finalTime = 1.0;
  const ocs2::vector_t initState = ocs2::vector_t::Zero(n);
  const ocs2::scalar_array_t partitioningTimes{0.0};

  // Solve
  solver->run(startTime, initState, finalTime, partitioningTimes);
  return solver->primalSolution(finalTime);
}

}  // namespace
}  // namespace ocs2

TEST(test_unconstrained, withFeedback) {
  int n = 3;
  int m = 2;
  const auto dynamics = ocs2::qp_solver::getRandomDynamics(n, m);
  const auto costs = ocs2::qp_solver::getRandomCost(n, m);
  const auto withEmptyConstraint = ocs2::solveWithFeedbackSetting(true, true, dynamics, costs);
  const auto withNullConstraint = ocs2::solveWithFeedbackSetting(true, false, dynamics, costs);

  // Compare
  const double tol = 1e-9;
  for (int i = 0; i < withEmptyConstraint.timeTrajectory_.size(); i++) {
    ASSERT_DOUBLE_EQ(withEmptyConstraint.timeTrajectory_[i], withNullConstraint.timeTrajectory_[i]);
    ASSERT_TRUE(withEmptyConstraint.stateTrajectory_[i].isApprox(withNullConstraint.stateTrajectory_[i], tol));
    ASSERT_TRUE(withEmptyConstraint.inputTrajectory_[i].isApprox(withNullConstraint.inputTrajectory_[i], tol));
    const auto t = withEmptyConstraint.timeTrajectory_[i];
    const auto& x = withEmptyConstraint.stateTrajectory_[i];
    ASSERT_TRUE(withEmptyConstraint.controllerPtr_->computeInput(t, x).isApprox(withNullConstraint.controllerPtr_->computeInput(t, x), tol));
  }
}

TEST(test_unconstrained, noFeedback) {
  int n = 3;
  int m = 2;
  const auto dynamics = ocs2::qp_solver::getRandomDynamics(n, m);
  const auto costs = ocs2::qp_solver::getRandomCost(n, m);
  const auto withEmptyConstraint = ocs2::solveWithFeedbackSetting(false, true, dynamics, costs);
  const auto withNullConstraint = ocs2::solveWithFeedbackSetting(false, false, dynamics, costs);

  // Compare
  const double tol = 1e-9;
  for (int i = 0; i < withEmptyConstraint.timeTrajectory_.size(); i++) {
    ASSERT_DOUBLE_EQ(withEmptyConstraint.timeTrajectory_[i], withNullConstraint.timeTrajectory_[i]);
    ASSERT_TRUE(withEmptyConstraint.stateTrajectory_[i].isApprox(withNullConstraint.stateTrajectory_[i], tol));
    ASSERT_TRUE(withEmptyConstraint.inputTrajectory_[i].isApprox(withNullConstraint.inputTrajectory_[i], tol));
    const auto t = withEmptyConstraint.timeTrajectory_[i];
    const auto& x = withEmptyConstraint.stateTrajectory_[i];
    ASSERT_TRUE(withEmptyConstraint.controllerPtr_->computeInput(t, x).isApprox(withNullConstraint.controllerPtr_->computeInput(t, x), tol));
  }
}
