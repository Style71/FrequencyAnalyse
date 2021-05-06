# ProtocolParser 类使用说明

## 功能描述

- `ProtocolParser`类实现了对满足`FrequencyAnalyseV3.0`协议规范的数据流的**连接**、**解析**与**校验**;
- 封装了报文发送函数，可以方便地使用`ProtocolParser`类接口函数，**发送**满足协议规范的报文

## 函数接口

- ```java
  public ProtocolParser();
  ```
  类构造函数，程序开始时创建类的实例即可:
  
  ```java
  ProtocolParser parser = new ProtocolParser();
  ```
  
  一个数据流可以同时feed多个解析类，但一个解析类只用于一个数据流的解析。
  
- ```java
  public byte ParsingMessage(byte[] msg, int len) 
  ```
  数据输入流解析函数。该函数获取数据流输入