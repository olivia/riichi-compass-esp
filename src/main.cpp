#include <Arduino.h>
#include "shared_defs.h"
#include "display_bsp.h"
#include "lvgl_bsp.h"
#include "button_bsp.h"
#include "lvgl.h" 
#include "ui.h"
#include "ui_helpers.h"
#include "animations.h"
#include <Preferences.h>
#include "riichi_helpers.h"
#include "i2c_bsp.h"
#include "codec_bsp.h"
#include <esp_timer.h>
#include "Roll6.c"

Preferences preferences;



static bool pinsReady = false;
static lv_subject_t br_name_subject;
static char curr_br_name_text_buffer[128];
static char prev_br_name_text_buffer[128];
static int muteVoices;
static lv_subject_t br_wind_subject;
static char curr_br_wind_text_buffer[128];
static char prev_br_wind_text_buffer[128];

static lv_subject_t br_score_subject;

static lv_subject_t tr_name_subject;
static char curr_tr_name_text_buffer[128];
static char prev_tr_name_text_buffer[128];

static lv_subject_t tr_wind_subject;
static char curr_tr_wind_text_buffer[128];
static char prev_tr_wind_text_buffer[128];

static lv_subject_t tr_score_subject;

static lv_subject_t tl_name_subject;
static char curr_tl_name_text_buffer[128];
static char prev_tl_name_text_buffer[128];


static lv_subject_t tl_wind_subject;
static char curr_tl_wind_text_buffer[128];
static char prev_tl_wind_text_buffer[128];

static lv_subject_t tl_score_subject;

static lv_subject_t bl_name_subject;
static char curr_bl_name_text_buffer[128];
static char prev_bl_name_text_buffer[128];


static lv_subject_t bl_wind_subject;
static char curr_bl_wind_text_buffer[128];
static char prev_bl_wind_text_buffer[128];
static lv_subject_t bl_score_subject;


static lv_subject_t modal_title_subject;
static char curr_modal_title_text_buffer[128];
static char prev_modal_title_text_buffer[128];

static lv_subject_t selectoption_subject;
static char curr_selectoption_text_buffer[128];
static char prev_selectoption_text_buffer[128];

static lv_subject_t optiondescription_subject;
static char curr_optiondescription_text_buffer[128];
static char prev_optiondescription_text_buffer[128];

static lv_subject_t honbacount_subject;
static lv_subject_t doracount_subject;


static lv_subject_t * subject_scores[] = {&br_score_subject, &tr_score_subject, &tl_score_subject, &bl_score_subject}; 
static lv_subject_t * subject_winds[] = {&br_wind_subject, &tr_wind_subject, &tl_wind_subject, &bl_wind_subject}; 

static int sensorMultiplierIndex;

static int rotationIndex = 0;
static int modalContentStep = -1;
static int lightState[] = {0,0,0,0};

static uint32_t counter = 0;
static uint32_t frames = 0;
static uint32_t last_report_frames = 0;
static uint32_t last_report_ms = 0;
static uint32_t fps_x100 = 0;
static uint32_t frame_us = 0;
static uint32_t flush_us = 0;
static int tCX = 80;
static int tCY = 30;
static int bCX = 320;
static int bCY = 267;
static int CIRCLE_RAD = 20;
static int modal_state = 0;
I2cMasterBus* I2cbus;
CodecPort *codecport = NULL;
static uint8_t *audio_ptr = NULL;
static bool is_Music = true;
EventGroupHandle_t CodecGroups;
static bool showSensors = false;
lv_obj_t * modalContentWinArr[9];
const lv_image_dsc_t* roundNumImgs[] ={&ui_img_1193668880, &ui_img_641408904, &ui_img_2052151144, &ui_img_1690547480};
const lv_image_dsc_t* playerWindsImgs[] ={&ui_img_toncb_png, &ui_img_nancb_png, &ui_img_shack_png, &ui_img_peicb_png};
static float sensorMultipliers[] = {0.25,0.5,0.75,1,1.25,1.5,2,3};
// Filter histories (shifted left by 8 bits for fixed-point math)
static int32_t emaFastScaledArr[LIGHT_SENSORS_COUNT] = { -1, -1, -1, -1 }; 
static int32_t emaSlowScaledArr[LIGHT_SENSORS_COUNT] = { -1, -1, -1, -1 };
// State tracking histories for edge detection
static SensorState_t currentStateArr[LIGHT_SENSORS_COUNT] = { STATE_STABLE_DARK, STATE_STABLE_DARK, STATE_STABLE_DARK, STATE_STABLE_DARK };
static SensorState_t previousStateArr[LIGHT_SENSORS_COUNT] = { STATE_STABLE_DARK, STATE_STABLE_DARK, STATE_STABLE_DARK, STATE_STABLE_DARK };
uint16_t stableSettleCounterArr[LIGHT_SENSORS_COUNT] = {0,0,0,0};
unsigned long lastTriggered[LIGHT_SENSORS_COUNT] = {0,0,0,0};
bool isArmedArr[LIGHT_SENSORS_COUNT] = {false,false,false,false}; // Tracks if a sensor is primed to fire
DisplayPort RlcdPort(12, 11, 5, 40, 41, 400, 300);

static void anim_rotation_cb(lv_obj_t * obj, int32_t v)
{
    lv_obj_set_style_transform_rotation(obj, v, 0);
}

