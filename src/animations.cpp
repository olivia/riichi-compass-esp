#include <Arduino.h>
#include "animations.h"
#include "lvgl.h"
#include "ui.h"
#include "ui_helpers.h"
#include "shared_defs.h"

const lv_image_dsc_t *ui_imgset_roll_6_frame__[5] = {&ui_img_roll_6_frame_0_png, &ui_img_roll_6_frame_1_png, &ui_img_roll_6_frame_2_png, &ui_img_roll_6_frame_3_png, &ui_img_roll_6_frame_8_png};
const lv_image_dsc_t *ui_imgset_roll_5_frame__[5] = {&ui_img_roll_6_frame_0_png, &ui_img_roll_6_frame_1_png, &ui_img_roll_6_frame_2_png, &ui_img_roll_6_frame_3_png, &ui_img_roll_5_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_4_frame__[5] = {&ui_img_roll_6_frame_0_png, &ui_img_roll_6_frame_1_png, &ui_img_roll_6_frame_2_png, &ui_img_roll_6_frame_3_png, &ui_img_roll_4_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_3_frame__[5] = {&ui_img_roll_6_frame_0_png, &ui_img_roll_6_frame_1_png, &ui_img_roll_6_frame_2_png, &ui_img_roll_6_frame_3_png, &ui_img_roll_3_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_2_frame__[5] = {&ui_img_roll_6_frame_0_png, &ui_img_roll_6_frame_1_png, &ui_img_roll_6_frame_2_png, &ui_img_roll_6_frame_3_png, &ui_img_roll_2_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_1_frame__[5] = {&ui_img_roll_6_frame_0_png, &ui_img_roll_6_frame_1_png, &ui_img_roll_6_frame_2_png, &ui_img_roll_6_frame_3_png, &ui_img_roll_1_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_6_inv_frame__[5] = {&ui_img_roll_6_inv_frame_0_png, &ui_img_roll_6_inv_frame_1_png, &ui_img_roll_6_inv_frame_2_png, &ui_img_roll_6_inv_frame_3_png, &ui_img_roll_6_inv_frame_8_png};
const lv_image_dsc_t *ui_imgset_roll_5_inv_frame__[5] = {&ui_img_roll_6_inv_frame_0_png, &ui_img_roll_6_inv_frame_1_png, &ui_img_roll_6_inv_frame_2_png, &ui_img_roll_6_inv_frame_3_png, &ui_img_roll_5_inv_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_4_inv_frame__[5] = {&ui_img_roll_6_inv_frame_0_png, &ui_img_roll_6_inv_frame_1_png, &ui_img_roll_6_inv_frame_2_png, &ui_img_roll_6_inv_frame_3_png, &ui_img_roll_4_inv_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_3_inv_frame__[5] = {&ui_img_roll_6_inv_frame_0_png, &ui_img_roll_6_inv_frame_1_png, &ui_img_roll_6_inv_frame_2_png, &ui_img_roll_6_inv_frame_3_png, &ui_img_roll_3_inv_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_2_inv_frame__[5] = {&ui_img_roll_6_inv_frame_0_png, &ui_img_roll_6_inv_frame_1_png, &ui_img_roll_6_inv_frame_2_png, &ui_img_roll_6_inv_frame_3_png, &ui_img_roll_2_inv_frame_7_png};
const lv_image_dsc_t *ui_imgset_roll_1_inv_frame__[5] = {&ui_img_roll_6_inv_frame_0_png, &ui_img_roll_6_inv_frame_1_png, &ui_img_roll_6_inv_frame_2_png, &ui_img_roll_6_inv_frame_3_png, &ui_img_roll_1_inv_frame_7_png};
const lv_image_dsc_t **allRolls[] = {ui_imgset_roll_6_frame__,ui_imgset_roll_5_frame__,ui_imgset_roll_4_frame__,ui_imgset_roll_3_frame__,ui_imgset_roll_2_frame__,ui_imgset_roll_1_frame__};
const lv_image_dsc_t **allRollsInv[] = {ui_imgset_roll_6_inv_frame__,ui_imgset_roll_5_inv_frame__,ui_imgset_roll_4_inv_frame__,ui_imgset_roll_3_inv_frame__,ui_imgset_roll_2_inv_frame__,ui_imgset_roll_1_inv_frame__};
lv_anim_t *activeDieAnimation;
lv_anim_t *activeDieInvAnimation;

 void anim_x_cb(void * var, int32_t v) {
    lv_obj_set_x((lv_obj_t *)var, v);
}

 void anim_y_cb(void * var, int32_t v) {
    lv_obj_set_y((lv_obj_t *)var, v);
}

/**
 * Create a playback animation
 */
void anim_x(void)
{
    lv_obj_t * obj = ui_diecontainer;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 10, 50);
    lv_anim_set_duration(&a, 1000);
    lv_anim_set_repeat_count(&a, 0);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, anim_x_cb);
    lv_anim_set_values(&a, -20, 0);
    lv_anim_start(&a);
}

/**
 * Create a playback animation
 */
void anim_y(void)
{
    lv_obj_t * obj = ui_diecontainer;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, 10, 50);
    lv_anim_set_duration(&a, 1000);
    lv_anim_set_repeat_count(&a, 0);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&a, anim_y_cb);
    lv_anim_set_values(&a, 20, 0);
    lv_anim_start(&a);
}

void anim_callback_set_image_frame(lv_anim_t* a, int32_t v)
{
    int newV;
    ui_anim_user_data_t *usr = (ui_anim_user_data_t *)a->user_data;
    int lastIndex = usr->imgset_size - 1;
    int map[] = {128, 256};
    newV = v;
    usr->val = v;
    if ( v<0 ) {
      v=0;
    }
    newV = v >= 20 ? lastIndex : v % lastIndex;
    lv_image_set_src(usr->target, usr->imgset[newV]);
}

lv_anim_t * rollany_Animation( lv_obj_t *TargetObject, lv_image_dsc_t **imgSet ,int delay)
{
lv_anim_t *out_anim;
anim_user_data_t *PropertyAnimation_0_user_data = (anim_user_data_t *)lv_malloc(sizeof(anim_user_data_t));
PropertyAnimation_0_user_data->target = TargetObject;
PropertyAnimation_0_user_data->imgset = imgSet;
PropertyAnimation_0_user_data->imgset_size = sizeof(ui_imgset_roll_6_frame__)/(sizeof(lv_image_dsc_t*));
PropertyAnimation_0_user_data->val = -1;
lv_anim_t PropertyAnimation_0;
lv_anim_init(&PropertyAnimation_0);
lv_anim_set_duration(&PropertyAnimation_0, 1000);
lv_anim_set_user_data(&PropertyAnimation_0, PropertyAnimation_0_user_data);
lv_anim_set_custom_exec_cb(&PropertyAnimation_0, anim_callback_set_image_frame );
lv_anim_set_values(&PropertyAnimation_0, 0, 21 );
lv_anim_set_path_cb( &PropertyAnimation_0, lv_anim_path_linear);
lv_anim_set_delay( &PropertyAnimation_0, delay + 0 );
lv_anim_set_deleted_cb( &PropertyAnimation_0, _ui_anim_callback_free_user_data );
lv_anim_set_reverse_duration(&PropertyAnimation_0, 0);
lv_anim_set_reverse_delay(&PropertyAnimation_0, 0);
 lv_anim_set_repeat_count(&PropertyAnimation_0, 0);
lv_anim_set_repeat_delay(&PropertyAnimation_0, 0);
lv_anim_set_early_apply( &PropertyAnimation_0, false );
 lv_anim_set_get_value_cb(&PropertyAnimation_0, &_ui_anim_callback_get_image_frame );
out_anim = lv_anim_start(&PropertyAnimation_0);

return out_anim;
}
void rollGifs(int location) {
  if (activeDieAnimation) {
    lv_anim_del_all();
  }


  int coords[] = {0,88, 88, 0, 0, -88, -88, 0};
  int width= 91;
  int height = 29;

  lv_image_set_src(ui_Image21, &ui_img_roll_6_frame_0_png);
  lv_image_set_src(ui_Image22, &ui_img_roll_6_inv_frame_0_png);

  int random_num_i = random(0, 6);
  int random_num_j = random(0, 6);;
  lv_obj_add_flag(ui_diecontainer, LV_OBJ_FLAG_HIDDEN);
  if (location % 2) {
    lv_obj_set_flex_flow(ui_diecontainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_width(ui_diecontainer, height);
    lv_obj_set_height(ui_diecontainer, width);

  } else {
    lv_obj_set_flex_flow(ui_diecontainer, LV_FLEX_FLOW_ROW);
    lv_obj_set_width(ui_diecontainer, width);
    lv_obj_set_height(ui_diecontainer, height);
    lv_obj_set_y(ui_statusicons, location == 0 ? -88 : 88);

  }

  lv_obj_set_x(ui_diecontainer,coords[location * 2]);
  lv_obj_set_y(ui_diecontainer,coords[location * 2 + 1]);


  if (location % 2) {
    anim_y();
  } else {
    anim_x();
  }
  activeDieAnimation = rollany_Animation(ui_Image21,(lv_image_dsc_t **)allRolls[random_num_i],0);
  activeDieInvAnimation = rollany_Animation(ui_Image22,(lv_image_dsc_t **)allRollsInv[random_num_j],100);
  lv_obj_remove_flag(ui_diecontainer, LV_OBJ_FLAG_HIDDEN);
}