#include "rhine/IR/BasicBlock.h"
#include "rhine/IR/Constant.h"
#include "rhine/IR/Instruction.h"
#include "rhine/IR/Type.h"
#include "rhine/Parse/ParseDriver.h"
#include "rhine/Parse/Parser.h"

#include <algorithm>
#include <vector>

#define K Driver->Ctx

namespace rhine {
bool Parser::matchesAnyTokenPair(std::map<int, std::string> &TokenPairs) {
  for (auto TokPair : TokenPairs)
    if (CurTok == TokPair.first)
      return true;
  return false;
}

void extractInstructionsFromStmt(Instruction *Top,
                                 std::vector<Value *> &Accumulate) {
  Accumulate.push_back(Top);
  for (auto Op : Top->operands()) {
    Value *Val = Op;
    if (auto Sub = dyn_cast<Instruction>(Val))
      extractInstructionsFromStmt(Sub, Accumulate);
  }
}

BasicBlock *Parser::parseBlock(int StartToken, std::string StartTokenStr,
                               std::map<int, std::string> EndTokens) {
  std::vector<Value *> StmList;

  if (StartToken && !getTok(StartToken)) {
    writeError("expected '" + StartTokenStr + "' to start block");
    return nullptr;
  }

  while (!matchesAnyTokenPair(EndTokens) && CurTok != END) {
    if (auto Stmt = parseSingleStmt()) {
      if (auto Inst = dyn_cast<Instruction>(Stmt)) {
        std::vector<Value *> Pieces;
        extractInstructionsFromStmt(Inst, Pieces);
        for (auto It = Pieces.rbegin(); It != Pieces.rend(); ++It)
          StmList.push_back(*It);
      } else
        StmList.push_back(Stmt);
    }
  }

  if (!EndTokens.count(CurTok)) {
    std::string ErrStr;
    for (auto Tok : EndTokens)
      ErrStr += "'" + Tok.second + "'";
    if (EndTokens.size() > 1)
      writeError("expected one of " + ErrStr + " to end block");
    else
      writeError("expected " + ErrStr + " to end block");
    return nullptr;
  }
  getTok();
  return BasicBlock::get("entry", StmList, K);
}

Function *Parser::buildFcn(std::string FcnName,
                           std::vector<Argument *> &ArgList, Type *ReturnType,
                           Location &FcnLoc) {
  std::vector<Type *> ATys;
  for (auto Sym : ArgList)
    ATys.push_back(Sym->getType());
  auto FTy = FunctionType::get(ReturnType, ATys, false);
  auto Fcn = Function::get(FcnName, FTy);
  Fcn->setArguments(ArgList);
  Fcn->setSourceLocation(FcnLoc);
  return Fcn;
}
}
