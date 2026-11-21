#include "command.h"
#include "robot_state.h"

#include <stdio.h>
#include <string.h>
#include <math.h>


/*
 * 功能：处理机器人控制指令
 * 参数：
 * command                      完整的机器人指令
 * robot_state                   机器人状态
 * response                     用于保存服务器响应
 * response_size response       响应数组大小
 */
void process_command(
    const char *command,
    RobotState *robot_state,
    char *response,
    size_t response_size)
{
    // 1. STATUS：查询机器人状态
    if(strcmp(command, "STATUS") == 0)
    {
        const char *state =
            robot_state->running ? "RUNNING" : "STOPPED";

        snprintf(response, response_size,
                 "%s v=%.2f w=%.2f\n",
                 state, robot_state->linear_velocity, robot_state->angular_velocity);
    }

    // 2. STOP：停止机器人
    else if(strcmp(command, "STOP") == 0)
    {
        robot_state_stop(robot_state);

        snprintf(response, response_size, "OK,STOP\n");
    }

    // 3. MOVE：控制机器人运动
    else if(strncmp(command, "MOVE ", 5) == 0)
    {
        double v, w;
        char extra;

        int ret = sscanf(command, "MOVE %lf %lf %c",
                         &v, &w, &extra);

        if(ret == 2&& isfinite(v) && isfinite(w))
        {
            robot_state_move(robot_state, v, w);
            snprintf(response, response_size, "OK,MOVE\n");
        }
        else
        {
            snprintf(response, response_size, "MOVE ERROR\n");
        }
    }

    // 4. 未知指令
    else
    {
        snprintf(response, response_size,
                 "ACK: message received\n");
    }
}
