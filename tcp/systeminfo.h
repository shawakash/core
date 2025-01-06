#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <sys/statvfs.h>
#include <dirent.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/stat.h>

#ifdef __linux__
    #include <sys/sysinfo.h>
#endif

using namespace std;

class SystemInfo {
    struct ProcessInfo {
        int pid;
        string name;
        string cmdline;
        double cpu_usage;
        long memory_usage;
        string state;
        string user;

        bool operator<(const ProcessInfo& other) const {
            // descending cpu usage
            return cpu_usage > other.cpu_usage;
        }
    };

    public:
        static string getSystemStats(int n = 10) {
            stringstream ss;

            ss << "CPU Usage:\n";
            getCPUUsage(ss);

            ss << "\nMemory Usage:\n";
            getMemoryInfo(ss);

            ss << "\nDisk Usage:\n";
            getDiskUsage(ss);

            ss << "\nTop " << n << " Processes:\n";
            getTopProcesses(ss, n);

            return ss.str();
        }

    private:
        static void getCPUUsage(stringstream& ss) {
            #ifdef __linux__
                ifstream stat_file("/proc/stat");
                string line;
                if(getline(stat_file, line)) {
                    ss << line << "\n";
                }
            #else
                ss << "CPU info not available on this platform\n";
            #endif
        }

        static void getMemoryInfo(stringstream& ss) {
            #ifdef __linux__
                struct sysinfo si;
                if (sysinfo(&si) == 0) {
                    double total_ram = si.totalram * si.mem_unit / (1024.0 * 1024.0 * 1024.0);
                    double free_ram = si.freeram * si.mem_unit / (1024.0 * 1024.0 * 1024.0);
                    double used_ram = total_ram - free_ram;

                    ss << fixed << setprecision(2);
                    ss << "Total RAM: " << total_ram << " GB\n";
                    ss << "Used RAM: " << used_ram << " GB\n";
                    ss << "Free RAM: " << free_ram << " GB\n";
                }
            #else
                ss << "Memory info not available on this platform\n";
            #endif
        }

        static void getDiskUsage(stringstream& ss) {
            struct statvfs buf;
            if(statvfs("/", &buf) == 0) {
                double total = (buf.f_blocks * buf.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                double free = (buf.f_bfree * buf.f_frsize) / (1024.0 * 1024.0 * 1024.0);
                double used = total - free;

                ss << fixed << setprecision(2);
                ss << "Total Disk Space: " << total << " GB\n";
                ss << "Used Disk Space: " << used << " GB\n";
                ss << "Free Disk Space: " << free << " GB\n";
            }
        }

        static string readProcessFile(const string& pid, const string& filename) {
            string content;
            ifstream file("/proc/" + pid + "/" + filename);
            if (file.is_open()) {
                getline(file, content);
            }
            return content;
        }

        static double getProcessCPUUsage(const string& pid) {
            static string prev_cpu;
            static string prev_proc_cpu;

            // Read total CPU time
            ifstream stat_file("/proc/stat");
            string cpu_line;
            getline(stat_file, cpu_line);

            // Read process CPU time
            string proc_stat = readProcessFile(pid, "stat");

            // Calculate CPU usage percentage
            double cpu_usage = 0.0;
            try {
                size_t pos = proc_stat.find(')');
                if (pos != string::npos) {
                    string cpu_str = proc_stat.substr(pos + 2);
                    stringstream ss(cpu_str);
                    vector<string> values;
                    string value;
                    while (ss >> value) {
                        values.push_back(value);
                    }
                    if (values.size() > 13) {
                        long utime = stol(values[11]);
                        long stime = stol(values[12]);
                        cpu_usage = (utime + stime) / (double)sysconf(_SC_CLK_TCK);
                    }
                }
            } catch (...) {
                // Handle parsing errors
            }
            return cpu_usage;
        }

        static void getTopProcesses(stringstream& ss, int n) {
            #ifdef __linux__
                vector<ProcessInfo> processes;
                DIR* dir = opendir("/proc");
                struct dirent* ent;

                if (dir != NULL) {
                    while ((ent = readdir(dir)) != NULL) {
                        if (ent->d_type == DT_DIR) {
                            string pid = ent->d_name;
                            if (pid.find_first_not_of("0123456789") == string::npos) {
                                ProcessInfo proc;
                                proc.pid = stoi(pid);

                                // Get process name
                                proc.name = readProcessFile(pid, "comm");

                                // Get command line
                                proc.cmdline = readProcessFile(pid, "cmdline");
                                replace(proc.cmdline.begin(), proc.cmdline.end(), '\0', ' ');

                                // Get CPU usage
                                proc.cpu_usage = getProcessCPUUsage(pid);

                                // Get memory usage
                                string statm = readProcessFile(pid, "statm");
                                stringstream ss(statm);
                                long pages;
                                ss >> pages;
                                proc.memory_usage = pages * sysconf(_SC_PAGESIZE) / 1024 / 1024; // Convert to MB

                                // Get process state
                                string stat = readProcessFile(pid, "stat");
                                size_t pos = stat.find(')');
                                if (pos != string::npos && stat.length() > pos + 2) {
                                    proc.state = stat[pos + 2];
                                }

                                // Get user (owner)
                                struct stat st;
                                string path = "/proc/" + pid + "/stat";
                                if (::stat(path.c_str(), &st) == 0) {
                                    struct passwd *pw = getpwuid(st.st_uid);
                                    if (pw != nullptr) {
                                        proc.user = pw->pw_name;
                                    }
                                }

                                processes.push_back(proc);
                            }
                        }
                    }
                    closedir(dir);

                    // Sort processes by CPU usage
                    sort(processes.begin(), processes.end());

                    // Print header with formatting
                    ss << setw(7) << "PID"
                       << setw(20) << "NAME"
                       << setw(10) << "CPU%"
                       << setw(10) << "MEM(MB)"
                       << setw(8) << "STATE"
                       << setw(15) << "USER"
                       << "  COMMAND\n";
                    ss << string(100, '-') << "\n";

                    // Print top n processes
                    for (size_t i = 0; i < min(size_t(n), processes.size()); ++i) {
                        const auto& proc = processes[i];
                        ss << setw(7) << proc.pid
                           << setw(20) << (proc.name.length() > 19 ? proc.name.substr(0, 16) + "..." : proc.name)
                           << setw(10) << fixed << setprecision(1) << proc.cpu_usage
                           << setw(10) << proc.memory_usage
                           << setw(8) << proc.state
                           << setw(15) << (proc.user.length() > 14 ? proc.user.substr(0, 11) + "..." : proc.user)
                           << "  " << (proc.cmdline.empty() ? proc.name : proc.cmdline) << "\n";
                    }
                }
            #else
                ss << "Process information not available on this platform\n";
            #endif
        }
};
