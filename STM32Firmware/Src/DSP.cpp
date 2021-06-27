#include "DSP.h"
#include "arm_math.h"
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "SysTime.h"
#include "process.h"

#define MAX_SAMPLE_POINTS 4096
#define STAGE_NUM 6

#define FREQWAVE_INIT(_name, _fs, _sample_points, _lower, _upper)        \
  float _name##_buffer[_sample_points];                                  \
  FreqWave _name =                                                       \
      {                                                                  \
        sample : _name##_buffer,                                         \
        tail : 0,                                                        \
        sample_freq : _fs,                                               \
        n : _sample_points,                                              \
        deltaT : (_sample_points / (float)_fs),                          \
        upperbound : (uint16_t)((_upper / (float)_fs) * _sample_points), \
        lowerbound : (uint16_t)((_lower / (float)_fs) * _sample_points)  \
      }

const float Gain[STAGE_NUM] = {0.01043620054, 0.009894934483, 0.009464552626, 0.009154595435, 0.00896828156, 0.09437258542};
const float NUM[STAGE_NUM][3] = {
    {1, 2, 1},
    {1, 2, 1},
    {1, 2, 1},
    {1, 2, 1},
    {1, 2, 1},
    {1, 1, 0}};
const float DEN[STAGE_NUM][3] = {
    {1, -1.901244521, 0.9429892898},

    {1, -1.802637815, 0.842217505},

    {1, -1.72423172, 0.7620899677},

    {1, -1.667764425, 0.7043827772},

    {1, -1.633822203, 0.6696953177},

    {1, -0.8112547994, 0}};

arm_rfft_fast_instance_f32 S;

arm_biquad_casd_df1_inst_f32 IIRFilterS;
static float Coeffs[5 * STAGE_NUM];
static float initState[4 * STAGE_NUM] = {0};

// !!! Caution: for the arm DSP fft function to work properly, the sample points must be 4096 \ 2048 \ 1024 \ 512 \ 256 \ 64.
// Meanwhile, testOutput_f32[MAX_SAMPLE_POINTS + 2] is used for temperarily storing the FFT data, thus the sample points mustn't be
// greater than MAX_SAMPLE_POINTS.
FREQWAVE_INIT(signal_400Hz_freq, SAMPLE_FREQ, 512, 180, 420);
FREQWAVE_INIT(signal_100Hz_freq, 400, 1024, 50, 120);
FREQWAVE_INIT(signal_35Hz_freq, 400, 4096, 10, 40);

float fmag[3] = {5, 15, 50};
float Vmag[3] = {0.2, 0.12, 0.09};
float fderivationfreq[3] = {0.0025, 0.01, 0.2};
float basefreq[3] = {30, 100, 300};

//float filteredOutput[1024];
bool channelEnable[3] = {true, true, true};
bool virtualVal = true;

#define SCALE_ADC_12BIT_CURRENT_INTEGRAL_TO_MAH (ADC_SCALE_BITS_TO_VOLT * SCALE_IMON_VOLT_TO_I_MA / (SAMPLE_FREQ * 3600.0))
BatteryStatus BattStatus;
static double BattEstTotalCapacity = 450; // Battery estimated capacity in mAh.

static bool isInRun = false;

extern int numOfSample;
extern int sample_cnt;
extern float originalInput[MAX_SAMPLE_NUM];
extern int dumpChannel;
extern PrintState stage;

void calculate_max_amp_freq(FreqWave *pFreqwave)
{
  WavePara wave;
  float fftOutput_f32[MAX_SAMPLE_POINTS + 2];

  int N = pFreqwave->n;
  if (arm_rfft_fast_init_f32(&S, N) == ARM_MATH_ARGUMENT_ERROR)
  {
    Error_Handler();
  }
  // temp = { real[0], real[(N/2)], real[1], imag[1], real[2], imag[2] ... real[(N/2)-1], imag[(N/2)-1] }
  arm_rfft_fast_f32(&S, pFreqwave->sample, fftOutput_f32, 0);
  fftOutput_f32[N] = fftOutput_f32[1];
  fftOutput_f32[N + 1] = 0;
  fftOutput_f32[1] = 0;

  /* Process the data through the Complex Magnitude Module for
  calculating the magnitude at each bin */
  N = pFreqwave->upperbound - pFreqwave->lowerbound + 1;
  arm_cmplx_mag_f32(&fftOutput_f32[pFreqwave->lowerbound * 2], fftOutput_f32, N);

  uint32_t binIndex;
  /* Calculates maxValue and returns corresponding BIN value */
  arm_max_f32(fftOutput_f32, N, &wave.mag, &binIndex);
  wave.mag *= (2.0 / pFreqwave->n);
  wave.freq = ((binIndex + pFreqwave->lowerbound) * pFreqwave->sample_freq) / pFreqwave->n;
  wave.t = GetUs();
  pFreqwave->freq.brute_push_back(wave);
}

