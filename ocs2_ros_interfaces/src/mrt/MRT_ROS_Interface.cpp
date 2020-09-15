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

#include "ocs2_ros_interfaces/mrt/MRT_ROS_Interface.h"

#include <ocs2_core/control/FeedforwardController.h>
#include <ocs2_core/control/LinearController.h>

namespace ocs2 {

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
MRT_ROS_Interface::MRT_ROS_Interface(std::string robotName /*= "robot"*/,
                                     ros::TransportHints mrtTransportHints /* = ::ros::TransportHints().tcpNoDelay()*/)
    : MRT_BASE(), robotName_(std::move(robotName)), mrtTransportHints_(mrtTransportHints) {
// Start thread for publishing
#ifdef PUBLISH_THREAD
  // Close old thread if it is already running
  shutdownPublisher();
  terminateThread_ = false;
  readyToPublish_ = false;
  publisherWorker_ = std::thread(&MRT_ROS_Interface::publisherWorkerThread, this);
#endif
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
MRT_ROS_Interface::~MRT_ROS_Interface() {
  shutdownNodes();
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::resetMpcNode(const CostDesiredTrajectories& initCostDesiredTrajectories) {
  this->policyReceivedEver_ = false;

  ocs2_msgs::reset resetSrv;
  resetSrv.request.reset = static_cast<uint8_t>(true);

  ros_msg_conversions::createTargetTrajectoriesMsg(initCostDesiredTrajectories, resetSrv.request.targetTrajectories);

  while (!mpcResetServiceClient_.waitForExistence(ros::Duration(5.0)) && ::ros::ok() && ::ros::master::check()) {
    ROS_ERROR_STREAM("Failed to call service to reset MPC, retrying...");
  }

  mpcResetServiceClient_.call(resetSrv);
  ROS_INFO_STREAM("MPC node is reset.");
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::setCurrentObservation(const SystemObservation& currentObservation) {
#ifdef PUBLISH_THREAD
  std::unique_lock<std::mutex> lk(publisherMutex_);
#endif

  // create the message
  ros_msg_conversions::createObservationMsg(currentObservation, mpcObservationMsg_);

  // publish the current observation
#ifdef PUBLISH_THREAD
  readyToPublish_ = true;
  lk.unlock();
  msgReady_.notify_one();
#else
  mpcObservationPublisher_.publish(mpcObservationMsg_);
#endif
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::publisherWorkerThread() {
  while (!terminateThread_) {
    std::unique_lock<std::mutex> lk(publisherMutex_);

    msgReady_.wait(lk, [&] { return (readyToPublish_ || terminateThread_); });

    if (terminateThread_) {
      break;
    }

    mpcObservationMsgBuffer_ = std::move(mpcObservationMsg_);

    readyToPublish_ = false;

    lk.unlock();
    msgReady_.notify_one();

    mpcObservationPublisher_.publish(mpcObservationMsgBuffer_);
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::readPolicyMsg(const ocs2_msgs::mpc_flattened_controller& msg, PrimalSolution& primalSolution,
                                      CommandData& commandData) {
  auto& timeBuffer = primalSolution.timeTrajectory_;
  auto& stateBuffer = primalSolution.stateTrajectory_;
  auto& inputBuffer = primalSolution.inputTrajectory_;
  auto& controlBuffer = primalSolution.controllerPtr_;

  ros_msg_conversions::readObservationMsg(msg.initObservation, commandData.mpcInitObservation_);
  ros_msg_conversions::readTargetTrajectoriesMsg(msg.planTargetTrajectories, commandData.mpcCostDesiredTrajectories_);
  primalSolution.modeSchedule_ = ros_msg_conversions::readModeScheduleMsg(msg.modeSchedule);

  const size_t N = msg.timeTrajectory.size();
  size_array_t stateDim(N);
  size_array_t inputDim(N);

  if (N == 0) {
    throw std::runtime_error("MRT_ROS_Interface::readPolicyMsg: Controller must not be empty");
  } else if (N != msg.stateTrajectory.size() && N != msg.inputTrajectory.size()) {
    throw std::runtime_error("MRT_ROS_Interface::readPolicyMsg: Controller must have same size");
  }

  timeBuffer.clear();
  timeBuffer.reserve(N);
  stateBuffer.clear();
  stateBuffer.reserve(N);
  inputBuffer.clear();
  inputBuffer.reserve(N);

  for (size_t i = 0; i < N; i++) {
    timeBuffer.emplace_back(msg.timeTrajectory[i]);
    stateDim[i] = msg.stateTrajectory[i].value.size();
    stateBuffer.emplace_back(Eigen::Map<const Eigen::VectorXf>(msg.stateTrajectory[i].value.data(), stateDim[i]).cast<scalar_t>());
    inputDim[i] = msg.inputTrajectory[i].value.size();
    inputBuffer.emplace_back(Eigen::Map<const Eigen::VectorXf>(msg.inputTrajectory[i].value.data(), inputDim[i]).cast<scalar_t>());
  }

  // check data size
  if (msg.data.size() != N) {
    throw std::runtime_error("MRT_ROS_Interface::readPolicyMsg: Data has the wrong length");
  }

  std::vector<std::vector<float> const*> controllerDataPtrArray(N, nullptr);
  for (int i = 0; i < N; i++) {
    controllerDataPtrArray[i] = &(msg.data[i].data);
  }

  // instantiate the correct controller
  switch (msg.controllerType) {
    case ocs2_msgs::mpc_flattened_controller::CONTROLLER_FEEDFORWARD: {
      auto controller = FeedforwardController::unFlatten(timeBuffer, controllerDataPtrArray);
      controlBuffer.reset(new FeedforwardController(std::move(controller)));
      break;
    }
    case ocs2_msgs::mpc_flattened_controller::CONTROLLER_LINEAR: {
      auto controller = LinearController::unFlatten(stateDim, inputDim, timeBuffer, controllerDataPtrArray);
      controlBuffer.reset(new LinearController(std::move(controller)));
      break;
    }
    default:
      throw std::runtime_error("MRT_ROS_Interface::readPolicyMsg: Unknown controllerType");
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::mpcPolicyCallback(const ocs2_msgs::mpc_flattened_controller::ConstPtr& msg) {
  const auto policyUpdated = static_cast<bool>(msg->controllerIsUpdated);

  if (policyUpdated) {
    // Read new policy from msg
    auto newSolution = std::unique_ptr<PrimalSolution>(new PrimalSolution);
    auto newCommand = std::unique_ptr<CommandData>(new CommandData);
    readPolicyMsg(*msg, *newSolution, *newCommand);

    // allow user to modify the buffer
    this->modifyBufferedSolution(*newCommand, *newSolution);

    // Fill the buffer under the mutex
    std::lock_guard<std::mutex> lk(this->policyBufferMutex_);
    this->primalSolutionBuffer_ = std::move(newSolution);
    this->commandBuffer_ = std::move(newCommand);

    const scalar_t partitionInitMargin = 1e-1;  //! @badcode Is this necessary?
    this->partitioningTimesUpdate(this->commandBuffer_->mpcInitObservation_.time - partitionInitMargin, this->partitioningTimesBuffer_);

    this->policyUpdatedBuffer_ = true;
    this->newPolicyInBuffer_ = true;

    if (!this->policyReceivedEver_) {
      this->policyReceivedEver_ = true;
      this->initPlanObservation_ = this->commandBuffer_->mpcInitObservation_;
      this->initCall(this->initPlanObservation_);
    }
  } else {
    std::lock_guard<std::mutex> lk(this->policyBufferMutex_);
    this->primalSolutionBuffer_ = std::unique_ptr<PrimalSolution>(new PrimalSolution);
    this->commandBuffer_ = std::unique_ptr<CommandData>(new CommandData);
    this->policyUpdatedBuffer_ = true;
    this->newPolicyInBuffer_ = true;
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::shutdownNodes() {
#ifdef PUBLISH_THREAD
  ROS_INFO_STREAM("Shutting down workers ...");

  shutdownPublisher();

  ROS_INFO_STREAM("All workers are shut down.");
#endif

  // clean up callback queue
  mrtCallbackQueue_.clear();
  mpcPolicySubscriber_.shutdown();

  // shutdown publishers
  mpcObservationPublisher_.shutdown();
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::shutdownPublisher() {
  std::unique_lock<std::mutex> lk(publisherMutex_);
  terminateThread_ = true;
  lk.unlock();

  msgReady_.notify_all();

  if (publisherWorker_.joinable()) {
    publisherWorker_.join();
  }
}

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::spinMRT() {
  mrtCallbackQueue_.callOne();
};

/******************************************************************************************************/
/******************************************************************************************************/
/******************************************************************************************************/
void MRT_ROS_Interface::launchNodes(ros::NodeHandle& nodeHandle) {
  this->reset();

  // display
  ROS_INFO_STREAM("MRT node is setting up ...");

  // Observation publisher
  mpcObservationPublisher_ = nodeHandle.advertise<ocs2_msgs::mpc_observation>(robotName_ + "_mpc_observation", 1);

  // SLQ-MPC subscriber
  auto ops = ros::SubscribeOptions::create<ocs2_msgs::mpc_flattened_controller>(
      robotName_ + "_mpc_policy",                                                         // topic name
      1,                                                                                  // queue length
      boost::bind(&MRT_ROS_Interface::mpcPolicyCallback, this, boost::placeholders::_1),  // callback
      ros::VoidConstPtr(),                                                                // tracked object
      &mrtCallbackQueue_                                                                  // pointer to callback queue object
  );
  ops.transport_hints = mrtTransportHints_;
  mpcPolicySubscriber_ = nodeHandle.subscribe(ops);

  // MPC reset service client
  mpcResetServiceClient_ = nodeHandle.serviceClient<ocs2_msgs::reset>(robotName_ + "_mpc_reset");

  // display
#ifdef PUBLISH_THREAD
  ROS_INFO_STREAM("Publishing MRT messages on a separate thread.");
#endif

  ROS_INFO_STREAM("MRT node is ready.");

  spinMRT();
}

}  // namespace ocs2
