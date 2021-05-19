//
// Created by ruben on 18.09.18.
//

#pragma once

#include <memory>

#include <ocs2_core/constraint/StateConstraint.h>
#include <ocs2_core/constraint/StateInputConstraint.h>
#include <ocs2_core/loopshaping/LoopshapingDefinition.h>

namespace ocs2 {
namespace LoopshapingConstraint {

std::unique_ptr<StateConstraint> create(const StateConstraint& systemConstraint,
                                        std::shared_ptr<LoopshapingDefinition> loopshapingDefinition);

std::unique_ptr<StateInputConstraint> create(const StateInputConstraint& systemConstraint,
                                             std::shared_ptr<LoopshapingDefinition> loopshapingDefinition);

}  // namespace LoopshapingConstraint
}  // namespace ocs2
