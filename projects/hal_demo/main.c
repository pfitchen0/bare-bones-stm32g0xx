#include <stdint.h>

#include "hal/gpio.h"

static const uint32_t kBlinkDelayIterations = 1000000;

static const Gpio kLed = {.port = kGpioC, .pin = 6};

void DelayIterations(uint32_t iterations) {
    while (iterations != 0) {
        iterations--;
    }
}

int main() {
    GpioSettings led_settings = {.mode = kOutput, .otype = kPushPull, .ospeed = kHigh, .pupd = kNone, .afsel = 0};
    ConfigureGpio(kLed, led_settings);

    while(1) {
        SetGpio(kLed, !(GetGpio(kLed)));
        DelayIterations(kBlinkDelayIterations);
    }

    return 0;
}
