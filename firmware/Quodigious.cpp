#include <stdint.h>
#include <errno.h>
#include <vector>
uint32_t
pow10(uint32_t count) {
    if (count == 0) {
        return 1;
    } else {
        return 10 * pow10(count - 1);
    }
}
std::vector<uint32_t> matches;
void
performQuodigious(int depth, uint32_t number = 0, uint32_t sum = 0, uint32_t product = 1) {
    if (depth == 0) {
       if ((number % sum == 0) && (number % product == 0)) {
           matches.push_back(number);
       }
    } else {
        for (int i = 2; i < 10; ++i) {
            performQuodigious(depth - 1,
                              (i * pow10(depth - 1)) + number,
                              sum + i,
                              product * i);

        }
    }
}

extern "C" volatile void start() {
    for (int i = 0; i < 22; ++i) {
        int depth = rand() % 10;
        if (depth == 0) {
            ++depth;
        }
        performQuodigious(depth);
    }
    while (true) {
        // do nothing
    }
}

extern "C" {
void
_exit(void) {

}
intptr_t brkSize = 0;
const intptr_t maximumDataAddress = 0x03FFFFFF;

int
brk(void* addr) {
    if (reinterpret_cast<intptr_t>(addr) <= maximumDataAddress) {
        brkSize = reinterpret_cast<intptr_t>(addr);
        return 0;
    } else {
        errno = ENOMEM;
        return -1;
    }
}
void*
sbrk(intptr_t increment) {
    void* oldBrkSize = reinterpret_cast<void*>(brkSize);
    intptr_t addr = increment + brkSize;
    if (addr <= maximumDataAddress) {
        return oldBrkSize;
    } else {
        errno = ENOMEM;
        return reinterpret_cast<void *>(-1);
    }
}

pid_t
getpid(void) {
    return 0;
}

int
kill(pid_t pid, int sig) {
    return 0;
}

ssize_t
read(int, void*, size_t) {
    return 0;
}

off_t
lseek(int, off_t, int) {
    return 0;
}

ssize_t
write(int, const void*, size_t) {
    return 0;
}

int
close(int) {
   return 0;
}

int
fstat(int, struct stat* buf) {
    return 0;
}

int
isatty(int) {
    return 1;
}

}
