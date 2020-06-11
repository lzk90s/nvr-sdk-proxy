#!/bin/sh

REGISTRY=registry.cn-hangzhou.aliyuncs.com
NAMESPACE=gosun
IMAGE=hikvision_sdk:latest

docker build -t ${REGISTRY}/${NAMESPACE}/${IMAGE} .
docker push ${REGISTRY}/${NAMESPACE}/${IMAGE}