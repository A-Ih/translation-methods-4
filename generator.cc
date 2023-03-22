#include "common.hh"
#include <iostream>

constexpr auto TEMPLATE = R"(
TOK1   [ \n]+
TOK2   [a-zA-Z][a-zA-Z0-9_]*

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
  ParseGrammar(TEMPLATE);
  auto [tokens, statements] = ConstSplit<2>(TEMPLATE, "\n%%");
  std::cout << "Here are the tokens: " << std::endl;
  std::cout << tokens << std::endl;
  std::cout << "Here are the statements: " << std::endl;
  std::cout << statements << std::endl;

}

