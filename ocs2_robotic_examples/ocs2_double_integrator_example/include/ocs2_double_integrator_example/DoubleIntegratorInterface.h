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

// C++
#include <cstdlib>
#include <iostream>
#include <string>

// OCS2
#include <ocs2_core/Types.h>
#include <ocs2_core/constraint/ConstraintBase.h>
#include <ocs2_core/cost/QuadraticCostFunction.h>
#include <ocs2_core/initialization/OperatingPoints.h>
#include <ocs2_oc/rollout/TimeTriggeredRollout.h>

#include <ocs2_mpc/MPC_SLQ.h>
#include <ocs2_robotic_tools/common/RobotInterface.h>

// Double Integrator
#include "cost/DoubleIntegratorCost.h"
#include "definitions.h"
#include "dynamics/DoubleIntegratorDynamics.h"

namespace ocs2 {
namespace double_integrator {

class DoubleIntegratorInterface final : public RobotInterface {
 public:
  /**
   * Constructor
   * @param [in] taskFileFolderName: The name of the folder containing task file
   */
  explicit DoubleIntegratorInterface(const std::string& taskFileFolderName);

  /**
   * Destructor
   */
  ~DoubleIntegratorInterface() override = default;

  const vector_t& getInitialState() { return initialState_; }
  SLQ_Settings& slqSettings() { return slqSettings_; };
  MPC_Settings& mpcSettings() { return mpcSettings_; };

  std::unique_ptr<ocs2::MPC_SLQ> getMpc();

  const vector_t& getXFinal() { return xFinal_; }

  const DoubleIntegratorDynamics& getDynamics() const override { return *linearSystemDynamicsPtr_; }

  const DoubleIntegratorCost& getCost() const override { return *linearSystemCostPtr_; }

  const RolloutBase& getRollout() const { return *ddpLinearSystemRolloutPtr_; }

  const OperatingPoints& getOperatingPoints() const override { return *linearSystemOperatingPointPtr_; }

 protected:
  /**
   * Loads the settings from the path file.
   *
   * @param [in] taskFile: Task's file full path.
   */
  void loadSettings(const std::string& taskFile);

  /**************
   * Variables
   **************/
  std::string taskFile_;
  std::string libraryFolder_;

  SLQ_Settings slqSettings_;
  MPC_Settings mpcSettings_;

  std::unique_ptr<RolloutBase> ddpLinearSystemRolloutPtr_;

  std::unique_ptr<DoubleIntegratorDynamics> linearSystemDynamicsPtr_;
  std::unique_ptr<DoubleIntegratorCost> linearSystemCostPtr_;
  std::unique_ptr<ConstraintBase> linearSystemConstraintPtr_;
  std::unique_ptr<OperatingPoints> linearSystemOperatingPointPtr_;

  // cost parameters
  matrix_t Q_;
  matrix_t R_;
  matrix_t QFinal_;
  vector_t xFinal_;
  vector_t xNominal_;
  vector_t uNominal_;

  size_t numPartitions_ = 0;
  scalar_array_t partitioningTimes_;
  vector_t initialState_;
};

}  // namespace double_integrator
}  // namespace ocs2
