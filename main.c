#include <string.h>
#include <stdbool.h>
#include "io.h"
#include "alt_string.h"
#include "altera_up_avalon_ps2.h"
#include "altera_up_avalon_rs232.h"
#include "altera_up_avalon_rs232_regs.h"
#include "FreeRTOS.h"
#include "task.h"
//#include "port.h"

#define TEST 1

volatile const unsigned int finish_addr = 0x00000000;
volatile const unsigned int intdisp_addr = 0x00000004;
volatile const unsigned int countdisp_addr = 0x00000008;
volatile const unsigned int print_addr = 0x00000010;
volatile const unsigned int flush_addr = 0x00002300;
volatile const unsigned int kb_buffer_addr = 0x00000300;

volatile alt_u32 * PLIC_IDRegister = NULL;
volatile alt_u32 * PS2_DATARegister = NULL;
/*
alt_u64 ullNextTime = 0ULL;
const alt_u64 * pullNextTime = &ullNextTime;
const alt_u32 uxTimerIncrementsForOneTick = ( alt_u32 ) ( ( configCPU_CLOCK_HZ ) / ( configTICK_RATE_HZ ) ); /* Assumes increment won't go over 32-bits. *
volatile alt_u64 * pullMachineTimerCompareRegister = NULL;
*/
/* -------------------------------------------------------------------------
 * 静态分配空闲任务所需的 内存控制块(TCB) 和 栈空间
 * ------------------------------------------------------------------------- */
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[ configMINIMAL_STACK_SIZE ];

/* -------------------------------------------------------------------------
 * keyboard
 * ------------------------------------------------------------------------- */
// 静态分配逻辑任务的内存
StaticTask_t xKeyboardTaskTCB;
StackType_t  uxKeyboardTaskStack[configMINIMAL_STACK_SIZE];

// 关键：定义任务句柄，汇编中断需要通过这个句柄找到目标任务
TaskHandle_t xKeyboardTaskHandle = NULL;
/* -------------------------------------------------------------------------
 * acker
 * ------------------------------------------------------------------------- */
StaticTask_t xAckerTaskTCB;
StackType_t  uxAckerTaskStack[configMINIMAL_STACK_SIZE];

TaskHandle_t xAckerTaskHandle = NULL;

#define FINISH_PROGRAM *((int*)(finish_addr)) = 1
#define DISPLAY_INT(num) *((int*)(intdisp_addr)) = num
#define DISPLAY_CUT(num) *((int*)(countdisp_addr)) = num

#define DISPLAY_CHAR(display_addr, chr) *((char*)(display_addr)) = chr
//#define FLUSH_CACHE *((int*)(flush_addr)) = 0
#define FLUSH_CACHE(num) *((int*)(flush_addr + num)) = 0
#define READ_KB_BUFF(num) *(((alt_u8*)(kb_buffer_addr)) + num)


#if TEST
	int count = 0;
    bool do_acker = true;
#endif

/**
 * @brief The enum type for the type of keyboard code received
 **/
typedef enum
{
	/** @brief Make code that corresponds to an ASCII character. For example, the ASCII make code for key <tt>[ A ] </tt> is 1C.
	 */
	KB_ASCII_MAKE_CODE = 1, 
	/** @brief Make code that corresponds to a non-ASCII character. For example, the binary (non-ASCII) make code for key <tt> [Left Alt]</tt> is 11.
	 */
	KB_BINARY_MAKE_CODE = 2,
	/** @brief Make code that has two bytes (the first byte is E0). For example, the long binary make code for key <tt>[Right Alt]</tt> is "E0 11".
	 */
	KB_LONG_BINARY_MAKE_CODE = 3,
	/** @brief Break code that has two bytes (the first byte is F0). For example, the break code for key <tt>[ A ]</tt> is "F0 1C".
	 */
	KB_BREAK_CODE = 4,
	/** @brief Long break code that has three bytes (with the first two bytes "E0 F0"). For example, the long break code for key <tt>[Right Alt]</tt> is "E0 F0 11".
	 */
	KB_LONG_BREAK_CODE = 5,
	/** @brief Scan codes that the decoding FSM is unable to decode.
	 */
	KB_INVALID_CODE = 6
} KB_CODE_TYPE;

#define SCAN_CODE_NUM  102

