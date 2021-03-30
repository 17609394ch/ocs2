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

#include <pinocchio/fwd.hpp>

#include <pinocchio/algorithm/kinematics.hpp>

#include <ocs2_mobile_manipulator_example/constraint/SelfCollisionConstraintCppAd.h>

#include <ocs2_robotic_tools/common/RotationTransforms.h>

namespace mobile_manipulator {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
SelfCollisionConstraintCppAd::SelfCollisionConstraintCppAd(ocs2::PinocchioInterface pinocchioInterface,
                                                           const ocs2::PinocchioStateInputMapping<scalar_t>& mapping,
                                                           ocs2::PinocchioGeometryInterface pinocchioGeometryInterface,
                                                           scalar_t minimumDistance, const std::string& modelName,
                                                           const std::string& modelFolder, bool recompileLibraries, bool verbose)
    : pinocchioInterface_(std::move(pinocchioInterface)),
      selfCollision_(pinocchioInterface_, std::move(pinocchioGeometryInterface), minimumDistance, modelName, modelFolder,
                     recompileLibraries, verbose),
      mappingPtr_(mapping.clone()) {}
/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
SelfCollisionConstraintCppAd::SelfCollisionConstraintCppAd(const SelfCollisionConstraintCppAd& rhs)
    : ocs2::StateConstraint(rhs),
      pinocchioInterface_(rhs.pinocchioInterface_),
      pinocchioInterfaceCachePtr_(nullptr),
      selfCollision_(rhs.selfCollision_),
      mappingPtr_(rhs.mappingPtr_->clone()) {}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
size_t SelfCollisionConstraintCppAd::getNumConstraints(scalar_t time) const {
  return selfCollision_.getNumCollisionPairs();
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
vector_t SelfCollisionConstraintCppAd::getValue(scalar_t time, const vector_t& state) const {
  ocs2::PinocchioInterface* pinocchioInterfacePtr = pinocchioInterfaceCachePtr_;
  // use local copy if cache is not set
  if (pinocchioInterfacePtr == nullptr) {
    pinocchioInterfacePtr = const_cast<ocs2::PinocchioInterface*>(&pinocchioInterface_);

    const auto q = mappingPtr_->getPinocchioJointPosition(state);
    const auto& model = pinocchioInterfacePtr->getModel();
    auto& data = pinocchioInterfacePtr->getData();
    pinocchio::forwardKinematics(model, data, q);
  }

  return selfCollision_.getValue(*pinocchioInterfacePtr);
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
VectorFunctionLinearApproximation SelfCollisionConstraintCppAd::getLinearApproximation(scalar_t time, const vector_t& state) const {
  const auto q = mappingPtr_->getPinocchioJointPosition(state);

  ocs2::PinocchioInterface* pinocchioInterfacePtr = pinocchioInterfaceCachePtr_;
  // use local copy if cache is not set
  if (pinocchioInterfacePtr == nullptr) {
    pinocchioInterfacePtr = const_cast<ocs2::PinocchioInterface*>(&pinocchioInterface_);

    const auto& model = pinocchioInterfacePtr->getModel();
    auto& data = pinocchioInterfacePtr->getData();
    pinocchio::forwardKinematics(model, data, q);
  }

  VectorFunctionLinearApproximation constraint;
  matrix_t dfdq, dfdv;
  std::tie(constraint.f, dfdq) = selfCollision_.getLinearApproximation(*pinocchioInterfacePtr, q);
  dfdv.setZero(dfdq.rows(), dfdq.cols());
  std::tie(constraint.dfdx, std::ignore) = mappingPtr_->getOcs2Jacobian(state, dfdq, dfdv);
  return constraint;
}

}  // namespace mobile_manipulator
