# Endfield Factory

This project aims to optimize the Automated Industry Complex (AIC) system of the
game Arknights:Endfield. To this goal, we model the system through a
mathematical representation known as Mixted Integer Linear Programming (MILP)
that is then optimized using a solver, here
[OR-Tools](https://developers.google.com/optimization).

The mathematical model aim to maximize the quantity of regional stock bills by
producing the best AIC products according on the mineral limitations (only
Valley IV for now).

## Mathematical Model

### Data

#### The Region Related Data

- $r$ $\in$ $R$: the region
- $bp_r$: the base power provided in the region $r$
- $s_r$: the per element storage in the region $r$

#### The Product Related Data

- $p$ $\in$ $P_r$: the products of the region $r$
- $v_p$: the value of the product $p$ (in stock bills)
- $o_p$: the quantity of originium needed for the product $p$
- $a_p$: the quantity of amethyst needed for the product $p$
- $f_p$: the quantity of ferrium needed for the product $p$
- $t_p$: the production time needed for the product $p$
- $d_p$: the depot length needed for the product $p$'s factory
- $w_p$: the width length needed for the product $p$'s factory
- $h_p$: the height length needed for the product $p$'s factory
- $ps_p$: the number of protocol stash needed for the product $p$'s factory
- $ru_p$: the number of refining unit needed for the product $p$'s factory
- $su_p$: the number of shredding unit needed for the product $p$'s factory
- $mu_p$: the number of moulding unit needed for the product $p$'s factory
- $spu_p$: the number of seed picking unit needed for the product $p$'s factory
- $plu_p$: the number of planting unit needed for the product $p$'s factory
- $gu_p$: the number of gearing unit needed for the product $p$'s factory
- $ftu_p$: the number of fitting unit needed for the product $p$'s factory
- $flu_p$: the number of filling unit needed for the product $p$'s factory
- $pku_p$: the number of packaging unit needed for the product $p$'s factory
- $gu_p$: the number of grinding unit needed for the product $p$'s factory

#### The Mineral Related Data

- $m$ $\in$ $M_r$: the minerals of the region $r$
- $l_m$: the limit of the mineral $m$ in the region

#### The Fuel Related Data

- $fu$ $\in$ $FU_r$ $\subseteq$ $P_r$: the fuel of the region $r$
- $p_{fu}$: the power that the fuel $fu$ provides
- $d_{fu}$: the duration during which the fuel $fu$ stands before being consumed

#### The Area Related Data

- $a$ $\in$ $A_r$: the areas of the region $r$
- $z_a$: the maximum number of zipline in the area $a$
- $d_a$: the maximum number of defenses in the area $a$
- $m_a$: the number of mining rig needed in the area $a$
- $w_a$: the PAC width in the area $a$
- $h_a$: the PAC height in the area $a$
- $wd_a$: the PAC depot width in the area $a$
- $hd_a$: the PAC depot height in the area $a$

#### The Facility Related Data

- $fa$ $\in$ $FA_r$: the facilities of the region $r$
- $p_{fa}$: the power usage of the facility $fa$

### Variables

- $\text{produce}_p$: the quantity of product $p$ to be produced (per minute)
- $\text{factory}_p^a$: the number of factory for product $p$ to be place in
  area $a$
- $\text{active}_fu$: the quantity of fuel $fu$ that must be active for
  providing power

### Objective

$$\max{\left[\sum_{p\in P_r} \left(produce_p . v_p\right) - \sum_{fu\in FU_r} \left(active_{fu} . \dfrac{60}{d_{fu}} . v_{fu}\right) \right]}$$

### Constraints

- $$produce_{fu} - \dfrac{60}{d_{fu}} . active_{fu} \geq 0 \quad \forall fu\in FU_r$$
  (cannot consume more fuel that produced)
- $$produce_{p} \leq \dfrac{s_r}{48 * 60} \forall p\in P_r\setminus FU_r$$ (we
  don't want to fill the region storage within 48 hours)
- $$$$
