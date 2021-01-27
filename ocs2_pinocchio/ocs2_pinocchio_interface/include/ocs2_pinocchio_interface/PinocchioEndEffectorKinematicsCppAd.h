/******************************************************************************
Copyright (c) 2021, Farbod Farshidian. All rights reserved.

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

#include <string>
#include <vector>

#include <ocs2_pinocchio_interface/EndEffectorKinematics.h>
#include <ocs2_pinocchio_interface/PinocchioInterface.h>
#include <ocs2_pinocchio_interface/PinocchioStateInputMapping.h>

#include <ocs2_core/automatic_differentiation/CppAdInterface.h>

namespace ocs2 {

class PinocchioEndEffectorKinematicsCppAd final : public EndEffectorKinematics<scalar_t> {
 public:
  using vector3_t = Eigen::Matrix<scalar_t, 3, 1>;
  using matrix3x_t = Eigen::Matrix<scalar_t, 3, Eigen::Dynamic>;
  using vector_t = Eigen::Matrix<scalar_t, Eigen::Dynamic, 1>;
  using quaternion_t = Eigen::Quaternion<scalar_t>;

  PinocchioEndEffectorKinematicsCppAd(PinocchioInterface pinocchioInterface, const PinocchioStateInputMapping<ad_scalar_t>& mapping,
                                      std::vector<std::string> endEffectorIds);

  ~PinocchioEndEffectorKinematicsCppAd() override = default;
  PinocchioEndEffectorKinematicsCppAd* clone() const override;
  PinocchioEndEffectorKinematicsCppAd& operator=(const PinocchioEndEffectorKinematicsCppAd&) = delete;

  void initialize(size_t stateDim, const std::string& modelName, const std::string& modelFolder, bool recompileLibraries = true,
                  bool verbose = false);

  const std::vector<std::string>& getIds() const override;

  std::vector<vector3_t> getPositions(const vector_t& state) override;
  std::vector<std::pair<vector3_t, quaternion_t>> getPoses(const vector_t& state) override {
    throw std::runtime_error("[PinocchioEndEffectorKinematicsCppAd] getPoseArray() not implemented");
  }
  std::vector<vector3_t> getVelocities(const vector_t& state, const vector_t& input) override {
    throw std::runtime_error("[PinocchioEndEffectorKinematicsCppAd] getVelocitiyArray() not implemented");
  }

  std::vector<VectorFunctionLinearApproximation> getPositionsLinearApproximation(const vector_t& state) override;

 private:
  PinocchioEndEffectorKinematicsCppAd(const PinocchioEndEffectorKinematicsCppAd& rhs);

  ad_vector_t getPositionsCppAd(const ad_vector_t& state);

  std::unique_ptr<CppAdInterface> positionCppAdInterfacePtr_;

  PinocchioInterface pinocchioInterface_;
  std::unique_ptr<PinocchioStateInputMapping<ad_scalar_t>> mappingPtr_;
  const std::vector<std::string> endEffectorIds_;
  std::vector<size_t> endEffectorFrameIds_;
};

}  // namespace ocs2
