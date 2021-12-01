/* @file  monitor.cpp
 * @date  <date>
 * @brief <Descriptions>
 *
 * Copyright (C) 2020 NIIC EDA
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms
 * of the BSD license.  See the LICENSE file for details.
 */

#include "util/monitor.h"

namespace open_edi {
namespace util {

/**
 * @brief Global variables for non-member function.
 *        In the system there should be only one MonitorManager.
 */
MonitorManager kMonitorManager;

/**
 * @brief Construct a new Monitor:: Monitor object.
 *        get   start time and set monitor state.
 */
Monitor::Monitor() {
    __init();
    if ( gettimeofday(&start_time_, NULL) < 0 ) {
        message->issueMsg("UTIL", 44, kError, __func__, strerror(errno));
    }
    if ( 0 != getrusage(RUSAGE_SELF, &start_rusage_) ) {
        message->issueMsg("UTIL", 45, kError, __func__, strerror(errno));
    }

    state_ = kMonitorRunning;
    kMonitorManager.addMonitor(this);
}


/// @brief ~Monitor Destructor of Monitor
Monitor::~Monitor() {
    kMonitorManager.deleteMonitor(this);
}

/**
 * @brief get virtural memory and physical memory from /proc/<pid>/status.
 *        get peak memory from them.
 */
void Monitor::setMemory(uint64_t virt_mem,
                        uint64_t hwm_mem, uint64_t phys_mem) {
    virtual_mem_ = virt_mem;
    vmHWM_ = hwm_mem;
    physical_mem_ = phys_mem;

    peak_virtual_mem_ = peak_virtual_mem_ >= virtual_mem_
                                        ? peak_virtual_mem_ : virtual_mem_;
    peak_physical_mem_ = peak_physical_mem_ >= physical_mem_
                                        ? peak_physical_mem_ : physical_mem_;
}

/**
 * @brief return VmHWM and set its unit.
 * 
 * @param unit 
 * @return std::string 
 */
std::string Monitor::__getVmHWM(char unit[]) {
    __getMemory();
    return __memoryUnitConversion(unit, vmHWM_);
}

/**
 * @brief return virtural memory and set its unit.
 * 
 * @param unit 
 * @return std::string 
 */
std::string Monitor::__getVirtMemory(char unit[]) {
    __getMemory();
    return __memoryUnitConversion(unit, virtual_mem_);
}

/**
 * @brief return physical memory and set its unit.
 * 
 * @param unit 
 * @return std::string 
 */
std::string Monitor::__getPhysMemory(char unit[]) {
    __getMemory();
    return __memoryUnitConversion(unit, physical_mem_);
}

/**
 * @brief return peak virtural memory and set its unit.
 * 
 * @param unit 
 * @return std::string 
 */
std::string Monitor::__getPeakVirtMemory(char unit[]) {
    __getMemory();
    return __memoryUnitConversion(unit, peak_virtual_mem_);
}

/**
 * @brief return peak physical memory and set its unit.
 * 
 * @param unit 
 * @return std::string 
 */
std::string Monitor::__getPeakPhysMemory(char unit[]) {
    __getMemory();
    return __memoryUnitConversion(unit, peak_physical_mem_);
}

/**
 * @brief pause monitor.
 */
void Monitor::pause() {
    if (kMonitorRunning == state_) {
        state_ = kMonitorPaused;
        if ( gettimeofday(&pause_start_time_, NULL) < 0 ) {
            message->issueMsg("UTIL", 44, kError, __func__, strerror(errno));
        }
        if ( 0 != getrusage(RUSAGE_SELF, &pause_start_rusage_) ) {
            message->issueMsg("UTIL", 45, kError, __func__, strerror(errno));
        }
    }
}

/**
 * @brief resume monitor.
 */
void Monitor::resume() {
    if (kMonitorPaused == state_) {
        struct timeval current_time;
        if ( gettimeofday(&current_time, NULL) < 0 ) {
            message->issueMsg("UTIL", 44, kError, __func__, strerror(errno));
        }
        start_time_.tv_sec += current_time.tv_sec - pause_start_time_.tv_sec;
        start_time_.tv_usec += current_time.tv_usec - pause_start_time_.tv_usec;
        
        struct rusage current_rusage;
        if ( 0 != getrusage(RUSAGE_SELF, &current_rusage) ) {
            message->issueMsg("UTIL", 45, kError, __func__, strerror(errno));
        }
        start_rusage_.ru_utime.tv_sec += current_rusage.ru_utime.tv_sec -
                                        pause_start_rusage_.ru_utime.tv_sec;
        start_rusage_.ru_utime.tv_usec += current_rusage.ru_utime.tv_usec -
                                        pause_start_rusage_.ru_utime.tv_usec;
        start_rusage_.ru_stime.tv_sec += current_rusage.ru_stime.tv_sec -
                                        pause_start_rusage_.ru_stime.tv_sec;
        start_rusage_.ru_stime.tv_usec += current_rusage.ru_stime.tv_usec -
                                        pause_start_rusage_.ru_stime.tv_usec;
    }
    state_ = kMonitorRunning;
}

/**
 * @brief reset monitor.
 */
void Monitor::reset() {
    if ( gettimeofday(&start_time_, NULL) < 0 ) {
        message->issueMsg("UTIL", 44, kError, __func__, strerror(errno));
    }
    if ( 0 != getrusage(RUSAGE_SELF, &start_rusage_) ) {
        message->issueMsg("UTIL", 45, kError, __func__, strerror(errno));
    }

    __init();
    state_ = kMonitorRunning;
}

/**
 * @brief output information in one line.
 * 
 * @param resource_types 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::printProcessBar(const char* description) {
#ifndef DEBUGVERSION
    std::string print_str = description;
    ResourceTypes resource_types = kElapsedTime | kCpuTime |
                                   kPhysicalPeak | kVirtualPeak;
    __updateCpuTime();
    __makePrintString(print_str, resource_types);
    printf("%s\r", print_str.c_str());
    fflush(stdout);
#endif
    return true;
}

/**
 * @brief output information in one line.
 * 
 * @param resource_types 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::printProcessBar(ResourceTypes resource_types,
                              const char* description) {
#ifndef DEBUGVERSION
    std::string print_str = description;
    __updateCpuTime();
    __makePrintString(print_str, resource_types);
    printf("%s\r", print_str.c_str());
    fflush(stdout);
#endif
    return true;
}

/**
 * @brief output information per line.
 * 
 * @param resource_types 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::printInternal(const char* description) {
    std::string print_str = description;
    ResourceTypes resource_types = kElapsedTime | kCpuTime |
                                   kPhysicalPeak | kVirtualPeak;
    __updateCpuTime();
    __getMemory();
    __makePrintStringInternal(print_str, resource_types);
    message->issueMsg("UTIL", 46, kInfo, print_str.c_str());
    fflush(stdout);
    return true;
}

/**
 * @brief output information per line.
 * 
 * @param resource_types 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::printInternal(ResourceTypes resource_types,
                           const char* description) {
    __updateCpuTime();
    __getMemory();
    std::string print_str = description;
    __makePrintStringInternal(print_str, resource_types);
    message->issueMsg("UTIL", 46, kInfo, print_str.c_str());
    fflush(stdout);
    return true;
}

/**
 * @brief output information per line.
 * 
 * @param resource_types 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::print(const char* description) {
    std::string print_str = description;
    ResourceTypes resource_types = kElapsedTime | kCpuTime |
                                   kPhysicalPeak | kVirtualPeak;
    __updateCpuTime();
    __getMemory();
    __makePrintString(print_str, resource_types);
    message->issueMsg("UTIL", 46, kInfo, print_str.c_str());
    fflush(stdout);
    return true;
}

/**
 * @brief output information per line.
 * 
 * @param resource_types 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::print(ResourceTypes resource_types,
                           const char* description) {
    __updateCpuTime();
    __getMemory();
    std::string print_str = description;
    __makePrintString(print_str, resource_types);
    message->issueMsg("UTIL", 46, kInfo, print_str.c_str());
    fflush(stdout);
    return true;
}

/**
 * @brief write output information to file openned by FILE *.
 * 
 * @param resource_types 
 * @param fp 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::print(ResourceTypes resource_types,
                           FILE *fp,
                           const char* description) {
    __updateCpuTime();
    __getMemory();
    std::string print_str = description;
    __makePrintString(print_str, resource_types);
    fprintf(fp, "%s\n", print_str.c_str());

    return true;
}

/**
 * @brief write output information to file openned by file stream.
 * 
 * @param resource_types 
 * @param fp 
 * @param description 
 * @return true 
 * @return false 
 */
bool Monitor::print(ResourceTypes resource_types,
                           std::ofstream &fp,
                           const char* description) {
    __updateCpuTime();
    __getMemory();
    std::string print_str = description;
    __makePrintString(print_str, resource_types);
    fp << print_str.c_str() << '\n';

    return true;
}

/**
 * @brief Initialize data members.
 */
void Monitor::__init() {
    // time
    elapsed_time_          = 0.0;
    cpu_time_              = 0.0;
    user_cpu_time_         = 0.0;
    sys_cpu_time_          = 0.0;

    // memory
    physical_mem_          = 0;
    virtual_mem_           = 0;
    vmHWM_                 = 0;
    peak_physical_mem_     = 0;
    peak_virtual_mem_      = 0;
    strcpy(unit_virt_, "KiB");
    strcpy(unit_phys_, "KiB");
    strcpy(unit_vmHWM_, "KiB");
}

/**
 * @brief check resource types and then append to type str.
 * 
 * @param str 
 * @param resource_types 
 */
void Monitor::__makePrintString(std::string &print_str,
                                   ResourceTypes resource_types) {
    if (resource_types & kElapsedTime) {
        print_str += " Elapse:" + __formatTime(elapsed_time_);
    }
    if (resource_types & kCpuTime) {
        print_str += " Cpu:" + __formatTime(cpu_time_);
    }
    if (resource_types & kUserCpuTime) {
        print_str += " Cpu/usr:" + __formatTime(user_cpu_time_);
    }
    if (resource_types & kSysCpuTime) {
        print_str += " Cpu/sys:" + __formatTime(sys_cpu_time_);
    }
    if (resource_types & kPhysicalMem) {
        print_str += " Phys/curr:" + __getPhysMemory(unit_phys_) + unit_phys_;
    }
    if (resource_types & kPhysicalPeak) {
        print_str += " Phys/peak:" + __getPeakPhysMemory(unit_phys_)+unit_phys_;
    }
    if (resource_types & kVirtualMem) {
        print_str += " Virt/curr:" + __getVirtMemory(unit_virt_) + unit_virt_;
    }
    if (resource_types & kVirtualPeak) {
        print_str += " Virt/peak:" + __getPeakVirtMemory(unit_virt_)+unit_virt_;
    }
    if (resource_types & kVmHWM) {
        print_str += " VmHWM:" + __getVmHWM(unit_vmHWM_) + unit_vmHWM_;
    }
}

/**
 * @brief check resource types and then append to type str.
 * 
 * @param str 
 * @param resource_types 
 */
void Monitor::__makePrintStringInternal(std::string &print_str,
                                   ResourceTypes resource_types) {
    std::ostringstream stream;
    stream << print_str;
    stream << std::fixed << std::setprecision(6);

    if (resource_types & kElapsedTime) {
        stream << " Elapse: " << elapsed_time_;
    }
    if (resource_types & kCpuTime) {
        stream << " Cpu: " << cpu_time_;
    }
    if (resource_types & kUserCpuTime) {
        stream << " Cpu/usr: " << user_cpu_time_;
    }
    if (resource_types & kSysCpuTime) {
        stream << " Cpu/sys: " << sys_cpu_time_;
    }
    if (resource_types & kPhysicalMem) {
        stream << " Phys/curr: " << physical_mem_;
    }
    if (resource_types & kPhysicalPeak) {
        stream << " Phys/peak: " << peak_physical_mem_;
    }
    if (resource_types & kVirtualMem) {
        stream << " Virt/curr: " << virtual_mem_;
    }
    if (resource_types & kVirtualPeak) {
        stream << " Virt/peak: " << peak_virtual_mem_;
    }
    if (resource_types & kVmHWM) {
        stream << " VmHWM: " << vmHWM_;
    }
    print_str = stream.str();
}
/**
 * @brief format time, 0:00:00.
 * 
 * @param t 
 * @return std::string 
 */
std::string Monitor::__formatTime(double t) {
    if (t < 1.0) {
        std::string time_str;
        std::stringstream ss;
        ss << std::fixed
            << std::setprecision(3) << (t * 1000.0);
        ss >> time_str;
        ss.clear();
        return (time_str + "ms");
    }
    else { 
        int time = static_cast<int>(t);
        int hour, min, sec;
        hour = time / 3600;
        min = (time - 3600 * hour) / 60;
        sec = time - 3600 * hour - 60 * min;

        return (std::to_string(hour) + ":"
            + ((std::to_string(min).size()==1) ? "0" : "")
            + std::to_string(min) + ":"
            + ((std::to_string(sec).size()==1) ? "0" : "")
            + std::to_string(sec) + "  ");
    }
}

/**
 * @brief converse memory unit, return memory string.
 * 
 * @param unit 
 * @param mem 
 * @return std::string 
 */
std::string Monitor::__memoryUnitConversion(char unit[], uint64_t mem) {
    double mem_size = static_cast<double>(mem);
    std::string mem_str = std::to_string(mem);
    size_t length = mem_str.length();

    if (length > 3 && length <= 6) {
        mem_size /= 1024.0;
        strncpy(unit, "MiB", 3);
    } else if (length > 6 && length <= 9) {
        mem_size /= (1024.0 * 1024.0);
        strncpy(unit, "GiB", 3);
    } else if (length > 9 && length <= 12) {
        mem_size /= (1024.0 * 1024.0 * 1024.0);
        strncpy(unit, "TiB", 3);
    }

    std::stringstream ss;
    ss << std::fixed << std::setprecision(2) << mem_size;
    mem_str = ss.str();
    if (5 == mem_str.length())
        mem_str.insert(0, " ");
    else if (4 == mem_str.length())
        mem_str.insert(0, "  ");

    return mem_str;
}

/**
 * @brief calculate elapsed time and cpu time,
 *        including user time and system time.
 * 
 */
void Monitor::__updateCpuTime() {
    struct rusage current_usage;
    if ( 0 != getrusage(RUSAGE_SELF, &current_usage) ) {
        message->issueMsg("UTIL", 45, kError, __func__, strerror(errno));
        return;
    }

    struct timeval current_time;
    if (gettimeofday(&current_time, NULL) < 0) {
        message->issueMsg("UTIL", 44, kError, __func__, strerror(errno));
        return;
    }
    elapsed_time_ = (current_time.tv_sec - start_time_.tv_sec)
                    + static_cast<double>(current_time.tv_usec
                        - start_time_.tv_usec) / 1000000.0;

    user_cpu_time_ = current_usage.ru_utime.tv_sec
                    - start_rusage_.ru_utime.tv_sec
                    + static_cast<double>(current_usage.ru_utime.tv_usec
                        - start_rusage_.ru_utime.tv_usec) / 1000000.0;
    sys_cpu_time_ = current_usage.ru_stime.tv_sec
                    - start_rusage_.ru_stime.tv_sec
                    + static_cast<double>(current_usage.ru_stime.tv_usec
                        - start_rusage_.ru_stime.tv_usec) / 1000000.0;
    cpu_time_ = user_cpu_time_ + sys_cpu_time_;
}

/**
 * @brief Get current memory from status file.
 */
void Monitor::__getMemory() {
    pid_t pid = getpid();
    char file_name[64] = {0};
    char line_buff[256] = {0};
    char name[32] = {0};
    uint64_t vmsize = 0;
    uint64_t vmhwm = 0;
    uint64_t vmrss = 0;
    char file_unit[32] = {0};

    snprintf(file_name, sizeof(file_name), "/proc/%d/status", pid);
    FILE *fp = fopen(file_name, "r");
    if (!fp) {
        message->issueMsg("UTIL", 47, kError, __func__, strerror(errno));
        return;
    }
    while (0 != fread(line_buff, 1, sizeof(line_buff), fp)) {
        sscanf(line_buff, "%s", name);
        if (0 == strncmp(name, "VmSize", 6)) {
            sscanf(line_buff, "%s %d %s", name, &virtual_mem_, file_unit);
            continue;
        } else if (0 == strncmp(name, "VmHWM", 5)) {
            sscanf(line_buff, "%s %d %s", name, &vmHWM_, file_unit);
            continue;
        } else if (0 == strncmp(name, "VmRSS", 5)) {
            sscanf(line_buff, "%s %d %s", name, &physical_mem_, file_unit);
            break;
        }
    }
    fclose(fp);
    // in case status file is null.
    if (0 == physical_mem_) {
        struct rusage current_rusage;
        if ( 0 != getrusage(RUSAGE_SELF, &current_rusage) ) {
            message->issueMsg("UTIL", 45, kError, __func__, strerror(errno));
        }
        virtual_mem_ = vmHWM_ = physical_mem_ = current_rusage.ru_maxrss;
    }
    setMemory(virtual_mem_, vmHWM_, physical_mem_);
}


/**
 * @brief Construct a new Monitor Manager:: Monitor Manager object.
 */
MonitorManager::MonitorManager() {
    int result = pthread_create(&thread_id_, NULL,
                                __calculateMemory, (void*)&monitors_);
    if (0 != result) {
        message->issueMsg("UTIL", 48, kError, __func__, strerror(errno));
        exit(1);
    }
    int err = pthread_rwlock_init(&monitors_lock_, NULL);
    if (0 != err) {
        message->issueMsg("UTIL", 49, kError, __func__, strerror(errno));
        exit(1);
    }
}

/**
 * @brief Destroy the Monitor Manager:: Monitor Manager object.
 */
MonitorManager::~MonitorManager() {
    pthread_cancel(thread_id_);
    pthread_rwlock_destroy(&monitors_lock_);
}

/**
 * @brief Add a monitor to vector.
 */
void MonitorManager::addMonitor(Monitor *monitor) {
    if (!monitor) {
        return;
    }
    pthread_rwlock_wrlock(&monitors_lock_);
    monitors_.push_back(monitor);
    pthread_rwlock_unlock(&monitors_lock_);
}

/**
 * @brief Delete a monitor from vector.
 */
void MonitorManager::deleteMonitor(Monitor *monitor) {
    if (!monitor) {
        return;
    }
    for (int i = 0; i < monitors_.size(); ++i) {
        if (monitors_[i] == monitor) {
            pthread_rwlock_wrlock(&monitors_lock_);
            monitors_.erase(monitors_.begin() + i);
            pthread_rwlock_unlock(&monitors_lock_);
            break;
        }
    }
}
/**
 * @brief thread for getting peak memory.
 * Memory information is from /proc/<pid>/status file
 */
void *MonitorManager::__calculateMemory(void *arg) {
    std::vector<Monitor*> *monitors = (std::vector<Monitor*>*)arg;
    pid_t pid = getpid();
    char file_name[64] = {0};
    char line_buff[256] = {0};
    char name[32] = {0};
    uint64_t vmsize = 0;
    uint64_t vmhwm = 0;
    uint64_t vmrss = 0;
    char file_unit[32] = {0};

    FILE *fp = nullptr;
    while (1) {
        if (monitors->empty()) {
            sleep(3);
            continue;
        }
        snprintf(file_name, sizeof(file_name), "/proc/%d/status", pid);
        fp = fopen(file_name, "r");
        if (!fp) {
            message->issueMsg("UTIL", 47, kError, __func__, strerror(errno));
            return nullptr;
        }
        while (0 != fgets(line_buff, sizeof(line_buff), fp)) {
            sscanf(line_buff, "%s", name);
            if (0 == strncmp(name, "VmSize", 6)) {
                sscanf(line_buff, "%s %d %s", name, &vmsize, file_unit);
                continue;
            } else if (0 == strncmp(name, "VmHWM", 5)) {
                sscanf(line_buff, "%s %d %s", name, &vmhwm, file_unit);
                continue;
            } else if (0 == strncmp(name, "VmRSS", 5)) {
                sscanf(line_buff, "%s %d %s", name, &vmrss, file_unit);
                break;
            }
        }
        fclose(fp);

        for (auto it = monitors->begin(); it != monitors->end(); ++it) {
            (*it)->setMemory(vmsize, vmhwm, vmrss);
        }
        sleep(3);
    }
}

}  // namespace util
}  // namespace open_edi
