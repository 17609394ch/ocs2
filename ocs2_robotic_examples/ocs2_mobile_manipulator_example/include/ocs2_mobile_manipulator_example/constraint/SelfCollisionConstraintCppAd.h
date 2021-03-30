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

#include <memory>

#include <ocs2_core/constraint/StateConstraint.h>
#include <ocs2_pinocchio_interface/PinocchioStateInputMapping.h>
#include <ocs2_self_collision/SelfCollisionCppAd.h>

#include <ocs2_mobile_manipulator_example/definitions.h>

namespace ocs2 {
namespace mobile_manipulator {

class SelfCollisionConstraintCppAd final : public StateConstraint {
 public:
  SelfCollisionConstraintCppAd(PinocchioInterface pinocchioInterface, const PinocchioStateInputMapping<scalar_t>& mapping,
                               PinocchioGeometryInterface pinocchioGeometryInterface, scalar_t minimumDistance,
                               const std::string& modelName, const std::string& modelFolder = "/tmp/ocs2", bool recompileLibraries = true,
                               bool verbose = true);
  ~SelfCollisionConstraintCppAd() override = default;
  SelfCollisionConstraintCppAd* clone() const override { return new SelfCollisionConstraintCppAd(*this); }

  size_t getNumConstraints(scalar_t time) const override;

  /** Get the self collision distance values
   * @note Requires pinocchio::forwardKinematics().
   */
  vector_t getValue(scalar_t time, const vector_t& state) const override;

  /** Get the self collision distance approximation
   * @note Requires pinocchio::forwardKinematics().
   */
  VectorFunctionLinearApproximation getLinearApproximation(scalar_t time, const vector_t& state) const override;

  /** Caches the pointer to the pinocchio interface with pre-computed kinematics. (optional) */
  void setPinocchioInterface(PinocchioInterface& pinocchioInterface) { pinocchioInterfaceCachePtr_ = &pinocchioInterface; }

 private:
  SelfCollisionConstraintCppAd(const SelfCollisionConstraintCppAd& rhs);

  PinocchioInterface pinocchioInterface_;
  PinocchioInterface* pinocchioInterfaceCachePtr_ = nullptr;
  SelfCollisionCppAd selfCollision_;
  std::unique_ptr<PinocchioStateInputMapping<scalar_t>> mappingPtr_;
};

}  // namespace mobile_manipulator
}  // namespace ocs2
