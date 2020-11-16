#pragma once

#include <unistd.h>

namespace sdkproxy {

#define READ_PIPE_NO  0
#define WRITE_PIPE_NO 1

class PipeGuard {
public:
    PipeGuard() {
        fd_[READ_PIPE_NO]  = -1;
        fd_[WRITE_PIPE_NO] = -1;
    }

    ~PipeGuard() {
        if (fd_[READ_PIPE_NO] >= 0) {
            close(fd_[READ_PIPE_NO]);
            fd_[READ_PIPE_NO] = -1;
        }
        if (fd_[WRITE_PIPE_NO] >= 0) {
            close(fd_[WRITE_PIPE_NO]);
            fd_[WRITE_PIPE_NO] = -1;
        }
    }

    int Init() {
        if (0 != pipe(fd_)) {
            return -1;
        }
        return 0;
    }

    int GetWriteFd() { return fd_[WRITE_PIPE_NO]; }

    int GetReadFd() { return fd_[READ_PIPE_NO]; }

    void NotifyWriteCompleted() {
        close(fd_[WRITE_PIPE_NO]);
        fd_[WRITE_PIPE_NO] = -1;
    }

    void NotifyReadCompleted() {
        close(fd_[READ_PIPE_NO]);
        fd_[READ_PIPE_NO] = -1;
    }

private:
    int fd_[2];
};

} // namespace sdkproxy