////////////////////////////////////////////////////////////////////
// Table of scan code, make code and their corresponding values 
// These data are useful for developing more features for the keyboard 
//
char *key_table[SCAN_CODE_NUM] = { "A", "B", "C", "D", "E", "F", "G", "H", "I", 
			"J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", 
			"X", "Y", "Z", "0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "`", 
			"-", "=", "\\", "BKSP", "SPACE", "TAB", "CAPS", "L SHFT", "L CTRL", 
			"L GUI", "L ALT", "R SHFT", "R CTRL", "R GUI", "R ALT", "APPS", 
			"ENTER", "ESC", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", 
			"F10", "F11", "F12", "SCROLL", "[", "INSERT", "HOME", "PG UP", 
			"DELETE", "END", "PG DN", "U ARROW", "L ARROW", "D ARROW", "R ARROW", 
			"NUM", "KP /", "KP *", "KP -", "KP +", "KP ENTER", "KP .", "KP 0", 
			"KP 1", "KP 2", "KP 3", "KP 4", "KP 5", "KP 6", "KP 7", "KP 8", "KP 9", 
			"]", ";", "'", ",", ".", "/" };

char ascii_codes[SCAN_CODE_NUM] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 
	'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 
	'W', 'X', 'Y', 'Z', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
	'`', '-', '=', 0, 0x08, 0, 0x09, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x0A, 
	0x1B, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, '[', 0, 0, 0, 0x7F, 0, 0, 
	0, 0, 0, 0, 0, '/', '*', '-', '+', 0x0A, '.', '0', '1', '2', '3', '4', 
	'5', '6', '7', '8', '9', ']', ';', '\'', ',', '.', '/' };

alt_u8 single_byte_make_code[SCAN_CODE_NUM] = { 0x1C, 0x32, 0x21, 0x23, 0x24, 
	0x2B, 0x34, 0x33, 0x43, 0x3B, 0x42, 0x4B, 0x3A, 0x31, 0x44, 0x4D, 0x15, 
	0x2D, 0x1B, 0x2C, 0x3C, 0x2A, 0x1D, 0x22, 0x35, 0x1A, 0x45, 0x16, 0x1E, 
	0x26, 0x25, 0x2E, 0x36, 0x3D, 0x3E, 0x46, 0x0E, 0x4E, 0x55, 0x5D, 0x66, 
	0x29, 0x0D, 0x58, 0x12, 0x14, 0, 0x11, 0x59, 0, 0, 0, 0, 0x5A, 0x76, 
	0x05, 0x06, 0x04, 0x0C, 0x03, 0x0B, 0x83, 0x0A, 0x01, 0x09, 0x78, 0x07, 
	0x7E, 0x54, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x77, 0, 0x7C, 0x7B, 0x79, 0, 
	0x71, 0x70, 0x69, 0x72, 0x7A, 0x6B, 0x73, 0x74, 0x6C, 0x75, 0x7D, 0x5B, 
	0x4C, 0x52, 0x41, 0x49, 0x4A };

alt_u8 multi_byte_make_code[SCAN_CODE_NUM] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x1F, 0, 0, 0x14, 0x27, 0x11, 0x2F, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0x70, 0x6C, 0x7D, 0x71, 
	0x69, 0x7A, 0x75, 0x6B, 0x72, 0x74, 0, 0x4A, 0, 0, 0, 0x5A, 0, 0, 0, 0, 
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
////////////////////////////////////////////////////////////////////

// States for the Keyboard Decode FSM 
typedef enum
{
	STATE_INIT,
	STATE_LONG_CODE,
	STATE_BREAK_CODE,
	STATE_LONG_BREAK_CODE,
	STATE_DONE
} DECODE_STATE;

/*
 * Allocate the device storage
 */
ALTERA_UP_AVALON_PS2_INSTANCE(PS2_KEYBOARD_0, ps2_keyboard_0);
ALTERA_UP_AVALON_RS232_INSTANCE(UART_0, uart_0);

alt_u8 kb_wptr = 0;
alt_u8 kb_rptr = 0;
alt_u8 kb_wptr_ = 0;
// char* print_addr = (char*)0x0;
unsigned print_count = 0;
// char str_[10] = {'a', 'a', 'a', 'a', 'a', '\0', '\0', '\0', '\0', '\0'};
// char str_[10] = "";

