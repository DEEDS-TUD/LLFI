#include "TraceVisitor.h"

// I have no idea if this is safe? I hate pointers!!
Instruction *TraceVisitor::getAllocaInsertPoint(Instruction *I) {
  return I->getParent()->getParent()->begin()->getFirstNonPHIOrDbgOrLifetime();
}

TraceVisitor::TraceVisitor(Function* func, LLVMContext *ctxt, Module *mod, DataLayout *dl,
                           int mtrace)
    : InstVisitor<TraceVisitor>() {
  context = ctxt;
  module = mod;
  dataLayout = dl;
  function = func; 
  maxtrace = mtrace;
  Instruction *instPoint = function->begin()->getFirstNonPHIOrDbgOrLifetime();
  std::string s(function->getName());
  AllocaInst *v = insertStringInstrumentation(s, instPoint, instPoint);
  insertFunctionEntry(v, instPoint);

}
int TraceVisitor::getSize(Type *type) {
  return dataLayout->getTypeAllocSize(type);
  //  float bitSize = (float)dataLayout->getTypeSizeInBits(type);
  //  return (int)ceil(bitSize / 8.0);
}

AllocaInst *
TraceVisitor::insertInstrumentation(Value *val, Type *type,
                                    Instruction *insertPoint,
                                    Instruction *alloca_insertPoint) {
  float bitSize;
  AllocaInst *aInst;
  if (type != Type::getVoidTy(*context)) {
    aInst = new AllocaInst(type, "llfi_trace", alloca_insertPoint);
    // Insert an instruction to Store the instruction Value!
    new StoreInst(val, aInst, insertPoint);
  } else {
    aInst = new AllocaInst(Type::getInt32Ty(*context), "llfi_trace",
                           alloca_insertPoint);
    new StoreInst(ConstantInt::get(IntegerType::get(*context, 32), 0), aInst,
                  insertPoint);
  }
  return aInst;
}
AllocaInst *
TraceVisitor::insertStringInstrumentation(std::string &str,
                                          Instruction *insertPoint,
                                          Instruction *alloca_insertPoint) {
  const char *content = str.c_str();
  ArrayRef<uint8_t> array_ref((uint8_t *)content, str.size() + 1);
  // llvm::Value* OPCodeName = llvm::ConstantArray::get(context,
  // opcode_name_array_ref);
  llvm::Value *value = llvm::ConstantDataArray::get(*context, array_ref);
  /********************************/

  AllocaInst *aInst =
      new AllocaInst(value->getType(), "llfi_trace", alloca_insertPoint);
  new StoreInst(value, aInst, insertPoint);
  return aInst;
}
AllocaInst *TraceVisitor::insertOpCode(Instruction *inst,
                                       Instruction *insertPoint,
                                       Instruction *alloca_insertPoint) {
  // Insert instructions to allocate stack memory for opcode name

  //  const char *tmpName = inst->getOpcodeName();
  std::string str(inst->getOpcodeName());
  if (isa<CallInst>(inst)) {
    CallInst *ci = dyn_cast<CallInst>(inst);
    Function *f = ci->getCalledFunction();
    std::string name(f->getName());
    str = str + "-" + name;
    if (f->isDeclaration()) {
      str = str + "-u";
    } else {
      str = str + "-d";
    }
  } else if (isa<ReturnInst>(inst)) {
    if (Function *f = dyn_cast<Function>(inst->getParent()->getParent())) {
      std::string name(f->getName());
      str = str + "-" + name;
    }
  }
  return insertStringInstrumentation(str, insertPoint, alloca_insertPoint);
}

Value *TraceVisitor::insertThreadMapping(Value *threadID,
                                         Instruction *insertPoint) {
  std::vector<Value *> values;
  std::vector<Type *> types;

  values.push_back(threadID);
  types.push_back(threadID->getType());

  ArrayRef<Type *> typeArrayRef(types);

  FunctionType *funcType =
      FunctionType::get(Type::getVoidTy(*context), typeArrayRef, false);

  ArrayRef<Value *> valueArrayRef(values);

  Constant *func = module->getOrInsertFunction("printMapping", funcType);
  return CallInst::Create(func, valueArrayRef, "", insertPoint);
}


