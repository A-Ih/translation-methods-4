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

/*******************************************************************************
*                          Grammars from gatevidalay                          *
*******************************************************************************/
// Taken from here: https://www.gatevidyalay.com/first-and-follow-compiler-design/

// problem-01
constexpr auto SAMPLE4 = R"(
A    haha

%%

s: A b d H;
b: C c;
c: B c | EPS;
d: e f;
e: G | EPS;
f: F | EPS;
)";

const TGrammar GRAMMAR4 = {
  .tokenToRegex = {
    { "A", "haha" },
  },
  .rules = {
    {
      "s",
      {
        {"A", "b", "d", "H"},
      },
    },
    {
      "b",
      {
        {"C", "c"},
      },
    },
    {
      "c",
      {
        {"B", "c"},
        {"EPS"},
      },
    },
    {
      "d",
      {
        {"e", "f"},
      },
    },
    {
      "e",
      {
        {"G"},
        {"EPS"},
      },
    },
    {
      "f",
      {
        {"F"},
        {"EPS"},
      },
    },
  },
  .first = {
    { "s", {"A"} },
    { "b", {"C"} },
    { "c", {"B", "EPS"} },
    { "d", {"G", "F", "EPS"} },
    { "e", {"G", "EPS"} },
    { "f", {"F", "EPS"} },
  },
};

// problem-02
constexpr auto SAMPLE5 = R"(
B    boba
%%
s: a;
a: A b | a D;
b: B;
c: G;
)";

const TGrammar GRAMMAR5 = {
  .tokenToRegex = {
    { "B", "boba" },
  },
  .rules = {
    { "s", { {"a"}, }, },
    { "a", { {"A", "b"}, {"a", "D"}, }, },
    { "b", { { "B" }, }, },
    { "c", { { "G" }, }, },
  },
  .first = {
    { "s", {"A"} },
    { "a", {"A"} },
    { "b", {"B"} },
    { "c", {"G"} },
  },
};

// problem-03
constexpr auto SAMPLE6 = R"(
LPAREN    [(]
RPAREN    [)]
COMMA    ,
A    kek
%%
s: LPAREN l RPAREN | A;
l: s l_prime;
l_prime: COMMA s l_prime | EPS;
)";

const TGrammar GRAMMAR6 = {
  .tokenToRegex = {
    { "LPAREN", "[(]" },
    { "RPAREN", "[)]" },
    { "COMMA", "," },
    { "A", "kek" },
  },
  .rules = {
    { "s", { {"LPAREN", "l", "RPAREN"}, { "A" }, }, },
    { "l", { {"s", "l_prime"}, }, },
    { "l_prime", { { "COMMA", "s", "l_prime" }, { "EPS" }, }, },
  },
  .first = {
    { "s", {"LPAREN", "A"} },
    { "l", {"LPAREN", "A"} },
    { "l_prime", {"COMMA", "EPS"} },
  },
};

// problem-04
constexpr auto SAMPLE7 = R"(
A    heh
B    42
%%
s: a A a B | b B b A;
a: EPS;
b: EPS;
)";

const TGrammar GRAMMAR7 = {
  .tokenToRegex = {
    { "A", "heh" },
    { "B", "42" },
  },
  .rules = {
    { "s", { {"a", "A", "a", "B"}, {"b", "B", "b", "A"}, }, },
    { "a", { { "EPS" }, }, },
    { "b", { { "EPS" }, }, },
  },
  .first = {
    { "s", {"A", "B"} },
    { "a", {"EPS"} },
    { "b", {"EPS"} },
  },
};

// problem-06
constexpr auto SAMPLE8 = R"(
A    heh
%%
s: a c b | c B b | b A;
a: D A | b c;
b: G | EPS;
c: H | EPS;
)";

const TGrammar GRAMMAR8 = {
  .tokenToRegex = {
    { "A", "heh" },
  },
  .rules = {
    { "s", { {"a", "c", "b"}, {"c", "B", "b"}, {"b", "A"}, }, },
    { "a", { {"D", "A"}, {"b", "c" }, }, },
    { "b", { { "G" }, { "EPS" }, }, },
    { "c", { { "H" }, { "EPS" }, }, },
  },
  .first = {
    { "s", {"D", "G", "H", "EPS", "B", "A"} },
    { "a", {"D", "G", "H", "EPS"} },
    { "b", {"G", "EPS"} },
    { "c", {"H", "EPS"} },
  },
};

struct GrammarTest : testing::TestWithParam<std::pair<std::string, TGrammar>> {};

TEST_P(GrammarTest, BasicChecks) {
  const auto& [text, expectedGrammar] = GetParam();

  std::shared_ptr<TGrammar> got;
  ASSERT_NO_THROW(got = ParseGrammar(text));

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
      std::pair{SAMPLE3, GRAMMAR3},
      std::pair{SAMPLE4, GRAMMAR4},
      std::pair{SAMPLE5, GRAMMAR5},
      std::pair{SAMPLE6, GRAMMAR6},
      std::pair{SAMPLE7, GRAMMAR7},
      std::pair{SAMPLE8, GRAMMAR8}
      // TODO: add more examples, I'm too lazy to do it now
));
