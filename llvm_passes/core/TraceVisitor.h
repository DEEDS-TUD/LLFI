#include "Utils.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/GlobalValue.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Module.h"
#include "llvm/InstVisitor.h"
#include "llvm/Pass.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/Support/raw_ostream.h"
#include <cmath>
#include <sstream>
#include <vector>

using namespace llvm;
struct TraceVisitor : public InstVisitor<TraceVisitor> {
private:
  LLVMContext *context;
  Module *module;
  DataLayout *dataLayout;
  int maxtrace;

public:
  TraceVisitor(LLVMContext *, Module *, DataLayout *, int maxtrace);
  void visitInstruction(Instruction &);
  // void visitLoadInst(LoadInst &);
  //  void visitFunction(Function &);
  //  void visitUnaryInstruction(UnaryInstruction &);
  // void visitStoreInst(StoreInst &);
  void visitCallInst(CallInst &);
  void visitBranchInst(BranchInst &);

private:

  Instruction* visitGeneric(Instruction &);
  Instruction *getInsertPoint(Instruction *);
  Instruction *getAllocaInsertPoint(Instruction *);
  AllocaInst* insertStringInstrumentation(std::string&, Instruction*, Instruction*);
  AllocaInst *insertInstrumentation(Value *, Type *, Instruction *,
                                    Instruction *);
  AllocaInst *insertIntrinsicInstrumentation(Function *, Instruction *,
                                             Instruction *);
  AllocaInst *insertBasicBlockInstrumentation(BasicBlock*, Instruction *, Instruction *);
  int getSize(Type *type);
  AllocaInst *insertOpCode(Instruction *, Instruction *, Instruction *);
  void insertCall(Instruction *, Instruction*, Instruction *, std::vector<AllocaInst *> &,
                  Instruction *);
  void checkSupport(Value*);
  void appendTypeChar(std::stringstream&, Value*, bool);
  Value* insertThreadMapping(Value*, Instruction*);
};
