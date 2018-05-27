#include<vector>
#include<cmath>
#include "llvm/InstVisitor.h"
#include "Utils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
struct TraceVisitor : public InstVisitor<TraceVisitor> {
private:
  LLVMContext *context;
  Module *module;
  DataLayout *dataLayout;
  int maxtrace;

public:
  TraceVisitor (LLVMContext*, Module*, DataLayout*, int maxtrace);
  void visitGeneric(CallInst&);
  //void visitInstruction(Instruction &);
  //void visitLoadInst(LoadInst &);
//  void visitFunction(Function &);
//  void visitUnaryInstruction(UnaryInstruction &);
  //void visitStoreInst(StoreInst &);
  void visitCallInst(CallInst &);
  void visitBranchInst(BranchInst &);

private:
  Instruction* getInsertPoint(Instruction*);
  Instruction* getAllocaInsertPoint(Instruction*);
  AllocaInst* insertInstrumentation(Value*, Type*, Instruction*, Instruction*);
  int getSize(Type* type);
  AllocaInst* insertOpCode(Instruction*, Instruction*, Instruction*);
  void insertCall(Instruction*, Instruction*, std::vector<AllocaInst*>&, Instruction*);
};
