/*
 * Copyright (C) 2013 Allwinnertech, kevin.z.m <kevin@allwinnertech.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Adjustable factor-based clock implementation
 */

#include <linux/clk-provider.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/string.h>

#include <linux/delay.h>

#include "clk-sunxi.h"
#include "clk-factors.h"


static int sunxi_clk_fators_enable(struct clk_hw *hw)
{
	struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;
    unsigned long reg = readl(factor->reg);

    reg = SET_BITS(config->enshift, 1, reg, 1);
    writel(reg, factor->reg);
    return 0;
}

static void sunxi_clk_fators_disable(struct clk_hw *hw)
{
	struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;
    unsigned long reg = readl(factor->reg);

    reg = SET_BITS(config->enshift, 1, reg, 0);
    writel(reg, factor->reg);
}

static int sunxi_clk_fators_is_enabled(struct clk_hw *hw)
{
	struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;
    unsigned long reg = readl(factor->reg);
    unsigned long val;

    val = GET_BITS(config->enshift, 1, reg);
    return val ? 1 : 0;
}

static unsigned long sunxi_clk_factors_recalc_rate(struct clk_hw *hw, unsigned long parent_rate)
{
    unsigned long reg;
    struct clk_factors_value factor_val;
	struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;

    if(!factor->calc_rate)
        return 0;

    reg = readl(factor->reg);
    if(config->nwidth)
        factor_val.factorn = GET_BITS(config->nshift, config->nwidth, reg);
    else
        factor_val.factorn = 0xffff;

    if(config->kwidth)
        factor_val.factork = GET_BITS(config->kshift, config->kwidth, reg);
    else
        factor_val.factork = 0xffff;

    if(config->mwidth)
        factor_val.factorm = GET_BITS(config->mshift, config->mwidth, reg);
    else
        factor_val.factorm = 0xffff;

    if(config->pwidth)
        factor_val.factorp = GET_BITS(config->pshift, config->pwidth, reg);
    else
        factor_val.factorp = 0xffff;

    if(config->d1width)
        factor_val.factord1 = GET_BITS(config->d1shift, config->d1width, reg);
    else
        factor_val.factord1 = 0xffff;

    if(config->d2width)
        factor_val.factord2 = GET_BITS(config->d2shift, config->d2width, reg);
    else
        factor_val.factord2 = 0xffff;

    if(config->frac) {
        factor_val.frac_mode = GET_BITS(config->modeshift, 1, reg);
        factor_val.frac_freq = GET_BITS(config->outshift, 1, reg);
    } else {
        factor_val.frac_mode = 0xffff;
        factor_val.frac_freq = 0xffff;
    }

    return factor->calc_rate(parent_rate, &factor_val);
}

static long sunxi_clk_factors_round_rate(struct clk_hw *hw, unsigned long rate, unsigned long *prate)
{
    struct clk_factors_value factor_val;
	struct sunxi_clk_factors *factor = to_clk_factor(hw);

    if(!factor->get_factors || !factor->calc_rate)
        return rate;

    factor->get_factors(rate, *prate, &factor_val);
    return factor->calc_rate(*prate, &factor_val);
}

static int sunxi_clk_factors_set_rate(struct clk_hw *hw, unsigned long rate, unsigned long parent_rate)
{
    unsigned long reg;
    struct clk_factors_value factor_val;
	struct sunxi_clk_factors *factor = to_clk_factor(hw);
    struct sunxi_clk_factors_config *config = factor->config;

    if(!factor->get_factors)
        return 0;

    factor->get_factors(rate, parent_rate, &factor_val);

    reg = readl(factor->reg);
    if(config->nwidth)
        reg = SET_BITS(config->nshift, config->nwidth, reg, factor_val.factorn);

    if(config->kwidth)
        reg = SET_BITS(config->kshift, config->kwidth, reg, factor_val.factork);

    if(config->mwidth)
        reg = SET_BITS(config->mshift, config->mwidth, reg, factor_val.factorm);

    if(config->pwidth)
        reg = SET_BITS(config->pshift, config->pwidth, reg, factor_val.factorp);

    if(config->d1width)
        reg = SET_BITS(config->d1shift, config->d1width, reg, factor_val.factord1);

    if(config->d2width)
        reg = SET_BITS(config->d2shift, config->d2width, reg, factor_val.factord2);

    if(config->frac) {
        reg = SET_BITS(config->modeshift, 1, reg, factor_val.frac_mode);
        reg = SET_BITS(config->outshift, 1, reg, factor_val.frac_freq);
    }

    writel(reg, factor->reg);
    if(GET_BITS(config->enshift, 1, reg))
    {
    		while(1)
    		{
    			  reg = readl(factor->lock_reg);
    				if(GET_BITS(factor->lock_bit, 1, reg))
    				break;
    		}   	
    }
    return 0;
}


