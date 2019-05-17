/*
 * ARM GICv2 test suite
 *
 * Copyright (c) 2018-2019 Luc Michel <luc.michel@greensocs.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The views and conclusions contained in the software and documentation are those
 * of the authors and should not be interpreted as representing official policies,
 * either expressed or implied, of the FreeBSD Project.
 */

#ifndef _TESTSUITE_VEXPRESS_GIC_H
#define _TESTSUITE_VEXPRESS_GIC_H

#include <stdint.h>
#include <stdbool.h>

#include "sys.h"
#include "drivers/arm/gic.h"
#include "misc.h"

struct gic_irq_info {
    int irq;
    int cpu_target;
    unsigned int group;
    unsigned int subgroup;
    unsigned int subprio;
};

#define GIC_IRQ_INFO_END { -1, 0, 0, 0, 0 }

struct gic_cpu_info {
    bool en_grp0;
    bool en_grp1;
    bool grp0_to_fiq;
    bool eoi_mode;
    bool cbpr;

    uint8_t bpr, abpr;
};

struct gic_virt_info {
    bool en;

    /* Maintenance interrupts */
    bool vgrp1die;
    bool vgrp1eie;
    bool vgrp0die;
    bool vgrp0eie;
    bool npie;
    bool lrenpie;
    bool uie;
};

struct gic_info {
    const struct gic_irq_info *irqs;

    struct gic_cpu_info cpu;

    struct gic_virt_info virt;
    struct gic_cpu_info vcpu;
};

static inline void gic_end_of_irq(phys_addr_t base, uint32_t irq)
{
    writel(base + GICC_EOIR, irq);
    mb();
    ibarrier();
}

static inline uint32_t gic_ack_irq(phys_addr_t base)
{
    return readl(base + GICC_IAR);
}

static inline uint32_t gic_running_prio(phys_addr_t base)
{
    return readl(base + GICC_RPR);
}

#ifdef GIC_VIFACE_BASE
static inline int gich_get_next_lr_entry(void)
{
    uint32_t next_lr = readl(GIC_VIFACE_BASE + GICH_ELRSR0);
    return ctz32(next_lr);
}
#endif

#include "trace.h"

#ifdef GIC_VIFACE_BASE
static inline void gich_set_lr_entry(int entry, bool hw, bool grp1,
                                     int state, int prio,
                                     int phys_id, int virt_id)
{
    uint32_t lr =
        (hw << 31)
        | (grp1 << 30)
        | (state << 28)
        | (prio << 23)
        | (phys_id << 10)
        | (virt_id << 0);

    assert(state < 4);
    assert(prio < 32);
    assert(phys_id < 1024);
    assert(virt_id < 1024);

    DPRINTF("about to overwrite entry %d = %x\n",
            entry, readl(GIC_VIFACE_BASE + GICH_LR + (entry << 2)));

    writel(GIC_VIFACE_BASE + GICH_LR + (entry << 2), lr);
    mb();
    ibarrier();
}

static inline uint32_t gich_read(size_t reg)
{
    return readl(GIC_VIFACE_BASE + reg);
}

static inline void gich_write(size_t reg, uint32_t val)
{
    writel(GIC_VIFACE_BASE + reg, val);
    mb();
}
#endif

#ifdef GIC_VCPU_BASE
static inline uint32_t gicv_read(size_t reg)
{
    return readl(GIC_VCPU_BASE + reg);
}

static inline void gicv_write(size_t reg, uint32_t val)
{
    writel(GIC_VCPU_BASE + reg, val);
    mb();
}
#endif

#ifdef GIC_VIFACE_BASE
static inline uint32_t gich_read_hcr(void)
{
    return readl(GIC_VIFACE_BASE + GICH_HCR);
}

static inline uint32_t gich_read_elrsr0(void)
{
    return readl(GIC_VIFACE_BASE + GICH_ELRSR0);
}

static inline uint32_t gich_read_elrsr1(void)
{
    return readl(GIC_VIFACE_BASE + GICH_ELRSR1);
}

static inline uint32_t gich_read_eisr0(void)
{
    return readl(GIC_VIFACE_BASE + GICH_EISR0);
}

static inline uint32_t gich_read_eisr1(void)
{
    return readl(GIC_VIFACE_BASE + GICH_EISR1);
}

static inline void gich_write_hcr(uint32_t val)
{
    writel(GIC_VIFACE_BASE + GICH_HCR, val);
    mb();
}

static inline uint32_t gich_read_misr(void)
{
    return readl(GIC_VIFACE_BASE + GICH_MISR);
}
#endif

static inline uint32_t gich_read_apr(void)
{
    return readl(GIC_VIFACE_BASE + GICH_APR);
}

void gic_configure(const struct gic_info *info);
void gic_teardown(const struct gic_info *info);

#endif
