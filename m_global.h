#ifndef __M_GLOBAL_H_
#define __M_GLOBAL_H_

/* 模块类型定义 */
#define MOD_TYPE_PWRCTL             0U                          /* 电源控制小板 */
#define MOD_TYPE_LORAMESH           1U                          /* LoraMesh小板，通过该小板无线透传数据 */
#define MOD_TYPE_BDS                2U                          /* 北斗定位小板 */

#ifndef MOD_SFT_TYPE
  #define MOD_SFT_TYPE              MOD_TYPE_BDS
#endif

/* 兼容历史拼写 DEF_WDG_ENABEL，业务代码建议使用 DEF_WDG_ENABLE。 */
#ifndef DEF_WDG_ENABEL
  #define DEF_WDG_ENABEL            1U
#endif
#define DEF_WDG_ENABLE              DEF_WDG_ENABEL              /* 硬件看门狗是否启用 */

/* 应用版本存放位置：为 bootloader/升级流程预留固定地址。 */
#define APP_VERSION_FLASH_END_ADDR  0x0800D000U

/* 公共外设参数 */
#define APP_DEBUG_USART_BAUD        115200U
#define APP_TIM2_PERIOD             10000U                      /* 10ms */
#define APP_TIM3_PERIOD             2000U                       /* 2ms */
#define APP_TIM_PRESCALER           8U

/* FreeRTOS 启动任务参数 */
#define APP_STARTUP_TASK_PRIO       1U
#define APP_STARTUP_TASK_STACK_SIZE 512U

#if MOD_SFT_TYPE == MOD_TYPE_PWRCTL
  #define APP_MODULE_NAME           "4GCTL"
  #define APP_MCU_VERSION           "4GCTL.260312.V0.00.03"
  #define APP_MODULE_USART1_BAUD    115200U

  /* 任务优先级规划 5-9，数字越大优先级越高 */
  #define APP_EG810M_TASK_PRIO      9U
  #define APP_DPC_RECV_TASK_PRIO    8U
  #define APP_DPC_DISPATCH_PRIO     7U
  #define APP_AUTO_COLLECT_PRIO     6U
  #define APP_DPC_SEND_TASK_PRIO    5U
  #define APP_PWRCTL_TASK_STACK     0x200U

  #define APP_UP_QUEUE_LENGTH       10U
  #define APP_DOWN_QUEUE_LENGTH     10U
  #define APP_QUEUE_ITEM_SIZE       sizeof(MSG_INFO *)

  /* 兼容历史宏名 */
  #define EG810MTASK_PRIO           APP_EG810M_TASK_PRIO
  #define DPCRecvTask_PRIO          APP_DPC_RECV_TASK_PRIO
  #define DPCDispatchTask_PRIO      APP_DPC_DISPATCH_PRIO
  #define AutoCollectTask_PRIO      APP_AUTO_COLLECT_PRIO
  #define DPCSendTask_PRIO          APP_DPC_SEND_TASK_PRIO
  #define TASK_SIZE                 APP_PWRCTL_TASK_STACK
  #define MAX_UP_MESSAGES           256U
  #define MAX_DOWN_MESSAGES         12U
#elif MOD_SFT_TYPE == MOD_TYPE_LORAMESH
  #define APP_MODULE_NAME           "LoraMesh"
  #define APP_MCU_VERSION           "LoraMesh.260428.V0.00.02"
  #define APP_MODULE_USART1_BAUD    9600U

  #define APP_LORAMESH_TASK_PRIO    9U
  #define APP_LORAMESH_TASK_STACK   0x200U

  /* 兼容历史宏名 */
  #define LoraMesh_Task_Prio        APP_LORAMESH_TASK_PRIO
  #define LoraMesh_Task_Size        APP_LORAMESH_TASK_STACK
#elif MOD_SFT_TYPE == MOD_TYPE_BDS
  #define APP_MODULE_NAME           "BDS"
  #define APP_MCU_VERSION           "BDS.260507.V0.00.01"
  #define APP_MODULE_USART1_BAUD    115200U
#else
  #error "Unsupported MOD_SFT_TYPE"
#endif

#endif
