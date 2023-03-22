#include "common.hh"
#include <iostream>

#include <absl/strings/str_split.h>
#include <absl/strings/str_format.h>
#include <absl/strings/substitute.h>

#include <absl/log/log.h>

constexpr auto TEMPLATE = R"(
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

int main() {
  auto grammar = ParseGrammar(TEMPLATE);
  LOG(INFO) << "Grammar successfully parsed" << std::endl;
  std::cout << "Here are the tokens: " << std::endl;
  for (const auto& [tok, regex] : grammar->tokenToRegex) {
    absl::PrintF("token `%s` is parsed by regex `%s`\n", tok, regex);
  }
  std::cout << "Here are the rules" << std::endl;

  for (const auto& [nterm, group] : grammar->rules) {
    for (const auto& rule : group) {
      absl::PrintF("%s -> %s\n", nterm, absl::StrJoin(rule, " "));
    }
  }
}

