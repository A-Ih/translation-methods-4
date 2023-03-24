#pragma once

#include <vector>
#include <string>

#include <absl/debugging/stacktrace.h>
#include <absl/debugging/symbolize.h>

namespace detail {

inline std::string GetStackTrace() {
    constexpr auto SIZE = 10;
    void* result[SIZE];
    int howMuch = absl::GetStackTrace(result, SIZE, 1);
    std::string trace{"\n"};
    char buf[1024];
    for (int i = howMuch - 1; i >= 0; i--) {
      const char* symbol = "(unknown)";
      if (absl::Symbolize(result[i], buf, sizeof(buf))) {
        symbol = buf;
      }
      trace += symbol;
      trace += "\n";
    }
    return trace;
}

}  // namespace detail

// This is made as macro so the message expression won't be computed when not
// needed (which should be almost always the case).
// The calculation of stack trace is placed in an external function so that the
// variables needed for it don't waste space in stack frame of the calling
// function
#define EXPECT(cond, message) \
  if (!(cond)) { \
    throw std::runtime_error(detail::GetStackTrace() + absl::StrFormat(__FILE__ ":%d %s", __LINE__, message)); \
  }
