# 基于libuv封装的网络库
* libuv 1.20.3
* googletest 1.8.1
* Visual Leak Detector (VLD) Version 2.5.1

# 编译
## 参数
### TCMALLOC
使用tcmalloc内存分配替换系统内存分配，默认不打开
### VLD
在Windows系统下，使用Visual Leak Detector做内存泄露检查，默认打开
### RELEASE
是否编译Release版本，默认是Debug版本

## windows
### debug
```cmd
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" -DRELEASE=OFF -DTCMALLOC=ON ..
msbuild libnet.sln /t:Build /p:Configuration=Debug /p:Platform="x64" /nologo
```
### release
```cmd
mkdir build
cd build
cmake -G "Visual Studio 15 2017 Win64" -DRELEASE=ON -DTCMALLOC=ON ..
msbuild libnet.sln /t:Build /p:Configuration=Release /p:Platform="x64" /nologo
```

## linux
### debug
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
make
```
### release
```bash
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
```