#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* -------------------------------------------------------------------------
 * 中断嵌套配置 (不支持嵌套)
 * ------------------------------------------------------------------------- */
/* 0 表示不支持嵌套中断。这会减小 portASM.S 中上下文切换的复杂度和栈开销 */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0 
#define configKERNEL_INTERRUPT_PRIORITY         0

#define configTICK_TYPE_WIDTH_IN_BITS           TICK_TYPE_WIDTH_32_BITS
#define configISR_STACK_SIZE_WORDS              ( 256 ) // 分配 1024 字节给中断专用

/* -------------------------------------------------------------------------
 * 1. 核心调度配置 (仅保留 tasks.c 必要功能)
 * ------------------------------------------------------------------------- */
#define configUSE_PREEMPTION                    1   // 抢占式调度
#define configUSE_IDLE_HOOK                     1   // 启用空闲钩子
#define configUSE_TICK_HOOK                     0   // 禁用滴答钩子
#define configCPU_CLOCK_HZ                      ( ( unsigned long ) 32768 ) // 需根据硬件实测修改
#define configTICK_RATE_HZ                      ( ( TickType_t ) 100 )         // 10ms 心跳
#define configMAX_PRIORITIES                    ( 3 )                          // 优先级越少，tasks.c 内部数组越小
#define configMINIMAL_STACK_SIZE                ( ( unsigned short ) 1000 )     // 仅供空闲任务使用
#define configMAX_TASK_NAME_LEN                 ( 1 )                          // 节省 RAM，不存储任务名

/* -------------------------------------------------------------------------
 * 2. 内存管理 (剔除 heap_x.c)
 * ------------------------------------------------------------------------- */
#define configSUPPORT_STATIC_ALLOCATION         1   // 必须开启，用于替代动态分配
#define configSUPPORT_DYNAMIC_ALLOCATION        0   // 禁用后无需编译任何 heap_x.c 文件

/* -------------------------------------------------------------------------
 * 3. 剔除所有外部组件 (不编译 queue.c, timers.c, event_groups.c 等)
 * ------------------------------------------------------------------------- */
#define configUSE_MUTEXES                       0
#define configUSE_RECURSIVE_MUTEXES             0
#define configUSE_COUNTING_SEMAPHORES           0
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIMERS                        0   // 彻底移除软件定时器支持
#define configUSE_EVENT_GROUPS                  0   // 彻底移除事件组支持
#define configUSE_STREAM_BUFFERS                0   // 彻底移除流缓冲支持

/* -------------------------------------------------------------------------
 * 4. RISC-V 架构特定 (针对 -march=rv32i)
 * ------------------------------------------------------------------------- */
/* 注意：由于使用了 rv32i，移植层(port.c)会自动跳过 FPU 寄存器的入栈出栈 */
#define configMTIME_BASE_ADDRESS                0x4000BFF8 // 若硬件无标准CLINT，设为0简化port.c
#define configMTIMECMP_BASE_ADDRESS             0x40004000

/* -------------------------------------------------------------------------
 * 5. API 裁剪 (只保留最基础的任务控制)
 * ------------------------------------------------------------------------- */
#define INCLUDE_vTaskPrioritySet                0
#define INCLUDE_uxTaskPriorityGet               0
#define INCLUDE_vTaskDelete                     0
#define INCLUDE_vTaskSuspend                    1   // 建议保留，以便任务阻塞
#define INCLUDE_vTaskDelayUntil                 0
#define INCLUDE_vTaskDelay                      1   // 基础延时功能
#define INCLUDE_xTaskGetIdleTaskHandle          0
#define INCLUDE_xTaskGetCurrentTaskHandle       0

/* -------------------------------------------------------------------------
 * 6. 静态分配必要的钩子函数 (当 configSUPPORT_STATIC_ALLOCATION=1 时必须实现)
 * ------------------------------------------------------------------------- */
/* 你需要在 main.c 中手动实现 vApplicationGetIdleTaskMemory 否则会链接报错 */

#endif /* FREERTOS_CONFIG_H */
