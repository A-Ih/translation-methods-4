#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

#include <absl/strings/str_split.h>
#include <absl/strings/str_format.h>

#include "debug.hh"

struct TGrammar {
  std::unordered_map<std::string, std::string> tokenToRegex;
  std::unordered_map<std::string, std::vector<std::vector<std::string>>> rules;

  // nonTerm -> set of tokens that can follow it
  std::unordered_map<std::string, std::unordered_set<std::string>> follow;

  // (term | nonTerm)* -> set of tokens that the arguments can start with
  // the arguments are joined with ' ' (so their representation remains unique)
  std::unordered_map<std::string, std::unordered_set<std::string>> first;
  // lhs -> (FIRST1(rhs), rhs)
  std::unordered_map<std::string, std::pair<std::vector<std::string>, std::unordered_set<std::string>>> first1;

  void CalculateFIRST();
  void CalculateFOLLOW();

// TODO: CalculateFIRST1, CalculateFOLLOW, устранить бесполезные символы (надо
// погуглить как  это делается, в конспекте под определением FIRST содержится
// описание бесполезных символов)
};

template<std::size_t N>
inline std::array<std::string, N> ConstSplit(std::string_view text, std::string_view sep) {
  // see: https://abseil.io/docs/cpp/guides/strings#filtering-predicates:~:text=v%5B2%5D%20%3D%3D%20%225%22-,Filtering%20Predicates,-Predicates%20can%20filter
  // for info on SkipWhitespace and SkipEmpty
  std::vector<std::string> result = absl::StrSplit(text, sep, absl::SkipWhitespace());
  EXPECT(result.size() == N, absl::StrFormat("Expected %d items after split, got %d", N, result.size()));
  std::array<std::string, N> arr;
  for (std::size_t i = 0; i < N; i++) {
    arr[i] = std::move(result[i]);
  }
  return arr;
}

std::shared_ptr<TGrammar> ParseGrammar(const std::string& grammarString);
