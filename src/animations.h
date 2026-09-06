#include "lvgl.h"
#ifndef ANIMATIONS_H
#define ANIMATIONS_H
extern lv_anim_t *activeDieAnimation;
extern lv_anim_t *activeDieInvAnimation;
void anim_callback_set_image_frame(lv_anim_t* a, int32_t v);
void anim_y(void);
void anim_x(void);
void anim_x_cb(void * var, int32_t v);
void anim_y_cb(void * var, int32_t v);
lv_anim_t * rollany_Animation( lv_obj_t *TargetObject, lv_image_dsc_t **imgSet ,int delay);
void rollGifs(int location);
#endif