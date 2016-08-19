/*
htop - unsupported/Platform.c
(C) 2014 Hisham H. Muhammad
(C) 2015 David C. Hunt
Released under the GNU GPL, see the COPYING file
in the source distribution for its full text.
*/

#include <pcp/pmapi.h>
#include "Platform.h"
#include "CPUMeter.h"
#include "MemoryMeter.h"
#include "SwapMeter.h"
#include "TasksMeter.h"
#include "LoadAverageMeter.h"
#include "ClockMeter.h"
#include "HostnameMeter.h"
#include "UptimeMeter.h"

#define NONE 0
/*{
#include "Action.h"
#include "BatteryMeter.h"
#include "SignalsPanel.h"
#include "UnsupportedProcess.h"
}*/

SignalItem Platform_signals[] = {
   { .name = " 0 Cancel",    .number =  0 },
};

int pcp_context = -1;

unsigned int Platform_numberOfSignals = sizeof(Platform_signals)/sizeof(SignalItem);

ProcessField Platform_defaultFields[] = { PID, USER, PRIORITY, NICE, M_SIZE, M_RESIDENT, STATE, PERCENT_CPU, PERCENT_MEM, TIME, COMM, 0 };

ProcessFieldData Process_fields[] = {
   [0] = { .name = "", .title = NULL, .description = NULL, .flags = 0, },
   [PID] = { .name = "PID", .title = "    PID ", .description = "Process/thread ID", .flags = 0, },
   [COMM] = { .name = "Command", .title = "Command ", .description = "Command line", .flags = 0, },
   [STATE] = { .name = "STATE", .title = "S ", .description = "Process state (S sleeping, R running, D disk, Z zombie, T traced, W paging)", .flags = 0, },
   [PPID] = { .name = "PPID", .title = "   PPID ", .description = "Parent process ID", .flags = 0, },
   [PGRP] = { .name = "PGRP", .title = "   PGRP ", .description = "Process group ID", .flags = 0, },
   [SESSION] = { .name = "SESSION", .title = "   SESN ", .description = "Process's session ID", .flags = 0, },
   [TTY_NR] = { .name = "TTY_NR", .title = "  TTY ", .description = "Controlling terminal", .flags = 0, },
   [TPGID] = { .name = "TPGID", .title = "  TPGID ", .description = "Process ID of the fg process group of the controlling terminal", .flags = 0, },
   [MINFLT] = { .name = "MINFLT", .title = "     MINFLT ", .description = "Number of minor faults which have not required loading a memory page from disk", .flags = 0, },
   [MAJFLT] = { .name = "MAJFLT", .title = "     MAJFLT ", .description = "Number of major faults which have required loading a memory page from disk", .flags = 0, },
   [PRIORITY] = { .name = "PRIORITY", .title = "PRI ", .description = "Kernel's internal priority for the process", .flags = 0, },
   [NICE] = { .name = "NICE", .title = " NI ", .description = "Nice value (the higher the value, the more it lets other processes take priority)", .flags = 0, },
   [STARTTIME] = { .name = "STARTTIME", .title = "START ", .description = "Time the process was started", .flags = 0, },

   [PROCESSOR] = { .name = "PROCESSOR", .title = "CPU ", .description = "Id of the CPU the process last executed on", .flags = 0, },
   [M_SIZE] = { .name = "M_SIZE", .title = " VIRT ", .description = "Total program size in virtual memory", .flags = 0, },
   [M_RESIDENT] = { .name = "M_RESIDENT", .title = "  RES ", .description = "Resident set size, size of the text and data sections, plus stack usage", .flags = 0, },
   [ST_UID] = { .name = "ST_UID", .title = " UID ", .description = "User ID of the process owner", .flags = 0, },
   [PERCENT_CPU] = { .name = "PERCENT_CPU", .title = "CPU% ", .description = "Percentage of the CPU time the process used in the last sampling", .flags = 0, },
   [PERCENT_MEM] = { .name = "PERCENT_MEM", .title = "MEM% ", .description = "Percentage of the memory the process is using, based on resident memory size", .flags = 0, },
   [USER] = { .name = "USER", .title = "USER      ", .description = "Username of the process owner (or user ID if name cannot be determined)", .flags = 0, },
   [TIME] = { .name = "TIME", .title = "  TIME+  ", .description = "Total time the process has spent in user and system time", .flags = 0, },
   [NLWP] = { .name = "NLWP", .title = "NLWP ", .description = "Number of threads in the process", .flags = 0, },
   [TGID] = { .name = "TGID", .title = "   TGID ", .description = "Thread group ID (i.e. process ID)", .flags = 0, },
   [100] = { .name = "*** report bug! ***", .title = NULL, .description = NULL, .flags = 0, },
};

