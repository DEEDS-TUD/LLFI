/************
/instTraceLib.c
/  This library is part of the greater LLFI framework
/  This library should be linked against programs that have had the LLFI
instTrace LLVM
/  pass performed on them
*************/

#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "Utils.h"
#include "unistd.h"

// Open a file (once) for writing. This file is not explicitly closed, must
// flush often!
static pthread_key_t fileKey;
static pthread_once_t fileKey_once = PTHREAD_ONCE_INIT;
extern long g_flag;
#define DELIMTER ':';

// destructor function which closes respective file if thread terminates
static void cleanAfterThread(void *ofile) { fclose((FILE *)ofile); }
// initialize pthread_key so inidiviual threads can later access their log files
// via pthread_getspecific
static void initKey() { pthread_key_create(&fileKey, cleanAfterThread); }

FILE *OutputFile() {
  FILE *ofile;
  pthread_once(&fileKey_once, initKey);
  // check if log file has yet to be created
  if ((ofile = (FILE *)pthread_getspecific(fileKey)) == NULL) {
    // include threadID in name of log file
    char filename[50];
    snprintf(filename, 50, "llfi.stat.trace%li.txt", pthread_self());
    ofile = fopen(filename, "w");

    pthread_setspecific(fileKey, ofile);
  }
  return ofile;
}


struct timespec GetTimeStamp() {
  struct timespec t;
  // Check which clock to use (CLOCK_REALTIME??)
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t;
}
void printTID(char *targetFunc) {
  OutputFile();
  //fprintf(OutputFile(), " \n");
  //fprintf(OutputFile(), "%s;%li;%s;%li\n",
  //        creatorThread, pthread_self(), targetFunc, GetTimeStamp()); 
}

void printMapping(pthread_t* createdThread) {
  struct timespec t = GetTimeStamp();
  fprintf(OutputFile(), "Mapping: %lld%.9ld,%li,%li\n", (long long) t.tv_sec, t.tv_nsec, pthread_self(), *createdThread);
}


void printContent(char *ptr, int size, char* type) {
  int i;
  // Handle endian switch
  fprintf(OutputFile(), "%s-%d-", type, size); 
  if (isLittleEndian()) {
    for (i = size - 1; i >= 0; i--) {
      fprintf(OutputFile(), "%02hhx", ptr[i]);
    }
  } else {
    for (i = 0; i < size; i++) {
      fprintf(OutputFile(), "%02hhx", ptr[i]);
    }
  }
}

void printGlobalVariables(char* name, char* ptr, int ptrSize, int size) {
  fprintf(OutputFile(), "GlobalVariables: %s,",name);
  char* type = "14";
  printContent(ptr, ptrSize, type);
  fprintf(OutputFile(), ",%d\n", size);
//  fprintf(OutputFile(), ",\n");
}

char* getNextType(char* res, char* types, int index) {
  if(strlen(types) < index*3) {
    fprintf(stderr, "Warning: wrong type string format...\n");
  }
  memcpy(res, &types[3*index], 2);
  res[2] = '\0';
  return res;
}

void printFunctionEntryArgs(char* fName, char* types, int count, ...) {
  int i = 0;
//  fprintf(OutputFile(), "Start: ");
   struct timespec t = GetTimeStamp();
  fprintf(OutputFile(), "%lld%.9ld,%li,0,call-%s-d,00-4-00000000", (long long) t.tv_sec, t.tv_nsec, pthread_self(), fName);
  va_list args;
  va_start(args,count);
  char res[3];
  for (i = 0; i < count; i++) {
    fprintf(OutputFile(), ",");
    char* ar = va_arg(args, char*);
    int size = va_arg(args, int);
    printContent(ar, size, getNextType(res, types, i));
  }
  va_end(args); 
  fprintf(OutputFile(), "\n");
}

static long instCount = 0;
static long cutOff = 0;
void printInstTracer(long instID, char *opcode, int maxPrints, int count, char* types, char* val, int v_size, ...) {
  //    char* ptr, int size, char*opPtr, int opSize) {
  va_list args;
  va_start(args, v_size);
  int i;
  instCount++;
  if (start_tracing_flag == TRACING_FI_RUN_FAULT_INSERTED) {
    start_tracing_flag = TRACING_FI_RUN_START_TRACING;
    cutOff = instCount + maxPrints;
    // Print faulty trace header (for analysis by traceDiff script)
    fprintf(OutputFile(), "#TraceStartInstNumber: %ld\n", instCount);
  }

  // These flags are set by faultinjection_lib.c (Faulty Run) or left
  // initialized in utils.c and left unchanged (Golden run)
  if ((start_tracing_flag == TRACING_GOLDEN_RUN) ||
      ((start_tracing_flag == TRACING_FI_RUN_START_TRACING) &&
       (instCount < cutOff))) {

    struct timespec t = GetTimeStamp();
    fprintf(OutputFile(), "%lld%.9ld,", (long long) t.tv_sec, t.tv_nsec);
    fprintf(OutputFile(), "%li,%ld,%s,",
            pthread_self(), instID, opcode);

//    char *ptr = va_arg(args, char *);
//    int size = va_arg(args, int);
    char res[3];
    printContent(val, v_size, getNextType(res, types, 0));
    for (i = 1; i < count; ++i) {
      fprintf(OutputFile(), ",");
      char *opPtr = va_arg(args, char *);
      int opSize = va_arg(args, int);
      printContent(opPtr, opSize, getNextType(res, types, i));
    }
    va_end(args);
    fprintf(OutputFile(), "\n");
    // no need to flush, since files are getting automatically closed once
    // threads exit
    // fflush(OutputFile());
  }
  if ((start_tracing_flag != TRACING_GOLDEN_RUN) && instCount >= cutOff) {
    start_tracing_flag = TRACING_FI_RUN_END_TRACING;
  }
}
// this has become obsolete with per-thread log files. Clean-up is now being
// done in cleanAfterThread
void postTracing() {
  // if (ofile != NULL)
  //  fclose(ofile);
}
