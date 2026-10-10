#include "core/automation_state.hpp"
#include <iostream>
#include <set>
#include <string>
#include <string_view>

int main(int argc, char **argv) {
  // The Go HTTP test consumes names emitted by the compiled C++ serializer.
  const bool dump = argc == 2 && std::string_view(argv[1]) == "--wire-states";
  std::set<std::string> names;
  for (const auto state : owh::AutomationStates) {
    const std::string name = owh::automationStateName(state);
    if (name.empty() || name.find_first_not_of("ABCDEFGHIJKLMNOPQRSTUVWXYZ_") != std::string::npos ||
        !names.insert(name).second) {
      std::cerr << "Invalid or duplicate automation wire state: " << name << '\n';
      return 1;
    }
    if (dump) std::cout << name << '\n';
  }
  // Pin the states omitted by the old server, as a regression and compatibility check.
  for (const auto *name : {"STANDBY", "MATCHMAKING", "WAITING_PAGE", "WAITING_GAME_EXIT", "IDLE", "NEAC_RETRY"}) {
    if (!names.count(name)) return 2;
  }
  return names.empty() ? 3 : 0;
}