static DECODE_STATE key_decode_state = STATE_INIT;

extern void freertos_risc_v_trap_handler( void );

/* 
 * vApplicationGetIdleTaskMemory 钩子函数
 * 内核在创建空闲任务时会自动调用此函数获取内存地址
 */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer,
                                    StackType_t **ppxIdleTaskStackBuffer,
                                    uint32_t *pulIdleTaskStackSize )
{
    /* 指向静态定义的 TCB 结构体 */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* 指向静态定义的栈数组 */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* 告知内核栈的大小（注意：单位是 Word，不是 Byte） */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}

int A(int x, int y)
{
#if TEST
	count++;
#endif
	if (x == 0) return y + 1;
	if (y == 0) return A(x - 1, 1);
	return A(x - 1, A(x, y - 1));
}
/*
void vPortSetupTimerInterrupt( void )
{
    alt_u32 ulCurrentTimeHigh, ulCurrentTimeLow;
    volatile alt_u32 * const pulTimeHigh = ( volatile alt_u32 * const ) ( ( configMTIME_BASE_ADDRESS ) + 4UL ); /* 8-byte type so high 32-bit word is 4 bytes up. *
    volatile alt_u32 * const pulTimeLow = ( volatile alt_u32 * const ) ( configMTIME_BASE_ADDRESS );

    pullMachineTimerCompareRegister = ( volatile alt_u64 * const ) ( configMTIMECMP_BASE_ADDRESS );

    do
    {
        ulCurrentTimeHigh = *pulTimeHigh;
        ulCurrentTimeLow = *pulTimeLow;
    } while( ulCurrentTimeHigh != *pulTimeHigh );

    ullNextTime = ( alt_u64 ) ulCurrentTimeHigh;
    ullNextTime <<= 32ULL; /* High 4-byte word is 32-bits up. *
    ullNextTime |= ( alt_u64 ) ulCurrentTimeLow;
    ullNextTime += ( alt_u64 ) uxTimerIncrementsForOneTick;
    *pullMachineTimerCompareRegister = ullNextTime;

    /* Prepare the time to use after the next tick interrupt. *
    ullNextTime += ( alt_u64 ) uxTimerIncrementsForOneTick;
}
*/
void ridecore_init(void)
{
    PLIC_IDRegister = ( volatile alt_u32 * )__IO_CALC_ADDRESS_NATIVE(PLIC_BASE, 4);
    PS2_DATARegister = ( volatile alt_u32 * )__IO_CALC_ADDRESS_NATIVE(PS2_KEYBOARD_0_BASE, 0);

    // set PLIC Edge/Level
    // 每个中断源都设置为Level类型
    IOWR(PLIC_BASE, 0, 0x0);

    // set PLIC Interrupt Priority
    // 每个中断源占4bit，7个中断源的优先级都设为1
    IOWR(PLIC_BASE, 1, 0x01111111);

    // set PLIC Interrupt Enable
    // 设置中断源[6]为enable(keyboard)
    // 设置中断源[5]为enable(uart)
    IOWR(PLIC_BASE, 2, 0x60);

    // set PLIC Priority Threshold
    // 不屏蔽任何src
    IOWR(PLIC_BASE, 3, 0x0);

    // Enable keyboard interrupts
    alt_up_ps2_enable_read_interrupt(&ps2_keyboard_0);
    // Enable uart read interrupts
    alt_up_rs232_enable_read_interrupt(&uart_0);

    /* 确保地址是 4 字节对齐的，且低两位为 0 (Direct Mode) */
    volatile alt_u32 * mtvec_val = ((volatile alt_u32 *)freertos_risc_v_trap_handler);
    __asm__ volatile ("csrw mtvec, %0" : : "r"(mtvec_val));

    // Enable global CPU interrupts
    ridecore_cpu_eint();

    //vPortSetupTimerInterrupt();

    // Enable mtime and external interrupts.
    //csr_set_bits_mie(( alt_u32 )0x880);
}

//helper function for get_next_state
unsigned get_multi_byte_make_code_index(alt_u8 code)
{
	unsigned i;
	for (i = 0; i < SCAN_CODE_NUM; i++ )
	{
		if ( multi_byte_make_code[i] == code )
			return i;
	}
	return SCAN_CODE_NUM;
}

