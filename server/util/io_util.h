#pragma  once

namespace sdkproxy {

class IoUtil {
public:
    static ssize_t writen(int fd, const char *buf, size_t n) {
        size_t nleft = n;
        const char *bufptr = buf;
        ssize_t nwrite;

        while (nleft > 0) {
            if ((nwrite = write(fd, bufptr, nleft)) <= 0) {
                if (errno == EINTR)
                    nwrite = 0;
                else
                    return -1;
            }

            nleft -= nwrite;
            bufptr += nwrite;
        }

        return n;
    }

    static ssize_t readn(int fd, char *buf, size_t n) {
        size_t nleft = n;
        char *bufptr = buf;
        ssize_t  nread;

        while (nleft > 0) {
            if ((nread = read(fd, bufptr, nleft)) < 0) {
                if (errno == EINTR) { //遇到中断
                    continue;
                } else        // 其他错误
                    return -1;
            } else if (nread == 0) { // 遇到EOF
                break;
            }

            nleft -= nread;
            bufptr += nread;
        }

        return (n - nleft);
    }
};
}