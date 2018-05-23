/************
/instTraceLib.c
/  This library is part of the greater LLFI framework
/  This library should be linked against programs that have had the LLFI instTrace LLVM
/  pass performed on them
*************/

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<time.h>
#include<stdarg.h>

#include "Utils.h"
#include "unistd.h"

//Open a file (once) for writing. This file is not explicitly closed, must flush often!
static pthread_key_t fileKey;
static pthread_once_t fileKey_once = PTHREAD_ONCE_INIT;
extern  long g_flag;

//destructor function which closes respective file if thread terminates
static void cleanAfterThread(void* ofile){
  fclose((FILE*) ofile);
}
//initialize pthread_key so inidiviual threads can later access their log files via pthread_getspecific
static void initKey(){
  pthread_key_create(&fileKey, cleanAfterThread);
}

FILE* OutputFile() {
  FILE* ofile;
  pthread_once(&fileKey_once, initKey);
  //check if log file has yet to be created
  if ((ofile  = (FILE*) pthread_getspecific(fileKey))  ==  NULL) {
    //include threadID in name of log file
    char  filename[50];
    snprintf(filename, 50, "llfi.stat.trace%li.txt", pthread_self());
    ofile = fopen(filename, "w");
    
    pthread_setspecific(fileKey, ofile);
  }
  return ofile;
}

long GetTimeStamp(){
  struct timespec t;
  // Check which clock to use (CLOCK_REALTIME??)
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_nsec;
}

void printTID(char* targetFunc){
	fprintf(OutputFile(), "#PTID: %li\t Name: %s\tTIMESTAMP: %li\n", pthread_self(), targetFunc, GetTimeStamp());
}

char* printContent(char* ptr, int size){
  int i;
//Handle endian switch
  if (isLittleEndian()) {
    for (i = size - 1; i >= 0; i--) {
      fprintf(OutputFile(), "%02hhx", ptr[i]);
    }
  } 
  else {
    for (i = 0; i < size; i++) {
      fprintf(OutputFile(), "%02hhx", ptr[i]);
    }
  }
}

static long instCount = 0;
static long cutOff = 0;
void printInstTracer(long instID, char *opcode, int maxPrints, int count, ...){
//    char* ptr, int size, char*opPtr, int opSize) {
  va_list args;
  va_start(args, count);
  int i;
  instCount++;
  if (start_tracing_flag == TRACING_FI_RUN_FAULT_INSERTED) {
    start_tracing_flag = TRACING_FI_RUN_START_TRACING;
    cutOff = instCount + maxPrints;
    //Print faulty trace header (for analysis by traceDiff script)
    fprintf(OutputFile(), "#TraceStartInstNumber: %ld\n", instCount);
  }
  
  //These flags are set by faultinjection_lib.c (Faulty Run) or left 
  // initialized in utils.c and left unchanged (Golden run)
  if ((start_tracing_flag == TRACING_GOLDEN_RUN) || 
      ((start_tracing_flag == TRACING_FI_RUN_START_TRACING) && 
       (instCount < cutOff))) {
    fprintf(OutputFile(), "PTID: %li\tID: %ld\tOPCode: %s\tValue: ", pthread_self(), instID, opcode);
  
   char* ptr = va_arg(args, char*); 
   int size = va_arg(args, int);
   printContent(ptr, size); 
   for (i = 1; i < count; ++i) {
    fprintf(OutputFile(), "\t Operand%d: ", i-1);
    char* opPtr = va_arg(args, char*);
    int opSize = va_arg(args, int);
    printContent(opPtr, opSize);
   }
   va_end(args); 
    fprintf(OutputFile(), "\tTIMESTAMP: %li\n", GetTimeStamp());

    //no need to flush, since files are getting automatically closed once threads exit
    //fflush(OutputFile()); 

  }
  if ((start_tracing_flag != TRACING_GOLDEN_RUN) && instCount >= cutOff )
  {
	start_tracing_flag = TRACING_FI_RUN_END_TRACING;
  }

}
//this has become obsolete with per-thread log files. Clean-up is now being done in cleanAfterThread
void postTracing() {
  //if (ofile != NULL)
  //  fclose(ofile);
}
