#include "rhine/IR/Constant.hpp"
#include "rhine/IR/Context.hpp"
#include "rhine/IR/Instruction.hpp"
#include "rhine/IR/UnresolvedValue.hpp"

namespace rhine {
Constant::Constant(Type *Ty, RTValue ID, unsigned NumOps, std::string Name)
    : User(Ty, ID, NumOps, Name) {}

bool Constant::classof(const Value *V) {
  return V->op() >= RT_ConstantInt && V->op() <= RT_ConstantFloat;
}

ConstantInt::ConstantInt(int Val, unsigned Bitwidth, Context *K)
    : Constant(IntegerType::get(Bitwidth, K), RT_ConstantInt), Val(Val) {}

void *ConstantInt::operator new(size_t s) { return User::operator new(s); }

ConstantInt *ConstantInt::get(int Val, unsigned Bitwidth, Context *K) {
  FoldingSetNodeID ID;
  void *IP;
  ConstantInt::Profile(ID, IntegerType::get(Bitwidth, K), Val);
  if (auto CInt = K->CIntCache.FindNodeOrInsertPos(ID, IP))
    return CInt;
  auto CInt = new ConstantInt(Val, Bitwidth, K);
  K->CIntCache.InsertNode(CInt, IP);
  return CInt;
}

IntegerType *ConstantInt::type() const {
  return cast<IntegerType>(Value::type());
}

bool ConstantInt::classof(const Value *V) { return V->op() == RT_ConstantInt; }

int ConstantInt::val() const { return Val; }

unsigned ConstantInt::bitwidth() const {
  if (auto ITy = dyn_cast<IntegerType>(VTy))
    return ITy->bitwidth();
  assert(0 && "ConstantInt of non IntegerType type");
  return 0;
}

void ConstantInt::Profile(FoldingSetNodeID &ID, const Type *Ty,
                          const int &Val) {
  ID.Add(Ty);
  ID.AddInteger(Val);
}

void ConstantInt::Profile(FoldingSetNodeID &ID) const { Profile(ID, VTy, Val); }

void ConstantInt::print(DiagnosticPrinter &Stream) const {
  Stream << Val << " " << *type();
}

ConstantBool::ConstantBool(bool Val, Context *K)
    : Constant(BoolType::get(K), RT_ConstantBool), Val(Val) {}

void *ConstantBool::operator new(size_t s) { return User::operator new(s); }

ConstantBool *ConstantBool::get(bool Val, Context *K) {
  FoldingSetNodeID ID;
  void *IP;
  ConstantBool::Profile(ID, BoolType::get(K), Val);
  if (auto CBool = K->CBoolCache.FindNodeOrInsertPos(ID, IP))
    return CBool;
  auto CBool = new ConstantBool(Val, K);
  K->CBoolCache.InsertNode(CBool, IP);
  return CBool;
}

BoolType *ConstantBool::type() const {
  return cast<BoolType>(Value::type());
}

bool ConstantBool::classof(const Value *V) {
  return V->op() == RT_ConstantBool;
}

float ConstantBool::val() const { return Val; }

void ConstantBool::Profile(FoldingSetNodeID &ID, const Type *Ty,
                           const bool &Val) {
  ID.Add(Ty);
  ID.AddBoolean(Val);
}

void ConstantBool::Profile(FoldingSetNodeID &ID) const {
  Profile(ID, VTy, Val);
}

void ConstantBool::print(DiagnosticPrinter &Stream) const {
  Stream << Val << " " << *type();
}

ConstantFloat::ConstantFloat(float Val, Context *K)
    : Constant(FloatType::get(K), RT_ConstantFloat), Val(Val) {}

void *ConstantFloat::operator new(size_t s) { return User::operator new(s); }

ConstantFloat *ConstantFloat::get(float Val, Context *K) {
  FoldingSetNodeID ID;
  void *IP;
  ConstantFloat::Profile(ID, FloatType::get(K), Val);
  if (auto CFlt = K->CFltCache.FindNodeOrInsertPos(ID, IP))
    return CFlt;
  auto CFlt = new ConstantFloat(Val, K);
  K->CFltCache.InsertNode(CFlt, IP);
  return CFlt;
}

FloatType *ConstantFloat::type() const {
  return cast<FloatType>(Value::type());
}

bool ConstantFloat::classof(const Value *V) {
  return V->op() == RT_ConstantFloat;
}

float ConstantFloat::val() const { return Val; }

void ConstantFloat::Profile(FoldingSetNodeID &ID, const Type *Ty,
                            const float &Val) {
  ID.Add(Ty);
  ID.AddInteger((long)Val);
}

void ConstantFloat::Profile(FoldingSetNodeID &ID) const {
  Profile(ID, VTy, Val);
}

void ConstantFloat::print(DiagnosticPrinter &Stream) const {
  Stream << Val << " " << *type();
}

Pointer::Pointer(Value *V, Type *Ty)
    : Constant(Ty, RT_Pointer, 0, V->getName()), Val(V) {}

void *Pointer::operator new(size_t s) { return User::operator new(s); }

Pointer *Pointer::get(Value *V) {
  FoldingSetNodeID ID;
  void *IP;
  auto Ty = PointerType::get(V->type());
  Pointer::Profile(ID, Ty, V);
  auto K = V->context();
  if (auto Ptr = K->PtrCache.FindNodeOrInsertPos(ID, IP))
    return Ptr;
  auto Ptr = new Pointer(V, Ty);
  K->PtrCache.InsertNode(Ptr, IP);
  return Ptr;
}

PointerType *Pointer::type() const {
  return cast<PointerType>(Value::type());
}

bool Pointer::classof(const Value *V) { return V->op() == RT_Pointer; }

void Pointer::setVal(Value *V) { Val = V; }

Value *Pointer::val() const { return Val; }

void Pointer::Profile(FoldingSetNodeID &ID, const Type *Ty, const Value *Val) {
  ID.Add(Ty);
  ID.AddPointer(Val);
}

void Pointer::Profile(FoldingSetNodeID &ID) const { Profile(ID, VTy, Val); }

void Pointer::print(DiagnosticPrinter &Stream) const {
  Stream << *Val << "*" << std::endl;
}
}
