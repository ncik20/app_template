#include <stdint.h>

//volatile const unsigned int disp_addr = 0x0;
volatile const unsigned int finish_addr = 0x00000000;
volatile const unsigned int intdisp_addr = 0x00000004;
volatile const unsigned int countdisp_addr = 0x00000008;
volatile const unsigned int flush_addr = 0x00002000;

//#define DISPLAY_CHAR(chr) *((int*)(disp_addr)) = chr
#define FINISH_PROGRAM *((int*)(finish_addr)) = 1
#define FLUSH_CACHE *((int*)(flush_addr)) = 0
#define DISPLAY_INT(num) *((int*)(intdisp_addr)) = num
#define DISPLAY_CUT(num) *((int*)(countdisp_addr)) = num
#define TEST 1

const int CSR_MSTATUS = 0x300;

#if TEST
	int count = 0;
#endif

/**********************************************************************//**
 * Write data to CPU configuration and status register (CSR).
 *
 * @param[in] csr_id ID of CSR to write. See #NEORV32_CSR_enum.
 * @param[in] data Data to write (uint32_t).
 **************************************************************************/
inline void __attribute__ ((always_inline)) csr_write(const int csr_id, uint32_t data) {

  register uint32_t csr_data = data;

  asm volatile ("csrw %[input_i], %[input_j]" :  : [input_i] "i" (csr_id), [input_j] "r" (csr_data));
}

int A(int x, int y)
{
#if TEST
	count++;
 	//DISPLAY_CUT(count);
#endif
	if (x == 0) return y + 1;
	if (y == 0) return A(x - 1, 1);
	return A(x - 1, A(x, y - 1));
}

int main()
{
  uint32_t mstatus = 0b1000;
  csr_write(CSR_MSTATUS, mstatus);
  DISPLAY_INT(A(3,3));
#if TEST
  DISPLAY_CUT(count);
#endif
  FINISH_PROGRAM;
  //FLUSH_CACHE;
  
  //mstatus = 0x20002021;
  //csr_write(CSR_MSTATUS, mstatus);
  while(1){;}

  return 0;
}

