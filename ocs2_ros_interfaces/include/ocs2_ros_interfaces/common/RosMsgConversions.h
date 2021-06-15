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

#include <ocs2_core/Types.h>
#include <ocs2_core/cost/CostDesiredTrajectories.h>
#include <ocs2_core/logic/ModeSchedule.h>
#include <ocs2_mpc/SystemObservation.h>
#include <ocs2_oc/oc_solver/PerformanceIndex.h>

// MPC messages
#include <ocs2_msgs/mode_schedule.h>
#include <ocs2_msgs/mpc_observation.h>
#include <ocs2_msgs/mpc_performance_indices.h>
#include <ocs2_msgs/mpc_target_trajectories.h>

namespace ocs2 {
namespace ros_msg_conversions {

/**
 * Creates the observation message.
 *
 * @param [in] observation: The observation structure.
 * @param [out] observationMsg: The observation message.
 */
void createObservationMsg(const SystemObservation& observation, ocs2_msgs::mpc_observation& observationMsg);

/**
 * Reads the observation message.
 *
 * @param [in] observationMsg: The observation message.
 * @param [out] observation: The observation structure.
 */
void readObservationMsg(const ocs2_msgs::mpc_observation& observationMsg, SystemObservation& observation);

/**
 * Creates the mode sequence message.
 *
 * @param [in] modeSchedule: The mode schedule which contains the event times and the mode sequence.
 * @param [out] modeScheduleMsg: The mode schedule message.
 */
void createModeScheduleMsg(const ModeSchedule& modeSchedule, ocs2_msgs::mode_schedule& modeScheduleMsg);

/**
 * Reads the mode sequence message.
 *
 * @param [in] modeScheduleMsg: The mode schedule message.
 * @return The mode schedule which contains the event times and the mode sequence.
 */
ModeSchedule readModeScheduleMsg(const ocs2_msgs::mode_schedule& modeScheduleMsg);

/**
 * Creates the performance indices message.
 *
 * @param [in] initTime: The initial time for which the MPC is computed.
 * @param [in] performanceIndices: The performance indices of the solver.
 * @param [out] performanceIndicesMsg: The performance indices ROS message.
 */
void createPerformanceIndicesMsg(scalar_t initTime, const PerformanceIndex& performanceIndices,
                                 ocs2_msgs::mpc_performance_indices& performanceIndicesMsg);

/**
 * Reads the performance indices message.
 *
 * @param [in] performanceIndicesMsg: The performance indices ROS message.
 * @return The performance indices of the solver.
 */
PerformanceIndex readPerformanceIndicesMsg(const ocs2_msgs::mpc_performance_indices& performanceIndicesMsg);

/**
 * Creates the target trajectories message.
 *
 * @param [in] costDesiredTrajectories: The desired trajectory of the cost.
 * @param [out] targetTrajectoriesMsg: The target trajectories message.
 */
void createTargetTrajectoriesMsg(const CostDesiredTrajectories& costDesiredTrajectories,
                                 ocs2_msgs::mpc_target_trajectories& targetTrajectoriesMsg);

/**
 * Returns the target trajectories message.
 *
 * @param [in] targetTrajectoriesMsg: The target trajectories message.
 * @return: The desired trajectory of the cost.
 */
CostDesiredTrajectories readTargetTrajectoriesMsg(const ocs2_msgs::mpc_target_trajectories& targetTrajectoriesMsg);

}  // namespace ros_msg_conversions
}  // namespace ocs2
