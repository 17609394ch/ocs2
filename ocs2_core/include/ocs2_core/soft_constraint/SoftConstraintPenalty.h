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

#include <ocs2_core/Types.h>
#include <ocs2_core/soft_constraint/penalties/PenaltyFunctionBase.h>

namespace ocs2 {

/**
 *   A helper class that implements the cost penalty for general constraint
 *   \f$ h_i(x, u) \quad \forall  i \in [1,..,M] \f$
 *
 *   penalty = \f$ \sum_{i=1}^{M} p(h_i(x, u)) \f$
 *
 *   This class uses the chain rule to compute the second-order approximation of the constraint-penalty. In the case that the
 *   second-order approximation of constraint is not provided, it employs a Gauss-Newton approximation technique which only
 *   relies on the first-order approximation.
 */
class SoftConstraintPenalty {
 public:
  /**
   * Constructor
   * @param [in] penaltyFunctionPtrArray: An array of pointers to the penalty function on the constraint.
   */
  SoftConstraintPenalty(std::vector<std::unique_ptr<PenaltyFunctionBase>> penaltyFunctionPtrArray);

  /**
   * Constructor
   * @param [in] numConstraints: The number of constraints.
   * @param [in] penaltyFunction: A pointer to the penalty function on the constraint.
   */
  SoftConstraintPenalty(size_t numConstraints, std::unique_ptr<PenaltyFunctionBase> penaltyFunctionPtr);

  /** Default destructor */
  ~SoftConstraintPenalty() = default;

  /** copy constructor */
  SoftConstraintPenalty(const SoftConstraintPenalty& other);

  /**
   * Get the penalty cost.
   *
   * @param [in] h: Vector of inequality constraint values
   * @return Penalty: The penalty cost.
   */
  scalar_t getValue(const vector_t& h) const;

  /**
   * Get the derivative of the penalty cost.
   * Implements the chain rule between the inequality constraint and penalty function.
   *
   * @param [in] h: The constraint linear approximation.
   * @return The penalty cost quadratic approximation.
   */
  ScalarFunctionQuadraticApproximation getQuadraticApproximation(const VectorFunctionLinearApproximation& h) const;

  /**
   * Get the derivative of the penalty cost.
   * Implements the chain rule between the inequality constraint and penalty function.
   *
   * @param [in] h: The constraint quadratic approximation.
   * @return The penalty cost quadratic approximation.
   */
  ScalarFunctionQuadraticApproximation getQuadraticApproximation(const VectorFunctionQuadraticApproximation& h) const;

 private:
  std::tuple<scalar_t, vector_t, vector_t> getPenaltyValue1stDev2ndDev(const vector_t& h) const;

  std::vector<std::unique_ptr<PenaltyFunctionBase>> penaltyFunctionPtrArray_;
};

}  // namespace ocs2
