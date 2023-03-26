
#include <fstream>

#include "parser.hh"
#include "ast.hh"

#include "backward.hpp"


struct TVisitor : IVisitor {
  // TODO: write the implementation of visit methods if there are any (they are
  // all abstract
  using T = int;

  T GetT(const TNode* n) {
    try {
      return std::any_cast<T>(n->value);
    } catch (...) {
      using namespace backward;
      StackTrace st; st.load_here(32);
      Printer p; p.print(st);
      std::cerr << "Caught in TNode* with name " << n->name << std::endl;
      throw;
    }
  }

  T GetT(const std::shared_ptr<TNode>& n) {
    return GetT(n.get());
  }

  bool IsEps(const TNode* n) {
    auto t = dynamic_cast<const TTree*>(n);
    if (t == nullptr) {
      return false;
    }
    return t->children.empty();
  }

  void visit_start(TTree* ctx) override {
    ctx->value = GetT(ctx->children[0]);
  }

  void visit_t_prime_before(TTree* ctx) override {
    ctx->value = GetT(ctx->parent) * GetT(ctx->children[1]);
  }

  void visit_t_prime_after(TTree* ctx) override {
    if (!IsEps(ctx->children[2].get())) {
      ctx->value = GetT(ctx->children[2]);
    }
  }

  void visit_t_before(TTree* ctx) override {
    ctx->value = GetT(ctx->children[0]);
  }

  void visit_t_after(TTree* ctx) override {
    if (!IsEps(ctx->children[1].get())) {
      ctx->value = GetT(ctx->children.at(1));
    }
  }

  void visit_e_prime_plus_before(TTree* ctx) override {
    ctx->value = GetT(ctx->parent) + GetT(ctx->children[1]);
  }

  void visit_e_prime_plus_after(TTree* ctx) override {
    if (!IsEps(ctx->children[2].get())) {
      ctx->value = GetT(ctx->children[2]);
    }
  }

  void visit_e_prime_minus_before(TTree* ctx) override {
    ctx->value = GetT(ctx->parent) - GetT(ctx->children[1]);
  }

  void visit_e_prime_minus_after(TTree* ctx) override {
    if (!IsEps(ctx->children[2].get())) {
      ctx->value = GetT(ctx->children[2]);
    }
  }

  void visit_f_paren(TTree* ctx) override {
    ctx->value = ctx->children[1]->value;
  }

  void visit_f_num(TTree* ctx) override {
    ctx->value = std::stoi(ctx->children[0]->name);
  }

  void visit_e_before(TTree* ctx) override {
    ctx->value = GetT(ctx->children[0]);
  }

  void visit_e_after(TTree* ctx) override {
    if (!IsEps(ctx->children[1].get())) {
      ctx->value = GetT(ctx->children.at(1));
    }
  }
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
  std::cerr << "The answer has value: " << result->value.has_value() << std::endl;
  std::cerr << "The answer has type: " << result->value.type().name() << std::endl;
  std::cerr << "The answer is " << std::any_cast<TVisitor::T>(result->value) << std::endl;
}
