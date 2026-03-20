#ifndef WS2812_EFFECTS_H
#define WS2812_EFFECTS_H

#include <stdint.h>
#include <vector>
#include <algorithm>

// 關鍵：讓所有包含此檔的 .cpp 都能存取同一個 LED_NUM
extern int LED_NUM; 

// 宣告在 main.cpp 實作的底層函數
extern void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b);
extern void ws2812_show();
extern void delay_ms(uint32_t ms);
extern uint32_t Wheel(uint8_t WheelPos);

// --- 燈效函數原型 ---
void eff_1_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_3_rainbow_cycle(int wait);
void eff_4_fire(int Cooling, int Sparking, int SpeedDelay);
void eff_13_cyberpulse(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_breathe_sync(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_color_flow(int wait);
void eff_soft_comet(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_twinkle(uint8_t r, uint8_t g, uint8_t b, int count, int wait);
void eff_dual_fade(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, int wait);

// 補齊 main.cpp 呼叫但缺少的原型
void eff_neon_chase(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_color_bounce(int wait);
void eff_aurora(uint8_t r, uint8_t g, uint8_t b, int wait);

#endif