static void Lvgl_FlushCallback(lv_display_t *drv, const lv_area_t *area, uint8_t *color_map) {
  int64_t t_start = esp_timer_get_time();
  int pixel_count = 0;
  uint16_t *buffer = (uint16_t *)color_map;
  for (int y = area->y1; y <= area->y2; y++) {
    for (int x = area->x1; x <= area->x2; x++) {
      uint8_t color = (*buffer < 0x7fff) ? ColorBlack : ColorWhite;
      RlcdPort.RLCD_SetPixel(x, y, color);
      buffer++;
            pixel_count++;

    }
  }
    int64_t t_math_done = esp_timer_get_time();

  RlcdPort.RLCD_Display();
    int64_t t_spi_done = esp_timer_get_time();
  int32_t math_ms = (int32_t)(t_math_done - t_start) / 1000;
  int32_t spi_ms  = (int32_t)(t_spi_done - t_math_done) / 1000;
  int32_t total_ms = (int32_t)(t_spi_done - t_start) / 1000;

  // Print results to the serial terminal every 30 frames
  // static int diagnostic_counter = 0;
  // if (++diagnostic_counter >= 30) {
  //   diagnostic_counter = 0;
  //   Serial.printf("[DIAGNOSTIC] Window: (%d,%d) to (%d,%d) | Total Pixels: %d\n", 
  //          area->x1, area->y1, area->x2, area->y2, pixel_count);
  //   Serial.printf("             -> Loop Processing Math : %d ms\n", math_ms);
  //   Serial.printf("             -> SPI Bus Transmission : %d ms\n", spi_ms);
  //   Serial.printf("             -> Total Driver Staging : %d ms\n", total_ms);
  // }

  lv_disp_flush_ready(drv);
}



//s0-3
int selectPins[] =
{
0,1,2,3
};

int signalPin = 17;

int n = sizeof(selectPins)/sizeof(selectPins[0]);
int sensorMultiplierNum = sizeof(sensorMultipliers)/sizeof(sensorMultipliers[0]);

// ESWN
char* discarders[] = {"apple", "banana", "cherry", "dewberry"};

char* validHan[] = {"1", "2", "3", "4", "5", "6", "7","8", "9", "10", "11", "12", "Y", "2XY", "3XY", "4XY"}; 
char* validFu[] = {"20", "25", "30", "40", "50", "60", "70","80", "90", "100", "110"}; 
#define TENPAISTATUSMODAL_INDEX 5
bool modalStepVisited[] = {
false, false, false, false, false, 
// TENPAI MODALS
false,false,false,false};
#define PLAYER_NUM 4
char* tenpaiModalOptions[] = {"NOTEN", "TENPAI"};
bool playerTenpaiFlags[] = {false, false, false, false};

static int hanIndex = 0;
static int fuIndex = 0;
static int discarderIndex = 0;
// starting from bottom right going counterclockwise
static int scoreOffsets[] = {0, 0, 0, 0};

bool modalOpen = false;
static WinType_t winType = WT_NONE;
static int seatRotationNum = 0;
static int honbaCount = 0;
static int carryoverDora = 0;
static int currentDoraCount = 0;


int playersInTenpai() {
  int tenpaiPlayers = 0;
  for (int i=0; i< PLAYER_NUM; i++) {
    tenpaiPlayers += playerTenpaiFlags[i];
  }  
  return tenpaiPlayers;
}

int calcTenpaiPayments() {
  int payments[] = {0, 3000, 1500, 1000, 0};
  return payments[playersInTenpai()];
}

int _scores[] = {
  25000, 25000, 25000, 25000
};

void setupObservers() {
  lv_subject_init_string(&br_name_subject, curr_br_name_text_buffer, prev_br_name_text_buffer, sizeof(curr_br_name_text_buffer), discarders[0]);
  lv_subject_init_string(&tr_name_subject, curr_tr_name_text_buffer, prev_tr_name_text_buffer, sizeof(curr_tr_name_text_buffer), discarders[1]);
  lv_subject_init_string(&tl_name_subject, curr_tl_name_text_buffer, prev_tl_name_text_buffer, sizeof(curr_tl_name_text_buffer), discarders[2]);
  lv_subject_init_string(&bl_name_subject, curr_bl_name_text_buffer, prev_bl_name_text_buffer, sizeof(curr_bl_name_text_buffer), discarders[3]);
  lv_subject_init_string(&modal_title_subject, curr_modal_title_text_buffer, prev_modal_title_text_buffer, sizeof(curr_modal_title_text_buffer), "DRAW");
  lv_subject_init_string(&selectoption_subject, curr_selectoption_text_buffer, prev_selectoption_text_buffer, sizeof(curr_selectoption_text_buffer), tenpaiModalOptions[0]);
  lv_subject_init_string(&optiondescription_subject, curr_optiondescription_text_buffer, prev_optiondescription_text_buffer, sizeof(curr_optiondescription_text_buffer), discarders[0]);

  //scores
  for (int i = 0; i < 4; i++) {
    lv_subject_init_int(subject_scores[i], _scores[i]);
  } 
  lv_subject_init_int(&honbacount_subject, 0);
  lv_subject_init_int(&doracount_subject, 0);

  //cui_offset_score = ui_comp_get_child(cui_player, UI_COMP_PLAYER_PANEL30_OFFSETSCORE);


  lv_label_bind_text(ui_comp_get_child(ui_player1, UI_COMP_PLAYER_CONTAINER_NAME), &br_name_subject, "%s");
  lv_label_bind_text(ui_comp_get_child(ui_player1, UI_COMP_PLAYER_PANEL8_SCORE), &br_score_subject, "%d");

  lv_label_bind_text(ui_comp_get_child(ui_player2, UI_COMP_PLAYER_CONTAINER_NAME),&tr_name_subject, "%s");
  lv_label_bind_text(ui_comp_get_child(ui_player2, UI_COMP_PLAYER_PANEL8_SCORE), &tr_score_subject, "%d");

  lv_label_bind_text(ui_comp_get_child(ui_player3, UI_COMP_PLAYER_CONTAINER_NAME), &tl_name_subject, "%s");
  lv_label_bind_text(ui_comp_get_child(ui_player3, UI_COMP_PLAYER_PANEL8_SCORE), &tl_score_subject, "%d");

  lv_label_bind_text(ui_comp_get_child(ui_player4, UI_COMP_PLAYER_CONTAINER_NAME), &bl_name_subject, "%s");
  lv_label_bind_text(ui_comp_get_child(ui_player4, UI_COMP_PLAYER_PANEL8_SCORE), &bl_score_subject, "%d");
  
  lv_label_bind_text(ui_modaltitle, &modal_title_subject, "%s");
  lv_label_bind_text(ui_selectoption, &selectoption_subject, "%s");
  lv_label_bind_text(ui_optiondescription, &optiondescription_subject, "%s");
  lv_label_bind_text(ui_doracount, &doracount_subject, "%d");
  lv_label_bind_text(ui_honbacount, &honbacount_subject, "%d");
}



