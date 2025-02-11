#ifndef HAL_GPIO_H_
#define HAL_GPIO_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct {
    volatile uint32_t moder, otyper, ospeedr, pupdr, idr, odr, bsrr, lckr, afrl, afrh, brr;
} GpioRegisters;

typedef enum {
    kGpioA, kGpioB, kGpioC, kGpioD, kGpioE, kGpioF
} GpioPort;

typedef struct {
    GpioPort port;
    uint8_t pin;
} Gpio;

typedef enum {
    kInput, kOutput, kAlternateFunction, kAnalog
} GpioMode;

typedef enum {
    kPushPull, kOpenDrain
} GpioOutputType;

typedef enum {
    kVeryLow, kLow, kHigh, kVeryHigh
} GpioOutputSpeed;

typedef enum {
    kNone, kPullUp, kPullDown
} GpioPullUpPullDown;

typedef struct {
    GpioMode mode;
    GpioOutputType otype;
    GpioOutputSpeed ospeed;
    GpioPullUpPullDown pupd;
    uint8_t afsel;
} GpioSettings;

void ConfigureGpio(Gpio gpio, GpioSettings settings);

void SetGpio(Gpio gpio, bool state);

bool GetGpio(Gpio gpio);

#endif  // HAL_GPIO_H_