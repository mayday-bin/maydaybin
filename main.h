#ifndef __MAIN_H__
#define __MAIN_H__

#include "m_global.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "bsp_uart.h"
#include "bsp_tim.h"
#include "bsp_rtc.h"
#include "FreeRTOS.h"
#include "task.h"
#include "bsp_app.h"
#include "queue.h"

#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
  // 任务优先级规划 5-9 数字越大优先级越高
  #define EG810MTASK_PRIO                         9
  #define DPCRecvTask_PRIO                        8
  #define DPCDispatchTask_PRIO                    7
  #define AutoCollectTask_PRIO                    6
  #define DPCSendTask_PRIO                        5

  // 任务堆栈大小
  #define TASK_SIZE                               0x200

  /* 通信协议消息处理部分 */
  #define MAX_UP_MESSAGES                         256                         // 上行消息队列长度
  #define MAX_DOWN_MESSAGES                       12                          // 下行消息队列长度
#elif MOD_SFT_TYPE==MOD_TYPE_LORAMESH
  #define LoraMesh_Task_Prio                      9

  #define LoraMesh_Task_Size                      0x200
#endif

/***************************************************/

#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
  extern QueueHandle_t g_pUpMsgQueen;                      // 上下行消息句柄
  extern QueueHandle_t g_pDownMsgQueen;
#endif

extern const char McuVersion[];

void IWDG_Feed(void);

#endif
