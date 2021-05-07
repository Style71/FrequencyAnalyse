# 协议解析器类用法

位置：`Message.cpp: ProtocolStream`

- 先根据你的程序框架，修改以下函数的具体实现：

  ```cpp
  void putchars(const char *pucArray, int size);
  void recv_frequency_info(uint8_t channel, WavePara &wave);
  void recv_battery_info(BatteryStatus &battery);
  ```

  其中`putchars`实现将指定数量的bytes通过蓝牙发送给下位机；`recv_frequency_info`实现你期望的收到频率报文后的程序行为；`recv_battery_info`实现你期望的收到电池状态报文后的程序行为。

- 将接收到的数据Bytes流输入给函数：

  ```cpp
  uint8_t ParsingMessage(uint8_t *msg, uint8_t len);
  ```

大功告成！

`type.h`文件定义了程序使用到的基本数据结构；

`DataStructure.cpp`/`DataStructure.h`实现了一个队列模板类，用于`ProtocolStream`类中的FIFO。你可以用Java的标准类替换。