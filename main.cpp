#define _USE_MATH_DEFINES  // 必須放在最前面
#include <cmath>
#include <math.h>
#include "ws2812_effects.h"
#include <stdlib.h>
#include <unistd.h>
#include <algorithm> // 必須包含這個標頭檔來使用 std::max
#include <iostream>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>

// --- 常數定義 ---
 
#define SPI_SPEED_HZ     32000000  // 32MHz
#define BITS_PER_LED     24
#define BYTES_PER_SPI_BIT 5        // 32MHz 下，1個燈帶位元由 5 Bytes 組成
 
//#define SPI_BUF_SIZE     (LED_NUM * BITS_PER_LED * BYTES_PER_SPI_BIT)

// --- 全域變數 ---
int LED_NUM=10;
int spi_fd = -1;
 // 初始化 SPI 裝置
// --- 關鍵修正：改用動態容器 ---
std::vector<uint8_t> spi_buffer;
uint8_t global_brightness = 50; // 0-255
 
/**
 * ============================================================================
 * RPi5 SPI 轉 ARGB 燈條接線指南 (適用於 32MHz 高速驅動)
 * ============================================================================
 * * [重要提示]
 * 1. 電壓轉換: RPi5 GPIO 輸出為 3.3V，WS2812B 建議輸入為 5V。
 * 若驅動 120 顆 LED 且訊號不穩，建議在 Pin 19 與 DI 之間加入高速電位轉換晶片 (如 74AHCT125)。
 * * 2. 阻抗匹配: 32MHz 是高頻訊號，建議在 Pin 19 串聯一個 33 ~ 100 歐姆的電阻，
 * 以減少訊號反射 (Signal Reflection)，這能解決尾端 LED 閃爍的問題。
 * * 3. 獨立供電: 120 顆 LED 全亮時電流可達 7A，絕對不可由 RPi5 的 5V 引腳供電。
 * 必須使用外部 5V 電源，並將電源 GND 與 RPi5 Pin 20 (GND) 連接。
 * * [接線圖示]
 * RPi5 Pin 19 (MOSI) ----> [33R 電阻] ----> LED Strip DI (Data Input)
 * RPi5 Pin 20 (GND)  --------------------> LED Strip GND
 * 外部電源 5V         --------------------> LED Strip 5V / VCC
 * 外部電源 GND        --------------------> LED Strip GND
 * ============================================================================
 */




// --- 基礎驅動函數 ---



void RPi_SPI_Init() {
    spi_fd = open("/dev/spidev0.0", O_RDWR);
    if (spi_fd < 0) {
        perror("無法開啟 SPI 裝置");
        exit(1);
    }

    uint8_t mode = SPI_MODE_0;
    uint8_t bits = 8;
    uint32_t speed = SPI_SPEED_HZ;
    ioctl(spi_fd, SPI_IOC_WR_MODE, &mode);
    ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);

    // --- 在此根據目前的 LED_NUM 分配空間 ---
    size_t total_size = LED_NUM * BITS_PER_LED * BYTES_PER_SPI_BIT;
    spi_buffer.assign(total_size, 0); 

    std::cout << "SPI 初始化成功，LED 數量: " << LED_NUM << std::endl;
}

// 將單顆 LED 的 RGB 轉換為 32MHz 的 SPI 訊號
void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= LED_NUM) return;

    r = (r * global_brightness) >> 8;
    g = (g * global_brightness) >> 8;
    b = (b * global_brightness) >> 8;

    uint32_t color = (g << 16) | (r << 8) | b; 

    for (int i = 23; i >= 0; i--) {
        int pos = (index * 24 + (23 - i)) * BYTES_PER_SPI_BIT;
        if (color & (1 << i)) {
            // 邏輯 '1': 維持原樣 (約 680ns)
            spi_buffer[pos]   = 0xFF;
            spi_buffer[pos+1] = 0xFF;
            spi_buffer[pos+2] = 0xF0; // 稍微調整結尾
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        } else {
            // 邏輯 '0': 大幅縮短高電平時間 (目標約 300ns - 350ns)
            // 原本是 0xFF, 0xE0 (13 bits)，現在改為 0xFE, 0x00 (7-8 bits)
            spi_buffer[pos]   = 0xFE; 
            spi_buffer[pos+1] = 0x00;
            spi_buffer[pos+2] = 0x00;
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        }
    }
}
void set_led_colorxxx(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= LED_NUM) return;

    // 應用亮度縮放 (同 TI 上的亮度控制邏輯)
    r = (r * global_brightness) >> 8;
    g = (g * global_brightness) >> 8;
    b = (b * global_brightness) >> 8;

    uint32_t color = (g << 16) | (r << 8) | b; // WS2812B 常用 GRB 順序

    for (int i = 23; i >= 0; i--) {
        int pos = (index * 24 + (23 - i)) * BYTES_PER_SPI_BIT;
        if (color & (1 << i)) {
            // 邏輯 '1': 高電平時間約 700ns -> 32MHz 下約 22 bits
            spi_buffer[pos]   = 0xFF;
            spi_buffer[pos+1] = 0xFF;
            spi_buffer[pos+2] = 0xFC; 
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        } else {
            // 邏輯 '0': 高電平時間約 350ns -> 32MHz 下約 11 bits
            spi_buffer[pos]   = 0xFF;
            spi_buffer[pos+1] = 0xE0;
            spi_buffer[pos+2] = 0x00;
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        }
    }
}

