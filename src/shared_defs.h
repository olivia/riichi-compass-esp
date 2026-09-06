#include "lvgl.h"

#ifndef SHARED_DEFINES_H
#define SHARED_DEFINES_H

#define ADC_RESOLUTION 4095.0 // 12-bit ADC
#define V_REF 3.3             // ESP32 internal reference voltage
#define BATTERY_PIN 4
#define RLCD_SCK_PIN 11
#define RLCD_MOSI_PIN 12
#define RLCD_DC_PIN 5
#define RLCD_CS_PIN 40
#define RLCD_RST_PIN 41
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))
typedef enum {
  WT_RON = 0,
  WT_TSUMO = 1,
  WT_DRAW = 2,
  WT_NONE = -1
} WinType_t;

#define LIGHT_SENSORS_COUNT 4
typedef enum {
    STATE_STABLE_COVERED       = 5, // Light is low and steady (covered)

    STATE_STABLE_DARK       = 0, // Light is low and steady (covered)
    STATE_STABLE_BRIGHT     = 1, // Light is high and steady (uncovered)
    STATE_RAPID_COVERING    = 2, // Signal is plunging sharply (hand dropping over it)
    STATE_RAPID_UNCOVERING  = 3, // Signal is spiking sharply (hand lifting away)
    STATE_GRADUAL_CHANGING  = 4  // Signal is moving slowly (room lights dimming/brightening)
} SensorState_t;
typedef struct _anim_user_data_t {
    lv_obj_t *target;
    lv_image_dsc_t **imgset;
    int32_t imgset_size;
    int32_t val;
} anim_user_data_t;

#endif