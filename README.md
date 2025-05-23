# drcom-jlu-qt
drcom for jlu in qt cross platform

跨平台 **win linux** [下载链接](https://github.com/KnCRJVirX/drcom-jlu-qt/releases)

# 警告
现在密码是明文保存，具体存在哪跟平台有关。用的QSettings存的，详情请参见官方文档：[https://doc.qt.io/qt-5/qsettings.html#platform-specific-notes](https://doc.qt.io/qt-5/qsettings.html#platform-specific-notes)

# 功能对比
| 功能                 | 官方 | 本版 | 说明                                                             |
|----------------------|------|------|------------------------------------------------------------------|
| 记住密码 自动登录    | √    | √    |                                                                  |
|      <<<已知问题>>>    |      |       |                                          |
| 多语言支持         | √    |      | 或许不会改进了...                                                  |
| 密文保存密码         | √    |      | 待改进                                                           |
| 被顶掉               | √    |      | 警告！巨大缺陷！本版掉线后会自动重启重新登录！所以顶不掉！待改进 |
| 释放socket           | √    |      | 不是每次关机前都能保证释放socket，导致有时候会报端口已占用错误，待改进 |
|      <<<优势>>>    |      |       |                                          |
| 打开速度             | 慢   | 快   | 我也不知道为什么官版打开那么慢                                   |
| 单实例               |      | √    | 开机自启慢的话可以直接打开不会报错说已经在运行                   |
| 快速注销             |      | √    | 官方版是真·注销，本版是直接关闭socket，所以不需要等20s的发包周期 |
| 托盘图标无bug          |      | √    | 不知道你们有没有碰到过官方win版托盘有俩图标的bug                 |
| 可选不弹出校园网之窗 |      | √    |                                                                  |
| 完全隐藏登录窗口     |      | √    |                                                                  |
| 适配高分屏           |      | √    |                                                                  |
| 快速重启客户端       |      | √    | 有时候重启功能不好使，点了重启当前退了没有蹦出来新的，待改进     |
| win版不需要管理员    |      | √    |                                                                  |
| linux版最小化到托盘  |      | √    |                                                                  |
| linux版不需要root    |      | √    |                                                                  |
| 不限制NAT           |      | √    | 并不支持有违校方意愿的做法，请自行承担后果                           |

# 注意事项
- 掉线后客户端自动重启重连尝试三次。自动重启登录成功后不弹窗口只最小化到托盘。注：自动重启功能依赖于“记住我”选项的勾选，否则没有账户密码自行重启也并没有什么用
- 连接JLU.PC登录的时候mac地址随便填就可以，或者随便选一个网卡也可以，只有有线网要求mac地址和网络中心的一致

# 截图
> WIN:

![n9c6aQ.png](https://s2.ax1x.com/2019/09/02/n9c6aQ.png)

> UBUNTU:

![nCtJ2Q.png](https://s2.ax1x.com/2019/09/02/nCtJ2Q.png)

> Ubuntu 18不显示托盘图标的bug的解决方案：
> [https://askubuntu.com/questions/1056226/ubuntu-budgie-18-04-lts-system-tray-icons-not-all-showing](https://askubuntu.com/questions/1056226/ubuntu-budgie-18-04-lts-system-tray-icons-not-all-showing)

# 感谢

**图标作者**
> [https://github.com/lyj3516](https://github.com/lyj3516)

**jlu的drcom协议细节**
> [https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md](https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md)

**唯一实例**
> [https://github.com/itay-grudev/SingleApplication](https://github.com/itay-grudev/SingleApplication)

# 特别感谢
**登录部分复制了jlu部分代码**
> [https://github.com/mchome/dogcom](https://github.com/mchome/dogcom)

# 许可证

[GNU Affero General Public License v3.0](https://github.com/code4lala/drcom-jlu-qt/blob/master/LICENSE)