Value *TraceVisitor::insertFunctionEntry(Value* fName, Instruction* insertPoint) {
  std::vector<Value *> values;
  std::vector<Type *> types;

  values.push_back(fName);
  types.push_back(fName->getType());

  ArrayRef<Type *> typeArrayRef(types);

  FunctionType *funcType =
      FunctionType::get(Type::getVoidTy(*context), typeArrayRef, false);

  ArrayRef<Value *> valueArrayRef(values);

  Constant *func = module->getOrInsertFunction("printFunctionEntry", funcType);
  return CallInst::Create(func, valueArrayRef, "", insertPoint);

}
void TraceVisitor::insertCall(Instruction *inst, Instruction *opCodeInst,
                              Instruction *typeString,
                              std::vector<AllocaInst *> &parameters,
                              Instruction *insertPoint) {

  // TODO remove the next if block!!

 /* 
  if(AllocaInst* ainst = dyn_cast<AllocaInst>(inst)) {

    parameters.push_back(ainst);
    //errs() << *ainst << " / " << *ainst->getType() << " / " << *ainst->getArraySize() << "\n";
    errs() << "Instruction: " << *ainst << "\n";
    errs() << "Type: " << *ainst->getType() << "\n";
    errs() << "AlloType: " << *ainst->getAllocatedType() << "\n";
    errs() << "Size: " << getSize(ainst->getAllocatedType()) << "\n";
  }
 */ 
  std::vector<Value *> values(parameters.size() * 2 + 5);
  ConstantInt *IDConstantInt = ConstantInt::get(IntegerType::get(*context, 32),
                                                llfi::getLLFIIndexofInst(inst));
  ConstantInt *maxTraceConstInt =
      ConstantInt::get(IntegerType::get(*context, 32), maxtrace);
  ConstantInt *countConstInt =
      ConstantInt::get(IntegerType::get(*context, 32), parameters.size());
  values[0] = IDConstantInt;
  values[1] = opCodeInst;
  values[2] = maxTraceConstInt;
  values[3] = countConstInt;
  values[4] = typeString;
  for (std::size_t i = 0; i != parameters.size(); i++) {
    values[2 * i + 5] = parameters[i];
    int bytesize = getSize((parameters[i])->getAllocatedType());
        values[2 * i + 6] =
        ConstantInt::get(IntegerType::get(*context, 32), bytesize);
  }

  std::vector<Type *> parameterVector(values.size());
  for (std::size_t i = 0; i != values.size(); i++) {
    parameterVector[i] = values[i]->getType();
  }
  // LLVM 3.3 Upgrade
  ArrayRef<Type *> parameterVector_array_ref(parameterVector);

  FunctionType *traceFuncType = FunctionType::get(
      Type::getVoidTy(*context), parameterVector_array_ref, true);
  Constant *traceFunc =
      module->getOrInsertFunction("printInstTracer", traceFuncType);

  ArrayRef<Value *> traceArgs_array_ref(values);

  CallInst::Create(traceFunc, traceArgs_array_ref, "", insertPoint);
}

AllocaInst *TraceVisitor::insertIntrinsicInstrumentation(
    Function *inst, Instruction *insertPoint, Instruction *alloca_insertPoint) {

  std::stringstream ss;
  ss << inst->getIntrinsicID();
  std::string str = ss.str();
  return insertStringInstrumentation(str, insertPoint, alloca_insertPoint);
}

void TraceVisitor::checkSupport(Value *val) {
  if (val->getType()->isAggregateType()) {
    errs() << "Warning: aggregate types (Structs or Arrays) are not "
              "supported...\n";
  }
  if (val->getType()->isVectorTy()) {
    errs() << "Warning: vector types are not supported...\n";
  }
}

void TraceVisitor::appendTypeChar(std::stringstream &ss, Value *val,
                                  bool init) {
  if (Function *f = dyn_cast<Function>(val)) {
    if (f->getIntrinsicID()) {
      ss << "16:";
      return;
    }
  }
  int id = static_cast<int>(val->getType()->getTypeID());
  if (id < 10) {
    ss << "0";
  }
  ss << id << ":";
}

