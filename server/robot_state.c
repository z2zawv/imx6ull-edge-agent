
#include "robot_state.h"

// 初始化机器人
void robot_state_init(RobotState *state)
{
    state->running = 0;
    state->linear_velocity = 0.0;
    state->angular_velocity = 0.0;
}

// 设置机器人运动参数
void robot_state_move(RobotState *state, double v, double w)
{
    state->running = 1;
    state->linear_velocity = v;
    state->angular_velocity = w;
}

// 停止机器人
void robot_state_stop(RobotState *state)
{
    state->running = 0;
    state->linear_velocity = 0.0;
    state->angular_velocity = 0.0;
}
