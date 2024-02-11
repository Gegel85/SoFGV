//
// Created by Gegel85 on 24/01/24
//

#ifndef SOFGV_CHARACTERPARAMS_HPP
#define SOFGV_CHARACTERPARAMS_HPP

// Wakeup protections
#define GRAB_INVUL_STACK 9
#define PROJ_INVUL_STACK 4

// Limit
#define MAX_LIMIT_EFFECT_TIMER (15 << 4)
#define DEC_LIMIT_EFFECT_TIMER(x) ((x) -= (1 << 4))

// Guard bar
#define TMP_GUARD_MAX (300)
#define GUARD_REGEN_CD_BLOCK (240)
#define GUARD_REGEN_CD_PARRY (60)
#define GUARD_REGEN_PER_FRAME (2)

// Negative penalty
#define MINIMUM_STALLING_STACKING (-800)
#define STALLING_HIT_REMOVE (75)
#define STALLING_BLOCK_REMOVE (25)
#define STALLING_BEING_HIT_REMOVE (50)
#define STALLING_BLOCKING_REMOVE (50)
#define STALLING_BLOCK_WIPE_THRESHOLD (350)
#define MAXIMUM_STALLING_STACKING (2700)
#define BACKING_STALLING_FACTOR (1.1f)
#define FORWARD_STALLING_FACTOR (0.9f)
#define GROUND_STALLING_FACTOR (0.25f)
#define MAX_STALLING_FACTOR_HEIGHT (0.f)
#define PASSIVE_STALLING_FACTOR (0.25f)
// 0.1% per frame max
#define METER_PENALTY_EQUATION(val, maxval) (((val) - START_STALLING_THRESHOLD) * (maxval) / (float)(MAXIMUM_STALLING_STACKING - START_STALLING_THRESHOLD) / 1000)

// Idle anim
#define IDLE_ANIM_FIRST 600
#define IDLE_ANIM_CHANCE 50
#define IDLE_ANIM_CD_MIN 300
#define IDLE_ANIM_CD_MAX 600

// Costs
#define TYPED_PARRY_COST 75
#define NEUTRAL_PARRY_COST 125
#define INSTALL_COST 200
#define INSTALL_DURATION 30

#define WALL_SLAM_HITSTUN_INCREASE 30
#define GROUND_SLAM_HITSTUN_INCREASE 30

#define REFLECT_PERCENT 60

// Buffer
#define COMBINATION_LENIENCY 4
#define MAX_FRAME_IN_BUFFER 60

#define SPECIAL_INPUT_BUFFER_PERSIST 10
#define DASH_BUFFER_PERSIST 6
#define HJ_BUFFER_PERSIST 6

#define NORMAL_BUFFER 4
#define HJ_BUFFER 10
#define DASH_BUFFER 15
#define QUARTER_CIRCLE_BUFFER 10
#define DP_BUFFER 15
#define HALF_CIRCLE_BUFFER 20
#define SPIRAL_BUFFER 30
#define CHARGE_PART_BUFFER 10
#define CHARGE_BUFFER 5
#define CHARGE_TIME 25

#endif //SOFGV_CHARACTERPARAMS_HPP
