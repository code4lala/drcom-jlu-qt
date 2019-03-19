# drcom-jlu-qt
drcom for jlu in qt cross platform


跨平台 **win linux mac**


win和linux测试稳定可用（mac尚未测试，没有设备T^T


下载链接：[https://github.com/code4lala/drcom-jlu-qt/releases](https://github.com/code4lala/drcom-jlu-qt/releases "https://github.com/code4lala/drcom-jlu-qt/releases")


# 已实现的功能： #


1. 自动识别mac地址 （可手动指定mac地址
2. **最小化到托盘** （linux也可以最小化到托盘啦
3. 单实例
4. 记住密码
5. 自动登录
6. 适配高分屏
7. 快速重启客户端（托盘和菜单，测试网线拔了重插（wifi掉线重连一个道理）的话QUdpSocket不会立即响应，不如直接重启客户端来得方便
8. 

# 截图： #
WIN:

![AmH6mR.png](https://s2.ax1x.com/2019/03/18/AmH6mR.png)

UBUNTU:

![AmHb7t.png](https://s2.ax1x.com/2019/03/18/AmHb7t.png)

注意：Ubuntu 18有个bug，有时候不知道为啥不显示托盘图标，此时按快捷键`Alt`+`F2`然后输入`r`回车等一下就可以刷出来了。
![AmHzcQ.png](https://s2.ax1x.com/2019/03/18/AmHzcQ.png)

# 待实现的功能： #


1. 密码加密保存
2. 掉线后提示用户掉线时直接出现一个`重启客户端`的按钮方便重新登录
3. 喵喵喵？

**注：**
现在密码是明文保存，具体存在哪跟平台有关。用的QSettings存的，详情请参见官方文档：[https://doc.qt.io/qt-5/qsettings.html#platform-specific-notes](https://doc.qt.io/qt-5/qsettings.html#platform-specific-notes "https://doc.qt.io/qt-5/qsettings.html#platform-specific-notes")

# 感谢： #


感谢[https://github.com/lyj3516](https://github.com/lyj3516 "https://github.com/lyj3516")提供的图标


**jlu的drcom协议细节**

[https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md](https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md "https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md")


**唯一实例**

[https://github.com/itay-grudev/SingleApplication](https://github.com/itay-grudev/SingleApplication "https://github.com/itay-grudev/SingleApplication")


# 特别感谢： #


[https://github.com/mchome/dogcom](https://github.com/mchome/dogcom "https://github.com/mchome/dogcom")


提取出来jlu部分的代码改动到此项目中


# 许可证： #


GNU Affero General Public License v3.0


# 更新日志： #

v 0.0.0.0 实现基本功能

v 1.0.0.1 修复适配高DPI时只窗口大小适配但字号不适配的bug

v 1.0.0.2 增加重启功能，调整字体为微软雅黑10号

v 1.0.0.3 没有这个版本，上次该发布0.2版本时候压缩包名字打错了。。。应该为1.0.0.2的，所以跳过这个版本号

v 1.0.0.4 优化用户体验，调整掉线时的提示信息，增加掉线时直接重启客户端的提示

