//-*- C++ -*-

#ifndef EXTERNALS_H
#define EXTERNALS_H

#include "llvm/IR/Function.h"

namespace rhine {
struct Externals {
  static llvm::Function *printf(llvm::Module *M);
  static llvm::Function *malloc(llvm::Module *M);
};
}

#endif
