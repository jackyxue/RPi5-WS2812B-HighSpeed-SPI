#ifndef WS2812_EFFECTS_H
#define WS2812_EFFECTS_H

#include <stdint.h>
#include <vector>
#include <algorithm>
#include <map>
#include <string>    // <--- 必須加上這一行
#include <functional> // 必須包含，用於 std::function


// 定義特效完整資訊的結構
struct EffectInfo {
    std::string name;        // 特效名稱
    std::string description; // 詳細幫助描述
    int category;            // 分類 (1:基礎, 2:生物, 3:物理, 4:隨機)
};

// 關鍵：讓所有包含此檔的 .cpp 都能存取同一個 LED_NUM
extern int LED_NUM; 



// 告知編譯器：這個 map 的實體在別的 .cpp 檔案裡
extern std::map<int, EffectInfo> effect_help_map;

// 宣告在 main.cpp 實作的底層函數
extern void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b);
extern void ws2812_show();
extern void delay_ms(uint32_t ms);
extern uint32_t Wheel(uint8_t WheelPos);

extern void render_fixed_hue_frame(float hue, float intensity);

// --- 燈效函數原型 ---
void eff_1_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_3_rainbow_cycle(int wait);
void eff_4_fire(int Cooling, int Sparking, int SpeedDelay);
void eff_13_cyberpulse(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_breathe_sync(uint8_t r, uint8_t g, uint8_t b, int wait);

void eff_heartbeat_sync(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_breathe_pro_tick(uint8_t r, uint8_t g, uint8_t b, float t) ;
void eff_heartbeat_biological(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_heartbeat_biological_slow(uint8_t r, uint8_t g, uint8_t b);
void eff_heartbeat_extreme(uint8_t r, uint8_t g, uint8_t b);
void eff_hyper_flash_rgb_fade();
void eff_hyper_flash_rgb_fade_fixed_color(float current_hue);

void eff_color_flow(int wait);
void eff_soft_comet(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_twinkle(uint8_t r, uint8_t g, uint8_t b, int count, int wait);
void eff_dual_fade(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, int wait);
 void eff_bio_pulse_suite(int duration_sec) ;

// 補齊 main.cpp 呼叫但缺少的原型
void eff_neon_chase(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_color_bounce(int wait);
void eff_aurora(uint8_t r, uint8_t g, uint8_t b, int wait);


void eff_meteor(uint8_t r, uint8_t g, uint8_t b, uint8_t decay, bool random_decay, int wait);
void eff_marbles(int wait);

void eff_lightning(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_theater_chase(uint8_t r, uint8_t g, uint8_t b, int wait);
void eff_multi_bounce(int wait);
void eff_gradient_scanner(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, int wait) ;


void eff_meteor(uint8_t r, uint8_t g, uint8_t b, uint8_t decay, bool random_decay, int wait) ;
void eff_marbles(int wait);


void eff_barber_pole(uint8_t r, uint8_t g, uint8_t b, int width, int wait);
void eff_barber_pole_multi(int width, int wait);



void eff_33_stack_tetris(uint8_t r, uint8_t g, uint8_t b, int wait);
// [ID 33] 俄羅斯方塊堆疊 (隨機長度 1-3)
void eff_33_stack_tetris_random(uint8_t r, uint8_t g, uint8_t b, int wait);
bool eff_33_stack_tetris_enhanced(uint8_t r, uint8_t g, uint8_t b, int wait);


void eff_30_pulse_collision_tick(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float speed, float t);
void eff_31_plasma_flow_tick(float t);

// 在原型宣告區加入
void eff_999_system_error_alert();




void print_detailed_help(char* prog_name);
void run_effect_by_id(int id) ;
void demo_all_effections(void);






#endif