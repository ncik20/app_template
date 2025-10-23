// #################################################################################################
// # << RIDECORE: ridecore_cpu.h - CPU Core Functions HW Driver >>                                   #
// # ********************************************************************************************* #
// # BSD 3-Clause License                                                                          #
// #                                                                                               #
// # Copyright (c) 2025, Stephan Nolting. All rights reserved.                                     #
// #                                                                                               #
// # Redistribution and use in source and binary forms, with or without modification, are          #
// # permitted provided that the following conditions are met:                                     #
// #                                                                                               #
// # 1. Redistributions of source code must retain the above copyright notice, this list of        #
// #    conditions and the following disclaimer.                                                   #
// #                                                                                               #
// # 2. Redistributions in binary form must reproduce the above copyright notice, this list of     #
// #    conditions and the following disclaimer in the documentation and/or other materials        #
// #    provided with the distribution.                                                            #
// #                                                                                               #
// # 3. Neither the name of the copyright holder nor the names of its contributors may be used to  #
// #    endorse or promote products derived from this software without specific prior written      #
// #    permission.                                                                                #
// #                                                                                               #
// # THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS   #
// # OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF               #
// # MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE    #
// # COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,     #
// # EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE #
// # GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED    #
// # AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING     #
// # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED  #
// # OF THE POSSIBILITY OF SUCH DAMAGE.                                                            #
// # ********************************************************************************************* #
// # The RIDECORE Processor - https://github.com/ncik20/ridecore                      (c) ncik20 #
// #################################################################################################


/**********************************************************************//**
 * @file ridecore_cpu.h
 * @author ncik20
 * @brief CPU Core Functions HW driver header file.
 **************************************************************************/

#ifndef ridecore_cpu_h
#define ridecore_cpu_h

#include "alt_types.h"

#define CSR_MSTATUS_MIE 4

/**********************************************************************//**
 * Prototype for "after-main handler". This function is called if main() returns.
 *
 * @param[in] return_code Return value of main() function.
 * @return Return value is irrelevant (there is no one left to check for it...).
 **************************************************************************/
extern int __neorv32_crt0_after_main(int32_t return_code) __attribute__ ((weak));


/**********************************************************************//**
 * Store unsigned word to address space if atomic access reservation is still valid.
 *
 * @note An unaligned access address will raise an alignment exception.
 *
 * @param[in] addr Address (32-bit).
 * @param[in] wdata Data word (32-bit) to store.
 * @return Operation status (32-bit, zero if success).
 **************************************************************************/
inline alt_u32 __attribute__ ((always_inline)) ridecore_cpu_store_conditional(alt_u32 addr, alt_u32 wdata) {

#if defined __riscv_atomic || defined __riscv_a
  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data = wdata;
  register alt_u32 reg_status;

  asm volatile ("sc.w %[status], %[da], (%[ad])" : [status] "=r" (reg_status) : [da] "r" (reg_data), [ad] "r" (reg_addr));

  return reg_status;
#else
  return 1; // always failing
#endif
}


/**********************************************************************//**
 * Conditional store unsigned word to address space.
 *
 * @note An unaligned access address will raise an alignment exception.
 *
 * @param[in] addr Address (32-bit).
 * @param[in] wdata Data word (32-bit) to store.
 **************************************************************************/
inline void ALT_ALWAYS_INLINE __builtin_stwio(alt_u32 addr, alt_u32 wdata) {

  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data = wdata;

  asm volatile ("sw %[da], 0(%[ad])" : : [da] "r" (reg_data), [ad] "r" (reg_addr));
}


/**********************************************************************//**
 * Store unsigned half-word to address space.
 *
 * @note An unaligned access address will raise an alignment exception.
 *
 * @param[in] addr Address (32-bit).
 * @param[in] wdata Data half-word (16-bit) to store.
 **************************************************************************/
inline void ALT_ALWAYS_INLINE __builtin_sthio(alt_u32 addr, alt_u16 wdata) {

  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data = (alt_u32)wdata;

  asm volatile ("sh %[da], 0(%[ad])" : : [da] "r" (reg_data), [ad] "r" (reg_addr));
}


/**********************************************************************//**
 * Store unsigned byte to address space.
 *
 * @param[in] addr Address (32-bit).
 * @param[in] wdata Data byte (8-bit) to store.
 **************************************************************************/
inline void ALT_ALWAYS_INLINE __builtin_stbio(alt_u32 addr, alt_u8 wdata) {

  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data = (alt_u32)wdata;

  asm volatile ("sb %[da], 0(%[ad])" : : [da] "r" (reg_data), [ad] "r" (reg_addr));
}


/**********************************************************************//**
 * Load unsigned word from address space and make reservation for atomic access.
 *
 * @note An unaligned access address will raise an alignment exception.
 *
 * @param[in] addr Address (32-bit).
 * @return Read data word (32-bit).
 **************************************************************************/
