#include "rhine/Parse/Parser.h"
#include "rhine/Parse/ParseTree.h"
#include "rhine/Parse/ParseDriver.h"
#include "rhine/IR/UnresolvedValue.h"
#include "rhine/IR/GlobalValue.h"
#include "rhine/IR/Instruction.h"
#include "rhine/IR/BasicBlock.h"
#include "rhine/IR/Constant.h"
#include "rhine/IR/Value.h"
#include "rhine/IR/Type.h"

#include <vector>

#define K Driver->Ctx

namespace rhine {
std::vector<Argument *> Parser::parseArgumentList(bool Optional,
                                                  bool Parenless) {
  if (!Parenless && !getTok('(')) {
    writeError("expected '(' to begin argument list", Optional);
    return {};
  }
  std::vector<Argument *> ArgumentList;
  while (CurTok != ')') {
    auto ArgLoc = CurLoc;
    auto ArgSema = CurSema;
    if (!getTok(LITERALNAME)) {
      if (Parenless)
        break;
      writeError("expected argument name");
      return {};
    }
    if (auto Ty = parseTypeAnnotation(true)) {
      auto Arg = Argument::get(*ArgSema.LiteralName, Ty);
      Arg->setSourceLocation(ArgLoc);
      ArgumentList.push_back(Arg);
    } else {
      writeError("malformed argument type specified");
      return {};
    }
  }
  if (!Parenless && !getTok(')')) {
    writeError("expected ')' to end function argument list");
    return {};
  }
  return ArgumentList;
}

Function *Parser::parseFcnDecl(bool Optional) {
  if (!getTok(DEF)) {
    writeError("expected 'def', to begin function definition", Optional);
    return nullptr;
  }
  auto FcnLoc = CurLoc;
  auto FcnName = *CurSema.LiteralName;
  if (!getTok(LITERALNAME)) {
    writeError("expected function name");
    return nullptr;
  }
  auto ArgList = parseArgumentList(true);
  auto OptionalTypeAnnLoc = CurLoc;
  auto Fcn = buildFcn(FcnName, ArgList, UnType::get(K), FcnLoc);
  if (auto Block = parseBlock(DOBLOCK, "do", {{ENDBLOCK, "end"}}))
    Fcn->push_back(Block);
  else {
    writeError("unexpected empty block");
    return nullptr;
  }
  return Fcn;
}
}