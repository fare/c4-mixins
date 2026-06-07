#pragma once

#include <c4/mixins.hpp>

#include <string_view>
#include <vector>

// Test-only fixture for asserting cooperative method order.
// This deliberately lives in tests/, not examples/.
struct name_trace_base : c4::mixin {
  void trace_names(std::vector<std::string_view>&) const {}
};
