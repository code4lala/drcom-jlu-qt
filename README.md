# drcom-jlu-qt
drcom for jlu in qt cross platform

跨平台 **win linux mac**

win和linux测试稳定可用（mac尚未测试，没有设备T^T

win兼容性从xp到win10均可用

linux release包敬请期待（Ubuntu18不能deploy，得先装个老版本Ubuntu然后再装老版本的qt的然后再部署

（不稳定的bug已修复。解决方案：掉线后客户端会尝试自行重启，如果是不稳定的bug导致的掉线的话就直接又重新登录成功了。自动重启登录成功后窗口默认最小化，不会影响您的其他操作，不会弹窗。要是wifi没信号了那就登录失败。注意：自动重启功能依赖于“记住我”选项的勾选）

下载链接：[https://github.com/code4lala/drcom-jlu-qt/releases](https://github.com/code4lala/drcom-jlu-qt/releases "https://github.com/code4lala/drcom-jlu-qt/releases")

# 已实现的功能： #

1. 自动识别mac地址 （可手动指定mac地址
2. **最小化到托盘** （linux也可以最小化到托盘啦
3. 单实例（您要是嫌开机自启慢的话呢直接打开就行，开机自启那个不会再开一个实例啦
4. 记住密码
5. 自动登录
6. 适配高分屏
7. 快速重启客户端（可以解决连不上，win缩放更改变丑等情况
8. 掉线后提示用户掉线时直接出现一个`重启客户端`的按钮方便重新登录
9. 

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
3. 

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

v 1.0.0.5 解决不稳定的bug，自动重启客户端重新登录，新增日志功能，方便查错

