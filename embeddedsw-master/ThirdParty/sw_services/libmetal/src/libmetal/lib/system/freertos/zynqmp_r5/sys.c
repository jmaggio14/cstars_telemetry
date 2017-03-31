/*
 * Copyright (c) 2016, Xilinx Inc. and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of Xilinx nor the names of its contributors may be used
 *    to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * @file	freertos/sys.c
 * @brief	machine specific system primitives implementation.
 */

#include <stdint.h>
#include "xil_cache.h"
#include "xreg_cortexr5.h"
#include "xil_mmu.h"
#include "xil_mpu.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "metal/io.h"
#include "metal/sys.h"


static unsigned int int_old_val = 0;

void sys_irq_restore_enable(void)
{
	Xil_ExceptionEnableMask(int_old_val);
}

void sys_irq_save_disable(void)
{
	unsigned int value = 0;

	value = mfcpsr() & XIL_EXCEPTION_ALL;

	if (value != int_old_val) {
		Xil_ExceptionDisableMask(XIL_EXCEPTION_ALL);
		int_old_val = value;
	}
}

void metal_machine_cache_flush(void *addr, unsigned int len)
{
	if (!addr & !len)
		Xil_DCacheFlush();
	else
		Xil_DCacheFlushRange((intptr_t)addr, len);
}

void metal_machine_cache_invalidate(void *addr, unsigned int len)
{
	if (!addr & !len)
		Xil_DCacheInvalidate();
	else
		Xil_DCacheInvalidateRange((intptr_t)addr, len);
}

/**
 * @brief poll function until some event happens
 */
void __attribute__((weak)) metal_generic_default_poll(void)
{
	asm volatile("wfi");
}

void metal_machine_io_mem_map(metal_phys_addr_t pa,
			       size_t size, unsigned int flags)
{
	unsigned int r5_flags;

	/* Assume DEVICE_SHARED if nothing indicates this is memory.  */
	r5_flags = DEVICE_SHARED;
	if ((flags & METAL_SHARED_MEM)) {
		r5_flags = NORM_SHARED_NCACHE;
		if ((flags & METAL_CACHE_WB)) {
			r5_flags = NORM_SHARED_WB_WA;
		} else if ((flags & METAL_CACHE_WT)) {
			r5_flags = NORM_SHARED_WT_NWA;
		}
	} else {
		r5_flags = NORM_NSHARED_NCACHE;
		if ((flags & METAL_CACHE_WB)) {
			r5_flags = NORM_NSHARED_WB_WA;
		} else if ((flags & METAL_CACHE_WT)) {
			r5_flags = NORM_NSHARED_WT_NWA;
		}
	}

	Xil_SetMPURegion(pa, size, r5_flags | PRIV_RW_USER_RW);
	return;
}
