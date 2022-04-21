# Zygisk-FridaGadget

## 说明

这是一个根据 zygisk模块 开发的 FridaGadget

## 构建

修改 local.properties 为自己的sdk路径

修改 /module/build.gradle 里面的 ndk 和 cmake 版本为自己的版本

需要配置jdk11的环境

构建完的文件在 /out 文件夹里

把生成zip传到手机里就可以用 magisk 安装了

## Zygisk-ModuleTemplate

[zygisk-module-sample](https://github.com/topjohnwu/zygisk-module-sample)

https://forum.xda-developers.com/t/discussion-magisk-the-age-of-zygisk.4393877/

## Others

You can install it in Magisk24.0+

After reboot

You can find log in logcat

```
logcat | grep Frida
netstat -tunlp | grep 26000

cd /data/local/tmp
echo "com.xxx.xxx" > app.list

adb forward tcp:27042 tcp:26000

frida-ps -Ra
frida -R gadget

objection -Ng gadget explore
```



