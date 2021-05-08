# FrequencyAnalyse
Analysing three frequency components in an anolog signal and transmitting to host using Bluetooth-UART.



## How to build this project

- Win64 platform

  Make sure the gnu-arm toolchain (`arm-none-eabi-g++` and `arm-none-eabi-gdb`) and `mingw32-make` is in your system path. You can type the following command to verify it:

  ```
  arm-none-eabi-g++ -v
  arm-none-eabi-gdb -v
  mingw32-make -v
  ```

- Ubuntu 20.04.2 LTS 

  Make a slight change to `.vscode/tasks.json` file, change "command" parameter to:

  ```json
  "command": "make"
  ```

  Then you can use vscode shortcuts `Ctrl+Shift+B` to build the project. Modify clean command at the end of `Makefile` file for linux style file deletion.

  ```makefile
  clean:
  	rm -rf $(BUILD_DIR)/*.d $(BUILD_DIR)/*.o $(BUILD_DIR)/*.lst
  ```

  If you installed **openocd** using `apt-get` command, then the **openocd** configuration file is located in `/usr/share/openocd/scripts` by default.

  Make changes to `.vscode/c_cpp_properties.json` file, change "intelliSenseMode" and "compilerPath" parameter to approprate value:
  
  ```json
  "intelliSenseMode": "linux-gcc-arm",
  "compilerPath": "/opt/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-g++",
  ```

  If you are not sure where your arm gcc compiler is, you can type the following command to find the path:
  
  ```
  whereis arm-none-eabi-g++
  ```

## TODO list
- ~~TIM2时钟源配置~~
- ~~ADC配置~~
  
  ```cpp
  hadc1.Init.ScanConvMode = ADC_SCAN_ENABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 3;
  hadc1.Init.DMAContinuousRequests = ENABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
  ```
- 检查初始化GPIO pin BT_AT_Pin时，应该赋以高电平还是低电平

## 测试 list

- ~~观察报文发送频率，以检查TIM2采样时钟是否配置正确~~
- 用Debug模式观察ADCBuffer数值，检查ADC采样设置是否正确；解释电压几百mV波动的原因；
- 解决蓝牙连接问题

## 关于使用 DCache 导致的内存不一致问题
[例说STM32F7高速缓存——Cache一致性问题（二）](https://blog.csdn.net/lu_embedded/article/details/78437778)

[例说STM32F7高速缓存——Cache一致性问题（三）](https://blog.csdn.net/lu_embedded/article/details/78439643)

[【STM32H7教程】第24章 STM32H7的Cache解读（非常重要）](https://www.cnblogs.com/armfly/p/11008913.html)

[实战经验|STM32F7 MPU Cache浅析 ](https://www.sohu.com/a/154296763_505803)

| Name | Capacity| Start Address | End Address |
| -------- | --------- | ----------- |----------- |
| ITCM RAM | 16Kbytes | 0x0000 0000 |0x0003 FFFF |
| DTCM RAM | 128Kbytes | 0x2000 0000 |0x2001 FFFF |
| SRAM1 | 368Kbytes | 0x2002 0000 |0x2007 BFFF |
| SRAM2 | 16Kbytes | 0x2007 C000 |0x2008 0000 |

| Name      | Start Address | End Address | Start Address | End Address | Capacity |
| --------- | ------------- | ----------- | ------------- | ----------- | -------- |
| Sector 0  | 0x0800 0000   | 0x0800 7FFF | 0x0020 0000   | 0x0020 7FFF | 32       |
| Sector 1  | 0x0800 8000   | 0x0800 FFFF | 0x0020 8000   | 0x0020 FFFF | 32       |
| Sector 2  | 0x0801 0000   | 0x0801 7FFF | 0x0021 0000   | 0x0021 7FFF | 32       |
| Sector 3  | 0x0801 8000   | 0x0801 FFFF | 0x0021 8000   | 0x0021 FFFF | 32       |
| Sector 4  | 0x0802 0000   | 0x0803 FFFF | 0x0022 0000   | 0x0023 FFFF | 128      |
| Sector 5  | 0x0804 0000   | 0x0807 FFFF | 0x0024 0000   | 0x0027 FFFF | 256      |
| Sector 6  | 0x0808 0000   | 0x080B FFFF | 0x0028 0000   | 0x002B FFFF | 256      |
| Sector 7  | 0x080C 0000   | 0x080F FFFF | 0x002C 0000   | 0x002F FFFF | 256      |
| Sector 8  | 0x0810 0000   | 0x0813 FFFF | 0x0030 0000   | 0x0033 FFFF | 256      |
| Sector 9  | 0x0814 0000   | 0x0817 FFFF | 0x0034 0000   | 0x0037 FFFF | 256      |
| Sector 10 | 0x0818 0000   | 0x081B FFFF | 0x0038 0000   | 0x003B FFFF | 256      |
| Sector 11 | 0x081C 0000   | 0x081F FFFF | 0x003C 0000   | 0x003F FFFF | 256      |