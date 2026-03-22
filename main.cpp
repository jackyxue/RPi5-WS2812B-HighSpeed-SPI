#define _USE_MATH_DEFINES  // 必須放在最前面
#include <cmath>
#include <math.h>
#include "ws2812_effects.h"
#include <stdlib.h>
#include <unistd.h>
#include <algorithm> // 必須包含這個標頭檔來使用 std::max
#include <chrono> // 用於精確時間計算
#include <iostream>
#include <vector>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <unistd.h>
#include <csignal>  // 必須包含此標頭檔

// --- 常數定義 ---

#define DEFAULT_DEMO_LED_NUMBER 30 //48
#define DEFAULT_DEMO_BRIGHTNESS 30
 
#define SPI_SPEED_HZ     32000000  // 32MHz
#define BITS_PER_LED     24
#define BYTES_PER_SPI_BIT 5        // 32MHz 下，1個燈帶位元由 5 Bytes 組成
 
 

// 建立影子緩衝區 (Shadow Buffer)
// 我們不給它固定大小，等初始化時再分配
std::vector<PrideRGB> led_shadow_buffer;
 
 
 
//#define SPI_BUF_SIZE     (LED_NUM * BITS_PER_LED * BYTES_PER_SPI_BIT)

// --- 全域變數 ---
int LED_NUM=DEFAULT_DEMO_LED_NUMBER;
int spi_fd = -1;
 // 初始化 SPI 裝置
// --- 關鍵修正：改用動態容器 ---
std::vector<uint8_t> spi_buffer;
uint8_t global_brightness = DEFAULT_DEMO_BRIGHTNESS; // 0-255
 
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
	
	
	// --- 重要修改：根據變數 LED_NUM 分配影子緩衝區空間 ---
    // assign 會將 vector 大小調整為 LED_NUM，並預設填入全黑 {0,0,0}
    led_shadow_buffer.assign(LED_NUM, {0, 0, 0});
	
	size_t required_size = LED_NUM * BITS_PER_LED * BYTES_PER_SPI_BIT + 100; // 100 bytes padding
    if (spi_buffer.size() < required_size) {
        spi_buffer.assign(required_size, 0);
    }

    // --- 在此根據目前的 LED_NUM 分配空間 ---
	
	
 
   size_t data_size = LED_NUM * BITS_PER_LED * BYTES_PER_SPI_BIT;
    
    // 關鍵修正：額外增加 200 Bytes 的 0x00 作為「物理重置信號」
    // 這確保最後一顆 LED 的數據能被完整推入移位暫存器
    size_t padding = 200; 
    data_size = 16 + data_size + padding; 
    spi_buffer.assign(data_size , 0); 
    
    std::cout << "SPI Buffer Size (含 Padding): " << spi_buffer.size() << std::endl;

 
    std::cout << "SPI 初始化成功，LED 數量: " << LED_NUM << std::endl;
	
	std::cout << "SPI 初始化成功，影子緩衝區已同步設定為 " << LED_NUM << " 顆燈。" << std::endl;
}


PrideRGB get_led_color(int index) {
    // 檢查邊界
    if (index < 0 || index >= LED_NUM) {
        return {0, 0, 0}; // 回傳全黑結構
    }
    
    // 關鍵：直接回傳 vector 裡的 PrideRGB 元素
    return led_shadow_buffer[index]; 
}

 
 
void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < 0 || index >= LED_NUM) return;

    // --- A. 同步到影子緩衝區 (儲存原始值) ---
    // 這裡保存 0-255 完整數值，這就是 Sinelon 尾巴絲滑的關鍵！
    led_shadow_buffer[index].r = r;
    led_shadow_buffer[index].g = g;
    led_shadow_buffer[index].b = b;

    // --- B. 亮度計算 (優化解析度) ---
    // 轉為 uint32_t 計算以防止相乘時溢位。
    // 如果你擔心燈條供電，可以限制 global_brightness 的最大值 (例如 128)
    uint8_t br = (uint8_t)((uint32_t)r * global_brightness / 255);
    uint8_t bg = (uint8_t)((uint32_t)g * global_brightness / 255);
    uint8_t bb = (uint8_t)((uint32_t)b * global_brightness / 255);

    // --- C. 轉換為 WS2812 格式 (注意：WS2812 順序通常是 GRB) ---
    // 這裡我們把計算後的 br, bg, bb 組合起來
    uint32_t color = (static_cast<uint32_t>(bg) << 16) | 
                     (static_cast<uint32_t>(br) << 8)  | 
                     static_cast<uint32_t>(bb); 

    // --- D. 寫入 SPI Buffer (5-byte bit-banging) ---
    // 保持你原有的 16 bytes offset (RESET 訊號空間)
    for (int i = 23; i >= 0; i--) {
        int pos = 16 + (index * 24 + (23 - i)) * 5; // BYTES_PER_SPI_BIT = 5
        
        if (color & (1 << i)) {
            // WS2812 邏輯 '1': 高電位時間較長
            spi_buffer[pos]   = 0xFF;
            spi_buffer[pos+1] = 0xFF;
            spi_buffer[pos+2] = 0xF0; 
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        } else {
            // WS2812 邏輯 '0': 高電位時間較短
            spi_buffer[pos]   = 0xFE; 
            spi_buffer[pos+1] = 0x00;
            spi_buffer[pos+2] = 0x00;
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        }
    }
} 
 

