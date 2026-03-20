#include "ws2812_effects.h"
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <vector>
#include <algorithm>

/**
 * 延時函式 (毫秒)
 */
void delay_ms(uint32_t ms) {
    usleep(ms * 1000); 
}

/**
 * 輔助函數：產生彩虹色 (Wheel)
 * 輸入 0-255，輸出 GRB 組合的 uint32_t
 */
uint32_t Wheel(uint8_t WheelPos) {
    WheelPos = 255 - WheelPos;
    if(WheelPos < 85) {
        return ((((uint32_t)(255 - WheelPos * 3)) << 16) | (WheelPos * 3));
    }
    if(WheelPos < 170) {
        WheelPos -= 85;
        return (((uint32_t)(WheelPos * 3) << 8) | (255 - WheelPos * 3));
    }
    WheelPos -= 170;
    return (((uint32_t)(WheelPos * 3) << 16) | ((uint32_t)(255 - WheelPos * 3) << 8));
}

// --- 特效 1: Color Wipe (擦拭) ---
void eff_1_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait) {
    for(int i = 0; i < LED_NUM; i++) {
        set_led_color(i, r, g, b);
        ws2812_show();
        delay_ms(wait);
    }
}

// --- 特效 3: Rainbow Cycle (虹彩循環) ---
void eff_3_rainbow_cycle(int wait) {
    uint16_t i, j;
    for(j = 0; j < 256 * 5; j++) { 
        for(i = 0; i < LED_NUM; i++) {
            uint32_t color = Wheel(((i * 256 / LED_NUM) + j) & 255);
            set_led_color(i, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
        }
        ws2812_show();
        delay_ms(wait);
    }
}

// --- 特效 4: Fire (火焰模擬) ---
// 修正：將原本編譯失敗的 static uint8_t heat[LED_NUM] 改為 std::vector
void eff_4_fire(int Cooling, int Sparking, int SpeedDelay) {
    static std::vector<uint8_t> heat;
    
    // 如果 LED_NUM 改變，自動重新調整 heat 緩衝區大小
    if (heat.size() != (size_t)LED_NUM) {
        heat.assign(LED_NUM, 0); 
    }

    // 1. 降溫
    for(int i = 0; i < LED_NUM; i++) {
        int cooldown = rand() % (((Cooling * 10) / LED_NUM) + 2);
        if(cooldown > heat[i]) heat[i] = 0;
        else heat[i] = heat[i] - cooldown;
    }

    // 2. 向上飄散
    for(int k = LED_NUM - 1; k >= 2; k--) {
        heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
    }

    // 3. 隨機產生火星
    if(rand() % 255 < Sparking) {
        int y = rand() % (LED_NUM > 7 ? 7 : LED_NUM);
        heat[y] = heat[y] + rand() % (160) + 95;
    }

    // 4. 轉換熱度為顏色
    for(int j = 0; j < LED_NUM; j++) {
        uint8_t t192 = (uint8_t)(((float)heat[j]/255.0)*191);
        uint8_t heat_ramp = (t192 & 0x3F) << 2; 

        if(t192 > 0x80) { // 最熱：白色/黃色
            set_led_color(j, 255, 255, heat_ramp);
        } else if(t192 > 0x40) { // 中熱：橙色
            set_led_color(j, 255, heat_ramp, 0);
        } else { // 基礎：紅色
            set_led_color(j, heat_ramp, 0, 0);
        }
    }
    ws2812_show();
    delay_ms(SpeedDelay);
}

// --- 特效 13: Cyberpulse (賽博脈衝) ---
void eff_13_cyberpulse(uint8_t r, uint8_t g, uint8_t b, int wait) {
    for(int brightness = 0; brightness < 255; brightness += 5) {
        for(int i = 0; i < LED_NUM; i++) {
            set_led_color(i, (r * brightness) >> 8, (g * brightness) >> 8, (b * brightness) >> 8);
        }
        ws2812_show();
        delay_ms(wait);
    }
}

// --- 1. 同步呼吸燈 ---
void eff_breathe_sync(uint8_t r, uint8_t g, uint8_t b, int wait) {
    for (int i = 0; i < 256; i++) {
        float intensity = (exp(sin(i/256.0 * M_PI * 2)) - 0.36787944) * 0.42545906;
        for (int j = 0; j < LED_NUM; j++) {
            set_led_color(j, r * intensity, g * intensity, b * intensity);
        }
        ws2812_show();
        delay_ms(wait);
    }
}

// --- 2. 幻彩推移流動 ---
void eff_color_flow(int wait) {
    static uint8_t j = 0;
    for (int i = 0; i < LED_NUM; i++) {
        uint32_t color = Wheel(((i * 256 / LED_NUM) + j) & 255);
        set_led_color(i, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
    }
    ws2812_show();
    j++; 
    delay_ms(wait);
}

// --- 3. 柔和彗星 (帶餘暉) ---
void eff_soft_comet(uint8_t r, uint8_t g, uint8_t b, int wait) {
    static int pos = 0;
    static int dir = 1;
    
    for (int i = 0; i < LED_NUM; i++) {
        float dist = abs(i - pos);
        float factor = std::max(0.0f, 1.0f - (dist / 5.0f)); // 尾巴長度約 5 顆
        set_led_color(i, r * factor, g * factor, b * factor);
    }
    ws2812_show();
    pos += dir;
    if (pos >= LED_NUM - 1 || pos <= 0) dir *= -1;
    delay_ms(wait);
}

// --- 4. 隨機閃爍 (星空) ---
void eff_twinkle(uint8_t r, uint8_t g, uint8_t b, int count, int wait) {
    for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);
    
    for(int i=0; i<count; i++) {
        int idx = rand() % LED_NUM;
        int bright = rand() % 150 + 100;
        set_led_color(idx, (r * bright) >> 8, (g * bright) >> 8, (b * bright) >> 8);
    }
    ws2812_show();
    delay_ms(wait);
}

// --- 5. 雙色交叉漸變 ---
void eff_dual_fade(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, int wait) {
    for (int step = 0; step < 100; step++) {
        float f = step / 100.0f;
        uint8_t r = r1 + (r2 - r1) * f;
        uint8_t g = g1 + (g2 - g1) * f;
        uint8_t b = b1 + (b2 - b1) * f;
        for (int i = 0; i < LED_NUM; i++) set_led_color(i, r, g, b);
        ws2812_show();
        delay_ms(wait);
    }
}

// --- 6. 霓虹追逐 ---
void eff_neon_chase(uint8_t r, uint8_t g, uint8_t b, int wait) {
    for (int a = 0; a < 3; a++) {
        for (int b_idx = 0; b_idx < LED_NUM; b_idx += 3) {
            if (a + b_idx < LED_NUM) set_led_color(a + b_idx, r, g, b);
        }
        ws2812_show();
        delay_ms(wait);
        for (int b_idx = 0; b_idx < LED_NUM; b_idx += 3) {
            if (a + b_idx < LED_NUM) set_led_color(a + b_idx, 0, 0, 0);
        }
    }
}

// --- 7. 炫彩彈跳 ---
void eff_color_bounce(int wait) {
    static int pos = 0;
    static int dir = 1;
    static uint8_t color_idx = 0;
    
    for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);
    
    uint32_t color = Wheel(color_idx++);
    set_led_color(pos, (color>>16)&0xFF, (color>>8)&0xFF, color&0xFF);
    
    ws2812_show();
    pos += dir;
    if(pos >= LED_NUM-1 || pos <= 0) dir *= -1;
    delay_ms(wait);
}

// --- 8. 極光呼吸 (Aurora) ---
void eff_aurora(uint8_t r, uint8_t g, uint8_t b, int wait) {
    static float offset = 0;
    for (int i = 0; i < LED_NUM; i++) {
        // 使用 Sin 波產生流動感
        float factor = (sin(i * 0.3f + offset) + 1.0f) / 2.0f;
        set_led_color(i, r * factor, g * factor, b * factor);
    }
    ws2812_show();
    offset += 0.15f;
    delay_ms(wait);
}