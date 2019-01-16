// DO NOT MODIFY
#include "_SoftwareFaultInjectors.cpp"

/*********************
 * DEFAULT INJECTORS *
 *********************/

// WrongPointer(Data)
static RegisterFaultInjector _Data_WrongPointerFIDLInjector("WrongPointer(Data)", BitCorruptionInjector::getBitCorruptionInjector());

// WrongRetrievedAddress(IO)
static RegisterFaultInjector _IO_WrongRetrievedAddressFIDLInjector("WrongRetrievedAddress(IO)", BitCorruptionInjector::getBitCorruptionInjector());

// NoOutput(API)
static RegisterFaultInjector _API_NoOutputFIDLInjector("NoOutput(API)", new HangInjector());

// MemoryLeak(Res)
static RegisterFaultInjector _Res_MemoryLeakFIDLInjector("MemoryLeak(Res)", new MemoryLeakInjector());

// MemoryExhaustion(Res)
static RegisterFaultInjector _Res_MemoryExhaustionFIDLInjector("MemoryExhaustion(Res)", new MemoryExhaustionInjector(true));

// LowMemory(Res)
static RegisterFaultInjector _Res_LowMemoryFIDLInjector("LowMemory(Res)", new MemoryExhaustionInjector(false));

// InvalidSender(MPI)
static RegisterFaultInjector _MPI_InvalidSenderFIDLInjector("InvalidSender(MPI)", BitCorruptionInjector::getBitCorruptionInjector());

// RaceCondition(Timing)
static RegisterFaultInjector _Timing_RaceConditionFIDLInjector("RaceCondition(Timing)", new PthreadRaceConditionInjector());

// NoOpen(API)
static RegisterFaultInjector _API_NoOpenFIDLInjector("NoOpen(API)", BitCorruptionInjector::getBitCorruptionInjector());

// BufferOverflowMemmove(Data)
static RegisterFaultInjector _Data_BufferOverflowMemmoveFIDLInjector("BufferOverflowMemmove(Data)", new ChangeValueInjector(45, false));

// NoAck(MPI)
static RegisterFaultInjector _MPI_NoAckFIDLInjector("NoAck(MPI)", new HangInjector());

// InappropriateClose(API)
static RegisterFaultInjector _API_InappropriateCloseFIDLInjector("InappropriateClose(API)", new InappropriateCloseInjector(true));

// UnderAccumulator(Res)
static RegisterFaultInjector _Res_UnderAccumulatorFIDLInjector("UnderAccumulator(Res)", new ChangeValueInjector(45, false));

// DeadLock(MPI)
static RegisterFaultInjector _MPI_DeadLockFIDLInjector("DeadLock(MPI)", BitCorruptionInjector::getBitCorruptionInjector());

// ThreadKiller(Res)
static RegisterFaultInjector _Res_ThreadKillerFIDLInjector("ThreadKiller(Res)", new PthreadThreadKillerInjector());

// StalePointer(Res)
static RegisterFaultInjector _Res_StalePointerFIDLInjector("StalePointer(Res)", new StalePointerInjector());

// WrongAPI(API)
static RegisterFaultInjector _API_WrongAPIFIDLInjector("WrongAPI(API)", BitCorruptionInjector::getBitCorruptionInjector());

// DataCorruption(Data)
static RegisterFaultInjector _Data_DataCorruptionFIDLInjector("DataCorruption(Data)", BitCorruptionInjector::getBitCorruptionInjector());

// DeadLock(Res)
static RegisterFaultInjector _Res_DeadLockFIDLInjector("DeadLock(Res)", new PthreadDeadLockInjector());

// InvalidMessage(MPI)
static RegisterFaultInjector _MPI_InvalidMessageFIDLInjector("InvalidMessage(MPI)", new ChangeValueInjector(1024, false));

// WrongSavedFormat(IO)
static RegisterFaultInjector _IO_WrongSavedFormatFIDLInjector("WrongSavedFormat(IO)", new WrongFormatInjector());

// BufferOverflowMalloc(Data)
static RegisterFaultInjector _Data_BufferOverflowMallocFIDLInjector("BufferOverflowMalloc(Data)", new ChangeValueInjector(-40, false));

// NoDrain(MPI)
static RegisterFaultInjector _MPI_NoDrainFIDLInjector("NoDrain(MPI)", new ChangeValueInjector(5000, true));

// WrongSource(Data)
static RegisterFaultInjector _Data_WrongSourceFIDLInjector("WrongSource(Data)", BitCorruptionInjector::getBitCorruptionInjector());

// NoClose(API)
static RegisterFaultInjector _API_NoCloseFIDLInjector("NoClose(API)", new InappropriateCloseInjector(false));

// WrongSavedAddress(IO)
static RegisterFaultInjector _IO_WrongSavedAddressFIDLInjector("WrongSavedAddress(IO)", BitCorruptionInjector::getBitCorruptionInjector());

// PacketStorm(MPI)
static RegisterFaultInjector _MPI_PacketStormFIDLInjector("PacketStorm(MPI)", new ChangeValueInjector(-40, false));

// BufferUnderflow(API)
static RegisterFaultInjector _API_BufferUnderflowFIDLInjector("BufferUnderflow(API)", new ChangeValueInjector(-40, false));

// InvalidPointer(Res)
static RegisterFaultInjector _Res_InvalidPointerFIDLInjector("InvalidPointer(Res)", BitCorruptionInjector::getBitCorruptionInjector());

// BufferOverflow(API)
static RegisterFaultInjector _API_BufferOverflowFIDLInjector("BufferOverflow(API)", new ChangeValueInjector(45, false));

// NoMessage(MPI)
static RegisterFaultInjector _MPI_NoMessageFIDLInjector("NoMessage(MPI)", new HangInjector());

// WrongRetrievedFormat(IO)
static RegisterFaultInjector _IO_WrongRetrievedFormatFIDLInjector("WrongRetrievedFormat(IO)", new WrongFormatInjector());

// CPUHog(Res)
static RegisterFaultInjector _Res_CPUHogFIDLInjector("CPUHog(Res)", new SleepInjector());

// WrongMode(API)
static RegisterFaultInjector _API_WrongModeFIDLInjector("WrongMode(API)", BitCorruptionInjector::getBitCorruptionInjector());

// WrongDestination(Data)
static RegisterFaultInjector _Data_WrongDestinationFIDLInjector("WrongDestination(Data)", BitCorruptionInjector::getBitCorruptionInjector());

// IncorrectOutput(API)
static RegisterFaultInjector _API_IncorrectOutputFIDLInjector("IncorrectOutput(API)", BitCorruptionInjector::getBitCorruptionInjector());

/********************
 * CUSTOM INJECTORS *
 ********************/

