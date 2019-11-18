// SPDX-License-Identifier: GPL-2.0
#include <linux/delay.h>

#include "debug.h"
#include "aura-gpu-reg.h"

#define mmMM_INDEX            0x0000
#define mmMM_DATA             0x0001

struct aura_reg_context {
    struct aura_reg_service service;
    spinlock_t              lock;
    resource_size_t         base;
    resource_size_t         size;
    void __iomem            *data;
    uint32_t                last_index;
    struct pci_dev          *pci_dev;
};

#if defined(DEBUG_GPU_REG)

static const char *reg_name(uint32_t reg)
{
    /* NOTE - These names are only valid for polaris cards */
    switch (reg) {
        case 0x16f4:
            return "GENERIC_I2C_CONTROL           ";
        case 0x16f5:
            return "GENERIC_I2C_INTERRUPT_CONTROL ";
        case 0x16f6:
            return "GENERIC_I2C_STATUS            ";
        case 0x16f7:
            return "GENERIC_I2C_SPEED             ";
        case 0x16f8:
            return "GENERIC_I2C_SETUP             ";
        case 0x16f9:
            return "GENERIC_I2C_TRANSACTION       ";
        case 0x16fa:
            return "GENERIC_I2C_DATA              ";
        case 0x16fb:
            return "GENERIC_I2C_PIN_SELECTION     ";
        case 0x16fc:
            return "GENERIC_I2C_PIN_DEBUG         ";
    }

    return "<null>";
}

#define log_reg_read(_reg, _value)\
    AURA_DBG("Reg Read: %s %x", reg_name(_reg), _value);

#define log_reg_write(_reg, _value)\
    AURA_DBG("Reg Write: %s %x", reg_name(_reg), _value);

#else

#define log_reg_read(_reg, _value)
#define log_reg_write(_reg, _value)

#endif

int32_t reg_read (
    struct aura_reg_service *service,
    uint32_t reg
){
    struct aura_reg_context *ctx = container_of(service, struct aura_reg_context, service);
    uint32_t ret;

    if (unlikely(service == NULL)) {
        AURA_ERR("mmio has not been configured");
        return 0;
    }

    if ((reg * 4) < ctx->size)
        ret = readl(((void __iomem *)ctx->data) + (reg * 4));
    else {
        unsigned long flags;

        spin_lock_irqsave(&ctx->lock, flags);
        writel((reg * 4), ((void __iomem *)ctx->data) + (mmMM_INDEX * 4));
        ret = readl(((void __iomem *)ctx->data) + (mmMM_DATA * 4));
        spin_unlock_irqrestore(&ctx->lock, flags);
    }

    log_reg_read(reg, ret);

    return ret;
}

void reg_write (
    struct aura_reg_service *service,
    uint32_t reg,
    uint32_t value
){
    struct aura_reg_context *ctx = container_of(service, struct aura_reg_context, service);

    if (unlikely(service == NULL)) {
        AURA_ERR("mmio has not been configured");
        return;
    }

    log_reg_write(reg, value);

    if ((reg * 4) < ctx->size)
        writel(value, ((void __iomem *)ctx->data) + (reg * 4));
    else {
        unsigned long flags;

        spin_lock_irqsave(&ctx->lock, flags);
        writel((reg * 4), ((void __iomem *)ctx->data) + (mmMM_INDEX * 4));
        writel(value, ((void __iomem *)ctx->data) + (mmMM_DATA * 4));
        spin_unlock_irqrestore(&ctx->lock, flags);
    }
}

uint32_t reg_field_get_value_ex(
    const struct reg_fields *field
){
    return (field->value & field->mask) >> field->shift;
}

uint32_t reg_field_set_value_ex(
    uint32_t value,
    struct reg_fields *field
){
    field->value = (value & ~field->mask) | (field->mask & (field->value << field->shift));

    return field->value;
}

