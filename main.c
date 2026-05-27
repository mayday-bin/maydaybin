#include "main.h"

#include "bsp_app.h"
#include "bsp_rtc.h"
#include "bsp_tim.h"
#include "bsp_uart.h"
#include "task.h"

#if MOD_SFT_TYPE == MOD_TYPE_PWRCTL
  #include "bsp_eg810m.h"
  #include "bsp_flash.h"
#elif MOD_SFT_TYPE == MOD_TYPE_LORAMESH
  #include "loramesh_proc.h"
#elif MOD_SFT_TYPE == MOD_TYPE_BDS
  #include "bds_proc.h"
#endif

/**************************** 任务句柄 ********************************/

static TaskHandle_t AppTaskCreate_Handle = NULL;        /* 创建任务句柄 */

#if MOD_SFT_TYPE == MOD_TYPE_PWRCTL
  static TaskHandle_t EG810M_Task_Handle = NULL;
  static TaskHandle_t DPCRecv_Task_Handle = NULL;
  static TaskHandle_t DPCDispatch_Task_Handle = NULL;
  static TaskHandle_t AutoCollect_Task_Handle = NULL;
  static TaskHandle_t DPCSend_Task_Handle = NULL;

  QueueHandle_t g_pUpMsgQueen = NULL;                   /* 上行消息句柄 */
  QueueHandle_t g_pDownMsgQueen = NULL;                 /* 下行消息句柄 */
#elif MOD_SFT_TYPE == MOD_TYPE_LORAMESH
  static TaskHandle_t LoraMesh_Task_Handle = NULL;
#elif MOD_SFT_TYPE == MOD_TYPE_BDS
  static TaskHandle_t BDS_Task_Handle = NULL;
#endif

/******************************* 版本 ************************************/

#define APP_AT(_addr) __attribute__ ((at(_addr)))
#define APP_VERSION_FLASH_ADDR (APP_VERSION_FLASH_END_ADDR - (sizeof(APP_MCU_VERSION) - 3U))

const char McuVersion[sizeof(APP_MCU_VERSION)] APP_AT(APP_VERSION_FLASH_ADDR) = APP_MCU_VERSION;

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void *argument);             /* 用于创建任务 */
static void BSP_Init(void);                            /* 用于初始化板载相关资源 */
static void AppStartupErrorTrap(void);
static BaseType_t CreateApplicationTasks(void);
static BaseType_t CreateTaskChecked(TaskFunction_t taskCode,
                                    const char *taskName,
                                    uint16_t stackDepth,
                                    UBaseType_t priority,
                                    TaskHandle_t *taskHandle);
#if MOD_SFT_TYPE == MOD_TYPE_PWRCTL
static BaseType_t CreateApplicationQueues(void);
#endif

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
/* 加了bootloader要修改对应的SCB->VTOR地址 */

int main(void)
{
    BaseType_t xReturn;

    BSP_Init();

    xReturn = xTaskCreate(AppTaskCreate,
                          "AppTaskCreate",
                          (uint16_t)APP_STARTUP_TASK_STACK_SIZE,
                          NULL,
                          (UBaseType_t)APP_STARTUP_TASK_PRIO,
                          &AppTaskCreate_Handle);
    if (xReturn != pdPASS)
    {
        AppStartupErrorTrap();
    }

    __enable_irq();                                      /* 使能全局中断 */
    vTaskStartScheduler();                               /* 启动任务，开启调度 */

    AppStartupErrorTrap();                               /* 正常不会执行到这里 */
}

/**
 * @brief 为了方便管理，所有的任务创建函数都放在这个函数里面
 */
static void AppTaskCreate(void *argument)
{
    BaseType_t xReturn;

    (void)argument;

    taskENTER_CRITICAL();
    xReturn = CreateApplicationTasks();
    taskEXIT_CRITICAL();

    if (xReturn != pdPASS)
    {
        AppStartupErrorTrap();
    }

    vTaskDelete(NULL);                                   /* 删除当前启动任务 */
}

