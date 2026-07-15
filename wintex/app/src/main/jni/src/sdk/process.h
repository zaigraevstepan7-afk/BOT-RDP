////PHONE\\\\\

#include "dirent.h"
#pragma once
struct Proc {
	pid_t pid;
	uintptr_t base;
} proc;

int getPid(const char* pkgName) {
	DIR* procDir = opendir("/proc");
    if (!procDir) return 0;
    struct dirent* dirEntry;
    while ((dirEntry = readdir(procDir))) {
        if (dirEntry->d_type != DT_DIR) continue;
        char* end;
        const long pid = strtol(dirEntry->d_name, &end, 10);
        if (*end != '\0') continue;
        char path[256];
        snprintf(path, sizeof(path), "/proc/%ld/cmdline", pid);
        const int fd = open(path, O_RDONLY);
        if (fd < 0) continue;
        char cmd[256];
        const ssize_t bytes = read(fd, cmd, sizeof(cmd)-1);
        close(fd);
        if (bytes <= 0) continue;
        cmd[bytes] = '\0';
        if (strstr(cmd, pkgName) != NULL) {
            closedir(procDir);
            return static_cast<int>(pid);
        }
    }
    closedir(procDir);
    return 0;
}

uintptr_t getBase(const char* libName) {
	char path[256];
	snprintf(path, sizeof(path), "/proc/%d/maps", proc.pid); 
	FILE* fp = fopen(path, "r");
    if (!fp) return 0;
    char line[512];
    uintptr_t base = 0;
    while (fgets(line, sizeof(line), fp)) {
        uintptr_t start, end;
        char flags[5];
        int offset = 0;
        if (sscanf(line, "%lx-%lx %4s %*x %*x:%*x %*d%n", &start, &end, flags, &offset) < 3) continue;
        if (offset > 0 && offset < static_cast<int>(sizeof(line))) {
            char* libPath = line + offset;
            while (*libPath == ' ' || *libPath == '\t') libPath++;
            if (strstr(libPath, libName) && !strncmp(flags, "r--p", 4) && (end - start) < 0x5100000) {
                base = start;
                break;
            }
        }	
    }
    fclose(fp);
    return base;
}

int getFd(const char* action) {
	string path = "/proc/" + to_string(proc.pid) + "/mem";
	int fd;
	if (action == "read") fd = open(path.c_str(), O_RDONLY, O_CLOEXEC);
	else if (action == "write") fd = open(path.c_str(), O_WRONLY, O_CLOEXEC);
	if (fd <= 0) exit(-1);
	return fd;
}

template<typename T>
T rpm(uintptr_t addr) {
	int fd = getFd("read");
	T value;
	if (pread(fd, &value, sizeof(T), addr) != sizeof(T)) {
		close(fd);
		return T{};
	}
	close(fd);
	return value;
}
	
template<typename T>
void wpm(uint64_t addr, const T& data) {
	int fd = getFd("write");
	if (pwrite(fd, &data, sizeof(T), addr) != sizeof(T)) {
		close(fd);
		return;
	}
	close(fd);
	return;
}

template<typename T>
void Change(uintptr_t addr, const std::function<void(T *)> &change) {
auto t = rpm<T>(addr);
change(&t);
wpm<T>(addr, t);
}

template<typename T>
struct [[maybe_unused]] nullable {
    alignas(4) bool hasValue;

    T value;
};

struct [[maybe_unused]] safe {
    int salt, value;
    template<typename T>
    void set(T val) {
        salt = std::is_same<T, float>::value;
        value = *(int *) &val;
    }
};