#include "gtest/gtest.h"
#include <absl/strings/str_split.h>
#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>

#include <gtest/gtest.h>

#include "common.hh"

TEST(GENERATOR_TEST, SANITY_CHECK) {
  EXPECT_EQ(0, 0);
}

constexpr auto TEMPLATE1 = R"(
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
};

struct GrammarParserTest : testing::TestWithParam<std::pair<std::string, TGrammar>> {};

TEST_P(GrammarParserTest, BasicChecks) {
  const auto& [text, expectedGrammar] = GetParam();

  std::shared_ptr<TGrammar> got;
  EXPECT_NO_THROW(got = ParseGrammar(text));

  EXPECT_EQ(expectedGrammar.tokenToRegex, got->tokenToRegex);
  EXPECT_EQ(expectedGrammar.rules, got->rules);
}

INSTANTIATE_TEST_SUITE_P(GrammarProcessing, GrammarParserTest, testing::Values(
      std::pair{TEMPLATE1, GRAMMAR1}
      // TODO: add more examples, I'm too lazy to do it now
));
