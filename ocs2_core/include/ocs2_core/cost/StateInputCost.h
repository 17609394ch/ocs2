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

#include <type_traits>

#include <ocs2_core/Types.h>
#include <ocs2_core/cost/CostDesiredTrajectories.h>

namespace ocs2 {

/** State-input cost term */
class StateInputCost {
 public:
  StateInputCost() = default;
  virtual ~StateInputCost() = default;
  virtual StateInputCost* clone() const = 0;

  /** Set cost term activity */
  void setActivity(bool activity) { active_ = activity; }

  /** Check if cost term is active */
  bool isActive() const { return active_; }

  /** Get cost term value */
  virtual scalar_t getValue(scalar_t time, const vector_t& state, const vector_t& input,
                            const CostDesiredTrajectories& desiredTrajectory) const = 0;

  /** Get cost term quadratic approximation */
  virtual ScalarFunctionQuadraticApproximation getQuadraticApproximation(scalar_t time, const vector_t& state, const vector_t& input,
                                                                         const CostDesiredTrajectories& desiredTrajectory) const = 0;

 protected:
  StateInputCost(const StateInputCost& rhs) = default;

 private:
  bool active_ = true;
};

// Template for conditional compilation using SFINAE
template <typename T>
using EnableIfStateInputCost_t = typename std::enable_if<std::is_same<T, StateInputCost>::value, bool>::type;

}  // namespace ocs2
