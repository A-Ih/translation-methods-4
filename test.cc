#include <absl/strings/str_split.h>
#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>

#include <range/v3/view/zip.hpp>

#include <gtest/gtest.h>

#include "common.hh"

TEST(GENERATOR_TEST, SANITY_CHECK) {
  EXPECT_EQ(0, 0);
}

constexpr auto SAMPLE1 = R"(
TOK1    [ \n]+
TOK2    [a-zA-Z][a-zA-Z0-9_]*

%%

file:
  statements TOK1 $trans_symb1
  | TOK2 $trans_symb2
;

statements:
  TOK2
  | TOK2 TOK2 $trans_symb3
;
)";

const TGrammar GRAMMAR1{
  .tokenToRegex = {
    {"TOK1", "[ \\n]+"},
    {"TOK2", "[a-zA-Z][a-zA-Z0-9_]*"},
  },
  .rules = {
    {
      "file",
      {
        {"statements", "TOK1", "$trans_symb1"},
        {"TOK2", "$trans_symb2"},
      },
    },
    {
      "statements",
      {
        {"TOK2"},
        {"TOK2", "TOK2", "$trans_symb3"},
      },
    },
  },
  .first = {
    { "file", {"TOK2"} },
    { "statements", {"TOK2"} },
    { "TOK1", {"TOK1"} },
    { "TOK2", {"TOK2"} },
    { "EPS", {"EPS"} },
  },
};

constexpr auto SAMPLE2 = R"(
NUM    [0-9]+
LPAREN    [(]
RPAREN    [)]
PLUS    [+]
ASTERISK    [*]

%%

e: e PLUS t | t;

t: t ASTERISK f | f;

f: LPAREN e RPAREN | NUM;
)";

const TGrammar GRAMMAR2{
  .tokenToRegex = {
    {"NUM", "[0-9]+"},
    {"LPAREN", "[(]"},
    {"RPAREN", "[)]"},
    {"PLUS", "[+]"},
    {"ASTERISK", "[*]"},
  },
  .rules = {
    {
      "e",
      {
        {"e", "PLUS", "t"},
        {"t"},
      },
    },
    {
      "t",
      {
        {"t", "ASTERISK", "f"},
        {"f"},
      },
    },
    {
      "f",
      {
        {"LPAREN", "e", "RPAREN"},
        {"NUM"},
      },
    },
  },
  .first = {
    { "e", {"LPAREN", "NUM"} },
    { "t", {"LPAREN", "NUM"} },
    { "f", {"LPAREN", "NUM"} },
    { "NUM", {"NUM"} },
    { "LPAREN", {"LPAREN"} },
    { "EPS", {"EPS"} },
  },
};

constexpr auto SAMPLE3 = R"(
NUM    [0-9]+
LPAREN    [(]
RPAREN    [)]
PLUS    [+]
ASTERISK    [*]

%%

e: t e_prime;
e_prime: PLUS t e_prime | EPS;
t: f t_prime;
t_prime: ASTERISK f t_prime | EPS;
f: LPAREN e RPAREN | NUM;

)";

const TGrammar GRAMMAR3{
  .tokenToRegex = {
    {"NUM", "[0-9]+"},
    {"LPAREN", "[(]"},
    {"RPAREN", "[)]"},
    {"PLUS", "[+]"},
    {"ASTERISK", "[*]"},
  },
  .rules = {
    {
      "e",
      {
        {"t", "e_prime"},
      },
    },
    {
      "e_prime",
      {
        {"PLUS", "t", "e_prime"},
        {"EPS"},
      },
    },
    { "t",
      {
        {"f", "t_prime"},
      },
    },
    { "t_prime",
      {
        {"ASTERISK", "f", "t_prime"},
        {"EPS"},
      },
    },
    {
      "f",
      {
        {"LPAREN", "e", "RPAREN"},
        {"NUM"},
      },
    },
  },
  .first = {
    { "e", {"LPAREN", "NUM"} },
    { "e_prime", {"EPS", "PLUS"} },
    { "t", {"LPAREN", "NUM"} },
    { "t_prime", {"EPS", "ASTERISK"} },
    { "f", {"LPAREN", "NUM"} },
    { "NUM", {"NUM"} },
    { "LPAREN", {"LPAREN"} },
    { "EPS", {"EPS"} },
  },
};

struct GrammarTest : testing::TestWithParam<std::pair<std::string, TGrammar>> {};

TEST_P(GrammarTest, BasicChecks) {
  const auto& [text, expectedGrammar] = GetParam();

  std::shared_ptr<TGrammar> got;
  EXPECT_NO_THROW(got = ParseGrammar(text));

  EXPECT_EQ(expectedGrammar.tokenToRegex, got->tokenToRegex);
  EXPECT_EQ(expectedGrammar.rules, got->rules);

  if (!expectedGrammar.first.empty()) {
    got->CalculateFIRST();
    for (const auto& [lhs, lhsFirst] : expectedGrammar.first) {
      EXPECT_EQ(lhsFirst, got->first[lhs]);
    }
  }
}

INSTANTIATE_TEST_SUITE_P(GrammarProcessing, GrammarTest, testing::Values(
      std::pair{SAMPLE1, GRAMMAR1},
      std::pair{SAMPLE2, GRAMMAR2},
      std::pair{SAMPLE3, GRAMMAR3}
      // TODO: add more examples, I'm too lazy to do it now
));
