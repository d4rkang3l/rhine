#include "rhine/IR.h"
#include "rhine/Context.h"
#include "rhine/LLVisitor.h"
#include "rhine/Externals.h"

namespace rhine {
//===--------------------------------------------------------------------===//
// LLVisitor visits.
//===--------------------------------------------------------------------===//
llvm::Type *LLVisitor::visit(IntegerType *V) {
  return RhBuilder.getInt32Ty();
}

llvm::Type *LLVisitor::visit(BoolType *V) {
  return RhBuilder.getInt1Ty();
}

llvm::Type *LLVisitor::visit(FloatType *V) {
  return RhBuilder.getFloatTy();
}

llvm::Type *LLVisitor::visit(StringType *V) {
  return RhBuilder.getInt8PtrTy();
}

llvm::Value *LLVisitor::visit(Symbol *V, Context *K) {
  assert(K && "null Symbol Table");
  return K->getMappingOrDie(V->getName());
}

llvm::Value *LLVisitor::visit(GlobalString *S) {
  auto SRef = llvm::StringRef(S->getVal());
  return RhBuilder.CreateGlobalStringPtr(SRef);
}

llvm::Constant *LLVisitor::visit(ConstantInt *I) {
  return llvm::ConstantInt::get(RhContext, APInt(32, I->getVal()));
}

llvm::Constant *LLVisitor::visit(ConstantBool *B) {
  return llvm::ConstantInt::get(RhContext, APInt(1, B->getVal()));
}

llvm::Constant *LLVisitor::visit(ConstantFloat *F) {
  return llvm::ConstantFP::get(RhContext, APFloat(F->getVal()));
}

llvm::Constant *LLVisitor::visit(Function *RhF, llvm::Module *M, Context *K) {
  auto FType = dyn_cast<FunctionType>(RhF->getType());
  auto RType = FType->getRTy()->toLL(M, K);
  std::vector<llvm::Type *> ArgTys;
  for (auto RhTy: FType->getATys())
    ArgTys.push_back(RhTy->toLL());
  auto ArgTyAr = makeArrayRef(ArgTys);
  auto F = llvm::Function::Create(llvm::FunctionType::get(RType, ArgTyAr, false),
                                  GlobalValue::ExternalLinkage,
                                  RhF->getName(), M);

  // Bind argument symbols to function argument values in symbol table
  auto ArgList = RhF->getArgumentList();
  auto S = ArgList.begin();
  auto V = F->arg_begin();
  auto SEnd = ArgList.end();
  auto VEnd = F->arg_end();
  for (; S != SEnd && V != VEnd; ++S, ++V)
    K->addMapping((*S)->getName(), V);

  // Add function symbol to symbol table
  K->addMapping(RhF->getName(), F);

  BasicBlock *BB = BasicBlock::Create(rhine::RhContext, "entry", F);
  RhBuilder.SetInsertPoint(BB);
  llvm::Value *LastLL;
  for (auto Val : RhF->getVal())
    LastLL = Val->toLL(M, K);
  RhBuilder.CreateRet(LastLL);
  return F;
}

llvm::Value *LLVisitor::visit(AddInst *A) {
  auto Op0 = A->getOperand(0)->toLL();
  auto Op1 = A->getOperand(1)->toLL();
  return RhBuilder.CreateAdd(Op0, Op1);
}

llvm::Value *LLVisitor::visit(CallInst *C, llvm::Module *M, Context *K) {
  llvm::Function *Callee;
  auto Name = C->getName();
  if (auto Result = K->getMapping(Name))
    Callee = dyn_cast<llvm::Function>(Result);
  else if (Name == "printf")
    Callee = Externals::printf(M);
  else if (Name == "malloc")
    Callee = Externals::malloc(M);
  else
    assert (0 && "Function lookup failed");

  auto Arg = C->getOperand(0);
  llvm::Value *ArgLL;
  if (auto Sym = dyn_cast<Symbol>(Arg))
    ArgLL = K->getMappingOrDie(Sym->getName());
  else
    ArgLL = Arg->toLL(M);

  return RhBuilder.CreateCall(Callee, ArgLL, C->getName());
}

llvm::Value *LLVisitor::visit(BindInst *B, llvm::Module *M, Context *K) {
  auto V = B->getVal()->toLL();
  K->addMapping(B->getName(), V);
  return nullptr;
}

void LLVisitor::visit(Module *RhM, llvm::Module *M, Context *K) {
  for (auto F: RhM->getVal())
    F->toLL(M, K);
}
}
