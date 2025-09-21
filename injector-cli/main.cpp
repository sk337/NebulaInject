#include <unistd.h>

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "injector/injector.h"

#ifdef _WIN32
#define LOAD_LIBRARY_PATH "libphantom.dll"
#elseif __APPLE__
#define LOAD_LIBRARY_PATH "libphantom.dylib"
#else
#define LOAD_LIBRARY_PATH "libphantom.so"
#endif

std::vector<int> findMinecraftPids() {
    std::vector<int> pids;
    namespace fs = std::filesystem;

    for (auto &entry : fs::directory_iterator("/proc")) {
        if (!entry.is_directory()) continue;
        std::string name = entry.path().filename().string();
        if (!std::all_of(name.begin(), name.end(), ::isdigit)) continue;

        // read whole cmdline (may contain multiple NUL-separated args)
        std::ifstream cmdfile(entry.path() / "cmdline", std::ios::binary);
        if (!cmdfile) continue;
        std::string cmd;
        cmd.assign(std::istreambuf_iterator<char>(cmdfile),
                   std::istreambuf_iterator<char>());

        if (cmd.empty()) continue;

        // replace NULs with spaces so substring-search works
        for (char &c : cmd)
            if (c == '\0') c = ' ';

        // detect java executable: prefer /proc/<pid>/exe symlink, fallback to
        // cmdline
        bool is_java = false;
        try {
            auto exe = fs::read_symlink(entry.path() / "exe");
            if (exe.filename().string().find("java") != std::string::npos)
                is_java = true;
        } catch (...) { /* ignore - permission or race */
        }

        if (!is_java && cmd.find("java") == std::string::npos) continue;

        if (cmd.find("minecraft") == std::string::npos) continue;

        pids.push_back(std::stoi(name));
    }

    return pids;
}

std::string getExecutablePath() {
    char path[4096];
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (count != -1) {
        path[count] = '\0';
        return std::string(path);
    }
    return "";
}

int inject(injector_pid_t pid) {
    injector_t *injector;
    void *handle;
    int res;
    if ((res = injector_attach(&injector, pid)) != INJERR_SUCCESS) {
        std::cout << "Failed to attach to process: " << injector_error()
                  << std::endl;
        return -1;
    }
    std::cout << "Attached to process." << std::endl;
    std::string libPath =
        std::filesystem::path(getExecutablePath()).parent_path() /
        LOAD_LIBRARY_PATH;
    std::cout << "Injecting library: " << libPath << std::endl;
    if ((res = injector_inject(injector, libPath.c_str(), &handle)) !=
        INJERR_SUCCESS) {
        std::cout << "Failed to inject library: " << injector_error()
                  << std::endl;
        injector_detach(injector);
        return -1;
    }
    std::cout << "Injected library." << std::endl;
    injector_call(injector, handle, "dllLoad");
    std::cout << "Called dllLoad." << std::endl;
    if ((res = injector_detach(injector)) != INJERR_SUCCESS) {
        std::cout << "Failed to detach: " << injector_error() << std::endl;
        return -1;
    }
    return 0;
}

int main() {
    std::cout << "Searching for Minecraft processes..." << std::endl;
    auto pids = findMinecraftPids();
    if (pids.empty()) {
        std::cout << "No Minecraft processes found." << std::endl;
        return 1;
    }
    if (pids.size() == 1) {
        std::cout << "Found Minecraft process with PID " << pids[0] << "."
                  << std::endl;
        std::cout << "Injecting..." << std::endl;
        if (inject(pids[0]) != 0) {
            std::cout << "Injection failed." << std::endl;
            return 1;
        }
        std::cout << "Injection successful." << std::endl;
        return 0;
    }
    std::cout << "Found " << pids.size()
              << " Minecraft process(es):" << std::endl;
    int index = 1;
    for (int pid : pids) {
        std::cout << index << ". - PID " << pid << std::endl;
        ++index;
    }
    std::cout << "select a process to inject into (1-" << pids.size() << "): ";
    int choice = 0;
    std::cin >> choice;
    if (choice < 1 || choice > pids.size()) {
        std::cout << "Invalid choice." << std::endl;
        return 1;
    }
    int target_pid = pids[choice - 1];
    std::cout << "Injecting into PID " << target_pid << "..." << std::endl;
    if (inject(target_pid) != 0) {
        std::cout << "Injection failed." << std::endl;
        return 1;
    }
    std::cout << "Injection successful." << std::endl;
}
