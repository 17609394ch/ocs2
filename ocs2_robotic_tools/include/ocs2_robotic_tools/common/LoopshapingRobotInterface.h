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

#include <typeinfo>

#include <ocs2_core/loopshaping/Loopshaping.h>

#include <ocs2_oc/synchronized_module/LoopshapingModeScheduleManager.h>

#include "ocs2_robotic_tools/common/RobotInterface.h"

namespace ocs2 {

class LoopshapingRobotInterface : public RobotInterface {
 public:
  /**
   * Constructor.
   * @param [in] robotInterfacePtr: A unique pointer to the original robot interface.
   * @param [in] loopshapingDefinitionPtr: A shared pointer to the loopshaping definition.
   * @param [in] preComputationPtr: A pointer to the optional pre-computation module.
   */
  LoopshapingRobotInterface(std::unique_ptr<RobotInterface> robotInterfacePtr,
                            std::shared_ptr<LoopshapingDefinition> loopshapingDefinitionPtr,
                            const PreComputation* preComputationPtr /* = nullptr*/);

  /** Destructor */
  ~LoopshapingRobotInterface() override = default;

  /**
   * @brief getModeScheduleManagerPtr
   * @return a shared pointer to the loopshaping definition.
   */
  std::shared_ptr<LoopshapingDefinition> getLoopshapingDefinition() const { return loopshapingDefinitionPtr_; };

  /**
   * @brief Gets a const reference to the underlying robot interface.
   * @tparam Derived: Derived class type (default: RobotInterface)
   * @return a const reference to the underlying robot interface.
   */
  template <class Derived = RobotInterface>
  const Derived& get() const {
    static_assert(std::is_base_of<RobotInterface, Derived>::value, "Template argument must derive from RobotInterface");
    const auto* p = dynamic_cast<Derived*>(robotInterfacePtr_.get());
    if (p == nullptr) {
      throw std::runtime_error("Loopshaping does not wrap a RobotInterface of type " + std::string(typeid(Derived).name()));
    }
    return *p;
  }

  std::shared_ptr<ModeScheduleManager> getModeScheduleManagerPtr() const override { return loopshapingModeScheduleManager_; }

  const LoopshapingDynamics& getDynamics() const override { return *dynamicsPtr_; }

  const LoopshapingCost& getCost() const override { return *costFunctionPtr_; }

  const LoopshapingConstraint* getConstraintPtr() const override { return constraintsPtr_.get(); }

  const LoopshapingOperatingPoint& getOperatingPoints() const override { return *operatingPointsPtr_; }

 private:
  std::unique_ptr<RobotInterface> robotInterfacePtr_;
  std::shared_ptr<LoopshapingDefinition> loopshapingDefinitionPtr_;
  std::shared_ptr<LoopshapingPreComputation> loopshapingPreComputationPtr_;

  std::unique_ptr<LoopshapingDynamics> dynamicsPtr_;
  std::unique_ptr<LoopshapingCost> costFunctionPtr_;
  std::unique_ptr<LoopshapingOperatingPoint> operatingPointsPtr_;
  std::unique_ptr<LoopshapingConstraint> constraintsPtr_;
  std::shared_ptr<LoopshapingModeScheduleManager> loopshapingModeScheduleManager_;
};

}  // namespace ocs2
