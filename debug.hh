#pragma once

#include <vector>
#include <string>

#include <absl/debugging/stacktrace.h>
#include <absl/debugging/symbolize.h>

// This is made as macro so the message expression won't be computed when not
// needed (which should be almost always the case)
#define EXPECT(cond, message) \
  if (!(cond)) { \
    constexpr auto SIZE = 10; \
    std::vector<void*> result{SIZE}; \
    int howMuch = absl::GetStackTrace(result.data(), SIZE, 0); \
    std::string trace{"\n"}; \
    std::vector<char> buf(1024); \
    for (int i = howMuch - 1; i >= 0; i--) { \
      const char* symbol = "(unknown)"; \
      if (absl::Symbolize(result[i], buf.data(), buf.size())) { \
        symbol = buf.data(); \
      } \
      trace += symbol; \
      trace += "\n"; \
    } \
    throw std::runtime_error(trace + absl::StrFormat(__FILE__ ":%d %s", __LINE__, message)); \
  }
