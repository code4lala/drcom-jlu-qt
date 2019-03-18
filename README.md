# drcom-jlu-qt
drcom for jlu in qt cross platform


跨平台 **win linux mac**


win和linux测试稳定可用（mac尚未测试，没有设备T^T


下载链接：[https://github.com/code4lala/drcom-jlu-qt/releases](https://github.com/code4lala/drcom-jlu-qt/releases "https://github.com/code4lala/drcom-jlu-qt/releases")


# 已实现的功能：#


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
2. 喵喵喵？

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