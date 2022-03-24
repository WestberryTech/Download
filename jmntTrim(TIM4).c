/**
 * @file jmntTrim.c
 * @brief
 * @author Joy (chang.li@westberrytech.com)
 * @version 1.0
 * @date 2022-01-12
 *
 * @copyright Copyright (c) 2022 Westberry Technology (ChangZhou) Corp., Ltd
 *
 */

#include <string.h>
#include <stdio.h>

#include "jmntTrim.h"

typedef struct {
  uint32_t TimeInterval; /* The time interval between two SOF.*/
  uint8_t RawValueTrim;  /* Trim value after power-on. */
  uint8_t RangTrim;      /* Trim range of adjustment. */
  uint8_t IsCompleted;   /* Adjust the complete. */
} jmntTrim_s;

#define ANCTL_MHSICR  (*((volatile uint32_t *)(0x40010428)))

static volatile jmntTrim_s jmntTrim;
static volatile uint32_t CntValueLast;

static void TIM4_Init(void);
static void startTrim(void);

/**
 * @brief Initialize the MHSI calibration funcation.
 *        It must be called after the main clock source is configured as MHSI.
 */
void jmntTrimInit(void) {

  CntValueLast = 0;
  jmntTrim.IsCompleted = 0;
  jmntTrim.TimeInterval = 0;
  jmntTrim.RangTrim = ADJUST_RANG;
  jmntTrim.RawValueTrim = (ANCTL_MHSICR >> 20) & 0xFF;
  SystemCoreClockUpdate();
  TIM4_Init();
}

/**
 * @brief TIM4 Initialization funcation.
 */
static void TIM4_Init(void) {
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_BMX1 | RCC_APB1Periph_TIM4, ENABLE);

  TIM_TimeBaseStructure.TIM_Period = 0xFFFFF;
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

  TIM_Cmd(TIM4, ENABLE);
}

/**
 * @brief Check whether the interval is valid.
 */
void CheckTune(void) {
  uint32_t CntValue = TIM4->CNT;

  if (!CntValueLast) {
    CntValueLast = CntValue;
  }
  else if (!jmntTrim.IsCompleted) {
    jmntTrim.TimeInterval = (CntValue - CntValueLast) & 0xFFFFF;

    if ((jmntTrim.TimeInterval > INR_HEAD) && (jmntTrim.TimeInterval < INR_TAIL)) {
      startTrim();
    }

    CntValueLast = CntValue;
  }
}

/**
 * @brief Start the calibration funcation.
 */
static void startTrim(void) {
  uint32_t osctune = (ANCTL_MHSICR >> 20) & 0xFF;
  uint32_t WritenVal = ANCTL_MHSICR & 0xF00FFFFF;

  if (osctune > jmntTrim.RawValueTrim) {
    if ((osctune - jmntTrim.RawValueTrim) > jmntTrim.RangTrim) {
      return;
    }
  }
  else {
    if ((jmntTrim.RawValueTrim - osctune) > jmntTrim.RangTrim) {
      return;
    }
  }

  if (jmntTrim.TimeInterval < ((SystemCoreClock / 1000) - OSC_RANG)) {
    if (osctune >= 0xFF) {
      return;
    }

    /* Unlocks write to ANCTL registers */
    PWR->ANAKEY1 = 0x03;
    PWR->ANAKEY2 = 0x0C;
    WritenVal |= ((osctune + 1) << 20);
    ANCTL_MHSICR = WritenVal;
  }
  else if (jmntTrim.TimeInterval > ((SystemCoreClock / 1000) + OSC_RANG)) {
    if (osctune <= 0x00) {
      return;
    }

    /* Unlocks write to ANCTL registers */
    PWR->ANAKEY1 = 0x03;
    PWR->ANAKEY2 = 0x0C;
    WritenVal |= ((osctune - 1) << 20);
    ANCTL_MHSICR = WritenVal;
  }

  if (osctune == ((ANCTL_MHSICR >> 20) & 0xFF)) {
    jmntTrim.IsCompleted = 1;
    TIM_Cmd(TIM4, DISABLE);
  }

#ifdef OSC_DEBUG
  static uint32_t send_count = 0;
  printf("%d, ANCTL_MHSICR = 0x%07X, osctune = 0x%02X,%d\n",
         send_count++,
         ANCTL_MHSICR, (ANCTL_MHSICR >> 20) & 0xFF,
         (ANCTL_MHSICR >> 20) & 0xFF);
#endif
}