inline alt_u32 __attribute__ ((always_inline)) ridecore_cpu_load_reservate_word(alt_u32 addr) {

  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data;

#if defined __riscv_atomic || defined __riscv_a
  asm volatile ("lr.w %[da], 0(%[ad])" : [da] "=r" (reg_data) : [ad] "r" (reg_addr));
#else
  asm volatile ("lw %[da], 0(%[ad])" : [da] "=r" (reg_data) : [ad] "r" (reg_addr));
#endif

  return (alt_u32)reg_data;
}



/**********************************************************************//**
 * Load unsigned word from address space.
 *
 * @note An unaligned access address will raise an alignment exception.
 *
 * @param[in] addr Address (32-bit).
 * @return Read data word (32-bit).
 **************************************************************************/
inline alt_u32 ALT_ALWAYS_INLINE __builtin_ldwio(alt_u32 addr) {

  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data;

  asm volatile ("lw %[da], 0(%[ad])" : [da] "=r" (reg_data) : [ad] "r" (reg_addr));

  return (alt_u32)reg_data;
}


/**********************************************************************//**
 * Load unsigned half-word from address space.
 *
 * @note An unaligned access address will raise an alignment exception.
 *
 * @param[in] addr Address (32-bit).
 * @return Read data half-word (16-bit).
 **************************************************************************/
inline alt_u16 ALT_ALWAYS_INLINE __builtin_ldhuio(alt_u32 addr) {

  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data;

  asm volatile ("lhu %[da], 0(%[ad])" : [da] "=r" (reg_data) : [ad] "r" (reg_addr));

  return (alt_u16)reg_data;
}


/**********************************************************************//**
 * Load unsigned byte from address space.
 *
 * @param[in] addr Address (32-bit).
 * @return Read data byte (8-bit).
 **************************************************************************/
inline alt_u8 ALT_ALWAYS_INLINE __builtin_ldbuio(alt_u32 addr) {

  register alt_u32 reg_addr = addr;
  register alt_u32 reg_data;

  asm volatile ("lbu %[da], 0(%[ad])" : [da] "=r" (reg_data) : [ad] "r" (reg_addr));

  return (alt_u8)reg_data;
}


/**********************************************************************//**
 * Read data from CPU configuration and status register (CSR).
 *
 * @param[in] csr_id ID of CSR to read. See #NEORV32_CSR_enum.
 * @return Read data (alt_u32).
 **************************************************************************/
inline alt_u32 __attribute__ ((always_inline)) ridecore_cpu_csr_read(const int csr_id) {

  register alt_u32 csr_data;

  asm volatile ("csrr %[result], %[input_i]" : [result] "=r" (csr_data) : [input_i] "i" (csr_id));
  
  return csr_data;
}


/**********************************************************************//**
 * Write data to CPU configuration and status register (CSR).
 *
 * @param[in] csr_id ID of CSR to write. See #NEORV32_CSR_enum.
 * @param[in] data Data to write (alt_u32).
 **************************************************************************/
inline void __attribute__ ((always_inline)) ridecore_cpu_csr_write(const int csr_id, alt_u32 data) {

  register alt_u32 csr_data = data;

  asm volatile ("csrw %[input_i], %[input_j]" :  : [input_i] "i" (csr_id), [input_j] "r" (csr_data));
}


/**********************************************************************//**
 * Put CPU into "sleep" mode.
 *
 * @note This function executes the WFI instruction.
 * The WFI (wait for interrupt) instruction will make the CPU stall until
 * an interrupt request is detected. Interrupts have to be globally enabled
 * and at least one external source must be enabled (like the MTI machine
 * timer interrupt) to allow the CPU to wake up again. If 'Zicsr' CPU extension is disabled,
 * this will permanently stall the CPU.
 **************************************************************************/
inline void __attribute__ ((always_inline)) ridecore_cpu_sleep(void) {

  asm volatile ("wfi");
}


/**********************************************************************//**
 * Enable global CPU interrupts (via MIE flag in mstatus CSR).
 **************************************************************************/
inline void __attribute__ ((always_inline)) ridecore_cpu_eint(void) {

  asm volatile ("csrrsi zero, mstatus, %0" : : "i" (1 << CSR_MSTATUS_MIE));
}


/**********************************************************************//**
 * Disable global CPU interrupts (via MIE flag in mstatus CSR).
 **************************************************************************/
inline void __attribute__ ((always_inline)) ridecore_cpu_dint(void) {

  asm volatile ("csrrci zero, mstatus, %0" : : "i" (1 << CSR_MSTATUS_MIE));
}


/**********************************************************************//**
 * Trigger breakpoint exception (via EBREAK instruction).
 **************************************************************************/
inline void __attribute__ ((always_inline)) ridecore_cpu_breakpoint(void) {

  asm volatile ("ebreak");
}


/**********************************************************************//**
 * Trigger "environment call" exception (via ECALL instruction).
 **************************************************************************/
inline void __attribute__ ((always_inline)) ridecore_cpu_env_call(void) {

  asm volatile ("ecall");
}


#endif // ridecore_cpu_h