// TODO: Check how to implement this later...
void TraceVisitor_appendTypeChar(std::stringstream &ss, Value *val, bool init) {
  std::string type_str;
  raw_string_ostream rso(type_str);
  val->getType()->print(rso);
  ss << rso.str();
}

Instruction *TraceVisitor::visitGeneric(Instruction& I) {
  
  Instruction* alloca_insertPoint = getAllocaInsertPoint(&I);
  Instruction* insertPoint = getInsertPoint(&I);
  
  //insertPoint = getInsertPoint(&I);
  if (!llfi::isLLFIIndexedInst(&I)) {
    //    errs() << "Warning: Found non-indexed instructions...\n";
    return insertPoint;
  }
  int counter = 0;
  std::vector<AllocaInst *> values;
  std::stringstream ss;
  //alloca_insertPoint = getAllocaInsertPoint(&I);
AllocaInst *aInst =
      insertInstrumentation(&I, I.getType(), insertPoint, alloca_insertPoint);
    values.push_back(aInst);
    checkSupport(&I);
  appendTypeChar(ss, &I, true);
    for (std::size_t i = 0; i != I.getNumOperands(); i++) {
    checkSupport(I.getOperand(i));
    appendTypeChar(ss, I.getOperand(i), false);
    if (Function *f = dyn_cast<Function>(I.getOperand(i))) {
      if (f->getIntrinsicID()) {
        values.push_back(
            insertIntrinsicInstrumentation(f, insertPoint, alloca_insertPoint));
      } else {
        std::stringstream s;
        s << f->getName().str();
        std::string sss = s.str();
        values.push_back(
            insertStringInstrumentation(sss, insertPoint, alloca_insertPoint));
      }
    } else if (BasicBlock *bb = dyn_cast<BasicBlock>(I.getOperand(i))) {
      // this operand is a label...
      values.push_back(
          insertBasicBlockInstrumentation(bb, insertPoint, alloca_insertPoint));
    }

    else {
      values.push_back(insertInstrumentation(I.getOperand(i),
                                             I.getOperand(i)->getType(),
                                             insertPoint, alloca_insertPoint));
    }
  }
  
  if (CallInst *c = dyn_cast<CallInst>(&I)) {
    if (Function *ff = c->getCalledFunction()) {
      if (ff->getName() == "pthread_create") {
        Value *v = c->getOperand(0);
        Instruction *instPoint = getInsertPoint(I.getNextNode());
        insertThreadMapping(v, instPoint);
      }
     /* 
      if (!ff->isDeclaration()) {
        Instruction *instPoint = getInsertPoint(ff->begin()->getFirstNonPHIOrDbgOrLifetime());
        std::string s(ff->getName());
        AllocaInst *v = insertStringInstrumentation(s, instPoint, getAllocaInsertPoint(instPoint));
        insertFunctionEntry(v, instPoint);
      }
      */
    }
  }
  if (AllocaInst* ai = dyn_cast<AllocaInst>(&I)) {
    int allocatedSize = getSize(ai->getAllocatedType());
    Value *v = ConstantInt::get(IntegerType::get(*context, 32), allocatedSize);
    checkSupport(v);
    appendTypeChar(ss, v, false);
    AllocaInst *tmp = insertInstrumentation(v, v->getType(), insertPoint, alloca_insertPoint);
    values.push_back(tmp);
  }
  std::string str = ss.str();
  AllocaInst *typeString =
      insertStringInstrumentation(str, insertPoint, alloca_insertPoint);
  AllocaInst *opCodeInst = insertOpCode(&I, insertPoint, alloca_insertPoint);
  insertCall(&I, opCodeInst, typeString, values, insertPoint);
  return insertPoint;
}

