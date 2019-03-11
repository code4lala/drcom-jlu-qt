# drcom-jlu-qt
drcom for jlu in qt cross platform


跨平台 **win linux mac**


win和linux测试稳定可用（mac尚未测试


# 已实现的功能：  #


1. 自动识别mac地址 （可手动指定mac地址
2. **最小化到托盘** （linux也可以最小化到托盘啦
3. 单实例
4. 记住密码
5. 自动登录
6. 

# 截图： #
WIN:

![APpDIS.png](https://s2.ax1x.com/2019/03/11/APpDIS.png)

UBUNTU:



# 待实现的功能： #


1. 密码加密保存 （打算放弃了
2. 喵喵喵？

**注：**
现在密码是明文保存，具体存在哪跟平台有关。用的QSettings存的，详情请参见官方文档：[https://doc.qt.io/qt-5/qsettings.html](https://doc.qt.io/qt-5/qsettings.html "https://doc.qt.io/qt-5/qsettings.html")

# 感谢： #


**jlu的drcom协议细节**

[https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md](https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md "https://github.com/drcoms/jlu-drcom-client/blob/master/jlu-drcom-java/jlu-drcom-protocol.md")


**唯一实例**

[https://github.com/itay-grudev/SingleApplication](https://github.com/itay-grudev/SingleApplication "https://github.com/itay-grudev/SingleApplication")


# 特别感谢： #


[https://github.com/mchome/dogcom](https://github.com/mchome/dogcom "https://github.com/mchome/dogcom")


提取出来jlu部分的代码改动到此项目中


# 许可证： #


GNU Affero General Public License v3.0