static BaseType_t CreateApplicationTasks(void)
{
#if MOD_SFT_TYPE == MOD_TYPE_PWRCTL
    BaseType_t xReturn;

    xReturn = CreateTaskChecked((TaskFunction_t)EG810M_TASK_STK,
                                "Egm810_Task",
                                (uint16_t)APP_PWRCTL_TASK_STACK,
                                (UBaseType_t)APP_EG810M_TASK_PRIO,
                                &EG810M_Task_Handle);
    if (xReturn != pdPASS)
    {
        return xReturn;
    }

    xReturn = CreateTaskChecked((TaskFunction_t)DPCRecvTask,
                                "DPCRecv_Task",
                                (uint16_t)APP_PWRCTL_TASK_STACK,
                                (UBaseType_t)APP_DPC_RECV_TASK_PRIO,
                                &DPCRecv_Task_Handle);
    if (xReturn != pdPASS)
    {
        return xReturn;
    }

    xReturn = CreateTaskChecked((TaskFunction_t)DPCDispatchTask,
                                "DPCDispatchTask",
                                (uint16_t)APP_PWRCTL_TASK_STACK,
                                (UBaseType_t)APP_DPC_DISPATCH_PRIO,
                                &DPCDispatch_Task_Handle);
    if (xReturn != pdPASS)
    {
        return xReturn;
    }

    xReturn = CreateTaskChecked((TaskFunction_t)AutoCollectTask,
                                "AutoCollectTask",
                                (uint16_t)APP_PWRCTL_TASK_STACK,
                                (UBaseType_t)APP_AUTO_COLLECT_PRIO,
                                &AutoCollect_Task_Handle);
    if (xReturn != pdPASS)
    {
        return xReturn;
    }

    return CreateTaskChecked((TaskFunction_t)DPCSendTask,
                             "DPCSendTask",
                             (uint16_t)APP_PWRCTL_TASK_STACK,
                             (UBaseType_t)APP_DPC_SEND_TASK_PRIO,
                             &DPCSend_Task_Handle);
#elif MOD_SFT_TYPE == MOD_TYPE_LORAMESH
    return CreateTaskChecked((TaskFunction_t)LoraMeshTask,
                             "LoraMesh_Task",
                             (uint16_t)APP_LORAMESH_TASK_STACK,
                             (UBaseType_t)APP_LORAMESH_TASK_PRIO,
                             &LoraMesh_Task_Handle);
#elif MOD_SFT_TYPE == MOD_TYPE_BDS
    return CreateTaskChecked((TaskFunction_t)BDSTask,
                             "BDS_Task",
                             (uint16_t)BDS_Task_Size,
                             (UBaseType_t)BDS_Task_Prio,
                             &BDS_Task_Handle);
#endif
}

static BaseType_t CreateTaskChecked(TaskFunction_t taskCode,
                                    const char *taskName,
                                    uint16_t stackDepth,
                                    UBaseType_t priority,
                                    TaskHandle_t *taskHandle)
{
    return xTaskCreate(taskCode,
                       taskName,
                       stackDepth,
                       NULL,
                       priority,
                       taskHandle);
}

/**
 * @brief 初始化看门狗
 */
void IWDG_Init(u8 prer, u16 rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);        /* 使能对IWDG->PR IWDG->RLR的写 */
    IWDG_SetPrescaler(prer);                             /* 设置IWDG分频系数 */
    IWDG_SetReload(rlr);                                 /* 设置IWDG装载值 */
    IWDG_ReloadCounter();
    IWDG_Enable();                                       /* 使能看门狗 */
}

/* 喂独立看门狗 */
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();
}

/**
 * @brief 板级外设初始化，所有板子上的初始化均可放在这个函数里面
 */
static void BSP_Init(void)
{
    /*
    * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15。
    * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
    * 都统一用这个优先级分组，千万不要再分组，切忌。
    */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);

    USART_Configuration(USART1, APP_MODULE_USART1_BAUD); /* 模块通信 */
    USART_Configuration(USART2, APP_DEBUG_USART_BAUD);   /* 与底板通讯，可编译为调试口 */

    TIMx_Configuration(TIM2, APP_TIM2_PERIOD, APP_TIM_PRESCALER);
    TIMx_Configuration(TIM3, APP_TIM3_PERIOD, APP_TIM_PRESCALER);

    RTC_Init();

#if MOD_SFT_TYPE == MOD_TYPE_PWRCTL
    EG810m_Gpio_Init();                                  /* 模块相关gpio口初始化 */
    param_GetFromFlash();                                /* 读取参数 */
    ParaSynProcess(1);

    g_nDTURecivePos = 0;
    RDP_CfgParam = RDP_CfgParamTmp;

    if (CreateApplicationQueues() != pdPASS)
    {
        AppStartupErrorTrap();
    }
#endif

#if DEF_WDG_ENABLE == 1
    IWDG_Init(IWDG_Prescaler_256, 1279);                 /* 10s溢出 */
#endif
}

#if MOD_SFT_TYPE == MOD_TYPE_PWRCTL
static BaseType_t CreateApplicationQueues(void)
{
    g_pUpMsgQueen = xQueueCreate(APP_UP_QUEUE_LENGTH, APP_QUEUE_ITEM_SIZE);
    g_pDownMsgQueen = xQueueCreate(APP_DOWN_QUEUE_LENGTH, APP_QUEUE_ITEM_SIZE);

    if ((g_pUpMsgQueen == NULL) || (g_pDownMsgQueen == NULL))
    {
        return pdFAIL;
    }

    return pdPASS;
}
#endif

static void AppStartupErrorTrap(void)
{
    __disable_irq();
    while (1)
    {
    }
}
