#include "main.h"

#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
  #include "bsp_eg810m.h"
  #include "bsp_flash.h"
#elif MOD_SFT_TYPE==MOD_TYPE_LORAMESH
  #include "loramesh_proc.h"
#elif MOD_SFT_TYPE==MOD_TYPE_BDS
  #include "bds_proc.h"
#endif

/**************************** 任务句柄 ********************************/

static TaskHandle_t AppTaskCreate_Handle = NULL;        /* 创建任务句柄 */

#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
  static TaskHandle_t EG810M_Task_Handle = NULL;
  static TaskHandle_t DPCRecv_Task_Handle = NULL;
  static TaskHandle_t DPCDispatch_Task_Handle = NULL;
  static TaskHandle_t AutoCollect_Task_Handle = NULL;
  static TaskHandle_t DPCSend_Task_Handle = NULL;

  QueueHandle_t g_pUpMsgQueen =NULL;                      // 上下行消息句柄
  QueueHandle_t g_pDownMsgQueen =NULL;
#elif MOD_SFT_TYPE==MOD_TYPE_LORAMESH
  static TaskHandle_t LoraMesh_Task_Handle = NULL;
#elif MOD_SFT_TYPE==MOD_TYPE_BDS
  static TaskHandle_t BDS_Task_Handle = NULL;
#endif

/******************************* 版本 ************************************/

#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
  #define MCU_VERSION "4GCTL.260312.V0.00.03"
#elif MOD_SFT_TYPE==MOD_TYPE_LORAMESH
  #define MCU_VERSION "LoraMesh.260428.V0.00.02"
#elif MOD_SFT_TYPE==MOD_TYPE_BDS
  #define MCU_VERSION "BDS.260507.V0.00.01"
#endif

#define __at(_addr) __attribute__ ((at(_addr)))
const char McuVersion[sizeof(MCU_VERSION)]__at(0x0800D000-(sizeof(MCU_VERSION)-3)) = MCU_VERSION;

/******************************* 宏定义 ************************************/
/**
 * @brief 写应用程序的时候，可能需要用到一些宏定义
 */
#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
  #define  QUEUE_LEN    20   /* 队列的长度，最大可包含多少个消息 */
  #define  QUEUE_SIZE   4   /* 队列中每个消息大小（字节） */
#endif

/*
*************************************************************************
*                             函数声明
*************************************************************************
*/
static void AppTaskCreate(void);/* 用于创建任务 */

static void BSP_Init(void);/* 用于初始化板载相关资源 */

/*****************************************************************
  * @brief  主函数
  * @param  无
  * @retval 无
  * @note   第一步：开发板硬件初始化 
            第二步：创建APP应用任务
            第三步：启动FreeRTOS，开始多任务调度
  ****************************************************************/
// 加了bootloader要修改对应的SCB->VTOR地址

int main(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
    /* 硬件初始化 */
    BSP_Init();
    /* 创建AppTaskCreate任务 */
    xReturn = xTaskCreate((TaskFunction_t )AppTaskCreate,  /* 任务入口函数 */
                        (const char*    )"AppTaskCreate",/* 任务名字 */
                        (uint16_t       )512,  /* 任务栈大小 */
                        (void*          )NULL,/* 任务入口函数参数 */
                        (UBaseType_t    )1, /* 任务的优先级 */
                        (TaskHandle_t*  )&AppTaskCreate_Handle);/* 任务控制块指针 */ 
    /* 启动任务调度 */
    if(pdPASS == xReturn){
        __enable_irq();  // 使能全局中断
        vTaskStartScheduler();   /* 启动任务，开启调度 */
    }
    else
        return -1;
  
    while(1);   /* 正常不会执行到这里 */
}

/**
 * @brief 为了方便管理，所有的任务创建函数都放在这个函数里面
 */
