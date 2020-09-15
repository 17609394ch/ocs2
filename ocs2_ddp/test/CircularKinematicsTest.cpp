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

#include <gtest/gtest.h>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>

#include <boost/filesystem.hpp>

#include <ocs2_ddp/ILQR.h>
#include <ocs2_ddp/SLQ.h>
#include <ocs2_oc/rollout/TimeTriggeredRollout.h>
#include <ocs2_oc/test/circular_kinematics.h>

class CircularKinematicsTest : public testing::Test {
 protected:
  static constexpr size_t STATE_DIM = 2;
  static constexpr size_t INPUT_DIM = 2;
  static constexpr ocs2::scalar_t expectedCost = 0.1;
  static constexpr ocs2::scalar_t expectedStateInputEqConstraintISE = 0.0;

  CircularKinematicsTest() {
    // rollout settings
    const auto rolloutSettings = []() {
      ocs2::rollout::Settings rolloutSettings;
      rolloutSettings.absTolODE_ = 1e-9;
      rolloutSettings.relTolODE_ = 1e-7;
      rolloutSettings.maxNumStepsPerSecond_ = 10000;
      return rolloutSettings;
    }();

    // dynamics and rollout
    systemPtr.reset(new ocs2::CircularKinematicsSystem);
    rolloutPtr.reset(new ocs2::TimeTriggeredRollout(*systemPtr, rolloutSettings));

    // cost function
    boost::filesystem::path filePath(__FILE__);
    std::string libraryFolder = filePath.parent_path().generic_string() + "/ddp_test_generated";
    costPtr.reset(new ocs2::CircularKinematicsCost);
    costPtr->initialize("circular_kinematics_cost", "/tmp/ocs2", true, false);

    // constraint
    constraintPtr.reset(new ocs2::CircularKinematicsConstraints);

    // operatingTrajectories
    operatingPointsPtr.reset(new ocs2::OperatingPoints(initState, ocs2::vector_t::Zero(INPUT_DIM)));
  }

  ocs2::ddp::Settings getSettings(ocs2::ddp::Algorithm algorithmType, size_t numThreads,
                                  ocs2::ddp_strategy::Type strategy, bool display = false) const {
    ocs2::ddp::Settings ddpSettings;
    ddpSettings.algorithm_ = algorithmType;
    ddpSettings.nThreads_ = numThreads;
    ddpSettings.displayInfo_ = false;
    ddpSettings.displayShortSummary_ = display;
    ddpSettings.checkNumericalStability_ = false;
    ddpSettings.debugPrintRollout_ = false;
    ddpSettings.absTolODE_ = 1e-9;
    ddpSettings.relTolODE_ = 1e-7;
    ddpSettings.maxNumStepsPerSecond_ = 10000;
    ddpSettings.backwardPassIntegratorType_ = ocs2::IntegratorType::ODE45;
    ddpSettings.maxNumIterations_ = 150;
    ddpSettings.minRelCost_ = 1e-3;
    ddpSettings.constraintTolerance_ = 1e-5;
    ddpSettings.constraintPenaltyInitialValue_ = 2.0;
    ddpSettings.constraintPenaltyIncreaseRate_ = 1.5;
    ddpSettings.preComputeRiccatiTerms_ = false;
    ddpSettings.useNominalTimeForBackwardPass_ = true;
    ddpSettings.strategy_ = strategy;
    ddpSettings.lineSearch_.minStepLength_ = 0.01;
    ddpSettings.lineSearch_.hessianCorrectionStrategy_ = ocs2::hessian_correction::Strategy::CHOLESKY_MODIFICATION;
    ddpSettings.lineSearch_.hessianCorrectionMultiple_ = 1e-3;
    return ddpSettings;
  }

  std::string getTestName(const ocs2::ddp::Settings& ddpSettings) const {
    std::string testName;
    testName += "Circular-Kinematics Test { ";
    testName += "Algorithm: " + ocs2::ddp::toAlgorithmName(ddpSettings.algorithm_) + ",  ";
    testName += "Strategy: " + ocs2::ddp_strategy::toString(ddpSettings.strategy_) + ",  ";
    testName += "#threads: " + std::to_string(ddpSettings.nThreads_) + " }";
    return testName;
  }

  void performanceIndexTest(const ocs2::ddp::Settings& ddpSettings, const ocs2::PerformanceIndex& performanceIndex) const {
    const auto testName = getTestName(ddpSettings);
    EXPECT_LT(performanceIndex.totalCost - expectedCost, 0.0)
        << "MESSAGE: " << testName << ": failed in the total cost test!";
    EXPECT_LT(fabs(performanceIndex.stateInputEqConstraintISE - expectedStateInputEqConstraintISE), 10 * ddpSettings.constraintTolerance_)
        << "MESSAGE: " << testName << ": failed in state-input equality constraint ISE test!";
  }

