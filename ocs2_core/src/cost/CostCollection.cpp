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

#include <ocs2_core/cost/CostCollection.h>

namespace ocs2 {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template <typename COST>
CostCollection<COST>::CostCollection(CostCollection&& rhs) noexcept : costTermMap_(std::move(rhs.costTermMap_)) {}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template <typename COST>
CostCollection<COST>::CostCollection(const CostCollection& rhs) {
  // Loop through all costs by name and clone into the new object
  costTermMap_.clear();
  for (const auto& costPair : rhs.costTermMap_) {
    add(costPair.first, std::unique_ptr<COST>(costPair.second->clone()));
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template <typename COST>
void CostCollection<COST>::add(std::string name, std::unique_ptr<COST> costTerm) {
  auto info = costTermMap_.emplace(std::move(name), std::move(costTerm));
  if (!info.second) {
    throw std::runtime_error("[CostCollection::add] Cost name already exists");
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template <>
template <>
scalar_t CostCollection<StateInputCost>::getValue(scalar_t time, const vector_t& state, const vector_t& input,
                                                  const CostDesiredTrajectories& desiredTrajectory) const {
  scalar_t cost = 0.0;

  // append vectors of cost values from each costTerm
  for (const auto& costPair : costTermMap_) {
    if (costPair.second->isActive()) {
      cost += costPair.second->getValue(time, state, input, desiredTrajectory);
    }
  }

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template <>
template <>
ScalarFunctionQuadraticApproximation CostCollection<StateInputCost>::getQuadraticApproximation(
    scalar_t time, const vector_t& state, const vector_t& input, const CostDesiredTrajectories& desiredTrajectory) const {
  ScalarFunctionQuadraticApproximation cost;
  cost.setZero(state.rows(), input.rows());

  for (const auto& costPair : costTermMap_) {
    if (costPair.second->isActive()) {
      cost += costPair.second->getQuadraticApproximation(time, state, input, desiredTrajectory);
    }
  }

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template <>
template <>
scalar_t CostCollection<StateCost>::getValue(scalar_t time, const vector_t& state, const CostDesiredTrajectories& desiredTrajectory) const {
  scalar_t cost = 0.0;

  // append vectors of cost values from each costTerm
  for (const auto& costPair : costTermMap_) {
    if (costPair.second->isActive()) {
      cost += costPair.second->getValue(time, state, desiredTrajectory);
    }
  }

  return cost;
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
template <>
template <>
ScalarFunctionQuadraticApproximation CostCollection<StateCost>::getQuadraticApproximation(
    scalar_t time, const vector_t& state, const CostDesiredTrajectories& desiredTrajectory) const {
  ScalarFunctionQuadraticApproximation cost;
  cost.f = 0.0;
  cost.dfdx.setZero(state.rows());
  cost.dfdxx.setZero(state.rows(), state.rows());
  // input derivatives are left empty initialized

  for (const auto& costPair : costTermMap_) {
    if (costPair.second->isActive()) {
      const auto costTermApproximation = costPair.second->getQuadraticApproximation(time, state, desiredTrajectory);
      cost.f += costTermApproximation.f;
      cost.dfdx += costTermApproximation.dfdx;
      cost.dfdxx += costTermApproximation.dfdxx;
    }
  }

  return cost;
}

// explicite template instantiation
template class CostCollection<StateCost>;
template class CostCollection<StateInputCost>;

}  // namespace ocs2
