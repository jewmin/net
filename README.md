# 基于libuv封装的网络库
* libuv 1.35.0
* googletest 1.8.1
* Visual Leak Detector (VLD) Version 2.5.1

# 编译
## 参数
### VLD
在Windows系统下，使用Visual Leak Detector做内存泄露检查，默认打开
### RELEASE
是否编译Release版本，默认是Debug版本

## windows
### debug
```cmd
.\build.bat vld
```
### release
```cmd
.\build.bat release
```

## linux
### debug
```bash
./build.sh
```
### release
```bash
./build.sh release
```