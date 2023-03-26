
#include <fstream>

#include "parser.hh"
#include "ast.hh"

struct TVisitor : IVisitor {
  // TODO: write the implementation of visit methods if there are any (they are
  // all abstract
  
};

std::shared_ptr<IVisitor> GetVisitor() {
  // TODO: tweak this function to your liking
  return std::make_shared<TVisitor>();
}

int main(int argc, char** argv) {
  std::shared_ptr<std::istream> source;
  if (argc == 1) {
    // read from stdin
    source = std::shared_ptr<std::istream>(&std::cin, [](auto) {});
    // custom noop deleter for std::cin
  } else {
    assert(argc == 2);
    source = std::make_shared<std::ifstream>(std::string{argv[1]});
  }
  auto lexer = std::make_shared<TLexer>(source);
  auto parser = std::make_shared<TParser>(lexer);

  auto result = parser->Parse();
  TreeToDot(std::cout, result.get());
}
