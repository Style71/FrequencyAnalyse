# FrequencyAnalyseV3.0

## How to build this project

- Prerequisite

  `arm-none-none-g++` version $\ge9.3.1$ 

  `arm-none-eabi-gdb`
  
  `make`(or `mingw32-make` in Windows)
  
  You can type the following command to check if gnu-arm toolchain (`arm-none-eabi-g++` and `arm-none-eabi-gdb`) and `make` is in your system path.:
  
  ```bash
  arm-none-eabi-g++ -v
  arm-none-eabi-gdb -v
  make -v
  ```
  
  For Windows:
  
  ```bash
  mingw32-make -v
  ```
  
- Build

  You can direct to /STM32Firmware/ directory and build the firmware using `make`(or `mingw32-make` in Windows) command. Alternately, you can use VScode shortcuts `Ctrl+Shift+B` to build the project, before that, make sure the "command" parameter in `.vscode/tasks.json` file is configured properly for your platform. For Ubuntu 20.04.2 LTS:

  ```json
  "command": "make"
  ```

  For Win64 platform:

  ```json
  "command": "mingw32-make"
  ```

- Edit
  If you want to use intelliSense when coding, make changes to `.vscode/c_cpp_properties.json` file, change "intelliSenseMode" and "compilerPath" parameter to appropriate value. For my Ubuntu platform, it is:

  ```json
  "intelliSenseMode": "linux-gcc-arm",
  "compilerPath": "/opt/gcc-arm-none-eabi-9-2020-q2-update/bin/arm-none-eabi-g++",
  ```

  If you are not sure where your arm gcc compiler is, you can type the following command in Ubuntu shell to find the path:

  ```
  which arm-none-eabi-g++
  ```

- Debug

  The project use `openocd` for 

  If you installed **openocd** using `apt-get` command, then the **openocd** configuration file is located in `/usr/share/openocd/scripts` by default.
  
  
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
- ~~检查初始化GPIO pin BT_AT_Pin时，应该赋以高电平还是低电平~~

## 测试 list

- ~~观察报文发送频率，以检查TIM2采样时钟是否配置正确~~
- ~~用Debug模式观察ADCBuffer数值，检查ADC采样设置是否正确；~~解释电压上百mV波动的原因；
- ~~解决蓝牙连接问题~~

## Troubleshooting

- 蓝牙芯片CH9143在上电后，MCU不能立即对蓝牙芯片发送数据，需要延时5s后再开始发送（蓝牙芯片固件问题，厂商设计人员已知晓，后续版本可能会修复）
- 蓝牙芯片在同时连接USB主机和蓝牙主机时，USB主机若不开启USB串口，则蓝牙主机不会收到任何信息（蓝牙芯片厂商有意如此设计）
- MCU在上电后第一次运行时无法响应接收到的串口数据，需要RESET后才能正常工作。一开始怀疑是上电时部分供电不稳定导致硬件初始化出现问题。在`SystemClock_Config()`之后引入了一个5000ms的延时，问题有所缓解。