float virtualValGenerator()
{
  static int n[3] = {0, 0, 0};
  float val;
  static float phy[3] = {0, 0, 0};
  static float fderivation[3] = {0, 0, 0};
  const int nMax[3] = {5 * SAMPLE_FREQ, 100 * SAMPLE_FREQ, 400 * SAMPLE_FREQ};

  for (int i = 0; i < 3; i++)
  {
    fderivation[i] = fmag[i] * arm_sin_f32(PI2 * fderivationfreq[i] * n[i] / SAMPLE_FREQ);
    phy[i] += PI2 * (basefreq[i] + fderivation[i]) / SAMPLE_FREQ;
    if (phy[i] >= PI2)
      phy[i] -= PI2;

    n[i]++;
    if (n[i] >= nMax[i]) // This value is calculated by the inverse of fderivationfreq[3].
      n[i] = 0;
  }

  val = Vmag[0] * arm_sin_f32(phy[0]) +
        Vmag[1] * arm_sin_f32(phy[1]) +
        Vmag[2] * arm_sin_f32(phy[2]);

  return val;
}

void signal_downsampling()
{
  static bool ADCBufferState = false;
  static int down_sampling_count[2] = {0};
  static uint64_t intCurrent = 0;

  if ((getCurrentADCBuffer() != ADCBufferState) && (isInRun))
  {
    ADCBufferState = getCurrentADCBuffer();
    //USART_Printf(&huart2, "Current buffer: %hhu", (uint8_t)ADCBufferState);
    uint16_t *currentBuffer = (ADCBufferState) ? ADCResult0 : ADCResult1;

    float val;
    for (int i = 0; i < ADC_BUFFER_SIZE; i++)
    {
      // Convert the signal value.
      if (i % 3 == 0)
      {
        val = virtualValGenerator();
        if (!virtualVal)
          val = currentBuffer[i] * ADC_SCALE_BITS_TO_VOLT;

        if ((stage == WaitSample) && (dumpChannel == 1) && (sample_cnt < numOfSample))
        {
          originalInput[sample_cnt++] = val;
          //filteredOutput[oneshoot_count++] = filtered_200Hz_cutoff;
        }

        for (int j = 0; j < 2; j++)
          down_sampling_count[j]++;

        signal_400Hz_freq.sample[signal_400Hz_freq.tail++] = val;
        if (signal_400Hz_freq.tail >= signal_400Hz_freq.n)
        {
          signal_400Hz_freq.tail = 0;
          if (channelEnable[0])
            calculate_max_amp_freq(&signal_400Hz_freq);
        }

        float filtered_200Hz_cutoff;
        arm_biquad_cascade_df1_f32(&IIRFilterS, &val, &filtered_200Hz_cutoff, 1);

        // SAMPLE_FREQ / .freq
        if (down_sampling_count[0] >= (10))
        {
          down_sampling_count[0] = 0;
          signal_100Hz_freq.sample[signal_100Hz_freq.tail++] = filtered_200Hz_cutoff;

          if (signal_100Hz_freq.tail >= signal_100Hz_freq.n)
          {
            signal_100Hz_freq.tail = 0;
            if (channelEnable[1])
              calculate_max_amp_freq(&signal_100Hz_freq);
          }
        }
        if (down_sampling_count[1] >= (10))
        {
          down_sampling_count[1] = 0;
          signal_35Hz_freq.sample[signal_35Hz_freq.tail++] = filtered_200Hz_cutoff;
          if (signal_35Hz_freq.tail >= signal_35Hz_freq.n)
          {
            signal_35Hz_freq.tail = 0;
            if (channelEnable[2])
              calculate_max_amp_freq(&signal_35Hz_freq);
          }
        }
      }
      else if (i % 3 == 1)
      {
        BattStatus.voltage = (uint16_t)(currentBuffer[i] * ADC_SCALE_BITS_TO_VOLT * SCALE_VMON_VOLT_TO_VIN_MV);

        if ((stage == WaitSample) && (dumpChannel == 2) && (sample_cnt < numOfSample))
        {
          *((uint16_t *)&originalInput[sample_cnt]) = BattStatus.voltage;
          sample_cnt++;
        }
      }
      else if (i % 3 == 2)
      {
        intCurrent += currentBuffer[i];
        BattStatus.current = (uint16_t)(currentBuffer[i] * ADC_SCALE_BITS_TO_VOLT * SCALE_IMON_VOLT_TO_I_MA);

        if ((stage == WaitSample) && (dumpChannel == 3) && (sample_cnt < numOfSample))
        {
          *((uint16_t *)&originalInput[sample_cnt]) = BattStatus.current;
          sample_cnt++;
        }
      }
    }
    BattStatus.t = GetUs();
    BattStatus.capacity = (1.0 - (SCALE_ADC_12BIT_CURRENT_INTEGRAL_TO_MAH * intCurrent / (BattEstTotalCapacity))) * 100.0;
  }
}

void InitFilter()
{
  // Initialise a 11-order Butterworth IIR filter with 120Hz passband and 200Hz stop frequency.
  for (int i = 0; i < STAGE_NUM; i++)
  {
    for (int j = 0; j < 3; j++)
      Coeffs[j + i * 5] = Gain[i] * NUM[i][j] / DEN[i][0];
    Coeffs[3 + i * 5] = -DEN[i][1] / DEN[i][0];
    Coeffs[4 + i * 5] = -DEN[i][2] / DEN[i][0];
  }
  arm_biquad_cascade_df1_init_f32(&IIRFilterS, STAGE_NUM, Coeffs, initState);

  isInRun = true;
}
