# 调试环境搭建指南
## 准备
使项目根目录如图所示。  
![](/doc/image/01.PNG)  
## 创建项目
选择`File` `Project` ，选择如图所示，点击`next`。  
![](/doc/image/02-01.PNG)  
输入`Project Name`，选择项目路径，选择`none`，点击`Finish`。  
![](/doc/image/02-02.PNG)  
## 安装调试工具
点击菜单栏`Help`，`Install New software`，进入如下界面，完成图示操作。  
![](/doc/image/03.PNG)  
## 创建Debug配置
点击`New Launch Configuration`创建新配置。  
![](/doc/image/04-01.PNG)  
按图所示选择相应项目，点击`Next`。  
![](/doc/image/04-02.PNG)  
选择项目，设置最终执行文件路径，点击`Build Configuartion`。**注意：如果提示缺少相应文件，无法完成操作，可以创建一个临时的假文件**    
![](/doc/image/04-03.PNG)  
输入以下内容，点击`Apply and Close`。  
```
all -j16 CROSS_COMPILE=arm-linux-gnueabihf- PLATFORM=arm32-realview G=on
```
![](/doc/image/04-03-01.PNG)  
点击设置`Browse`，选择之前的工具链目录下的compiler\arm-linux-gnueabihf\bin\arm-linux-gnueabihf-gdb.exe，设置Port为10001，默认值经常会被占用。  
![](/doc/image/04-04.PNG)  
## 调试
点击Debug按钮，等待操作完成。如果没有启动QEMU，可以在tools目录下手动运行
realview-dbg.bat。  
![](/doc/image/05.PNG)  