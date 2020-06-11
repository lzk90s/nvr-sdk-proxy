#!/bin/sh

REGISTRY=registry.cn-hangzhou.aliyuncs.com
NAMESPACE=gosun
IMAGE=hikvisionnvr_sdk:latest

docker build -t ${REGISTRY}/${NAMESPACE}/${IMAGE} .
docker push ${REGISTRY}/${NAMESPACE}/${IMAGE}