static void AppTaskCreate(void)
{
    BaseType_t xReturn = pdPASS;/* 定义一个创建信息返回值，默认为pdPASS */
    taskENTER_CRITICAL();           //进入临界区taskEXIT_CRITICAL();            //退出临界区

#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
    /* 创建Receive_Task任务 */
    xReturn = xTaskCreate((TaskFunction_t)EG810M_TASK_STK,          /* 任务入口函数 */
                        (const char*     )"Egm810_Task",            /* 任务名字 */
                        (uint16_t        )TASK_SIZE,                /* 任务栈大小 */
                        (void*           )NULL,                     /* 任务入口函数参数 */
                        (UBaseType_t     )EG810MTASK_PRIO,          /* 任务的优先级 */
                        (TaskHandle_t*   )&EG810M_Task_Handle);     /* 任务控制块指针 */
                        
    xReturn = xTaskCreate((TaskFunction_t)DPCRecvTask, (const char*)"DPCRecv_Task",
              (uint16_t)TASK_SIZE, (void*)NULL, (UBaseType_t)DPCRecvTask_PRIO, (TaskHandle_t*)&DPCRecv_Task_Handle);
     
    xReturn = xTaskCreate((TaskFunction_t)DPCDispatchTask, (const char*)"DPCDispatchTask",
              (uint16_t)TASK_SIZE, (void*)NULL, (UBaseType_t)DPCDispatchTask_PRIO, (TaskHandle_t*)&DPCDispatch_Task_Handle);
          
    xReturn = xTaskCreate((TaskFunction_t)AutoCollectTask, (const char*)"AutoCollectTask",
              (uint16_t)TASK_SIZE, (void*)NULL, (UBaseType_t)AutoCollectTask_PRIO, (TaskHandle_t*)&AutoCollect_Task_Handle);
               
    xReturn = xTaskCreate((TaskFunction_t)DPCSendTask, (const char*)"DPCSendTask",
              (uint16_t)TASK_SIZE, (void*)NULL, (UBaseType_t)DPCSendTask_PRIO, (TaskHandle_t*)&DPCSend_Task_Handle);
#elif MOD_SFT_TYPE==MOD_TYPE_LORAMESH
    xReturn = xTaskCreate((TaskFunction_t)LoraMeshTask, (const char*)"LoraMesh_Task",
              (uint16_t)LoraMesh_Task_Size, (void*)NULL, (UBaseType_t)LoraMesh_Task_Prio, (TaskHandle_t*)&LoraMesh_Task_Handle);
#elif MOD_SFT_TYPE==MOD_TYPE_BDS
    xReturn = xTaskCreate((TaskFunction_t)BDSTask, (const char*)"BDS_Task",
              (uint16_t)BDS_Task_Size, (void*)NULL, (UBaseType_t)BDS_Task_Prio, (TaskHandle_t*)&BDS_Task_Handle);
#endif

    if (pdPASS == xReturn)
    {
        //printf("创建Receive_Task任务成功!\r\n");
    }

    vTaskDelete(AppTaskCreate_Handle); //删除AppTaskCreate任务
    taskEXIT_CRITICAL();            //退出临界区
}

/**
 * @brief 初始化看门狗
 */
void IWDG_Init(u8 prer, u16 rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);               // 使能对IWDG->PR IWDG->RLR的写
    IWDG_SetPrescaler(prer);                                    // 设置IWDG分频系数
    IWDG_SetReload(rlr);                                        // 设置IWDG装载值
    IWDG_ReloadCounter();                                       // reload
    IWDG_Enable();                                              // 使能看门狗
}

// 喂独立看门狗
void IWDG_Feed(void)
{
    IWDG_ReloadCounter();                                       // reload
}

/**
 * @brief 板级外设初始化，所有板子上的初始化均可放在这个函数里面
 */
static void BSP_Init(void)
{
    /*
    * STM32中断优先级分组为4，即4bit都用来表示抢占优先级，范围为：0~15
    * 优先级分组只需要分组一次即可，以后如果有其他的任务需要用到中断，
    * 都统一用这个优先级分组，千万不要再分组，切忌。
    */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
    USART_Configuration(USART1, 115200);                                /* 4G模块通信 */
#elif MOD_SFT_TYPE==MOD_TYPE_LORAMESH
    USART_Configuration(USART1, 9600);                                  /* loramesh模块通信 */
#elif MOD_SFT_TYPE==MOD_TYPE_BDS
    USART_Configuration(USART1, 115200);                                /* BDS模块通信 (BDS_TXD->PA10/BDS_RXD->PA9) */
#endif
    USART_Configuration(USART2, 115200);                                /* 与底板通讯，可编译为调试口 */

    TIMx_Configuration(TIM2, 10000, 8);                                 /* 10ms */
    TIMx_Configuration(TIM3, 2000, 8);                                  /* 2ms */

    RTC_Init();

    /* 电源控制小板 */
#if MOD_SFT_TYPE==MOD_TYPE_PWRCTL
    EG810m_Gpio_Init();                                                 /* 模块相关gpio口初始化 */
    param_GetFromFlash();                                               /* 读取参数 */
    ParaSynProcess(1);

    g_nDTURecivePos = 0;
    RDP_CfgParam = RDP_CfgParamTmp;

    /* 创建消息队列相关 */
    /* 创建Test_Queue */    /* 消息队列的长度 */    /* 消息的大小 */
    g_pUpMsgQueen = xQueueCreate(10, sizeof(MSG_INFO*));
    g_pDownMsgQueen = xQueueCreate(10, sizeof(MSG_INFO*));
#endif

#if DEF_WDG_ENABEL==1
    IWDG_Init(IWDG_Prescaler_256, 1279);                                /* 10s溢出 */
#endif
}
