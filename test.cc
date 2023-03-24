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

start: file;

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
    { "start", { { "file" }, }, },
    {
      "file", {
        {"statements", "TOK1", "$trans_symb1"},
        {"TOK2", "$trans_symb2"},
      },
    },
    {
      "statements", {
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
  .follow = {
    { "start", { "EOF" } },
    { "file", { "EOF" } },
    { "statements", { "TOK1" } },
  },
};

constexpr auto SAMPLE2 = R"(
NUM    [0-9]+
LPAREN    [(]
RPAREN    [)]
PLUS    [+]
ASTERISK    [*]

%%

start: e;

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
    { "start", { { "e" }, }, },
    {
      "e", {
        {"e", "PLUS", "t"},
        {"t"},
      },
    },
    {
      "t", {
        {"t", "ASTERISK", "f"},
        {"f"},
      },
    },
    {
      "f", {
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
  .follow = {
    { "start", { "EOF" } },
    { "e", { "EOF", "PLUS", "RPAREN" } },
    { "t", { "EOF", "PLUS", "ASTERISK", "RPAREN" } },
    { "f", { "EOF", "PLUS", "ASTERISK", "RPAREN" } },
  },
};

constexpr auto SAMPLE3 = R"(
NUM    [0-9]+
LPAREN    [(]
RPAREN    [)]
PLUS    [+]
ASTERISK    [*]

%%

start: e;
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
    { "start", { { "e" }, }, },
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
  .follow = {
    { "start", { "EOF" } },
    { "e", { "EOF", "RPAREN", } },
    { "e_prime", { "EOF", "RPAREN", } },
    { "t", { "EOF", "PLUS", "RPAREN" } },
    { "t_prime", { "EOF", "PLUS", "RPAREN" } },
    { "f", { "EOF", "PLUS", "ASTERISK", "RPAREN" } },
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

start: s;
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
    { "start", { { "s" }, }, },
    {
      "s", {
        {"A", "b", "d", "H"},
      },
    },
    {
      "b", {
        {"C", "c"},
      },
    },
    {
      "c", {
        {"B", "c"},
        {"EPS"},
      },
    },
    {
      "d", {
        {"e", "f"},
      },
    },
    {
      "e", {
        {"G"},
        {"EPS"},
      },
    },
    {
      "f", {
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
  .follow = {
    { "start", {"EOF"} },
    { "s", {"EOF"} },
    { "b", {"G", "F", "H"} },
    { "c", {"G", "F", "H"} },
    { "d", { "H" } },
    { "e", {"F", "H"} },
    { "f", { "H" } },
  },
};

// problem-02
constexpr auto SAMPLE5 = R"(
B    boba
%%
start: s;
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
    { "start", { { "s" }, }, },
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
  .follow = {
    { "start", {"EOF"} },
    { "s", {"EOF"} },
    { "a", {"D", "EOF"} },
    { "b", {"EOF", "D"} },
    { "c", {} },
  },
};

// problem-03
constexpr auto SAMPLE6 = R"(
LPAREN    [(]
RPAREN    [)]
COMMA    ,
A    kek
%%
start: s;
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
    { "start", { { "s" }, }, },
    { "s", { {"LPAREN", "l", "RPAREN"}, { "A" }, }, },
    { "l", { {"s", "l_prime"}, }, },
    { "l_prime", { { "COMMA", "s", "l_prime" }, { "EPS" }, }, },
  },
  .first = {
    { "s", {"LPAREN", "A"} },
    { "l", {"LPAREN", "A"} },
    { "l_prime", {"COMMA", "EPS"} },
  },
  .follow = {
    { "start", {"EOF"} },
    { "s", {"EOF", "COMMA", "RPAREN"} },
    { "l", {"RPAREN"} },
    { "l_prime", {"RPAREN"} },
  },
};

// problem-04
constexpr auto SAMPLE7 = R"(
A    heh
B    42
%%
start: s;
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
    { "start", { { "s" }, }, },
    { "s", { {"a", "A", "a", "B"}, {"b", "B", "b", "A"}, }, },
    { "a", { { "EPS" }, }, },
    { "b", { { "EPS" }, }, },
  },
  .first = {
    { "s", {"A", "B"} },
    { "a", {"EPS"} },
    { "b", {"EPS"} },
  },
  .follow = {
    { "start", {"EOF"} },
    { "s", {"EOF"} },
    { "a", {"A", "B"} },
    { "b", {"A", "B"} },
  },
};

// problem-06
constexpr auto SAMPLE8 = R"(
A    heh
%%
start: s;
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
    { "start", { { "s" }, }, },
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
  .follow = {
    { "start", {"EOF"} },
    { "s", {"EOF"} },
    { "a", {"H", "G", "EOF"} },
    { "b", {"A", "H", "G", "EOF"} },
    { "c", {"G", "B", "H", "EOF"} },
  },
};

struct TParam {
  std::string grammarString;
  TGrammar grammar;
  bool isLL1;
};

struct GrammarTest : testing::TestWithParam<TParam> {};

TEST_P(GrammarTest, BasicChecks) {
  const auto& [text, expectedGrammar, isLL1] = GetParam();

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
  if (!expectedGrammar.follow.empty()) {
    got->CalculateFOLLOW();
    for (const auto& [lhs, lhsFollow] : expectedGrammar.follow) {
      EXPECT_EQ(lhsFollow, got->follow[lhs]);
    }

    EXPECT_EQ(isLL1, got->IsLL1());
  }

}

INSTANTIATE_TEST_SUITE_P(GrammarProcessing, GrammarTest, testing::Values<TParam>(
      TParam{SAMPLE1, GRAMMAR1, false},
      TParam{SAMPLE2, GRAMMAR2, false},
      TParam{SAMPLE3, GRAMMAR3, true},
      TParam{SAMPLE4, GRAMMAR4, true},
      TParam{SAMPLE5, GRAMMAR5, false},
      TParam{SAMPLE6, GRAMMAR6, true},
      TParam{SAMPLE7, GRAMMAR7, true},
      TParam{SAMPLE8, GRAMMAR8, false}
      // TODO: add more examples, I'm too lazy to do it now
));
