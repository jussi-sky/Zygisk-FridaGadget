# Zygisk-ModuleTemplate
Zygisk module template. refer to  [Riru-ModuleTemplate](https://github.com/RikkaApps/Riru-ModuleTemplate) and [zygisk-module-sample](https://github.com/topjohnwu/zygisk-module-sample)


## Build
You must modify local.properties to adapt your SDK path

You can also modify  /module/build.gradle file to match your NDK and CMAKE versions.

Make sure you have a jdk11 environment.

on the command line
run
```
gradlew :module:assembleRelease
```
or click
```
build.bat
```

## Others
builded files in /out

And You can install it in Magisk24.0+

After reboot

you can find log in LogCat
![png](/img/template.png)


## 说明
这是一个生成zygisk模块的模板,之前找了好久才找到了一个生成riru的模板,但是不能直接用,要改

所以我参考了zygisk-lsposed的customize.sh稍微改了下适用最新的zygisk模块

## 构建
修改local.properties为自己的sdk路径

你也可以修改/module/build.gradle里面的ndk和cmake版本为自己的版本

需要配置jdk11的环境

命令行下执行
```
gradlew :module:assembleRelease
```
或者双击
```
build.bat
```

构建完的文件在/out文件夹里.

把生成zip传到手机里就可以用magisk安装了.

