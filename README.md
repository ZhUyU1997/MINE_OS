# MINE操作系统内核
## 简介
本项目将《一个64位操作系统的设计与实现》（田宇）一书中的MINE操作系统内核移植到ARM处理器上。
## 使用指南
### 准备
一台PC
### 运行环境搭建
#### windows
- [安装git shell](https://gitforwindows.org/)
- [安装MinGW](http://www.mingw.org/)  
由于编译MINE需要实现编译一些本地应用，所以这是必须的，最后将bin目录添加入环境变量
- [安装工具链](https://pan.baidu.com/s/18nwU9GWxeKk57CfIWhW0ng )
提取码：3zas   
解压，并将bin,arm-linux-gnieabihf/bin目录添加入环境变量。
bin目录包含常用的Linux基础命令，**必须放在第一个**，否则编译会失败。  
原因如下：编译需要find命令，但Linux和Windows的find命令用法不同，当然git shell内置了find，或许不会出现这种问题。  
另外这种方式可能会导致一些应用不能正常工作，可以通过恢复环境变量修复。
- [安装qemu](https://pan.baidu.com/s/17vjjLS7i70nGXm_ZLGWv5A)
提取码：h5kg   
解压到项目根目录下的tools文件夹，没有请自己创建
#### WSL & Linux
```sh
sudo apt-get install git
sudo apt-get install build-essential
sudo apt-get install gcc-arm-linux-gnueabihf
sudo apt-get install gdb-multiarch
sudo apt-get install qemu-system-arm
```
WSL默认没有安装图形界面，需要你手动安装。这里自行百度。
### 编译
Windows右击打开git bash运行以下命令
```sh
make -j4 CROSS_COMPILE=arm-linux-gnueabihf- PLATFORM=arm32-realview
```
### 运行
- 以Windows为例，运行tools/qemu-system/windows/realview-run.bat
### 调试环境搭建
#### eclipse
* [安装eclipse CDT](https://pan.baidu.com/s/1jM5nixlzl3XwzxNH0sI1sQ)
提取码：yrw7   
eclipse官网下载亦可
* [eclipse调试环境搭建指南](/doc/eclipse-debug-guide.md)
#### vscode
* 项目已包含配置文件，F5直接运行
## TODO
- [x] 完善vfs，支持多文件系统
- [ ] 开发devfs，抽象设备文件
- [ ] 移植C库
- [ ] 移植基础命令
- [ ] 支持RISC-V
- [x] 支持armv7
- [ ] 移植字体引擎
- [ ] 移植GUI

## 联系
- MINE官方QQ群：144571173
- 个人QQ：891085309