// IMAGES AND IMAGE SETS
///////////////////// ANIMATIONS ////////////////////


void updateBatteryLevel(lv_timer_t * timer) {
    
      // Read raw ADC value from GPIO4
    int raw_adc = analogRead(BATTERY_PIN);
    
    // Convert ADC to voltage measured at the pin
    float pin_voltage = (raw_adc / ADC_RESOLUTION) * V_REF;
    
    // Multiply by 3 to reverse the hardware voltage divider circuit
    float battery_voltage = pin_voltage * 3.0; 
    
    // Map the voltage roughly to 0-100% battery capacity
    float percentage = ((battery_voltage - 2.5) / (4.2 - 2.5)) * 100.0;
    if (percentage > 80) {
        lv_label_set_text(ui_batteryicon, LV_SYMBOL_BATTERY_FULL);
    } else if (percentage > 50) {
        lv_label_set_text(ui_batteryicon, LV_SYMBOL_BATTERY_3);
    } else if (percentage > 20) {
        lv_label_set_text(ui_batteryicon, LV_SYMBOL_BATTERY_2);
    } else if (percentage > 5) {
        lv_label_set_text(ui_batteryicon, LV_SYMBOL_BATTERY_1);
    } else {
        lv_label_set_text(ui_batteryicon, LV_SYMBOL_BATTERY_EMPTY);
    }

}




void resetModalStepHistory() {
  size_t length = sizeof(modalStepVisited) / sizeof(modalStepVisited[0]);
  for (int i = 0; i< length; i++) {
    modalStepVisited[i] = false;
  }
  modalContentStep = -1;
}



void applyTsumoOffsets(int playerId, int dealerScore, int nonDealerScore ) {
  int doraScore = lv_subject_get_int(&doracount_subject) * 1000;
  for (int i = 0; i < 4; i++) {
    if (playerId == i) {
      scoreOffsets[i] = doraScore + dealerScore + nonDealerScore * 2;
    } else {
      scoreOffsets[i] = ((isDealer(i, seatRotationNum)) ? -dealerScore : -nonDealerScore); 
    }
  }
}
lv_obj_t * getPlayerComponent(int i) {
  switch (i) {
    case 0:
      return ui_player1;
    case 1:
      return ui_player2;
    case 2:
      return ui_player3;
    case 3:
      return ui_player4;
    default:
      return NULL;
  }
}

