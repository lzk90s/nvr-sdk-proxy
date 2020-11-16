## nvr sdk 代理

基于第三方厂家提供的c++sdk，进行了封装，并对外提供http服务，方便调用

1. 海康设备sdk
2. 大华平台sdk
3. 大华设备sdk

查询某个第三方平台/NVR下的所有通道接口，添加设备时方便查看

```
http://server_ip:7011/sdkproxy.DeviceQueryService/Query?ip=172.18.18.188&port=37777&user=admin&password=admin

server_ip：本服务的地址
ip：NVR/相机/第三方平台的IP地址
user：NVR/相机/第三方平台的登录用户
password：NVR/相机/第三方平台的登录密码
```
