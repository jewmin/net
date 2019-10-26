# 基于libuv封装的网络库
* libuv 1.20.3
* googletest 1.8.1
* Visual Leak Detector (VLD) Version 2.5.1

# 编译
## windows
```cmd
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" ..
```
## linux
### debug
```bash
mkdir build
cd build
cmake ..
make
```
### release
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```