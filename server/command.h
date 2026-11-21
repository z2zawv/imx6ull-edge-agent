
#ifndef COMMAND_H
#define COMMAND_H

#include <stddef.h>
#include "robot_state.h"

// 处理机器人控制指令
void process_command(
    const char *command,
    RobotState *robot_state,
    char *response,
    size_t response_size
);

#endif
