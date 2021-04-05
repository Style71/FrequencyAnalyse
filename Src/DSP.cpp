#include "DSP.h"
#include "arm_math.h"
#include "main.h"
#include "adc.h"
#include "usart.h"
#include "SysTime.h"

#define MAX_SAMPLE_POINTS 4096
#define STAGE_NUM 6

#define FREQWAVE_INIT(_name, _fs, _sample_points, _lower, _upper)           \
  float _name##_buffer[_sample_points];                                     \
  FreqWave _name =                                                          \
      {                                                                     \
          .sample = _name##_buffer,                                         \
          .tail = 0,                                                        \
          .sample_freq = _fs,                                               \
          .n = _sample_points,                                              \
          .deltaT = (_sample_points / (float)_fs),                          \
          .upperbound = (uint16_t)((_upper / (float)_fs) * _sample_points), \
          .lowerbound = (uint16_t)((_lower / (float)_fs) * _sample_points), \
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

float testOutput_f32[MAX_SAMPLE_POINTS + 2];

// !!! Caution: for the arm DSP fft function to work properly, the sample points must be 4096 \ 2048 \ 1024 \ 512 \ 256 \ 64.
// Meanwhile, testOutput_f32[MAX_SAMPLE_POINTS + 2] is used for temperarily storing the FFT data, thus the sample points mustn't be
// greater than MAX_SAMPLE_POINTS.
FREQWAVE_INIT(signal_400Hz_freq, SAMPLE_FREQ, 512, 180, 420);
FREQWAVE_INIT(signal_100Hz_freq, 400, 1024, 50, 120);
FREQWAVE_INIT(signal_35Hz_freq, 400, 4096, 10, 40);

float mag1[3] = {1, 2, 3};
float mag2[3] = {0.2, 0.12, 0.09};
float freq1[3] = {50, 256, 600};
float freq2[3] = {30, 100, 400};

#define ONESHOOT_SIZE 40960
float originalInput[ONESHOOT_SIZE];
//float filteredOutput[1024];
int oneshoot_count = ONESHOOT_SIZE;
PrintState stage = Normal;
bool channelEnable[3] = {true, true, true};

static bool isInRun = false;

void calculate_max_amp_freq(FreqWave *pFreqwave)
{
  WavePara wave;

  int N = pFreqwave->n;
  if (arm_rfft_fast_init_f32(&S, N) == ARM_MATH_ARGUMENT_ERROR)
  {
    Error_Handler();
  }
  // temp = { real[0], real[(N/2)], real[1], imag[1], real[2], imag[2] ... real[(N/2)-1], imag[(N/2)-1] }
  arm_rfft_fast_f32(&S, pFreqwave->sample, testOutput_f32, 0);
  testOutput_f32[N] = testOutput_f32[1];
  testOutput_f32[N + 1] = 0;
  testOutput_f32[1] = 0;

  /* Process the data through the Complex Magnitude Module for
  calculating the magnitude at each bin */
  N = pFreqwave->upperbound - pFreqwave->lowerbound + 1;
  arm_cmplx_mag_f32(&testOutput_f32[pFreqwave->lowerbound * 2], testOutput_f32, N);

  uint32_t binIndex;
  /* Calculates maxValue and returns corresponding BIN value */
  arm_max_f32(testOutput_f32, N, &wave.mag, &binIndex);
  wave.mag *= (2.0 / pFreqwave->n);
  wave.freq = ((binIndex + pFreqwave->lowerbound) * pFreqwave->sample_freq) / pFreqwave->n;
  wave.t = GetUs();
  pFreqwave->freq.brute_push_back(wave);
}

void signal_downsampling()
{
  static bool ADCBufferState = false;
  static int n = 0;
  static int down_sampling_count[2] = {0};

  if ((getCurrentADCBuffer() != ADCBufferState) && (isInRun))
  {
    ADCBufferState = getCurrentADCBuffer();
    //USART_Printf(&huart2, "Current buffer: %hhu", (uint8_t)ADCBufferState);
    uint16_t *currentBuffer = (ADCBufferState) ? ADCResult0 : ADCResult1;

    for (int i = 0; i < ADC_BUFFER_SIZE; i++)
    {
      // Convert the signal value.
      float val = currentBuffer[i] * 8.056640625e-4;
      /*val = mag2[0] * arm_sin_f32(PI2 * n * freq2[0] / SAMPLE_FREQ) +
            mag2[1] * arm_sin_f32(PI2 * n * freq2[1] / SAMPLE_FREQ) +
            mag2[2] * arm_sin_f32(PI2 * n * freq2[2] / SAMPLE_FREQ);
      n++;
      if (n > 400)
        n = 0;*/

      if (oneshoot_count < ONESHOOT_SIZE)
      {
        originalInput[oneshoot_count++] = val;
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
  }
}

void PrintLoop()
{
  static int print_cnt = 0;
  static int wait_cnt = 0;
  WavePara para;

  switch (stage)
  {
  case Normal:
    if (!signal_400Hz_freq.freq.isEmpty())
    {
      para = signal_400Hz_freq.freq.pop_front();
      if (channelEnable[0])
      {
        USART_Printf(&huart2, "f1 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_400Hz_freq.deltaT, para.mag * 1000);
        USART_Printf(&huart1, "f1 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_400Hz_freq.deltaT, para.mag * 1000);
      }
    }
    if (!signal_100Hz_freq.freq.isEmpty())
    {
      para = signal_100Hz_freq.freq.pop_front();
      if (channelEnable[1])
      {
        USART_Printf(&huart2, "f2 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_100Hz_freq.deltaT, para.mag * 1000);
        USART_Printf(&huart1, "f2 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_100Hz_freq.deltaT, para.mag * 1000);
      }
    }
    if (!signal_35Hz_freq.freq.isEmpty())
    {
      para = signal_35Hz_freq.freq.pop_front();
      if (channelEnable[2])
      {
        USART_Printf(&huart2, "f3 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_35Hz_freq.deltaT, para.mag * 1000);
        USART_Printf(&huart1, "f3 = (%.2fs, %.2f+-%.2fHz, %.2fmV)\r\n", para.t / 1000000.0, para.freq, 1.0 / signal_35Hz_freq.deltaT, para.mag * 1000);
      }
    }
    break;

  case WaitSample:
    if (oneshoot_count >= ONESHOOT_SIZE)
    {
      stage = DumpSample;
      print_cnt = 0;
    }
    break;

  case DumpSample:
    if (print_cnt < ONESHOOT_SIZE)
    {
      USART_Printf(&huart2, "%.4f\t", originalInput[print_cnt++]);
      USART_Printf(&huart1, "%.4f\t", originalInput[print_cnt++]);
    }
    else
      stage = AfterDump;
    break;

  case AfterDump:
    if (wait_cnt < PRINTLOOP_FREQ * 3)
      wait_cnt++;
    else
    {
      wait_cnt = 0;
      stage = Normal;
    }
    break;

  default:

    break;
  }

  /*uint64_t current = GetSysTicks();
  Time currentTime = TimeConvert(current);
  USART_Printf(&huart2, "Ticks: %llu, time: %02u:%02u:%02u.%03u%03u, timer2:%u, ", current, currentTime.ulHour, currentTime.ulMinite,
               currentTime.ulSecond, currentTime.ulMs, currentTime.ulUs, Timer3Count);
  double average = sampleCount;
  average *= (216000000);
  average /= current;
  USART_Printf(&huart2, "Buffer: %u, Transferred: %u, average:%.0f\n", (bool)(hdma_adc1.Instance->CR & DMA_SxCR_CT), 3600 - __HAL_DMA_GET_COUNTER(&hdma_adc1), average);*/
  /*static int stage = 0;
  //static bool nextRound = 0;
  if (stage < (NPT / 2 + 1))
  {
    USART_Printf(&huart2, "%i %.2f\r\n", stage, testOutput_f32[stage]);
    stage++;
  }
  else
  {
    //if (!nextRound)
    //{
      //nextRound = true;
      for (uint16_t i = 0; i < NPT; i++)
      {
        testInput_f32[i] = mag2[0] * arm_sin_f32(PI2 * i * freq2[0] / Fs) +
                           mag2[1] * arm_sin_f32(PI2 * i * freq2[1] / Fs) +
                           mag2[2] * arm_sin_f32(PI2 * i * freq2[2] / Fs);
      }
      if (arm_rfft_fast_init_f32(&S, NPT) == ARM_MATH_ARGUMENT_ERROR)
      {
        Error_Handler();
      }
      arm_rfft_fast_f32(&S, testInput_f32, testOutput_f32, ifftFlag);
      testOutput_f32[NPT] = testOutput_f32[1];
      testOutput_f32[NPT + 1] = 0;
      testOutput_f32[1] = 0;
      arm_cmplx_mag_f32(testOutput_f32, testOutput_f32, NPT / 2 + 1);
      stage = 0;
    //}
  }*/
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