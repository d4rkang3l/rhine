#include "rhine/IR.h"
#include "rhine/Externals.h"

#include "llvm/IR/Type.h"
#include "llvm/ADT/ArrayRef.h"
#include "llvm/ExecutionEngine/MCJIT.h"

#define THIS_FPTR(FPtr) ((this)->*(FPtr))

using namespace llvm;

namespace rhine {
Externals::Externals(Context *K_) : K(K_) {
  PrintfTy =
    FunctionType::get(IntegerType::get(32, K), {StringType::get(K)}, true, K);
  MallocTy =
    FunctionType::get(StringType::get(K), {IntegerType::get(64, K)}, false, K);
  ToStringTy =
    FunctionType::get(StringType::get(K), {IntegerType::get(32, K)}, false, K);
  auto PrintfTyPtr = PointerType::get(PrintfTy, K);
  auto MallocTyPtr = PointerType::get(MallocTy, K);
  auto ToStringTyPtr = PointerType::get(ToStringTy, K);

  ExternalsMapping.insert(
      std::make_pair("println", ExternalsRef(PrintfTyPtr, &Externals::printf)));
  ExternalsMapping.insert(
      std::make_pair("malloc", ExternalsRef(MallocTyPtr, &Externals::malloc)));
  ExternalsMapping.insert(
      std::make_pair("toString", ExternalsRef(ToStringTyPtr, &Externals::toString)));
}

Externals *Externals::get(Context *K) {
  if (!K->ExternalsCache)
    K->ExternalsCache = new (K->RhAllocator) Externals(K);
  return K->ExternalsCache;
}

PointerType *Externals::getMappingTy(std::string S) {
  auto V = ExternalsMapping.find(S);
  return V == ExternalsMapping.end() ? nullptr : V->second.FTy;
}

llvm::Constant *Externals::getMappingVal(std::string S, llvm::Module *M) {
  auto V = ExternalsMapping.find(S);
  return V == ExternalsMapping.end() ? nullptr : THIS_FPTR(V->second.FHandle)(M);
}

llvm::Constant *Externals::printf(llvm::Module *M) {
  // getOrInsertFunction::
  //
  // Look up the specified function in the module symbol table.
  // Four possibilities: 1. If it does not exist, add a prototype for the function
  // and return it. 2. If it exists, and has a local linkage, the existing
  // function is renamed and a new one is inserted. 3. Otherwise, if the existing
  // function has the correct prototype, return the existing function. 4. Finally,
  // the function exists but has the wrong prototype: return the function with a
  // constantexpr cast to the right prototype.
  auto FTy = cast<llvm::FunctionType>(PrintfTy->toLL(M, K));
  return M->getOrInsertFunction("std_Void_print__String", FTy);
}

llvm::Constant *Externals::malloc(llvm::Module *M) {
  auto FTy = cast<llvm::FunctionType>(MallocTy->toLL(M, K));
  return M->getOrInsertFunction("malloc", FTy);
}

llvm::Constant *Externals::toString(llvm::Module *M) {
  auto FTy = cast<llvm::FunctionType>(ToStringTy->toLL(M, K));
  return M->getOrInsertFunction("std_String_toString__Int", FTy);
}

extern "C" {
  __attribute__((used, noinline))
  const char *std_String_toString__Int(int toConvert) {
    return std::to_string(toConvert).c_str();
  }
  __attribute__((used, noinline))
  void std_Void_print__String(const char *toPrint) {
    std::cout << toPrint;
  }
  __attribute__((used, noinline))
  void std_Void_println__String(const char *toPrint) {
    std::cout << toPrint << std::endl;
  }
}
}
