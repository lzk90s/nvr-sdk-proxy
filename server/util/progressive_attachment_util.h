#pragma once

#include <brpc/progressive_attachment.h>
#include <brpc/errno.pb.h>

namespace sdkproxy {
class ProgressiveAttachmentUtil {
public:
    static ssize_t writen(brpc::ProgressiveAttachment *pa, const char *buf, size_t n) {
        size_t nleft = n;
        const char *bufptr = buf;
        ssize_t nwrite;

        while (nleft > 0) {
            if ((pa->Write(bufptr, nleft)) == -1) {
                if (errno == 1011) {    //EOVERCROWDED
                    nwrite = 0;
                } else
                    return -1;
            } else {
                nwrite = nleft;
            }
            nleft -= nwrite;
            bufptr += nwrite;
        }

        return n;
    }
};

}