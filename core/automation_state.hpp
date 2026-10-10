#pragma once

namespace owh {
// Automation state keeps its existing meaning. It is not match confirmation.
enum class AutomationState {
#define OWH_AUTOMATION_STATE(name) name,
#include "server/protocol/automation_states.inc"
#undef OWH_AUTOMATION_STATE
};

inline constexpr AutomationState AutomationStates[] = {
#define OWH_AUTOMATION_STATE(name) AutomationState::name,
#include "server/protocol/automation_states.inc"
#undef OWH_AUTOMATION_STATE
};

inline constexpr const char *automationStateName(AutomationState state) {
  switch (state) {
#define OWH_AUTOMATION_STATE(name) case AutomationState::name: return #name;
#include "server/protocol/automation_states.inc"
#undef OWH_AUTOMATION_STATE
  }
  return "UNKNOWN";
}
} // namespace owh