MeterClass* Platform_meterTypes[] = {
   &CPUMeter_class,
   &ClockMeter_class,
   &LoadAverageMeter_class,
   &LoadMeter_class,
   &MemoryMeter_class,
   &SwapMeter_class,
   &TasksMeter_class,
   &BatteryMeter_class,
   &HostnameMeter_class,
   &UptimeMeter_class,
   &AllCPUsMeter_class,
   &AllCPUs2Meter_class,
   &LeftCPUsMeter_class,
   &RightCPUsMeter_class,
   &LeftCPUs2Meter_class,
   &RightCPUs2Meter_class,
   &BlankMeter_class,
   NULL
};

void Platform_setBindings(Htop_Action* keys) {
   (void) keys;
}

int Platform_numberOfFields = 100;

extern char Process_pidFormat[20];

ProcessPidColumn Process_pidColumns[] = {
   { .id = 0, .label = NULL },
};

static int lookupMetric(char *metric, pmAtomValue *atom, int type, int inst) {
   // Create context
   if(pcp_context == -1) {
      pcp_context = pmNewContext(PM_CONTEXT_LOCAL, "local:");
   }
   // Still no context, bail out
   if(pcp_context < 0) {
      return -1;
   }

   pmID pmid;

   // Get the PMID.
   if(pmLookupName(1, &metric, &pmid) < 0) {
      return -1;
   }

   // Do the fetch
   pmResult *result;
   if(pmFetch(1, &pmid, &result) < 0) {
      return -1;
   }

   // Extract
   pmValueSet *res = result[0].vset[0];
   int i;
   for(i = 0;i < inst; i++){
     pmExtractValue(res->valfmt, &res->vlist[i], type, &atom[i], type);
   }
   pmFreeResult(result);
  //  pmDestroyContext(pcp_context);

   return 0;
}

int Platform_getUptime() {

   pmAtomValue uptime_atom[1];

   if(lookupMetric("kernel.all.uptime", uptime_atom, PM_TYPE_U32, 1) < 0) {
      // TODO: is -1 appropriate here?
      return -1;
   }

   return uptime_atom[0].ul;
}

void Platform_getLoadAverage(double* one, double* five, double* fifteen) {
  pmAtomValue loadavg_atom[3];
  if(lookupMetric("kernel.all.load", loadavg_atom, PM_TYPE_FLOAT, 3) < 0){
    *one = 0;
    *five = 0;
    *fifteen = 0;
  }
   *one = loadavg_atom[0].f;
   *five = loadavg_atom[1].f;
   *fifteen = loadavg_atom[2].f;
}

int Platform_getMaxPid() {
   return 1;
}

double Platform_setCPUValues(Meter* this, int cpu) {
   (void) this;
   (void) cpu;
   return 0.0;
}

void Platform_setMemoryValues(Meter* this) {
   pmAtomValue mem_atom[1];
   unsigned long long int usedMem, buffersMem, cachedMem, totalMem;
   if(lookupMetric("hinv.physmem", mem_atom, PM_TYPE_U32, 1) < 0){
     this->total = 0;
   }
   totalMem = (mem_atom[0].ul * 1024);
   if(lookupMetric("mem.util.used", mem_atom, PM_TYPE_U64, 1) < 0){
     this->values[0] = 0;
   }
   usedMem = mem_atom[0].ull;
   if(lookupMetric("mem.util.bufmem", mem_atom, PM_TYPE_U64, 1) < 0){
     this->values[1] = 0;
   }
   buffersMem = mem_atom[0].ull;
   if(lookupMetric("mem.util.cached", mem_atom, PM_TYPE_U64, 1) < 0){
     this->values[2] = 0;
   }
   cachedMem = mem_atom[0].ull;
   usedMem -= buffersMem + cachedMem;
   this->values[0] = usedMem;
   this->values[1] = buffersMem;
   this->values[2] = cachedMem;
   this->total = totalMem;
}

void Platform_setSwapValues(Meter* this) {
   pmAtomValue swap_atom[1];
   unsigned long long int totalSwap, usedSwap, freeSwap;
   if(lookupMetric("mem.util.swapTotal", swap_atom, PM_TYPE_U64, 1) < 0){
     (void) this;
   }
   totalSwap = swap_atom[0].ull;
   if(lookupMetric("mem.util.swapFree", swap_atom, PM_TYPE_U64, 1) < 0){
     (void) this;
   }
   freeSwap = swap_atom[0].ull;
   usedSwap = totalSwap - freeSwap;
   this->total = totalSwap;
   this->values[0] = usedSwap;
}

bool Process_isThread(Process* this) {
   (void) this;
   return false;
}

char* Platform_getProcessEnv(pid_t pid) {
   (void) pid;
   return NULL;
}