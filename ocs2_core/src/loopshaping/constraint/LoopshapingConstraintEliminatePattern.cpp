/******************************************************************************
Copyright (c) 2020, Ruben Grandia. All rights reserved.

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

#include <ocs2_core/loopshaping/constraint/LoopshapingConstraintEliminatePattern.h>

namespace ocs2 {

VectorFunctionQuadraticApproximation LoopshapingConstraintEliminatePattern::inequalityConstraintQuadraticApproximation(scalar_t t,
                                                                                                                       const vector_t& x,
                                                                                                                       const vector_t& u) {
  const bool isDiagonal = loopshapingDefinition_->isDiagonal();
  const auto& s_filter = loopshapingDefinition_->getInputFilter();
  const vector_t x_system = loopshapingDefinition_->getSystemState(x);
  const vector_t u_system = loopshapingDefinition_->getSystemInput(x, u);
  const size_t FILTER_STATE_DIM = s_filter.getNumStates();
  const auto h_system = systemConstraint_->inequalityConstraintQuadraticApproximation(t, x_system, u_system);

  VectorFunctionQuadraticApproximation h;
  h.f = h_system.f;

  h.dfdx.resize(h.f.rows(), x.rows());
  h.dfdx.leftCols(x_system.rows()) = h_system.dfdx;

  if (isDiagonal) {
    h.dfdx.rightCols(FILTER_STATE_DIM).noalias() = h_system.dfdu * s_filter.getCdiag();
    h.dfdu.noalias() = h_system.dfdu * s_filter.getDdiag();
  } else {
    h.dfdx.rightCols(FILTER_STATE_DIM).noalias() = h_system.dfdu * s_filter.getC();
    h.dfdu.noalias() = h_system.dfdu * s_filter.getD();
  }

  h.dfdxx.resize(h.f.rows());
  h.dfduu.resize(h.f.rows());
  h.dfdux.resize(h.f.rows());
  matrix_t dfduu_C;  // temporary variable
  for (size_t i = 0; i < h.f.rows(); i++) {
    // dfdxx
    h.dfdxx[i].resize(x.rows(), x.rows());
    h.dfdxx[i].topLeftCorner(x_system.rows(), x_system.rows()) = h_system.dfdxx[i];
    if (isDiagonal) {
      h.dfdxx[i].topRightCorner(x_system.rows(), FILTER_STATE_DIM).noalias() = h_system.dfdux[i].transpose() * s_filter.getCdiag();
      dfduu_C.noalias() = h_system.dfduu[i] * s_filter.getCdiag();
      h.dfdxx[i].bottomRightCorner(FILTER_STATE_DIM, FILTER_STATE_DIM).noalias() = s_filter.getCdiag() * dfduu_C;
    } else {
      h.dfdxx[i].topRightCorner(x_system.rows(), FILTER_STATE_DIM).noalias() = h_system.dfdux[i].transpose() * s_filter.getC();
      dfduu_C.noalias() = h_system.dfduu[i] * s_filter.getC();
      h.dfdxx[i].bottomRightCorner(FILTER_STATE_DIM, FILTER_STATE_DIM).noalias() = s_filter.getC().transpose() * dfduu_C;
    }
    h.dfdxx[i].bottomLeftCorner(FILTER_STATE_DIM, x_system.rows()) =
        h.dfdxx[i].topRightCorner(x_system.rows(), FILTER_STATE_DIM).transpose();

    // dfduu
    if (isDiagonal) {
      h.dfduu[i].noalias() = s_filter.getDdiag() * h_system.dfduu[i] * s_filter.getDdiag();
    } else {
      h.dfduu[i].noalias() = s_filter.getD().transpose() * h_system.dfduu[i] * s_filter.getD();
    }

    // dfdux
    h.dfdux[i].resize(u.rows(), x.rows());
    if (isDiagonal) {
      h.dfdux[i].leftCols(x_system.rows()).noalias() = s_filter.getDdiag() * h_system.dfdux[i];
      h.dfdux[i].rightCols(FILTER_STATE_DIM).noalias() = s_filter.getDdiag() * dfduu_C;
    } else {
      h.dfdux[i].leftCols(x_system.rows()).noalias() = s_filter.getD().transpose() * h_system.dfdux[i];
      h.dfdux[i].rightCols(FILTER_STATE_DIM).noalias() = s_filter.getD().transpose() * dfduu_C;
    }
  }

  return h;
}

vector_t LoopshapingConstraintEliminatePattern::stateInputEqualityConstraint(scalar_t t, const vector_t& x, const vector_t& u) {
  const vector_t x_system = loopshapingDefinition_->getSystemState(x);
  const vector_t u_system = loopshapingDefinition_->getSystemInput(x, u);
  return systemConstraint_->stateInputEqualityConstraint(t, x_system, u_system);
}

VectorFunctionLinearApproximation LoopshapingConstraintEliminatePattern::stateInputEqualityConstraintLinearApproximation(
    scalar_t t, const vector_t& x, const vector_t& u) {
  const bool isDiagonal = loopshapingDefinition_->isDiagonal();
  const auto& s_filter = loopshapingDefinition_->getInputFilter();
  const vector_t x_system = loopshapingDefinition_->getSystemState(x);
  const vector_t u_system = loopshapingDefinition_->getSystemInput(x, u);
  const auto g_system = systemConstraint_->stateInputEqualityConstraintLinearApproximation(t, x_system, u_system);

  VectorFunctionLinearApproximation g;
  g.f = g_system.f;

  // dfdx
  g.dfdx.resize(g_system.f.rows(), x.rows());
  g.dfdx.leftCols(x_system.rows()) = g_system.dfdx;
  if (isDiagonal) {
    g.dfdx.rightCols(s_filter.getNumStates()).noalias() = g_system.dfdu * s_filter.getCdiag();
  } else {
    g.dfdx.rightCols(s_filter.getNumStates()).noalias() = g_system.dfdu * s_filter.getC();
  }

  // dfdu
  g.dfdu.resize(g_system.f.rows(), u.rows());
  if (isDiagonal) {
    g.dfdu.leftCols(s_filter.getNumInputs()).noalias() = g_system.dfdu * s_filter.getDdiag();
  } else {
    g.dfdu.leftCols(s_filter.getNumInputs()).noalias() = g_system.dfdu * s_filter.getD();
  }
  g.dfdu.rightCols(u.rows() - s_filter.getNumInputs()).setZero();

  return g;
}

}  // namespace ocs2
