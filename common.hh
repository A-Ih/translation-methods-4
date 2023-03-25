#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <regex>

#include <range/v3/view/any_view.hpp>

#include <absl/strings/str_split.h>
#include <absl/strings/str_format.h>

#include "debug.hh"

static const std::regex TOKEN_REGEX{"[A-Z][A-Z0-9_]*"};
static const std::regex NONTERMINAL_REGEX{"[a-z][a-z0-9_]*"};
static const std::regex TS_REGEX{"\\$[a-z][a-z0-9_]*"};

constexpr auto IS_TOKEN = [] (std::string_view s) { return std::regex_match(s.begin(), s.end(), TOKEN_REGEX); };
constexpr auto IS_NTERM = [] (std::string_view s) { return std::regex_match(s.begin(), s.end(), NONTERMINAL_REGEX); };
constexpr auto IS_TS = [] (std::string_view s) { return std::regex_match(s.begin(), s.end(), TS_REGEX); };

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
  bool IsLL1();

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

std::unordered_set<std::string>& CalculateRecurFIRST(TGrammar& grammar, ranges::any_view<std::string, ranges::category::bidirectional | ranges::category::sized> alpha);
std::shared_ptr<TGrammar> ParseGrammar(const std::string& grammarString);