AllocaInst *
TraceVisitor::insertBasicBlockInstrumentation(BasicBlock *inst,
                                              Instruction *insertPoint,
                                              Instruction *alloca_insertPoint) {
  std::stringstream ss;
  if (!llfi::isLLFIIndexedInst(inst->getFirstNonPHIOrDbgOrLifetime())) {
    errs() << "Warning: first node " << *inst->getFirstNonPHIOrDbgOrLifetime()
           << " is not indexed...\n";
  }

  ss << llfi::getLLFIIndexofInst(inst->getFirstNonPHIOrDbgOrLifetime());
  std::string str = ss.str();
  const char *intrinsicNamePt = str.c_str();
  ArrayRef<uint8_t> intrinsic_name_array_ref((uint8_t *)intrinsicNamePt,
                                             str.size() + 1);
  llvm::Value *intrinsicName =
      llvm::ConstantDataArray::get(*context, intrinsic_name_array_ref);

  AllocaInst *aInst = new AllocaInst(intrinsicName->getType(), "llfi_trace",
                                     alloca_insertPoint);
  new StoreInst(intrinsicName, aInst, insertPoint);
  return aInst;
}
void TraceVisitor::visitInstruction(Instruction &I) { visitGeneric(I); }

void TraceVisitor::visitBranchInst(BranchInst &BI) { visitGeneric(BI); }
void TraceVisitor::visitCallInst(CallInst &CI) {
  if (!llfi::isLLFIIndexedInst(&CI)) {
    return;
  }
  Instruction *pthreadInsert = visitGeneric(CI);
  // only detects direct calls to pthread_create
  if (Function *calledFunc = CI.getCalledFunction()) {
    // shouldnt be done, names of values only for debugging. Take mangling
    // into consideration
    if (calledFunc->getName() == "pthread_create") {
      // errs() << "pthread_create function in: "<< CIentified\n";
      // get second argument of pthread_create (the target function)

      if (User *user = dyn_cast<User>(CI.getOperand(2))) {
        // isolate target of pthread_create
        if (Function *target = dyn_cast<Function>(
                user->stripPointerCasts())) { // getOperand(0))){

          // insert right at the beginning of the function
          Instruction *insertPoint =
              target->begin()->getFirstNonPHIOrDbgOrLifetime();

          // Allocate space on stack to pass the targetName at runtime
          // const char *targetNamePt = target->getName().data();
          // const std::string str(target->getName());
          std::string tmp(target->getName());
          std::stringstream ss;
          ss << tmp;
          std::string str = ss.str();
          const char *targetNamePt = str.c_str();

          ArrayRef<uint8_t> targetName_array_ref((uint8_t *)targetNamePt,
                                                 str.size() + 1);
          llvm::Value *targetName =
              llvm::ConstantDataArray::get(*context, targetName_array_ref);

          AllocaInst *targetNamePtr = new AllocaInst(
              targetName->getType(), "pthread-target", insertPoint);
          new StoreInst(targetName, targetNamePtr, insertPoint);

          // Create function signature
          std::vector<Type *> paramVector(1, targetNamePtr->getType());
          ArrayRef<Type *> paramVector_array_ref(paramVector);
          FunctionType *printTIDType = FunctionType::get(
              Type::getVoidTy(*context), paramVector_array_ref, false);
          Constant *printTIDFunc =
              module->getOrInsertFunction("printTID", printTIDType);

          // Prepare actual arguments which are passed
          std::vector<Value *> argVector(1, targetNamePtr);
          ArrayRef<Value *> argVector_array_ref(argVector);

          // Create and insert function call
          CallInst::Create(printTIDFunc, argVector_array_ref, "", insertPoint);
        }
      }
      //}
    }
  }
}

Instruction *TraceVisitor::getInsertPoint(Instruction *llfiIndexedInst) {
 
  Instruction *insertPoint;
  if (!llfiIndexedInst->isTerminator()) {
    insertPoint =
        llfi::getInsertPtrforRegsofInst(llfiIndexedInst, llfiIndexedInst);
    // if insert point is a call to inject fault, insert printInstTrace after
    // the injectFault call
    // iff injectFault occurs AFTER the targeted instruction (ie. dst targeted)
    insertPoint = llfi::changeInsertPtrIfInjectFaultInst(insertPoint);
  } else {
    // if terminator, insert before function
    insertPoint = llfiIndexedInst;
  }
  return insertPoint;
}
