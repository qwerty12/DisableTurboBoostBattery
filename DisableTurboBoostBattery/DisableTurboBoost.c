/*
 * Copyright (c) 2012 Adam Strzelecki
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Description:
 *   Disables upon load and re-enables upon unload TurboBoost Intel Core
 *   CPU feature using MSR register MSR_IA32_MISC_ENABLE (0x1a0)
 */

#include <libkern/libkern.h>
#include <mach/mach_types.h>
#include <i386/proc_reg.h>

#include "DisableTurboBoost.h"

extern void mp_rendezvous_no_intrs(
		void (*action_func)(void *),
		void *arg);

const uint64_t disableTurboBoost = 0x4000000000LL;

static void disable_tb_cb(__unused void * param_not_used) {
	wrmsr64(MSR_IA32_MISC_ENABLE, rdmsr64(MSR_IA32_MISC_ENABLE) | disableTurboBoost);
}

static void enable_tb_cb(__unused void * param_not_used) {
	wrmsr64(MSR_IA32_MISC_ENABLE, rdmsr64(MSR_IA32_MISC_ENABLE) & ~disableTurboBoost);
}

void disable_tb() {
	uint64_t prev = rdmsr64(MSR_IA32_MISC_ENABLE);
	mp_rendezvous_no_intrs(disable_tb_cb, NULL);
	printf("Disabled Turbo Boost: %llx -> %llx\n", prev, rdmsr64(MSR_IA32_MISC_ENABLE));
}

void enable_tb() {
	uint64_t prev = rdmsr64(MSR_IA32_MISC_ENABLE);
	mp_rendezvous_no_intrs(enable_tb_cb, NULL);
	printf("(Re-)enabled Turbo Boost: %llx -> %llx\n", prev, rdmsr64(MSR_IA32_MISC_ENABLE));
}
