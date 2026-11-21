
#ifndef ROBOT_STATE_H
#define ROBOT_STATE_H

// 定义机器人状态
typedef struct
{
    int    running;
    double linear_velocity;
    double angular_velocity;

} RobotState;

// 初始化机器人状态
void robot_state_init(RobotState *state);

// 设置机器人运动状态
void robot_state_move(RobotState *state, double v, double w);

// 停止机器人
void robot_state_stop(RobotState *state);

#endif
