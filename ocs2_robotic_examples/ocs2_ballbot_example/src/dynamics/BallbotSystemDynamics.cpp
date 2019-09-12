

#include <ocs2_ballbot_example/dynamics/BallbotSystemDynamics.h>

// robcogen
#include <ocs2_robotic_tools/rbd_libraries/robcogen/iit/rbd/rbd.h>
#include <ocs2_robotic_tools/rbd_libraries/robcogen/iit/rbd/traits/TraitSelector.h>

#include "ocs2_ballbot_example/generated/forward_dynamics.h"
#include "ocs2_ballbot_example/generated/inertia_properties.h"
#include "ocs2_ballbot_example/generated/inverse_dynamics.h"
#include "ocs2_ballbot_example/generated/jsim.h"
#include "ocs2_ballbot_example/generated/transforms.h"

namespace ocs2 {
namespace ballbot {

void BallbotSystemDynamics::systemFlowMap(ad_scalar_t time, const ad_dynamic_vector_t& state, const ad_dynamic_vector_t& input,
                                          ad_dynamic_vector_t& stateDerivative) const {
  // compute actuationMatrix S_transposed which appears in the equations: M(q)\dot v + h = S^(transpose)\tau
  Eigen::Matrix<ad_scalar_t, 5, 3> S_transposed = Eigen::Matrix<ad_scalar_t, 5, 3>::Zero();

  const ad_scalar_t cyaw = cos(state(2));
  const ad_scalar_t cpitch = cos(state(3));
  const ad_scalar_t croll = cos(state(4));

  const ad_scalar_t syaw = sin(state(2));
  const ad_scalar_t spitch = sin(state(3));
  const ad_scalar_t sroll = sin(state(4));

  const ad_scalar_t sqrt_2 = ad_scalar_t(sqrt(2.0));
  const ad_scalar_t sqrt_3 = ad_scalar_t(sqrt(3.0));

  const ad_scalar_t c1 = (sqrt_2 * ballRadius_) / (4.0 * wheelRadius_);
  const ad_scalar_t c3 = (sqrt_2 * cpitch * syaw);
  const ad_scalar_t c4 = croll * syaw - cyaw * spitch * sroll;
  const ad_scalar_t c5 = sroll * syaw + croll * cyaw * spitch;
  const ad_scalar_t c6 = sqrt_2 * cpitch * cyaw;

  S_transposed(0, 0) =
      -(sqrt_2 * (cyaw * sroll - croll * spitch * syaw)) / (2 * wheelRadius_) - (sqrt_2 * cpitch * syaw) / (2 * wheelRadius_);
  S_transposed(0, 1) = (sqrt_2 * cpitch * syaw) / (4 * wheelRadius_) -
                       (sqrt_2 * (cyaw * sroll - croll * spitch * syaw)) / (2 * wheelRadius_) -
                       (sqrt_2 * sqrt_3 * (croll * cyaw + spitch * sroll * syaw)) / (4 * wheelRadius_);
  S_transposed(0, 2) = (sqrt_2 * cpitch * syaw) / (4 * wheelRadius_) -
                       (sqrt_2 * (cyaw * sroll - croll * spitch * syaw)) / (2 * wheelRadius_) +
                       (sqrt_2 * sqrt_3 * (croll * cyaw + spitch * sroll * syaw)) / (4 * wheelRadius_);

  S_transposed(1, 0) = (c6) / (2.0 * wheelRadius_) - (sqrt_2 * (c5)) / (2.0 * wheelRadius_);
  S_transposed(1, 1) =
      -(sqrt_2 * (c5)) / (2.0 * wheelRadius_) - (c6) / (4.0 * wheelRadius_) - (sqrt_2 * sqrt_3 * (c4)) / (4.0 * wheelRadius_);
  S_transposed(1, 2) =
      (sqrt_2 * sqrt_3 * (c4)) / (4.0 * wheelRadius_) - (c6) / (4.0 * wheelRadius_) - (sqrt_2 * (c5)) / (2.0 * wheelRadius_);

  S_transposed(2, 0) = -(sqrt_2 * ballRadius_ * (spitch + cpitch * croll)) / (2 * wheelRadius_);
  S_transposed(2, 1) = (sqrt_2 * ballRadius_ * (spitch - 2 * cpitch * croll + sqrt_3 * cpitch * sroll)) / (4 * wheelRadius_);
  S_transposed(2, 2) = -(sqrt_2 * ballRadius_ * (2 * cpitch * croll - spitch + sqrt_3 * cpitch * sroll)) / (4 * wheelRadius_);

  S_transposed(3, 0) = 2.0 * c1 * sroll;
  S_transposed(3, 1) = c1 * (2.0 * sroll + sqrt_3 * croll);
  S_transposed(3, 2) = c1 * (2.0 * sroll - sqrt_3 * croll);

  S_transposed(4, 0) = 2.0 * c1;
  S_transposed(4, 1) = -c1;
  S_transposed(4, 2) = -c1;

  // test for the autogenerated code
  iit::Ballbot::tpl::JointState<ad_scalar_t> qdd;
  iit::Ballbot::tpl::JointState<ad_scalar_t> new_input = S_transposed * input;

  using trait_t = typename iit::rbd::tpl::TraitSelector<ad_scalar_t>::Trait;
  iit::Ballbot::dyn::tpl::InertiaProperties<trait_t> inertias;
  iit::Ballbot::tpl::MotionTransforms<trait_t> transforms;
  iit::Ballbot::dyn::tpl::ForwardDynamics<trait_t> forward_dyn(inertias, transforms);
  forward_dyn.fd(qdd, state.template head<5>(), state.template tail<5>(), new_input);

  // dxdt
  stateDerivative.template head<5>() = state.template tail<5>();
  stateDerivative.template tail<5>() = qdd;
}

}  // namespace ballbot
}  // namespace ocs2