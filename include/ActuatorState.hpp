#pragma once

struct ActuatorState {
    bool fan = false;
    bool pump = false;
    bool growLight = false;
    int ventPercent = 20;
};
