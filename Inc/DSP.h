#ifndef DSP_H_
#define DSP_H_

#include "DataStructure.h"

#define NPT 1024
#define Fs 1280 //Sample frequency 5120Hz
#define PI2 6.28318530717959

typedef struct _WavePara
{
    float freq;
    float mag;
    uint32_t t;
    _WavePara()
    {
        freq = 0;
        mag = 0;
        t = 0;
    }
} WavePara;

typedef struct
{
    Queue<WavePara, FREQWAVE_BUFFER_SIZE> freq; // FFT max bin frequency time series.
    float *sample;                              // Pointer of the samples.
    uint16_t tail;                              // Tail index of sample.
    float sample_freq;                          // Actual input signal sample frequency for FFT.
    uint16_t n;                                 // Sample points.
    float deltaT;                               // Time window of input signal used for FFT, the frequency precision is 1/deltaT(aka sample_freq/n).
    uint16_t upperbound;                        // Upper bound index of the main frequency of the input signal.
    uint16_t lowerbound;                        // Lower bound index of the main frequency of the input signal.
} FreqWave;

typedef struct _BattStatus
{
    uint32_t t;       // System time in us.
    uint16_t voltage; // Battery voltage in mV.
    uint16_t current; // System current in mA.
    double capacity;  // Battery capacity in percentage.
    _BattStatus()
    {
        t = 0;
        voltage = 0;
        current = 0;
        capacity = 100;
    }
} BatteryStatus;
//extern float testOutput[NPT];
//void arm_rfft_fast_f32_app(void);

void signal_downsampling();

enum PrintState
{
    Normal = 0,
    WaitSample = 1,
    DumpSample = 2,
    AfterDump = 3
};
extern bool channelEnable[3];
#define PRINTLOOP_FREQ 500
void PrintLoop();

void InitFilter();

#endif