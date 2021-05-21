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

#include <ocs2_core/cost/StateCostCollection.h>
#include <ocs2_core/cost/StateInputCostCollection.h>

class SimpleQuadraticCost final : public ocs2::StateInputCost {
 public:
  SimpleQuadraticCost(ocs2::matrix_t Q, ocs2::matrix_t R) : Q_(std::move(Q)), R_(std::move(R)) {}
  ~SimpleQuadraticCost() override = default;

  SimpleQuadraticCost* clone() const override { return new SimpleQuadraticCost(*this); }

  ocs2::scalar_t getValue(ocs2::scalar_t t, const ocs2::vector_t& x, const ocs2::vector_t& u, const ocs2::CostDesiredTrajectories&,
                          const ocs2::PreComputation&) const override {
    return 0.5 * x.dot(Q_ * x) + 0.5 * u.dot(R_ * u);
  }

  ocs2::ScalarFunctionQuadraticApproximation getQuadraticApproximation(ocs2::scalar_t t, const ocs2::vector_t& x, const ocs2::vector_t& u,
                                                                       const ocs2::CostDesiredTrajectories&,
                                                                       const ocs2::PreComputation&) const override {
    ocs2::ScalarFunctionQuadraticApproximation quadraticApproximation;
    quadraticApproximation.f = 0.5 * x.dot(Q_ * x) + 0.5 * u.dot(R_ * u);
    quadraticApproximation.dfdx = Q_ * x;
    quadraticApproximation.dfdu = R_ * u;
    quadraticApproximation.dfdxx = Q_;
    quadraticApproximation.dfduu = R_;
    quadraticApproximation.dfdux.setZero(u.rows(), x.rows());
    return quadraticApproximation;
  }

 private:
  ocs2::matrix_t Q_;
  ocs2::matrix_t R_;
};

class StateInputCost_TestFixture : public ::testing::Test {
 public:
  const size_t STATE_DIM = 4;
  const size_t INPUT_DIM = 2;

  StateInputCost_TestFixture() {
    // Define cost parameters
    ocs2::matrix_t Q = ocs2::matrix_t::Random(STATE_DIM, STATE_DIM);
    ocs2::matrix_t R = ocs2::matrix_t::Random(INPUT_DIM, INPUT_DIM);
    Q = (Q + Q.transpose()).eval();
    R = (R + R.transpose()).eval();

    x = ocs2::vector_t::Random(STATE_DIM);
    u = ocs2::vector_t::Random(INPUT_DIM);
    t = 0.0;

    auto cost = std::unique_ptr<SimpleQuadraticCost>(new SimpleQuadraticCost(std::move(Q), std::move(R)));
    expectedCost = cost->getValue(t, x, u, desiredTrajectory, ocs2::PreComputation());
    expectedCostApproximation = cost->getQuadraticApproximation(t, x, u, desiredTrajectory, ocs2::PreComputation());

    costCollection.add("Simple quadratic cost", std::move(cost));
  }

  ocs2::CostDesiredTrajectories desiredTrajectory;
  ocs2::StateInputCostCollection costCollection;

  ocs2::vector_t x;
  ocs2::vector_t u;
  ocs2::scalar_t t;
  ocs2::scalar_t expectedCost;
  ocs2::ScalarFunctionQuadraticApproximation expectedCostApproximation;
};

TEST_F(StateInputCost_TestFixture, getStateInputCost) {
  const auto cost = costCollection.getValue(t, x, u, desiredTrajectory, ocs2::PreComputation());
  EXPECT_NEAR(cost, expectedCost, 1e-6);
}

TEST_F(StateInputCost_TestFixture, getStateInputCostApproximation) {
  const auto cost = costCollection.getQuadraticApproximation(t, x, u, desiredTrajectory, ocs2::PreComputation());
  EXPECT_NEAR(cost.f, expectedCost, 1e-6);
  EXPECT_TRUE(cost.dfdx.isApprox(expectedCostApproximation.dfdx));
  EXPECT_TRUE(cost.dfdu.isApprox(expectedCostApproximation.dfdu));
  EXPECT_TRUE(cost.dfdxx.isApprox(expectedCostApproximation.dfdxx));
  EXPECT_TRUE(cost.dfduu.isApprox(expectedCostApproximation.dfduu));
  EXPECT_TRUE((cost.dfdux.array() == 0.0).all());
}

TEST_F(StateInputCost_TestFixture, canGetCostFunction) {
  const auto& costFunction = costCollection.get("Simple quadratic cost");
}

TEST_F(StateInputCost_TestFixture, throwsIfCostdoesNotExist) {
  EXPECT_THROW(
      {
        const auto& costFunction = costCollection.get("Nonexisting cost");
        // Should not reach this
      },
      std::out_of_range);
}