//helper function for get_next_state
unsigned get_single_byte_make_code_index(alt_u8 code)
{
	unsigned i;
	for (i = 0; i < SCAN_CODE_NUM; i++ )
	{
		if ( single_byte_make_code[i] == code )
			return i;
	}
	return SCAN_CODE_NUM;
}

//helper function for decode_scancode
/* FSM Diagram (Main transitions)
 * Normal bytes: bytes that are not 0xF0 or 0xE0
  +--<--+
  |     |                                   
  |     |
  V    INIT ------ 0xF0 ----> BREAK CODE
  |     |                         |
  |     |         LONG_BREAK_CODE-+
  |    0xE0      /                |
 Normal |       /                Normal
  |     |     0xF0                |
  |     V     /                   |
  |    LONG  /                    V
  |    CODE --- Normal -------> DONE
  |          (long make code)    /|\
  |                               |
  +-------------------------------|

 */
DECODE_STATE get_next_state(DECODE_STATE state, alt_u8 byte, 
		KB_CODE_TYPE *decode_mode, alt_u8 *buf, char *ascii)
{
	DECODE_STATE next_state = STATE_INIT;
	unsigned idx = SCAN_CODE_NUM;
	*ascii = 0;
	switch (state)
	{
		case STATE_INIT:
			if ( byte == 0xE0 )
			{	
				// this could be a long break code or a long make code
				next_state = STATE_LONG_CODE;
			}
			else if (byte == 0xF0)
			{
				// it is a break code
				next_state = STATE_BREAK_CODE;
			}
			else
			{
				// it is a normal make code
				idx = get_single_byte_make_code_index(byte);
				if ( (idx < 40 || idx == 68 || idx > 79) && ( idx != SCAN_CODE_NUM ) )
				{
					*decode_mode = KB_ASCII_MAKE_CODE;
					*ascii = ascii_codes[idx];
					*buf = byte;
				}
				else 
				{
					*decode_mode = KB_BINARY_MAKE_CODE;
					*buf = byte;
				}
				next_state = STATE_DONE;
			}
			break;
		case STATE_LONG_CODE:
			if ( byte != 0xF0 && byte!= 0xE0)
			{
				*decode_mode = KB_LONG_BINARY_MAKE_CODE;
				*buf = byte;
				next_state = STATE_DONE;
			}
			else
			{
				*decode_mode = KB_BREAK_CODE;
				next_state = STATE_LONG_BREAK_CODE;
			}
			break;
		case STATE_BREAK_CODE:
			if ( byte != 0xF0 && byte != 0xE0)
			{
				*decode_mode = KB_BREAK_CODE;
				*buf = byte;
				next_state = STATE_DONE;
			}
			else
			{
				next_state = STATE_BREAK_CODE;
				*decode_mode = KB_BREAK_CODE;
			}
			break;
		case STATE_LONG_BREAK_CODE:
			if ( byte != 0xF0 && byte != 0xE0)
			{
				*decode_mode = KB_LONG_BREAK_CODE;
				*buf = byte;
				next_state = STATE_DONE;
			}
			else
			{
				next_state = STATE_LONG_BREAK_CODE;
				*decode_mode = KB_LONG_BREAK_CODE;
			}
			break;
		default:
			*decode_mode = KB_INVALID_CODE;
			next_state = STATE_INIT;
	}
	return next_state;
}

void translate_make_code(KB_CODE_TYPE decode_mode, alt_u8 makecode, char *str)
{
	unsigned idx;
	switch (decode_mode)
	{
		case KB_ASCII_MAKE_CODE:
			idx = get_single_byte_make_code_index(makecode);
            alt_up_rs232_write_data(&uart_0, ascii_codes[idx]);
			// strcpy(str, key_table[idx]);
			break;
		case KB_BINARY_MAKE_CODE:
			idx = get_single_byte_make_code_index(makecode);
            alt_up_rs232_write_data(&uart_0, ascii_codes[idx]);
			// strcpy(str, key_table[idx]);
			break;
		case KB_LONG_BINARY_MAKE_CODE:
			idx = get_multi_byte_make_code_index(makecode);
			// strcpy(str, key_table[idx]);
			break;
		default:
			str[0] = 0;
			break;
	}
}

