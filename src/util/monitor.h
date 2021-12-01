/* @file  monitor.h
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#ifndef SRC_UTIL_MONITOR_H_
#define SRC_UTIL_MONITOR_H_
#include <sys/types.h>
#include <sys/resource.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>
#include <unordered_map>
#include <string>
#include <iomanip>

#include "util/message.h"

namespace open_edi {
namespace util {

using ResourceTypes = uint32_t;

const uint32_t kMaxNumMonitorId = UINT_MAX - 1;
const uint32_t kInvalidMonitorId = 0;

enum ResourceType : uint32_t {
    kElapsedTime        = 0x00000001,    // current time - start time
    kCpuTime            = 0x00000002,    // UserCpuTime + SysCpuTime
    kUserCpuTime        = 0x00000004,
    kSysCpuTime         = 0x00000008,
    kPhysicalMem        = 0x00000010,    // VmRSS in /proc/<pid>/status
    kPhysicalPeak       = 0x00000020,
    kVirtualMem         = 0x00000040,    // VmSize in /proc/<pid>/status
    kVirtualPeak        = 0x00000080,
    kVmHWM              = 0x00000100,
    kUnkown
};

class Monitor {
  public:
    Monitor();
    ~Monitor();
    struct timeval getStartTime() { return start_time_; }
    struct rusage getStartRusage() { return start_rusage_; }
    double getElapsedTime() {__updateCpuTime(); return elapsed_time_; }
    double getCpuTime() {__updateCpuTime(); return cpu_time_; }
    double getUserCpuTime() {__updateCpuTime(); return user_cpu_time_; }
    double getSysCpuTime() {__updateCpuTime(); return sys_cpu_time_; }

    void pause();
    void reset();
    void resume();
    
    void setMemory(uint64_t virt_mem, uint64_t hwm_mem, uint64_t phys_mem);
    uint64_t getVirtMemory() {__getMemory(); return virtual_mem_;}
    uint64_t getPhysMemory() {__getMemory(); return physical_mem_;}
    uint64_t getPeakVirtMemory() {__getMemory(); return peak_virtual_mem_;}
    uint64_t getPeakPhysMemory() {__getMemory(); return peak_physical_mem_;}
    uint64_t getVmHWM() {__getMemory(); return vmHWM_;}

    bool printProcessBar(const char* description = "");
    bool printProcessBar(ResourceTypes resource_types, const char* description);
    bool printInternal(const char* description = "");
    bool printInternal(ResourceTypes resource_types, const char* description);
    bool print(const char* description = "");
    bool print(ResourceTypes resource_types, const char* description);
    bool print(ResourceTypes resource_types, FILE *fp, const char* description);
    bool print(ResourceTypes resource_types, std::ofstream &fp,
                                                       const char* description);
    enum MonitorState {
        kMonitorRunning,
        kMonitorPaused,
        kUnknown
    };

  private:
    void __init();
    void __makePrintString(std::string &type_str, ResourceTypes resource_types);
    void __makePrintStringInternal(std::string &type_str,
                                   ResourceTypes resource_types);
    std::string __formatTime(double);
    std::string __memoryUnitConversion(char unit[], uint64_t mem);
    std::string __getVmHWM(char unit[]);
    std::string __getVirtMemory(char unit[]);
    std::string __getPhysMemory(char unit[]);
    std::string __getPeakVirtMemory(char unit[]);
    std::string __getPeakPhysMemory(char unit[]);
    void __updateCpuTime();
    void __getMemory();

    struct timeval start_time_;
    struct rusage start_rusage_;
    double elapsed_time_;
    double cpu_time_;
    double user_cpu_time_;
    double sys_cpu_time_;
    struct timeval pause_start_time_;         // record paused time
    struct rusage  pause_start_rusage_;
    uint64_t physical_mem_;
    uint64_t virtual_mem_;
    uint64_t vmHWM_;    // high water mark
    uint64_t peak_physical_mem_;
    uint64_t peak_virtual_mem_;

    char unit_virt_[6];
    char unit_phys_[6];
    char unit_vmHWM_[6];
    MonitorState state_;
};  // class Monitor

class MonitorManager {
  public:
    MonitorManager();
    ~MonitorManager();
    void addMonitor(Monitor *monitor);
    void deleteMonitor(Monitor *monitor);

  private:
    static void *__calculateMemory(void *arg);

    pthread_t thread_id_;
    std::vector<Monitor *> monitors_;
    pthread_rwlock_t monitors_lock_;
};

}  // namespace util
}  // namespace open_edi

#endif  // SRC_UTIL_MONITOR_H_