TEST_F(StateInputCost_TestFixture, throwsWhenAddExistingCost) {
  std::unique_ptr<ocs2::StateInputCost> copy(costCollection.get("Simple quadratic cost").clone());
  EXPECT_THROW(
      {
        costCollection.add("Simple quadratic cost", std::move(copy));
        // Should not reach this
      },
      std::runtime_error);
}

TEST_F(StateInputCost_TestFixture, canDeactivateCost) {
  auto& costFunction = costCollection.get("Simple quadratic cost");
  costFunction.setActivity(false);
  const auto cost = costCollection.getValue(t, x, u, desiredTrajectory, ocs2::PreComputation());
  EXPECT_EQ(cost, 0.0);
}

TEST_F(StateInputCost_TestFixture, moveConstrut) {
  ocs2::StateInputCostCollection newColleciton(std::move(costCollection));
  const auto cost = newColleciton.getValue(t, x, u, desiredTrajectory, ocs2::PreComputation());
  EXPECT_NEAR(cost, expectedCost, 1e-6);
}

TEST_F(StateInputCost_TestFixture, moveAssign) {
  ocs2::StateInputCostCollection newColleciton;
  newColleciton = std::move(costCollection);
  const auto cost = newColleciton.getValue(t, x, u, desiredTrajectory, ocs2::PreComputation());
  EXPECT_NEAR(cost, expectedCost, 1e-6);
}

class SimpleQuadraticFinalCost final : public ocs2::StateCost {
 public:
  SimpleQuadraticFinalCost(ocs2::matrix_t Q) : Q_(std::move(Q)) {}
  ~SimpleQuadraticFinalCost() override = default;

  SimpleQuadraticFinalCost* clone() const override { return new SimpleQuadraticFinalCost(*this); }

  ocs2::scalar_t getValue(ocs2::scalar_t t, const ocs2::vector_t& x, const ocs2::CostDesiredTrajectories&,
                          const ocs2::PreComputation&) const override {
    return 0.5 * x.dot(Q_ * x);
  }

  ocs2::ScalarFunctionQuadraticApproximation getQuadraticApproximation(ocs2::scalar_t t, const ocs2::vector_t& x,
                                                                       const ocs2::CostDesiredTrajectories&,
                                                                       const ocs2::PreComputation&) const override {
    ocs2::ScalarFunctionQuadraticApproximation quadraticApproximation;
    quadraticApproximation.f = 0.5 * x.dot(Q_ * x);
    quadraticApproximation.dfdx = Q_ * x;
    quadraticApproximation.dfdxx = Q_;
    return quadraticApproximation;
  }

 private:
  ocs2::matrix_t Q_;
};

class StateCost_TestFixture : public ::testing::Test {
 public:
  const size_t STATE_DIM = 4;
  const size_t INPUT_DIM = 2;

  StateCost_TestFixture() {
    // Define cost parameters
    ocs2::matrix_t Q = ocs2::matrix_t::Random(STATE_DIM, STATE_DIM);
    Q = (Q + Q.transpose()).eval();

    x = ocs2::vector_t::Random(STATE_DIM);
    t = 0.0;

    auto cost = std::unique_ptr<SimpleQuadraticFinalCost>(new SimpleQuadraticFinalCost(std::move(Q)));
    expectedCost = cost->getValue(t, x, desiredTrajectory, ocs2::PreComputation());
    expectedCostApproximation = cost->getQuadraticApproximation(t, x, desiredTrajectory, ocs2::PreComputation());

    costCollection.add("Simple quadratic final cost", std::move(cost));
  }

  ocs2::CostDesiredTrajectories desiredTrajectory;
  ocs2::StateCostCollection costCollection;

  ocs2::vector_t x;
  ocs2::scalar_t t;
  ocs2::scalar_t expectedCost;
  ocs2::ScalarFunctionQuadraticApproximation expectedCostApproximation;
};

TEST_F(StateCost_TestFixture, testStateCost) {
  const auto cost = costCollection.getValue(t, x, desiredTrajectory, ocs2::PreComputation());
  EXPECT_NEAR(cost, expectedCost, 1e-6);
}

TEST_F(StateCost_TestFixture, testStateCostApproximation) {
  const auto cost = costCollection.getQuadraticApproximation(t, x, desiredTrajectory, ocs2::PreComputation());
  EXPECT_NEAR(cost.f, expectedCost, 1e-6);
  EXPECT_TRUE(cost.dfdx.isApprox(expectedCostApproximation.dfdx));
  EXPECT_TRUE(cost.dfdxx.isApprox(expectedCostApproximation.dfdxx));
}
