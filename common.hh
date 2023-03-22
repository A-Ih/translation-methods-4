#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <absl/strings/str_split.h>
#include <absl/strings/substitute.h>

struct TGrammar {
  std::unordered_map<std::string, std::string> tokenToRegex;
  std::unordered_map<std::string, std::vector<std::vector<std::string>>> rules;

  // nonTerm -> set of tokens that can follow it
  std::unordered_map<std::string, std::unordered_set<std::string>> follow;

  // (term | nonTerm) -> set of tokens that the arguments can start with
  std::unordered_map<std::string, std::unordered_set<std::string>> first;

  // We also have FIRST1 which is calculated for each rule do we'll leave it
  // until generation
};

template<std::size_t N>
inline std::array<std::string, N> ConstSplit(const std::string& text, std::string_view sep) {
  std::vector<std::string> result = absl::StrSplit(text, sep);
  if (result.size() != N) {
    throw std::runtime_error(absl::Substitute("Expected $0 items after split, got $1", N, result.size()));
  }
  std::array<std::string, N> arr;
  for (std::size_t i = 0; i < N; i++) {
    arr[i] = std::move(result[i]);
  }
  return arr;
}

inline std::shared_ptr<TGrammar> ParseGrammar(const std::string& grammarString) {
  auto [tokens, productions] = ConstSplit<2>(grammarString, "\n%%");

  return {};
}