// 模擬 TI 的 Show 函數，將 Buffer 送出
void ws2812_show() {
    // 使用 .data() 取得 vector 內部陣列的指標
    // 使用 .size() 取得動態計算出的總位元組長度
    if (write(spi_fd, spi_buffer.data(), spi_buffer.size()) < 0) {
        perror("SPI 傳輸失敗");
    }
    usleep(300); // WS2812B 需要至少 300us 的低電平作為 Reset 訊號
}
//----------------------EFFECTIONS SATRT HERE ------------------------//


// --- 燈效函數宣告 (具體實作請參考你之前的 empty.c) ---
// 這裡僅示意，編譯時需確保這些函數已定義
extern void eff_1_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait);
extern void eff_3_rainbow_cycle(int wait);
extern void eff_13_cyberpulse(uint8_t r, uint8_t g, uint8_t b, int wait);

// --- 主程式 ---
int main(int argc, char *argv[]) {
    // 1. 初始化 SPI
    RPi_SPI_Init();
    std::cout << "ARGB 10 燈示範程式啟動..." << std::endl;

    try {
        while (true) {
            // --- 特效 1: 經典呼吸燈 (Breathe) ---
            std::cout << "執行：深藍色呼吸..." << std::endl;
            for(int i = 0; i < 3; i++) { // 跑 3 次呼吸循環
                eff_breathe_sync(0, 100, 255, 10); 
            }

            // --- 特效 2: 幻彩推移 (Color Flow) ---
            std::cout << "執行：幻彩流動..." << std::endl;
            for(int i = 0; i < 500; i++) { 
                eff_color_flow(20); 
            }

            // --- 特效 3: 柔和彗星 (Soft Comet) ---
            std::cout << "執行：紅色彗星..." << std::endl;
            for(int i = 0; i < 100; i++) {
                eff_soft_comet(255, 0, 0, 60); 
            }

            // --- [補上] 特效 4: 隨機閃爍 (Starlight / Twinkle) ---
            std::cout << "執行：星空閃爍..." << std::endl;
            for(int i = 0; i < 200; i++) {
                // 每次隨機點亮 2 顆燈，顏色設為暖白色
                eff_twinkle(255, 200, 150, 2, 100); 
            }

            // --- [補上] 特效 5: 雙色交叉漸變 (Dual Color Crossfade) ---
            std::cout << "執行：賽博龐克漸變 (青色 <-> 桃紅)..." << std::endl;
            for(int i = 0; i < 5; i++) {
                // 從青色 (0, 255, 255) 漸變到 桃紅 (255, 0, 150)
                eff_dual_fade(0, 255, 255, 255, 0, 150, 20);
                // 再變回來
                eff_dual_fade(255, 0, 150, 0, 255, 255, 20);
            }
            
			// --- 補上特效 4: 火焰模擬 ---
    std::cout << "執行：火焰模擬..." << std::endl;
    for(int i = 0; i < 500; i++) {
        eff_4_fire(55, 120, 15); 
    }

    // --- 補上特效 6: 霓虹追逐 ---
    std::cout << "執行：黃色霓虹追逐..." << std::endl;
    for(int i = 0; i < 30; i++) {
        eff_neon_chase(255, 255, 0, 100); 
    }

    // --- 補上特效 7: 炫彩彈跳 ---
    std::cout << "執行：炫彩彈跳..." << std::endl;
    for(int i = 0; i < 200; i++) {
        eff_color_bounce(50); 
    }

    // --- 補上特效 8: 極光呼吸 ---
    std::cout << "執行：青綠色極光..." << std::endl;
    for(int i = 0; i < 200; i++) {
        eff_aurora(0, 255, 150, 40); 
    }
			
			
			
            std::cout << "--- 循環結束，重新開始 ---" << std::endl;
        }
    } catch (...) {
        if (spi_fd >= 0) close(spi_fd);
    }

    return 0;
}
int mainXX(int argc, char *argv[]) {
    // 1. 初始化
    RPi_SPI_Init();

    std::cout << "開始執行 ARGB 燈效展示 (LED 數量: " << LED_NUM << ")" << std::endl;

    try {
        // 2. 主循環 (仿照 TI MSPM0G 的 while(1))
        while (true) {
            // 範例：執行特效 13
            std::cout << "執行特效 13: Cyberpulse" << std::endl;
            for(int i = 0; i < 3; i++) {
                eff_13_cyberpulse(255, 0, 0, 5); 
            }

            // 範例：執行特效 1
            std::cout << "執行特效 1: Color Wipe" << std::endl;
            eff_1_color_wipe(0, 255, 127, 20);

            // 範例：執行特效 3
            std::cout << "執行特效 3: Rainbow Cycle" << std::endl;
            eff_3_rainbow_cycle(10);
            
            // 可以繼續加入你其他的 eff_X 函數...
			// 執行 10 次呼吸
    for(int i=0; i<10; i++) eff_breathe_sync(0, 150, 255, 30); // 淺藍色呼吸
    
    // 執行一段時間的彗星
    for(int i=0; i<100; i++) eff_soft_comet(255, 20, 0, 80); // 紅色彗星
			
			
        }
    } catch (...) {
        // 異常處理
        close(spi_fd);
    }

    close(spi_fd);
    return 0;
}