void do_key_pressed(void) {

    alt_u8 byte = READ_KB_BUFF(kb_rptr);

    KB_CODE_TYPE decode_mode;
    alt_u8 buf;
    char ascii;

    decode_mode = KB_INVALID_CODE;

    key_decode_state = get_next_state(key_decode_state, byte, &decode_mode, &buf, &ascii);

    if (key_decode_state == STATE_DONE) {
        // decode
        char str_[10] = "";
        translate_make_code(decode_mode, buf, str_);
/*
        for (int i = 0; i < 256; i += 16) {
            FLUSH_CACHE(i);
        }
*/
/*
        // print;
        int i = 0;

        while (str_[i] != 0) {
            DISPLAY_CHAR(((char*)(print_addr) + print_count), str_[i++]);
            print_count++;
        }
*/
        key_decode_state = STATE_INIT;
    }

    kb_rptr++;
}

void SendByte(unsigned char dat) {
    alt_up_rs232_write_data(&uart_0, dat);    // 写入数据寄存器
    // while(!TI);    // 等待发送完成
    // TI = 0;        // 清除发送中断标志
}

void SendString(char *str_) {
    int i = 0;

    while (str_[i] != 0) {
        alt_up_rs232_write_data(&uart_0, str_[i++]);
    }
}

/* 解决 RV32I 缺乏硬件 CLZ 指令的问题 */
int __clzsi2(unsigned int x) {
    int n = 32;
    unsigned int y;

    y = x >> 16; if (y != 0) { n -= 16; x = y; }
    y = x >> 8;  if (y != 0) { n -= 8;  x = y; }
    y = x >> 4;  if (y != 0) { n -= 4;  x = y; }
    y = x >> 2;  if (y != 0) { n -= 2;  x = y; }
    y = x >> 1;  if (y != 0) return n - 2;
    return n - x;
}

void vApplicationIdleHook( void )
{
    /* 
       这里写你希望在 CPU 没事干的时候执行的代码。
       比如：
       1. 让 CPU 进入低功耗睡眠模式 (Sleep)
       2. 喂狗 (Watchdog)
       3. 统计 CPU 利用率
       4. 闪烁一个状态指示灯
    */
}

 /* An interrupt handler. The interrupt handler does not perform any processing,  
    instead it unblocks a high priority task in which the event that generated the  
    interrupt is processed. If the priority of the task is high enough then the  
    interrupt will return directly to the task (so it will interrupt one task but  
    return to a different task), so the processing will occur contiguously in time -  
    just as if all the processing had been done in the interrupt handler itself. */  
void vWakeupHandlingKeyboardTask( void )  
{  
    BaseType_t xHigherPriorityTaskWoken;  

    /* Clear the interrupt. */  
    // prvClearInterruptSource();  

    /* xHigherPriorityTaskWoken must be initialised to pdFALSE. If calling  
       vTaskNotifyGiveFromISR() unblocks the handling task, and the priority of  
       the handling task is higher than the priority of the currently running task,  
       then xHigherPriorityTaskWoken will automatically get set to pdTRUE. */  
    xHigherPriorityTaskWoken = pdFALSE;  

    /* Unblock the handling task so the task can perform any processing necessitated  
       by the interrupt. xHandlingTask is the task's handle, which was obtained  
       when the task was created. */  
    vTaskNotifyGiveIndexedFromISR( xKeyboardTaskHandle, 0, &xHigherPriorityTaskWoken );  

    /* Force a context switch if xHigherPriorityTaskWoken is now set to pdTRUE.  
       The macro used to do this is dependent on the port and may be called  
       portEND_SWITCHING_ISR. */  
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );  
}  

/* A task that blocks waiting to be notified that the peripheral needs servicing,  
   processing all the events pending in the peripheral each time it is notified to   
   do so. */  
void vHandlingKeyboardTask( void *pvParameters )  
{  
    for( ;; )  
    {  
        /* Block indefinitely (without a timeout, so no need to check the function's  
           return value) to wait for a notification. Here the RTOS task notification  
           is being used as a binary semaphore, so the notification value is cleared  
           to zero on exit. NOTE! Real applications should not block indefinitely,  
           but instead time out occasionally in order to handle error conditions  
           that may prevent the interrupt from sending any more notifications. */  
        ulTaskNotifyTakeIndexed( 0,               /* Use the 0th notification */  
                                 pdFALSE,         /* Clear the notification value   
                                                     before exiting. */  
                                 portMAX_DELAY ); /* Block indefinitely. */  

        /* The RTOS task notification is used as a binary (as opposed to a  
           counting) semaphore, so only go back to wait for further notifications  
           when all events pending in the peripheral have been processed. */  
        kb_wptr_ = kb_wptr;

        while (kb_rptr < kb_wptr_)
        {
            do_key_pressed();
        }
    }
}