static const struct clk_ops clk_factors_ops = {
    .enable = sunxi_clk_fators_enable,
    .disable = sunxi_clk_fators_disable,
    .is_enabled = sunxi_clk_fators_is_enabled,

    .recalc_rate = sunxi_clk_factors_recalc_rate,
    .round_rate = sunxi_clk_factors_round_rate,
    .set_rate = sunxi_clk_factors_set_rate,
};
void sunxi_clk_get_factors_ops(struct clk_ops* ops)
{
    memcpy(ops,&clk_factors_ops,sizeof(clk_factors_ops));
}
/**
 * clk_register_factors - register a factors clock with
 * the clock framework
 * @dev: device registering this clock
 * @name: name of this clock
 * @parent_name: name of clock's parent
 * @flags: framework-specific flags
 * @reg: register address to adjust factors
 * @config: shift and width of factors n, k, m, p, div1 and div2
 * @get_factors: function to calculate the factors for a given frequency
 * @lock: shared register lock for this clock
 */
struct clk *sunxi_clk_register_factors(struct device *dev,void __iomem *base,spinlock_t *lock,struct factor_init_data* init_data)
{
    struct sunxi_clk_factors *factors;
    struct clk *clk;
    struct clk_init_data init;

    /* allocate the factors */
    factors = kzalloc(sizeof(struct sunxi_clk_factors), GFP_KERNEL);
    if (!factors) {
        pr_err("%s: could not allocate factors clk\n", __func__);
        return ERR_PTR(-ENOMEM);
    }
#ifdef __SUNXI_ALL_CLK_IGNORE_UNUSED__
		init.flags |= CLK_IGNORE_UNUSED;
#endif
    init.name = init_data->name;
    init.ops = init_data->priv_ops?(init_data->priv_ops):(&clk_factors_ops);
    init.flags |=init_data->flags;
    init.parent_names = init_data->parent_names;
    init.num_parents = init_data->num_parents;

    /* struct clk_factors assignments */
    factors->reg = base + init_data->reg;
    factors->lock_reg = base + init_data->lock_reg;   
    factors->lock_bit = init_data->lock_bit; 
    factors->config = init_data->config;
    factors->lock = lock;
    factors->hw.init = &init;
    factors->get_factors = init_data->get_factors;
    factors->calc_rate = init_data->calc_rate;

    /* register the clock */
    clk = clk_register(dev, &factors->hw);

    if (IS_ERR(clk))
        kfree(factors);

    return clk;
}
int sunxi_clk_get_common_factors(struct sunxi_clk_factors_config* f_config,struct clk_factors_value *factor, struct sunxi_clk_factor_freq table[],unsigned long index,unsigned long tbl_size)
{
    if(index >=tbl_size/sizeof(struct sunxi_clk_factor_freq))
      return -1;
    factor->factorn = (table[index].factor >>f_config->nshift)&((1<<(f_config->nwidth))-1);
    factor->factork = (table[index].factor >>f_config->kshift)&((1<<(f_config->kwidth))-1);
    factor->factorm = (table[index].factor >>f_config->mshift)&((1<<(f_config->mwidth))-1);
    factor->factorp = (table[index].factor >>f_config->pshift)&((1<<(f_config->pwidth))-1);   
    factor->factord1 = (table[index].factor >>f_config->d1shift)&((1<<(f_config->d1width))-1); 
    factor->factord2 = (table[index].factor >>f_config->d2shift)&((1<<(f_config->d2width))-1);
    if(f_config->frac)
    {
      factor->frac_mode = (table[index].factor >>f_config->modeshift)&1;
      factor->frac_freq = (table[index].factor >>f_config->outshift)&1;
    }
    return 0;
}
