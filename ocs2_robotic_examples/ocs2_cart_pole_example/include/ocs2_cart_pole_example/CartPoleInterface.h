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

#pragma once

// C++
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>

// OCS2
#include <ocs2_core/Types.h>
#include <ocs2_core/constraint/ConstraintBase.h>
#include <ocs2_core/cost/QuadraticCostFunction.h>
#include <ocs2_core/initialization/OperatingPoints.h>
#include <ocs2_oc/rollout/TimeTriggeredRollout.h>

#include <ocs2_mpc/MPC_DDP.h>
#include <ocs2_robotic_tools/common/RobotInterface.h>

// CartPole
#include "ocs2_cart_pole_example/CartPoleParameters.h"
#include "ocs2_cart_pole_example/definitions.h"
#include "ocs2_cart_pole_example/dynamics/CartPoleSystemDynamics.h"

namespace ocs2 {
namespace cartpole {

class CartPoleInterface final : public RobotInterface {
 public:
  /**
   * Constructor
   *
   * @param [in] taskFileFolderName: The name of the folder containing task file
   */
  explicit CartPoleInterface(const std::string& taskFileFolderName);

  /**
   * Destructor
   */
  ~CartPoleInterface() override = default;

  const vector_t& getInitialState() { return initialState_; }
  const vector_t& getInitialTarget() { return xFinal_; }

  ddp::Settings& ddpSettings() { return ddpSettings_; }

  mpc::Settings& mpcSettings() { return mpcSettings_; }

  std::unique_ptr<MPC_DDP> getMpc();

  const CartPoleSytemDynamics& getDynamics() const override { return *cartPoleSystemDynamicsPtr_; }

  const QuadraticCostFunction& getCost() const override { return *cartPoleCostPtr_; }

  const RolloutBase& getRollout() const { return *ddpCartPoleRolloutPtr_; }

  const OperatingPoints& getOperatingPoints() const override { return *cartPoleOperatingPointPtr_; }

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

  ddp::Settings ddpSettings_;
  mpc::Settings mpcSettings_;

  std::unique_ptr<RolloutBase> ddpCartPoleRolloutPtr_;

  std::unique_ptr<CartPoleSytemDynamics> cartPoleSystemDynamicsPtr_;
  std::unique_ptr<QuadraticCostFunction> cartPoleCostPtr_;
  std::unique_ptr<ConstraintBase> cartPoleConstraintPtr_;
  std::unique_ptr<OperatingPoints> cartPoleOperatingPointPtr_;

  // cost parameters
  matrix_t qm_{STATE_DIM, STATE_DIM};
  matrix_t rm_{INPUT_DIM, INPUT_DIM};
  matrix_t qmFinal_{STATE_DIM, STATE_DIM};

  vector_t initialState_{STATE_DIM};
  vector_t xFinal_{STATE_DIM};
};

}  // namespace cartpole
}  // namespace ocs2