void vHandlingAckerTask( void *pvParameters ) {
    for(;;) {
        // 你的逻辑代码
        vTaskDelay(pdMS_TO_TICKS(1000*60)); 

#if TEST
        if (do_acker) {
            char str_[20] = "";
            count = 0;
            int num = A(3, 3);

            // DISPLAY_CUT(count);
            // DISPLAY_INT(num);
            // FINISH_PROGRAM;

            itoa(num, str_, 10); // 10表示10进制
            SendString(str_);
            //SendString("\n");

            itoa(count, str_, 10); // 10表示10进制
            SendString(str_);
        }
#endif
    }
}

int main()
{
  int key_pressed;

  ridecore_init();

  // 静态创建任务
  xKeyboardTaskHandle = xTaskCreateStatic(
      vHandlingKeyboardTask,        // 任务函数
      "do_key_pressed",             // 任务名称
      configMINIMAL_STACK_SIZE,     // 栈大小
      NULL,                         // 参数
      configMAX_PRIORITIES - 1,     // <--- 这里的 "1" 就是优先级设置！
      uxKeyboardTaskStack,          // 静态栈
      &xKeyboardTaskTCB             // 静态 TCB
  );

  // 静态创建任务
  xAckerTaskHandle = xTaskCreateStatic(
      vHandlingAckerTask,           // 任务函数
      "acker",                      // 任务名称
      configMINIMAL_STACK_SIZE,     // 栈大小
      NULL,                         // 参数
      1,                            // <--- 这里的 "1" 就是优先级设置！
      uxAckerTaskStack,             // 静态栈
      &xAckerTaskTCB                // 静态 TCB
  );

  vTaskStartScheduler();

  while(1);
/*
  // Hello, World!
  // 72 101 108 108 111 44 32 87 111 114 108 100 33
  alt_up_rs232_write_data(&uart_0, 72);
  alt_up_rs232_write_data(&uart_0, 101);
  alt_up_rs232_write_data(&uart_0, 108);
  alt_up_rs232_write_data(&uart_0, 108);
  alt_up_rs232_write_data(&uart_0, 111);
  alt_up_rs232_write_data(&uart_0, 44);
  alt_up_rs232_write_data(&uart_0, 32);
  alt_up_rs232_write_data(&uart_0, 87);
  alt_up_rs232_write_data(&uart_0, 111);
  alt_up_rs232_write_data(&uart_0, 114);
  alt_up_rs232_write_data(&uart_0, 108);
  alt_up_rs232_write_data(&uart_0, 100);
  alt_up_rs232_write_data(&uart_0, 33);
*/


/*
  char str[20] = "";
  itoa(num, str, 10); // 10表示10进制
  SendString(str);
  SendString("\n");

#if TEST
  itoa(count, str, 10);
  SendString(str);
  SendString("\n");
#endif
*/

/*
#if TEST
  char str_[20] = "";
  int num = A(3, 3);

  DISPLAY_CUT(count);
  DISPLAY_INT(num);
  FINISH_PROGRAM;

  itoa(num, str_, 10); // 10表示10进制
  SendString(str_);
  //SendString("\n");

  itoa(count, str_, 10); // 10表示10进制
  SendString(str_);
#endif

  while(1) {
    ridecore_cpu_deint();
    while (ridecore_cpu_csr_read(CSR_MIE) & MIE_MEI_BIT_MASK);

    key_pressed = (kb_rptr != kb_wptr) ? 1 : 0;

    ridecore_cpu_eeint();
    while ((ridecore_cpu_csr_read(CSR_MIE) & MIE_MEI_BIT_MASK) == 0);

    if (key_pressed) {
#if TEST
        //DISPLAY_CUT(++count);
#endif
        do_key_pressed();
    }
  }
*/
  return 0;
}