uint32_t reg_update_ex(
    struct aura_reg_service *service,
    uint32_t addr,
    const struct reg_fields *fields,
    ssize_t cnt
){
    uint32_t ret;
    uint32_t value = 0, mask = 0;

    WARN_ON(cnt <= 0);

    while (cnt) {
        value = (value & ~fields->mask) | (fields->mask & (fields->value << fields->shift));
        mask = mask | fields->mask;
        fields++;
        cnt--;
    }

    /* mmio write directly */
    ret = reg_read(service, addr);
    ret = (ret & ~mask) | value;

    reg_write(service, addr, ret);

    return ret;
}

uint32_t reg_set_ex(
    struct aura_reg_service *service,
    uint32_t addr,
    uint32_t init,
    const struct reg_fields *fields,
    ssize_t cnt
){
    uint32_t value = 0, mask = 0;

    WARN_ON(cnt == 0);

    while (cnt) {
        value = (value & ~fields->mask) | (fields->mask & (fields->value << fields->shift));
        mask = mask | fields->mask;
        fields++;
        cnt--;
    }

    /* mmio write directly */
    init = (init & ~mask) | value;
    reg_write(service, addr, init);

    return init;
}

uint32_t reg_get_ex(
    struct aura_reg_service *service,
    uint32_t addr,
    struct reg_fields *fields,
    ssize_t cnt
){
    uint32_t value, i;

    WARN_ON(cnt <= 0);

    value = reg_read(service, addr);

    for (i = 0; i < cnt; i++) {
        fields[i].value = (value & fields[i].mask) >> fields[i].shift;
    }

    return value;
}

uint32_t reg_get_value(
    struct aura_reg_service *service,
    uint32_t addr,
    struct reg_fields *fields,
    uint32_t *value
){
    uint32_t ret = reg_get_ex(service, addr, fields, 1);

    BUG_ON(value == NULL);

    *value = fields->value;
    return ret;
}

uint32_t reg_update_seq_ex(
    struct aura_reg_service *service,
    uint32_t addr,
    const struct reg_fields *fields,
    ssize_t cnt
){
    uint32_t value, i;

    WARN_ON(cnt <= 1);

    value = reg_update_ex(service, addr, fields, 1);

    for (i = 1; i <= cnt; i++) {
        value = reg_set_ex(service, addr, value, &fields[i], 1);
    }

    return value;
}

void reg_wait_ex(
    struct aura_reg_service *service,
    uint32_t addr,
    struct reg_fields *field,
    uint32_t attempts,
    uint32_t timeout
){
    uint32_t value;

    WARN_ON(timeout * attempts <= 200);

    do {
        value = reg_read(service, addr);
        value = (value & field->mask) >> field->shift;

        if (value == field->value) {
            return;
        }

        if (timeout >= 1000)
            msleep(timeout / 1000);
        else if (timeout > 0)
            udelay(timeout);

        attempts--;
    } while(attempts);
}

struct aura_reg_service *aura_gpu_reg_create(
    struct pci_dev *pci_dev
){
    struct aura_reg_context *ctx;
    error_t err = -ENOMEM;

    ctx = kzalloc(sizeof(*ctx), GFP_KERNEL);
    if (!ctx)
        goto error;

    spin_lock_init(&ctx->lock);

    ctx->pci_dev = pci_dev;
    ctx->base    = pci_resource_start(pci_dev, 5);
    ctx->size    = pci_resource_len(pci_dev, 5);
    ctx->data    = ioremap(ctx->base, ctx->size);

    if (ctx->data == NULL)
        goto error_free_context;

    AURA_DBG("Mapped ports at base=0x%16llx, size=0x%16llx to %p", ctx->base, ctx->size, ctx->data);

    return &ctx->service;

error_free_context:
    kfree(ctx);
error:
    return ERR_PTR(err);
}

void aura_gpu_reg_destroy (
    struct aura_reg_service *service
){
    struct aura_reg_context *ctx = container_of(service, struct aura_reg_context, service);

    if (WARN_ON(service == NULL))
        return;

    AURA_DBG("Unmapping mm data");
    iounmap(ctx->data);
    kfree(ctx);
}
