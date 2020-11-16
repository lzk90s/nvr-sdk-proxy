################### 编译c++程序 ###################
FROM ubuntu:18.04 as base

#FROM registry.cn-hangzhou.aliyuncs.com/gosun/dahua_sdk:latest as dahua_sdk
FROM registry.cn-hangzhou.aliyuncs.com/gosun/dahuanvr_sdk:V3.052.0000002.0.R.201103 as dahuanvr_sdk
FROM registry.cn-hangzhou.aliyuncs.com/gosun/hikvisionnvr_sdk:6.1.4.42 as hikvisionnvr_sdk
FROM registry.cn-hangzhou.aliyuncs.com/gosun/brpc_opencv_ffmpeg:1.2.4 as builder

WORKDIR /root
ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC
ENV DAHUA_SDK_HOME /root/dahua_sdk
ENV DAHUANVR_SDK_HOME /root/dahuanvr_sdk
ENV HIKVISION_SDK_HOME /root/hikvision_sdk
ENV HIKVISIONNVR_SDK_HOME /root/hikvisionnvr_sdk
ENV LC_ALL en_US.UTF-8
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:/usr/local/lib:${DAHUA_SDK_HOME}/lib:${HIKVISION_SDK_HOME}/lib:${DAHUANVR_SDK_HOME}/lib:${HIKVISIONNVR_SDK_HOME}/lib:${HIKVISIONNVR_SDK_HOME}/lib/HCNetSDKCom



#拷贝sdk
#COPY --from=dahua_sdk $DAHUA_SDK_HOME $DAHUA_SDK_HOME
COPY --from=dahuanvr_sdk $DAHUANVR_SDK_HOME $DAHUANVR_SDK_HOME
COPY --from=hikvisionnvr_sdk $HIKVISIONNVR_SDK_HOME $HIKVISIONNVR_SDK_HOME

RUN rm -rf /etc/apt/sources.list.d/* && \
    apt-get update && \
    apt-get install -y libasound2-dev libxv1 libgl1-mesa-dev


ADD . /root/build

RUN cd /root/build && \
    xmake f -m debug --cxx=g++ && \
	xmake && \
    xmake install


################### 生成runtime ###################
FROM base

WORKDIR /root
ENV CFLAGS -fPIC
ENV CXXFLAGS -fPIC
ENV DAHUA_SDK_HOME /root/dahua_sdk
ENV DAHUANVR_SDK_HOME /root/dahuanvr_sdk
ENV HIKVISION_SDK_HOME /root/hikvision_sdk
ENV HIKVISIONNVR_SDK_HOME /root/hikvisionnvr_sdk
ENV LC_ALL en_US.UTF-8
ENV LD_LIBRARY_PATH ${LD_LIBRARY_PATH}:/usr/local/lib:${DAHUA_SDK_HOME}/lib:${HIKVISION_SDK_HOME}/lib:${DAHUANVR_SDK_HOME}/lib:${HIKVISIONNVR_SDK_HOME}/lib:${HIKVISIONNVR_SDK_HOME}/lib/HCNetSDKCom


RUN sed -i 's/archive.ubuntu.com/mirrors.aliyun.com/g' /etc/apt/sources.list  && \
    rm -rf /etc/apt/sources.list.d/* && \
    apt-get update && \
    apt-get install --fix-missing -y --no-install-recommends \
    tzdata net-tools inetutils-ping libasound2-dev libssl1.0.0 libssl-dev  libxv1 libgl1-mesa-dev && \
    apt-get autoclean && apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    ##修改时区
    ln -sf /usr/share/zoneinfo/Asia/Shanghai /etc/localtime && \
    echo "Asia/Shanghai" > /etc/timezone


#拷贝sdk
#COPY --from=dahua_sdk $DAHUA_SDK_HOME $DAHUA_SDK_HOME
COPY --from=dahuanvr_sdk $DAHUANVR_SDK_HOME $DAHUANVR_SDK_HOME
COPY --from=hikvisionnvr_sdk $HIKVISIONNVR_SDK_HOME $HIKVISIONNVR_SDK_HOME
#拷贝生成的文件
COPY --from=builder /usr/local/bin/ /usr/local/bin/
COPY --from=builder /usr/local/lib/*.so /usr/local/lib/

ENTRYPOINT sdk_proxy_server