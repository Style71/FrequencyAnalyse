# FrequencyAnalyse

## About

Analyzing three frequency components in an analog signal and transmitting to host using Bluetooth-UART.

This project consists of five independent directories:

- `STM32Firmware`, the firmware of the STM32F767 MCU on the PCB/fPCB, which analyzing three frequency components in an analog signal using FFT, and transmitting the frequency info to host using Bluetooth-UART. The project is a mixture of C, C++ and Assembly language, and it is configured/edited/built/debugged using VScode IDE and build with `make`, see `STM32Firmware/README.md` for detailed description.
- `Hardware`, the PCB projects of the PCB/fPCB board. There are two subdirectories in this directory, `Hardware/SignalAnalyseV1.0` is the PCB project of prototype board, which is used for circuit validation and firmware development; `Hardware/SignalAnalyseV3.0.pdf` is the PCB project of the final fPCB board.
- `JavaProgram`, the Java project for the validation of the communication protocol parsing algorithm, see `JavaProgram/README.md` for detailed description.
- `Simulation`, Multisim simulation project of the envelope detector and bandpass filter circuit.
- `AndroidProject`, the Android program for time-frequency graph plotting and channel enable.
- `Doc`, documents about the signal processing algorithm used in this project, about the code structure and third-party API used in the firmware, about the PCB design consideration and circuit simulation results, and some technical notes during coding.

## Code download

There are two branches in this git repository:

- `master`-- Firmware for SignalAnalyseV1.0 prototype board.
- `V3.0`-- Firmware for SignalAnalyseV3.0 fPCB board, this branch is the final version of the entire project, use this branch as default. 

Use the following command to clone the project to your local host.

```bash
git clone https://github.com/Style71/FrequencyAnalyse.git
git checkout V3.0
git submodule update --init --recursive
```
