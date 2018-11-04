# MINE操作系统内核
## 简介
本项目将《一个64位操作系统的设计与实现》（田宇）一书中的MINE操作系统内核移植到ARM9处理器上
## 使用指南
### 准备
1. 一台Windows电脑  
2. JZ2440开发板
3. 一张2-32g的sd卡
### 环境搭建
#### windows
1. 安装XShell ，MobaXterm（推荐）等拥有串口通讯功能的软件 
2. 安装DNW(控制台版本，用于下载代码)  
安装教程：http://blog.chinaunix.net/uid-29641438-id-4462545.html  
3. 安装git shell（建议安装）
4. 安装工具链
https://blog.csdn.net/alan00000/article/details/51724252
#### linux
1. 同样安装串口工具
2. 由于dnw linux版本安装比较困难，所以可以linux编译，windows用dnw下载。如果纯linux环境，可以考虑ftp，nfs等。
3. 安装git工具（建议安装）
#### 其他平台
告辞
### 编译
进入项目目录
```sh
make
make user
```
- 将user目录下的app.bin更名为init.bin。
- 将sd卡格式化为fat32文件系统，并将init.bin放在根目录
- 将sd卡插在开发板上
### 下载
为简化操作，先设置uboot，将代码直接下载到内存0x30100000位置。
```sh
make dnw
```
## 移植建议
考虑jz2440开发板比较小众，如果各位手头恰好有其他如mini2440,tq2440之类的开发板，那么请看下面内容。
1. 先让代码启动起来，并能看到串口输出（比对开发板的串口是否与代码设置的移植，不一致请调整）
2. 根据需要调整LCD驱动的设置，以适应自己的LCD。如果嫌麻烦，也可以将LCD相关代码直接移除。
3. 调试你的sd控制器
## TODO
1. 完善vfs，支持多文件系统
2. 开发类似devfs的东东，存储设备文件
3. 完全移植书中的用户空间代码（现在的只是hello world），
所以现在是3万行代码写了一个hello world？(笑。。)
4. 完善Makefile，现在的问题：
- 当删除一个头文件，编译会失败（因为旧的依赖文件还依赖这个头文件），参考linux内核fixdep.c解决


- 不能检查依赖的变化，如:

```makfile
obj-y +=a.o
obj-y +=b.o

built-in.o: $(obj-y)
```
当删除obj-y +=b.o时，built-in.o并不会发生变化
- 不能检查工具链和编译参数的生变化
- 不支持多架构，参考xboot
- 不支持配置编译
5. 支持armv7架构



## 联系
- MINE官方QQ群：144571173
- 个人QQ: 891085309（备注：内核）