// assign offset scores, score are counter clockwise from winner
void repaintPlayers() {
  lv_obj_t * cui_player;
  lv_obj_t * cui_offset_score_container;
  lv_obj_t * cui_offset_score;

  for (int i=0; i<4;i++){
    cui_player = getPlayerComponent(i);
    cui_offset_score_container = ui_comp_get_child(cui_player, UI_COMP_PLAYER_PANEL30);
    cui_offset_score = ui_comp_get_child(cui_player, UI_COMP_PLAYER_PANEL30_OFFSETSCORE);

    if (scoreOffsets[i]) {
      char result[100]; 
      // Concatenate safely
      snprintf(result, sizeof(result), "%+d", scoreOffsets[i]);      
      lv_label_set_text(cui_offset_score, result);
      lv_obj_remove_flag(cui_offset_score_container, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(cui_offset_score_container, LV_OBJ_FLAG_HIDDEN);
    }
  }
}
// Automatically calculate the number of elements in the array

lv_obj_t * getModalStepContent(int index) {
  // Safe bounds check: Ensure index is between 0 and the maximum array size
  if (index >= 0 && index < 9) {
    return modalContentWinArr[index];
  }
  return NULL;
}


void setupStepValues(int step) {
  if (step < 4 && step >= 0) {
    lv_subject_copy_string(&modal_title_subject, discarders[rotationIndex]);
  } else if (step >= TENPAISTATUSMODAL_INDEX && step < TENPAISTATUSMODAL_INDEX + 4) {
    lv_subject_copy_string(&modal_title_subject, "DRAW");
    lv_subject_copy_string(&optiondescription_subject, discarders[(step - TENPAISTATUSMODAL_INDEX + rotationIndex) & 3]);
    lv_subject_copy_string(&selectoption_subject, tenpaiModalOptions[playerTenpaiFlags[step- TENPAISTATUSMODAL_INDEX]]);
  }
  else if (step == 4) {
    lv_subject_copy_string(&modal_title_subject, "RESULTS");
    if (winType == WT_DRAW) {
      lv_obj_add_flag(ui_handscore, LV_OBJ_FLAG_HIDDEN);
    } else if (hanIndex >= 4) {
      lv_obj_remove_flag(ui_scoredescription, LV_OBJ_FLAG_HIDDEN);
      lv_obj_add_flag(ui_fucontainerdetail, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_remove_flag(ui_scoredescription, LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(ui_funumdetail, LV_OBJ_FLAG_HIDDEN);
      lv_label_set_text(ui_funumdetail, validFu[fuIndex]);
    }
    lv_label_set_text(ui_handetails, validHan[hanIndex]);

    if (!isDealer(rotationIndex, seatRotationNum) && winType == WT_TSUMO) {
      lv_obj_remove_flag(ui_auxpayerdetail, LV_OBJ_FLAG_HIDDEN);
    } else {
      lv_obj_add_flag(ui_auxpayerdetail, LV_OBJ_FLAG_HIDDEN);
    }
    int score = getHandScore(hanIndex,fuIndex, rotationIndex, seatRotationNum);
    int doraScore = lv_subject_get_int(&doracount_subject) * 1000;
    int honbaScore = lv_subject_get_int(&honbacount_subject) * 300;
    int indivHonbaScore = honbaScore / 3;
    if (winType == WT_RON) {
      // RON
      char payerScore[100];
      int payerScoreNum = roundScore(score) + honbaScore;
      sprintf(payerScore, "%d", payerScoreNum);
      scoreOffsets[rotationIndex] = payerScoreNum + doraScore;
      scoreOffsets[discarderIndex] = -payerScoreNum;
      lv_label_set_text(ui_payernamedetail, discarders[discarderIndex]);
      lv_label_set_text(ui_payerscoredetail, payerScore);
    } else if (winType == WT_TSUMO && isDealer(rotationIndex, seatRotationNum)) {
      // DEALER TSUMO
      char payerScore[100];
      int payerScoreNum = roundScore(score/3) + indivHonbaScore;
      applyTsumoOffsets(rotationIndex, payerScoreNum, payerScoreNum);
      sprintf(payerScore, "%d", payerScoreNum);
      lv_label_set_text(ui_payernamedetail, "ALL");
      lv_label_set_text(ui_payerscoredetail, payerScore);

    } else if (winType == WT_TSUMO && !isDealer(rotationIndex, seatRotationNum)) {
      char dealerScore[100];
      char nonDealerScore[100];
      int nonDealerScoreNum = roundScore(score/4) + indivHonbaScore;
      int dealerScoreNum = roundScore(score/2) + indivHonbaScore;
      applyTsumoOffsets(rotationIndex, dealerScoreNum, nonDealerScoreNum);
      sprintf(dealerScore, "%d", dealerScoreNum);
      sprintf(nonDealerScore, "%d", nonDealerScoreNum);
      // NON-DEALER TSUMO
      lv_label_set_text(ui_payernamedetail, "DEALER");
      lv_label_set_text(ui_auxpayernamedetail, "NON-DEALER");
      lv_label_set_text(ui_payerscoredetail, dealerScore);
      lv_label_set_text(ui_auxpayerscoredetail, nonDealerScore);
    } else if (winType == WT_DRAW) {
      // DRAW
      int negPayments[] = {0, -1000, -1500, -3000, 0};
      int posPayments[] = {0, 3000, 1500, 1000, 0};
      int tenpaiNum = playersInTenpai();
      char payerScore[100];
      sprintf(payerScore, "%d", -negPayments[tenpaiNum]);

      lv_label_set_text(ui_payernamedetail, "NOTEN");
      lv_label_set_text(ui_payerscoredetail, payerScore);

      for (int i = 0; i < 4; i++) {
        scoreOffsets[(rotationIndex+i)&3] = playerTenpaiFlags[i] ? posPayments[tenpaiNum] : negPayments[tenpaiNum];
      }
    }
    repaintPlayers();
  }
  
}
void initStepValues(int step) {
  switch (step) {
    case 1: 
      hanIndex = 0;
      lv_label_set_text(ui_hancount, validHan[hanIndex]);
      break;
    case 2:
      fuIndex = 2;
      lv_label_set_text(ui_fucount, validFu[fuIndex]);
      break;
    case 3:
      discarderIndex = (rotationIndex +1) & 3;
      lv_label_set_text(ui_discarder, discarders[discarderIndex]);
      break;
    case 4:
      break;
    case 5:
    case 6:
    case 7:
    case 8:
      playerTenpaiFlags[step- TENPAISTATUSMODAL_INDEX] = 0;
    break;

    default:
      break;
  }
}

void setRotation(int rotIdx) {
  rotationIndex = rotIdx;
  lv_obj_set_style_transform_rotation(ui_modalcontainer,(3600 - 900 * rotIdx) % 3600, LV_PART_MAIN| LV_STATE_DEFAULT);
  lv_obj_update_layout(ui_modalcontainer); 
}



void setupModalContentArr() {
  modalContentWinArr[0] = ui_win1;
  modalContentWinArr[1] = ui_win2;
  modalContentWinArr[2] = ui_win3;
  modalContentWinArr[3] = ui_win4;
  modalContentWinArr[4] = ui_win5;
  modalContentWinArr[TENPAISTATUSMODAL_INDEX] = ui_selectoptioncontent;
  modalContentWinArr[TENPAISTATUSMODAL_INDEX + 1] = ui_selectoptioncontent;
  modalContentWinArr[TENPAISTATUSMODAL_INDEX + 2] = ui_selectoptioncontent;
  modalContentWinArr[TENPAISTATUSMODAL_INDEX + 3] = ui_selectoptioncontent;
}
void resetModalLabels() {
  modalContentStep = 0;
  hanIndex = 0;
  fuIndex = 2;
  discarderIndex = 0;
  lv_label_set_text(ui_fucount, validFu[fuIndex]);
  lv_label_set_text(ui_hancount, validHan[hanIndex]);
  lv_label_set_text(ui_discarder, discarders[(rotationIndex+1)&3]);
  lv_subject_copy_string(&modal_title_subject, discarders[rotationIndex]);
}
void setModalVisibility(bool visible) {
  if (visible) {
    if (modalContentStep > 0){
      lv_obj_add_flag(getModalStepContent(modalContentStep), LV_OBJ_FLAG_HIDDEN);
      
    }
    resetModalLabels();
    lv_obj_add_flag(ui_roundpane, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_diecontainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(ui_statusicons, LV_OBJ_FLAG_HIDDEN);

    lv_obj_remove_flag(ui_modalcontainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(getModalStepContent(0), LV_OBJ_FLAG_HIDDEN);
  } else {
    lv_obj_add_flag(ui_modalcontainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(getModalStepContent(modalContentStep), LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_roundpane, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_diecontainer, LV_OBJ_FLAG_HIDDEN);
    lv_obj_remove_flag(ui_statusicons, LV_OBJ_FLAG_HIDDEN);
    rotationIndex = -1;
    modalContentStep = -1;
  }
}

void transitionToStep(int nextStep) {
  lv_obj_t * prevStepUI = getModalStepContent(modalContentStep);
  lv_obj_t * nextStepUI = getModalStepContent(nextStep);
  if (prevStepUI) {
    lv_obj_add_flag(prevStepUI, LV_OBJ_FLAG_HIDDEN);
  }
  if (!nextStepUI) {
    setModalVisibility(false);
    resetModalStepHistory();
  } else {
    lv_obj_remove_flag(nextStepUI, LV_OBJ_FLAG_HIDDEN);
  }
  modalContentStep = nextStep;
  if (!modalStepVisited[nextStep] && nextStep >= 0) {
    initStepValues(nextStep);
    modalStepVisited[nextStep] = true;
  }
  setupStepValues(nextStep);
}

void handleModalStep1(int button) {
  switch(button) {
    case 0:
      winType = WT_DRAW;
      transitionToStep(TENPAISTATUSMODAL_INDEX);
      break;
    case 1:
      // Ron
      winType = WT_RON;
      transitionToStep(1);
      break;
    case 2:
      // Tsumo
      winType = WT_TSUMO;
      transitionToStep(1);
      break;
    case 3:
      transitionToStep(-1);
      break;
    default: 
      break;
  }

}
void handleModalStep2(int button) {
  bool skipDiscard = !!winType;
  bool skipFu = hanIndex >= 4;
  switch(button) {
    case 0:
      transitionToStep(skipFu ? skipDiscard ? 4 : 3 : 2);
      break;
    case 1:
      hanIndex = MIN(hanIndex + 1, 15);
      lv_label_set_text(ui_hancount, validHan[hanIndex]);
      break;
    case 2:
      hanIndex = MAX(hanIndex -1, 0);
      lv_label_set_text(ui_hancount, validHan[hanIndex]);

      break;
    case 3:
      lv_obj_add_flag(getModalStepContent(1), LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(getModalStepContent(0), LV_OBJ_FLAG_HIDDEN);
      modalContentStep--;

      break;
    default: 
      break;
  }
}

void handleModalStep3(int button ) {
  bool skipDiscard = !!winType;
  switch(button) {
    case 0:
      transitionToStep(skipDiscard ? 4 : 3);
      break;
    case 1:
      // Ron
      fuIndex = MIN(fuIndex + 1, 10);
      lv_label_set_text(ui_fucount, validFu[fuIndex]);
      break;
    case 2:
      // Tsumo
      // Ron
      fuIndex = MAX(fuIndex -1, 0);
      lv_label_set_text(ui_fucount, validFu[fuIndex]);

      break;
    case 3:
      lv_obj_add_flag(getModalStepContent(2), LV_OBJ_FLAG_HIDDEN);
      lv_obj_remove_flag(getModalStepContent(1), LV_OBJ_FLAG_HIDDEN);
      modalContentStep = 1;

      break;
    default: 
      break;
  }
}

void handleModalStep4(int button) {
  int newIndex;
  bool skipFu = hanIndex >= 4;

  switch(button) {
    case 0:
      transitionToStep(4);
      break;
    case 1:
      newIndex = (discarderIndex + 1) & 3;
      newIndex += ((newIndex == rotationIndex) ? 1 : 0);
      discarderIndex = newIndex & 3;
      lv_label_set_text(ui_discarder, discarders[discarderIndex]);
      break;
    case 2:
      newIndex = (4+ discarderIndex - 1) & 3;
      newIndex += ((newIndex == rotationIndex) ? -1 : 0);
      discarderIndex = (newIndex+ 4) & 3;
      lv_label_set_text(ui_discarder, discarders[discarderIndex]);
      break;
    case 3:
      transitionToStep(skipFu ? 1 : 2);
      break;
    default: 
      break;
  }
}

void applyPlayerOffsets() {
  for (int i=0; i< 4; i++) {
    int newScore = lv_subject_get_int(subject_scores[i]) + scoreOffsets[i];
    lv_subject_set_int(subject_scores[i], newScore);
    scoreOffsets[i] = 0;
    _scores[i] = newScore;
  }
  repaintPlayers();
}

// struct ModalSelectOptionRoute {
//   char titleLabel[20];
//   char prevLabel[20];
//   char nextLabel[20];
//   char selectLabel[20];
//   char * optionLabels[];
// };

// ModalSelectOptionRoute routes[] = {
//   {""}
// }

void setPlayerWindImgs() {
    for (int i = 0; i < 4; i++) {
      lv_image_set_src(ui_comp_get_child(getPlayerComponent((seatRotationNum + i) & 3), UI_COMP_PLAYER_WINDIMG), playerWindsImgs[i]);
  }
}
void incrementRound() {
  if (winType == WT_NONE) {
    Serial.println("Somehow incremented round without ending it?");
    return;
  }
  int dealerIndex = seatRotationNum & 3;

  bool changeWinds = (rotationIndex != dealerIndex && winType != WT_DRAW) || (winType == WT_DRAW && !playerTenpaiFlags[(4+ dealerIndex - rotationIndex) & 3]);
  bool resetHonba = changeWinds && winType != WT_DRAW;
  seatRotationNum += changeWinds ? 1 : 0;
  honbaCount = resetHonba ? 0 : (honbaCount + 1);
  setPlayerWindImgs();
  if (winType == WT_TSUMO || winType == WT_RON) {
    carryoverDora = 0;
    currentDoraCount = 0;
  }
  else {
    carryoverDora += currentDoraCount;
    currentDoraCount = 0;
  }
  if (changeWinds) {
    lv_image_set_src(ui_centerroundnum, roundNumImgs[seatRotationNum & 3]);
    lv_image_set_src(ui_centerroundwind, !((seatRotationNum>>2)&1) ? &ui_img_587546794 : &ui_img_centernan_png);
  }
  lv_subject_set_int(&honbacount_subject, honbaCount);
  lv_subject_set_int(&doracount_subject, carryoverDora);

}

void clearPlayerOffsets() {
  for (int i = 0; i < 4; i++) {
    scoreOffsets[i] = 0;
  }
  repaintPlayers();
}


void handleTenpaiStatusModal(int button) {
  int newIndex;
  bool skipFu = hanIndex >= 4;
  int tenpaiPlayerIndex = modalContentStep - TENPAISTATUSMODAL_INDEX;

  switch(button) {
    case 0:
      //playerTenpaiFlags
      lv_subject_copy_string(&selectoption_subject, tenpaiModalOptions[0]);
      transitionToStep(modalContentStep == (TENPAISTATUSMODAL_INDEX + 3) ? (TENPAISTATUSMODAL_INDEX - 1) : (modalContentStep + 1));
      break;
    case 1:
    case 2:
      playerTenpaiFlags[tenpaiPlayerIndex] = !playerTenpaiFlags[tenpaiPlayerIndex];
      lv_subject_copy_string(&selectoption_subject, tenpaiModalOptions[playerTenpaiFlags[tenpaiPlayerIndex]]);
      break;
    case 3:      
      lv_subject_copy_string(&selectoption_subject, tenpaiModalOptions[0]);
      transitionToStep(modalContentStep == TENPAISTATUSMODAL_INDEX ? 0 : (modalContentStep - 1));
      break;
    default: 
      break;
  }
}


void handleModalStep5(int button) {
  bool skipDiscard = !!winType;
  bool isDraw = winType == 2;
  bool skipFu = hanIndex >= 4;

    switch(button) {
    case 0:
      incrementRound();
      applyPlayerOffsets();
      transitionToStep(-1);
      break;
    case 1:
      break;
    case 2:
      break;
    case 3:
      transitionToStep(isDraw ? (TENPAISTATUSMODAL_INDEX + 3) : skipDiscard ? skipFu ? 1 : 2 : 3);
      clearPlayerOffsets();
      break;
    default: 
      break;
    }
}

void (*modalHandlers[])(int) = {
  handleModalStep1,
  handleModalStep2,
  handleModalStep3,
  handleModalStep4,
  handleModalStep5,
  handleTenpaiStatusModal,
  handleTenpaiStatusModal,
  handleTenpaiStatusModal,
  handleTenpaiStatusModal
};

void updateMultiplier(int button) {
  int newIndex = (sensorMultiplierNum + sensorMultiplierIndex + (button < 2 ? 1: -1))% sensorMultiplierNum;
  sensorMultiplierIndex = newIndex;
  preferences.begin("ecompass", false);
  preferences.putInt("sensorMultiplierIndex", sensorMultiplierIndex);
  preferences.end();
}

// 3. Automatically calculate the number of handlers
#define HANDLERS_ARR_SIZE (sizeof(modalHandlers) / sizeof(modalHandlers[0]))

void handleButton(int button) {
  lv_obj_t * current_screen = lv_screen_active();

  Serial.printf("Button pressed: %d, current_screen: %p, modalContentStep: %d\n", button, current_screen, modalContentStep);

  int offsetButton = (button + 4 - rotationIndex) & 3;
  if (current_screen == ui_Screen1 && modalContentStep >=0 && modalContentStep< HANDLERS_ARR_SIZE) {
    modalHandlers[modalContentStep](offsetButton);
  } else if (current_screen == ui_Screen1) {
    setRotation(button);
    setModalVisibility(true);
  } else {
    updateMultiplier(button);
  }
}

void handleLongButton(int button) {
  lv_obj_t * current_screen = lv_screen_active();
  if (current_screen == ui_Screen1) {
    rollGifs(button);
  }
}



void resetPlayerTenpaiFlags() {
  for (int i = 0; i<4; i++) {
    playerTenpaiFlags[i] = false;
  }
}


void BOOT_LoopTask(void *arg) {
  for (;;) {
    EventBits_t even = xEventGroupWaitBits(BootButtonGroups, (0x01 | 0x02 | 0x04), pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));
    if (even & 0x01) {
    } else if (even & 0x02) {
    }
  }
}

void Codec_LoopTask(void *arg) {
  bool is_eco = 0;
  for (;;) {
    EventBits_t even = xEventGroupWaitBits(CodecGroups, (0x01 | 0x02 | 0x04| 0x08), pdTRUE, pdFALSE, pdMS_TO_TICKS(8 * 1000));
    if (even) 
      {
      int indexArgs = even;
      int index = 0;
      while (indexArgs > 0 && (indexArgs & 0x01) != 1) {
        index++;
        indexArgs >>= 1;
      }
      
      codecport->CodecPort_SetSpeakerVol(90);
      uint32_t bytes_sizt;
      size_t bytes_write = 0;
      uint8_t *data_ptr = codecport->CodecPort_GetPcmData(&bytes_sizt, index);
      while (bytes_write < bytes_sizt) {
        codecport->CodecPort_PlayWrite(data_ptr, 256);
        data_ptr += 256;
        bytes_write += 256;
        if (!is_Music)
          break;
      }
      codecport->CodecPort_SetSpeakerVol(100);
    } else {
    }
  }
}

void screen1TimeoutTask(lv_timer_t * timer) {
  // sensor id -> user
  int sensorMap[] = {1,2,0,3};
  int riichi_count = 0; 
  for (int i=0; i<4;i++) {
      lv_obj_t * riichi_icon;
      riichi_icon = ui_comp_get_child(getPlayerComponent(sensorMap[i]), UI_COMP_PLAYER_CONTAINER_DOT);
      if (!lv_obj_has_flag(riichi_icon, LV_OBJ_FLAG_HIDDEN) && currentStateArr[i] != STATE_STABLE_COVERED ) {
        lv_obj_add_flag(riichi_icon, LV_OBJ_FLAG_HIDDEN);
        lv_subject_set_int(subject_scores[sensorMap[i]], _scores[sensorMap[i]]);
      } else if (lv_obj_has_flag(riichi_icon, LV_OBJ_FLAG_HIDDEN)&& currentStateArr[i] == STATE_STABLE_COVERED ) {
        lv_obj_remove_flag(riichi_icon, LV_OBJ_FLAG_HIDDEN);
        lv_subject_set_int(subject_scores[sensorMap[i]], -1000 + _scores[sensorMap[i]]);
      }
    riichi_count += (currentStateArr[i] == STATE_STABLE_COVERED) ? 1 : 0;
  }
  if (currentDoraCount != riichi_count) {
    currentDoraCount = riichi_count;
    lv_subject_set_int(&doracount_subject, riichi_count + carryoverDora);
  }
  

}
void screen2TimeoutTask() {
  lv_bar_set_value(ui_Bar1, lightState[0], LV_ANIM_ON); 
  lv_bar_set_value(ui_Bar2, lightState[1], LV_ANIM_ON); 
  lv_bar_set_value(ui_Bar3, lightState[2], LV_ANIM_ON); 
  lv_bar_set_value(ui_Bar4, lightState[3], LV_ANIM_ON);
}
void handleScreenTimeoutTask(lv_timer_t * timer) {
  lv_obj_t * current_screen = lv_screen_active();
  lv_obj_t * next_screen = NULL;

  // 1. Identify which screen is currently on the display
  if (current_screen == ui_Screen1) {
    screen1TimeoutTask(timer);
  } 
  else if (current_screen == ui_Screen2) {
    screen2TimeoutTask();
  } 


}

void KEY_LoopTask(void *arg) {
  for (;;) {
    EventBits_t even = xEventGroupWaitBits(GP18ButtonGroups, (0x01 | 0x02 | 0x04), pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));
    if (even & 0x01) {
      preferences.begin("ecompass", false);
      muteVoices = !muteVoices;
      preferences.putInt("mute", muteVoices); // Save new value to flash

      preferences.end();
      if (Lvgl_lock(-1)) {
        lv_label_set_text(ui_speakericon, muteVoices ? LV_SYMBOL_MUTE : LV_SYMBOL_VOLUME_MAX);
        Lvgl_unlock();
      }

    } else if (even & 0x02) {
      is_Music = true;
      xEventGroupSetBits(CodecGroups, 0x04);
    } else if (even & 0x04) {
      if (Lvgl_lock(-1)) {
        lv_obj_t * current_screen = lv_screen_active();
        lv_obj_t * next_screen = NULL;

        // 1. Identify which screen is currently on the display
        if (current_screen == ui_Screen1) {
            next_screen = ui_Screen2;
            showSensors = true;
        } 
        else if (current_screen == ui_Screen2) {
            next_screen = ui_Screen1;
            showSensors = false;
        } 
        // 2. Perform the safe transition if a target was found
        if (next_screen != NULL) {
          lv_screen_load_anim(next_screen, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
        }

        Lvgl_unlock();
      }

    }
  }
}

void TL_LoopTask(void *arg) {
  for (;;) {
    EventBits_t even = xEventGroupWaitBits(TLButtonGroups, (0x01 | 0x02 | 0x04), pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));
    if (even & 0x01) {
      if (Lvgl_lock(-1)) {
        handleButton(2);
        Lvgl_unlock();
      }

    } else if (even & 0x02) {
      if (Lvgl_lock(-1)) {
        Lvgl_unlock();

      }
    } else if (even & 0x04) {
      if (Lvgl_lock(-1)) {
        handleLongButton(2);
        Lvgl_unlock();
      }
    }
  }
}

void TR_LoopTask(void *arg) {
  for (;;) {
    
    EventBits_t even = xEventGroupWaitBits(TRButtonGroups, (0x01 | 0x02 | 0x04), pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));

    if (even & 0x01) {

      if (Lvgl_lock(-1)) {
        handleButton(1);
        Lvgl_unlock();
      }
    } else if (even & 0x02) {

    } else if (even & 0x04) {
      if (Lvgl_lock(-1)) {
        handleLongButton(1);
        Lvgl_unlock();
      }
    }
  }
}


void BR_LoopTask(void *arg) {
  for (;;) {
    EventBits_t even = xEventGroupWaitBits(BRButtonGroups, (0x01 | 0x02 | 0x04), pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));
    if (even & 0x01) {
    if (Lvgl_lock(-1)) {
        handleButton(0);
        Lvgl_unlock();
      }    
    } else if (even & 0x02) {
    } else if (even & 0x04) {
      modalOpen = !modalOpen;
      if (Lvgl_lock(-1)) {
        handleLongButton(0);

        Lvgl_unlock();
      }
    }
  }
}

void BL_LoopTask(void *arg) {
  for (;;) {
    EventBits_t even = xEventGroupWaitBits(BLButtonGroups, (0x01 | 0x02 | 0x04), pdTRUE, pdFALSE, pdMS_TO_TICKS(2000));
    if (even & 0x01) {
      if (Lvgl_lock(-1)) {
          handleButton(3);
          Lvgl_unlock();
      }    
    } else if (even & 0x02) {
    } else if (even & 0x04) {
      if (Lvgl_lock(-1)) {
        handleLongButton(3);
        Lvgl_unlock();
      }
    }
  }
}



void processLightSensor(int rawValue, int sensor_id) {
    long currTime = millis();
    SensorState_t prevState = currentStateArr[sensor_id];
    SensorState_t currState = (rawValue > 1000) ? STATE_STABLE_DARK : currentStateArr[sensor_id];
    if (prevState == STATE_STABLE_COVERED && currState == STATE_STABLE_DARK) {
      currentStateArr[sensor_id] = currState;
    }
    else if (prevState != STATE_STABLE_COVERED && rawValue < 250 && (lastTriggered[sensor_id] + 5000 * 1) < currTime) {
        is_Music = true;

        currState = STATE_STABLE_COVERED;
        currentStateArr[sensor_id] = currState;
        lastTriggered[sensor_id] = currTime;
        if (!muteVoices) {
          xEventGroupSetBits(CodecGroups, 1 << sensor_id);
        }
        
    }
    else if (prevState != STATE_STABLE_COVERED && rawValue < 250) {
        currState = STATE_STABLE_COVERED;
        currentStateArr[sensor_id] = currState;
    }
    previousStateArr[sensor_id] = currState;
    
}

void SENSOR_LoopTask(void *arg) {
  const TickType_t xSettleDelay = pdMS_TO_TICKS(1); // 1ms for hardware stabilization
  const TickType_t xLoopDelay = pdMS_TO_TICKS(50);  // 50ms cycle rate for the whole task

  for (;;) {
    if (pinsReady) {
    for(int i=12; i<16; i++)
    {
    
      int s0 = (i>>0)&1;
      int s1 = (i>>1)&1;
      int s2 = (i>>2)&1;
      int s3 = (i>>3)&1;
      digitalWrite(selectPins[0], s0);
      digitalWrite(selectPins[1], s1);
      digitalWrite(selectPins[2], s2);
      digitalWrite(selectPins[3], s3);
      vTaskDelay(xSettleDelay);
      switchState[i-12] = analogRead(signalPin)>2048;
    }
    if (modalContentStep == -1) {
      for(int i=8; i<12; i++)
      {
        int s0 = (i>>0)&1;
        int s1 = (i>>1)&1;
        int s2 = (i>>2)&1;
        int s3 = (i>>3)&1;
        digitalWrite(selectPins[0], s0);
        digitalWrite(selectPins[1], s1);
        digitalWrite(selectPins[2], s2);
        digitalWrite(selectPins[3], s3);
        vTaskDelay(xSettleDelay);
        int rawVal = MIN(analogRead(signalPin) * sensorMultipliers[sensorMultiplierIndex], 4095.0);
        lightState[i-8] = floor(180.0*rawVal/4095.0);
        processLightSensor(rawVal, i-8);
      }
    }
  } else {
      vTaskDelay(xSettleDelay);
  }
  }
}




unsigned long previousMillis = 0; 
const unsigned long interval = 2000; // Interval in milliseconds (1 second)



void setupTimers() {
  lv_timer_create(handleScreenTimeoutTask, 100, NULL);
  lv_timer_create(updateBatteryLevel, 30000, NULL);
}




void setupPreferences() {
  preferences.begin("ecompass", false);
  // Read stored value, default to 0 if it doesn't exist yet
  muteVoices = preferences.getInt("mute", 0);
  sensorMultiplierIndex = preferences.getInt("sensorMultiplierIndex",3);
  lv_label_set_text(ui_speakericon, muteVoices ? LV_SYMBOL_MUTE : LV_SYMBOL_VOLUME_MAX);
  // Close preferences (optional, but good practice)
  preferences.end();
}


static bool isFirstPaint = true;

static void on_screen_first_paint(void *user_data) {
    if (!isFirstPaint) return;
    isFirstPaint = false;
    // This executes when the screen is loaded and painted
  Serial.println("Screen painted for the first time. Starting GIF roll...");
}

void setup()
{
  Serial.begin(115200);

    // 2. WAIT for the computer's Serial Monitor to connect.
    // The internal USB CDC driver will hold the code right here!
  while (!Serial) {
      delay(10); 
  }

  // 3. Give the terminal program an extra moment to open its window
  Serial.println("Serial Monitor connected. Starting setup...");
  I2cbus = new I2cMasterBus(14, 13, 1); 
  audio_ptr = (uint8_t *)heap_caps_malloc(288 * 1000 * sizeof(uint8_t), MALLOC_CAP_SPIRAM);
  assert(audio_ptr);
  randomSeed(analogRead(0));

   RlcdPort.RLCD_Init();

   Lvgl_PortInit(300, 300, Lvgl_FlushCallback);

   uint32_t start_time = millis(); 

   if (Lvgl_lock(-1)) {
    ui_init();
    Serial.printf("UI initialized in %lu ms\n", millis() - start_time);
    setupPreferences();
    setupModalContentArr();
    setPlayerWindImgs();
    setupObservers();
    setupTimers();
    updateBatteryLevel(NULL);
    repaintPlayers();
    rollGifs(0);
    Lvgl_unlock();


   }
  CodecGroups = xEventGroupCreate();
  Custom_ButtonInit();
  Multiplexer_ButtonInit();
  codecport = new CodecPort(*I2cbus, "S3_RLCD_4_2");
  codecport->CodecPort_SetInfo("es8311 & es7210", 1, 16000, 2, 16);
  codecport->CodecPort_SetSpeakerVol(100);
  codecport->CodecPort_SetMicGain(35);
  //xTaskCreatePinnedToCore(BOOT_LoopTask, "BOOT_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(KEY_LoopTask, "KEY_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TL_LoopTask, "TL_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(TR_LoopTask, "TR_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(BR_LoopTask, "BR_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(BL_LoopTask, "BL_LoopTask", 4 * 1024, NULL, 2, NULL, 1);
  xTaskCreatePinnedToCore(Codec_LoopTask, "Codec_LoopTask", 4 * 1024, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(SENSOR_LoopTask, "SENSOR_LoopTask", 4 * 1024, NULL, 4, NULL, 1);
  for(int i=0; i<n; i++)
  {
    pinMode(selectPins[i],OUTPUT);
  }
  pinsReady = true;
  Serial.println("Setup complete.");

}


void loop()
{
}