void set_led_color_v1p1v2(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index < 0 || index >= LED_NUM) return;


    // --- A. 同步到影子緩衝區 (儲存原始值) ---
    led_shadow_buffer[index].r = r;
    led_shadow_buffer[index].g = g;
    led_shadow_buffer[index].b = b;

    // --- B. 亮度計算 ---
    // 使用你原有的 global_brightness 邏輯
    uint8_t br = (r * global_brightness) >> 8;
    uint8_t bg = (g * global_brightness) >> 8;
    uint8_t bb = (b * global_brightness) >> 8;

    // --- C. 轉換為 WS2812 格式 (GRB 順序) ---
    uint32_t color = (bg << 16) | (br << 8) | bb; 

    // --- D. 寫入 SPI Buffer (你原有的 5-byte bit-banging 邏輯) ---
    for (int i = 23; i >= 0; i--) {
        int pos = 16 + (index * 24 + (23 - i)) * BYTES_PER_SPI_BIT;
        if (color & (1 << i)) {
            // 邏輯 '1'
            spi_buffer[pos]   = 0xFF;
            spi_buffer[pos+1] = 0xFF;
            spi_buffer[pos+2] = 0xF0; 
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        } else {
            // 邏輯 '0'
            spi_buffer[pos]   = 0xFE; 
            spi_buffer[pos+1] = 0x00;
            spi_buffer[pos+2] = 0x00;
            spi_buffer[pos+3] = 0x00;
            spi_buffer[pos+4] = 0x00;
        }
    }
}
 

// 模擬 TI 的 Show 函數，將 Buffer 送出
void ws2812_show() {
    // 檢查 spi_fd 是否有效
    if (spi_fd < 0) return;

    // 直接傳輸 spi_buffer。因為我們在初始化時已經預留了末尾的 0x00，
    // 所以這裡不需要再用 push_back，直接 write 即可。
    ssize_t n = write(spi_fd, spi_buffer.data(), spi_buffer.size());
    
    if (n < 0) {
        perror("SPI 傳輸失敗");
        if (errno == EMSGSIZE) {
            std::cerr << "錯誤：資料量太大，請檢查 /boot/firmware/cmdline.txt 是否加入 spidev.bufsiz=65536" << std::endl;
        }
    }
    
    // WS2812B 需要 Reset 訊號。
    // 在 32MHz SPI 下，數據傳完後維持低電平 300us 是必要的。
    usleep(300); 
}
 
//----------------------EFFECTIONS SATRT HERE ------------------------//

// --- 信號處理函式 ---
 

void LED_System_Cleanup(int signum) {
    if (signum == SIGINT) {
        std::cout << "\n[!] 偵測到 Ctrl+C，正在關閉燈條..." << std::endl;
    }
    
    // 1. 強制所有燈珠熄滅
    for (int i = 0; i < LED_NUM; i++) {
        set_led_color(i, 0, 0, 0);
    }
    ws2812_show();
    
    // 2. 關閉 SPI 資源
    if (spi_fd >= 0) {
        close(spi_fd);
        spi_fd = -1;
    }
    
    if (signum != 0) exit(signum);
}



// --- 燈效函數宣告 (具體實作請參考你之前的 empty.c) ---
// 這裡僅示意，編譯時需確保這些函數已定義
extern void eff_1_color_wipe(uint8_t r, uint8_t g, uint8_t b, int wait);
extern void eff_3_rainbow_cycle(int wait);
extern void eff_13_cyberpulse(uint8_t r, uint8_t g, uint8_t b, int wait);

#include <ctime>    // 必須包含，為了 time(NULL)
#include <csignal>  // 為了 signal() 和 SIGINT

int main(int argc, char *argv[]) {
    // 註冊 Ctrl+C 信號處理
    signal(SIGINT, LED_System_Cleanup);
    
    // 初始化 SPI
    RPi_SPI_Init();
    
    std::cout << "ARGB NUMBER " << LED_NUM << " 燈示範程式啟動..." << std::endl; 
    srand(static_cast<unsigned int>(time(NULL)));
	base_time = std::chrono::high_resolution_clock::now();

    if (argc > 1) {
        std::string arg = argv[1];

        if (arg == "off") {
            LED_System_Cleanup(0);
            return 0;
        }

        int id = -1; // 將 id 定義在 try 之外，確保 catch 也能存取
        try {
            id = std::stoi(arg);
            if (effect_help_map.count(id)) {
                // 印出帶有 ID 的啟動訊息
                printf("EFFECT ID : [%02d] 正在循環執行特效: %s\n", id, effect_help_map[id].name.c_str());
                while (true) { 
                    // 傳入當前時間 t 給特效迴圈
                    run_effect_loop(id, get_elapsed_t());
                }
            } else {
                std::cout << "錯誤：無效的 ID " << id << std::endl;
                eff_999_system_error_alert(); // 先閃爍警告
                print_detailed_help(argv[0]);
                return 1;
            }
        } catch (...) {
            // 如果 stoi 轉換失敗（例如輸入 abc），這裡處理錯誤
            std::cout << "錯誤：輸入內容 [" << arg << "] 不是有效的數字。" << std::endl;
            eff_999_system_error_alert(); 
            print_detailed_help(argv[0]);
            return 1;
        }
    } else {
        // 全自動展示模式
        std::cout << ">>> 啟動全自動展示模式 (每 10 秒切換)..." << std::endl;
        while (true) {
            for (auto const& [id, info] : effect_help_map) {
                // 這裡我們直接傳入 ID 給 loop，它內部會處理 [ID] 前綴列印
                auto start = std::chrono::high_resolution_clock::now();
                while (std::chrono::duration<float>(std::chrono::high_resolution_clock::now() - start).count() < 10.0f) {
                    // 傳入當前時間 t 給特效迴圈
                    run_effect_loop(id, get_elapsed_t());
                }
            }
        }
    }

    LED_System_Cleanup(0);
    return 0;
}
 