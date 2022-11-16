# GCC 编译环境介绍（Ubuntu）

## 安装依赖
```sh
apt install autoconf automake gcc g++ binutils 
apt install qemu-system-x86
```

## 构建系统
```sh
automake
autoreconf -fiv
```

## 进入项目根目录
```sh
cd HelloX_OS
```

## 配置说明
```sh
Automake配置文件 =》 HelloX_OS/kernel/kernel.mk
# Automake使用的编译选项，内核编译各个模块Makefile.am都include该文件， 如果模块需要自定义参数，则可以修改模块下的Makefile.am。
编译脚本文件 => HelloX_OS/kernel/make/mkhellox.sh
```

### 配置脚本
```sh
    配置脚本 ./amake.sh [stm32|x86] # 可以指定编译平台，默认x86
```

## 编译内核

```sh
./amake.sh [stm32|x86] #默认:x86，使用 stm32 选项时，会使用 arm-none-eabi-{gcc|gas|ld}工具链
cd kernel/make
./mkhellox.sh make #负责编译，生成的 master.bin， 拷贝 master.bin 到 HelloX_OS/tools/vfmaker
```

## 清理
```sh
./mkhellox.sh clean
```

## 运行
```sh
cd HelloX_OS/tools/vfmaker
sudo qemu-system-i386 -fda VFLOPPY.VFD
```
