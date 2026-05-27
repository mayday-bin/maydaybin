#ifndef __MAIN_H__
#define __MAIN_H__

#include "m_global.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "queue.h"

/***************************************************/

#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
  extern QueueHandle_t g_pUpMsgQueen;                      // 上下行消息句柄
  extern QueueHandle_t g_pDownMsgQueen;
#endif

extern const char McuVersion[];

void IWDG_Feed(void);
void IWDG_Init(u8 prer, u16 rlr);

#endif
