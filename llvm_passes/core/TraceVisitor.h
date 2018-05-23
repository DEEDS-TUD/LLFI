#include "llvm/InstVisitor.h"
#include "Utils.h"

using namespace llvm;

struct TraceVisitor : public InstVisitor<TraceVisitor> {

  void visitLoadInst(LoadInst &LI) {
    if(llfi::isLLFIIndexedInst(&LI))
      LI.print(errs() << " -- \n");
  }
  void visitUnaryInstruction(UnaryInstruction &UI){
     
  } 
};
