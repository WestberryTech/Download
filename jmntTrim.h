/**
 * @file jmntTrim.h
 * @brief 
 * @author Joy (chang.li@westberrytech.com)
 * @version 1.0
 * @date 2022-01-12
 * 
 * @copyright Copyright (c) 2022 Westberry Technology (ChangZhou) Corp., Ltd
 * 
 */

#ifndef __JMNTTRIM_H
#define __JMNTTRIM_H

#include "wb32f10x.h"

/* The range of sof frame interval TIMER counts. */
#define INR_HEAD 44500
#define INR_TAIL 51500

/* Register adjustment range. */
#define ADJUST_RANG 10
/* Frequency accuracy range. us */
#define OSC_RANG 300


extern void jmntTrimInit(void);
extern void CheckTune(void);

#endif /* __JMNTTRIM_H */