  const ocs2::scalar_t startTime = 0.0;
  const ocs2::scalar_t finalTime = 10.0;
  const ocs2::vector_t initState = (ocs2::vector_t(STATE_DIM) << 1.0, 0.0).finished();  // radius 1.0
  const ocs2::scalar_array_t partitioningTimes{startTime, (startTime + finalTime) / 2.0, finalTime};

  std::unique_ptr<ocs2::SystemDynamicsBase> systemPtr;
  std::unique_ptr<ocs2::TimeTriggeredRollout> rolloutPtr;
  std::unique_ptr<ocs2::CostFunctionBaseAD> costPtr;
  std::unique_ptr<ocs2::ConstraintBase> constraintPtr;
  std::unique_ptr<ocs2::OperatingPoints> operatingPointsPtr;
};

constexpr size_t CircularKinematicsTest::STATE_DIM;
constexpr size_t CircularKinematicsTest::INPUT_DIM;
constexpr ocs2::scalar_t CircularKinematicsTest::expectedCost;
constexpr ocs2::scalar_t CircularKinematicsTest::expectedStateInputEqConstraintISE;

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_F(CircularKinematicsTest, slq_single_thread_linesearch) {
  // ddp settings
  const auto ddpSettings = getSettings(ocs2::ddp::Algorithm::SLQ, 1, ocs2::ddp_strategy::Type::LINE_SEARCH);

  // instantiate
  ocs2::SLQ ddp(rolloutPtr.get(), systemPtr.get(), constraintPtr.get(), costPtr.get(), operatingPointsPtr.get(), ddpSettings);

  if (ddpSettings.displayInfo_ || ddpSettings.displayShortSummary_) {
    std::cerr << "\n" << getTestName(ddpSettings) << "\n";
  }

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get performance index
  const auto performanceIndex = ddp.getPerformanceIndeces();

  // performanceIndeces test
  performanceIndexTest(ddpSettings, performanceIndex);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_F(CircularKinematicsTest, slq_multi_thread_linesearch) {
  // ddp settings
  const auto ddpSettings = getSettings(ocs2::ddp::Algorithm::SLQ, 3, ocs2::ddp_strategy::Type::LINE_SEARCH);

  // instantiate
  ocs2::SLQ ddp(rolloutPtr.get(), systemPtr.get(), constraintPtr.get(), costPtr.get(), operatingPointsPtr.get(), ddpSettings);

  if (ddpSettings.displayInfo_ || ddpSettings.displayShortSummary_) {
    std::cerr << "\n" << getTestName(ddpSettings) << "\n";
  }

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get performance index
  const auto performanceIndex = ddp.getPerformanceIndeces();

  // performanceIndeces test
  performanceIndexTest(ddpSettings, performanceIndex);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_F(CircularKinematicsTest, ilqr_single_thread_linesearch) {
  // ddp settings
  const auto ddpSettings = getSettings(ocs2::ddp::Algorithm::ILQR, 1, ocs2::ddp_strategy::Type::LINE_SEARCH);

  // instantiate
  ocs2::ILQR ddp(rolloutPtr.get(), systemPtr.get(), constraintPtr.get(), costPtr.get(), operatingPointsPtr.get(), ddpSettings);

  if (ddpSettings.displayInfo_ || ddpSettings.displayShortSummary_) {
    std::cerr << "\n" << getTestName(ddpSettings) << "\n";
  }

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get performance index
  const auto performanceIndex = ddp.getPerformanceIndeces();

  // performanceIndeces test
  performanceIndexTest(ddpSettings, performanceIndex);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
TEST_F(CircularKinematicsTest, ilqr_multi_thread_linesearch) {
  // ddp settings
  const auto ddpSettings = getSettings(ocs2::ddp::Algorithm::ILQR, 3, ocs2::ddp_strategy::Type::LINE_SEARCH);

  // instantiate
  ocs2::ILQR ddp(rolloutPtr.get(), systemPtr.get(), constraintPtr.get(), costPtr.get(), operatingPointsPtr.get(), ddpSettings);

  if (ddpSettings.displayInfo_ || ddpSettings.displayShortSummary_) {
    std::cerr << "\n" << getTestName(ddpSettings) << "\n";
  }

  // run ddp
  ddp.run(startTime, initState, finalTime, partitioningTimes);
  // get performance index
  const auto performanceIndex = ddp.getPerformanceIndeces();

  // performanceIndeces test
  performanceIndexTest(ddpSettings, performanceIndex);
}
