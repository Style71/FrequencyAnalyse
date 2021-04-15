# FrequencyAnalyseV1.0
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
- TIM2时钟源配置
- ADC配置

## 关于使用 DCache 导致的内存不一致问题
[例说STM32F7高速缓存——Cache一致性问题（二）](https://blog.csdn.net/lu_embedded/article/details/78437778)

[例说STM32F7高速缓存——Cache一致性问题（三）](https://blog.csdn.net/lu_embedded/article/details/78439643)

[【STM32H7教程】第24章 STM32H7的Cache解读（非常重要）](https://www.cnblogs.com/armfly/p/11008913.html)

[实战经验|STM32F7 MPU Cache浅析 ](https://www.sohu.com/a/154296763_505803)