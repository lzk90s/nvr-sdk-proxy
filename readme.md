## 1.MergeRequest提交信息规范

#### Title
简要说明本次提交相关内容
#### Description 
注：每行后面要加两个空格

修改内容：修改相机抓拍统计告警   
JIRA：https://jira.gosuncs.com:8444/browse/DEV-2 （jira链接）  
合入人：XXXX  
合入分支：master  
# thirdparty-sdk-proxy

## 第三方SDK代理服务

基于第三方厂家提供的c++sdk，进行了封装，并对外提供http服务，方便调用

1. 海康设备sdk
2. 大华平台sdk
3. 大华设备sdk

查询某个第三方平台/NVR下的所有通道接口，添加设备时方便查看

```
http://3rdparty-sdk-proxy:7011/sdkproxy.DeviceQueryService/Query?ip=172.18.18.188&port=37777&user=admin&password=admin

3rdparty-sdk-proxy：本服务的地址
ip：NVR/相机/第三方平台的IP地址
port：NVR/相机/第三方平台的端口
user：NVR/相机/第三方平台的登录用户
password：NVR/相机/第三方平台的登录密码
```

## sidecar

使用spring-cloud sidecar，使得c++项目可以和springcloud框架集成