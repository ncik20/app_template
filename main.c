#include "HAL/inc/io.h"
#include "drivers/inc/altera_up_avalon_ps2.h"

//volatile const unsigned int disp_addr = 0x0;
volatile const unsigned int finish_addr = 0x00000000;
volatile const unsigned int intdisp_addr = 0x00000004;
volatile const unsigned int countdisp_addr = 0x00000008;
volatile const unsigned int kb_buffer_addr = 0x00000010;
volatile const unsigned int flush_addr = 0x00002000;

//#define DISPLAY_CHAR(chr) *((int*)(disp_addr)) = chr
#define FINISH_PROGRAM *((int*)(finish_addr)) = 1
#define FLUSH_CACHE *((int*)(flush_addr)) = 0
#define DISPLAY_INT(num) *((int*)(intdisp_addr)) = num
#define DISPLAY_CUT(num) *((int*)(countdisp_addr)) = num

/*
 * Allocate the device storage
 */
ALTERA_UP_AVALON_PS2_INSTANCE(PS2_KEYBOARD_0, ps2_keyboard_0);

extern unsigned char kb_wptr;
unsigned char kb_rptr;

void ridecore_init(void)
{
    // set PLIC Edge/Level
    // 每个中断源都设置为Level类型
    IOWR(PLIC_BASE, 0, 0x0);

    // set PLIC Interrupt Priority
    // 每个中断源占4bit，7个中断源的优先级都设为1
    IOWR(PLIC_BASE, 1, 0x01111111);

    // set PLIC Interrupt Enable
    // 设置中断源[6]为enable
    IOWR(PLIC_BASE, 2, 0x40);

    // set PLIC Priority Threshold
    // 不屏蔽任何src
    IOWR(PLIC_BASE, 3, 0x0);

    // Enable keyboard interrupts
    alt_up_ps2_enable_read_interrupt(&ps2_keyboard_0);

    // Enable global CPU interrupts
    ridecore_cpu_eint();
}

void do_key_pressed(void) {
}

int main()
{
  int key_pressed;

  ridecore_init();

  while(1) {
    ridecore_cpu_dint();
    key_pressed = (kb_rptr != kb_wptr) ? 1 : 0;
    ridecore_cpu_eint();

    if (key_pressed) {
        do_key_pressed();
    }
  }

  return 0;
}

