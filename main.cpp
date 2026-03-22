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

#define DEFAULT_DEMO_LED_NUMBER 75
 
#define SPI_SPEED_HZ     32000000  // 32MHz
#define BITS_PER_LED     24
#define BYTES_PER_SPI_BIT 5        // 32MHz 下，1個燈帶位元由 5 Bytes 組成
 
//#define SPI_BUF_SIZE     (LED_NUM * BITS_PER_LED * BYTES_PER_SPI_BIT)

// --- 全域變數 ---
int LED_NUM=DEFAULT_DEMO_LED_NUMBER;
int spi_fd = -1;
 // 初始化 SPI 裝置
// --- 關鍵修正：改用動態容器 ---
std::vector<uint8_t> spi_buffer;
uint8_t global_brightness = 255; // 0-255
 
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
}

// 將單顆 LED 的 RGB 轉換為 32MHz 的 SPI 訊號
void set_led_color(int index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= LED_NUM) return;

    r = (r * global_brightness) >> 8;
    g = (g * global_brightness) >> 8;
    b = (b * global_brightness) >> 8;

    uint32_t color = (g << 16) | (r << 8) | b; 

    for (int i = 23; i >= 0; i--) {
        int pos = 16 + (index * 24 + (23 - i)) * BYTES_PER_SPI_BIT;
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
void ws2812_show_v0() {
	// 使用 .data() 取得 vector 內部陣列的指標
    // 使用 .size() 取得動態計算出的總位元組長度
    ssize_t n = write(spi_fd, spi_buffer.data(), spi_buffer.size());
    if (n < 0) {
        perror("SPI 傳輸失敗");
        if (errno == EMSGSIZE) {
            std::cerr << "錯誤：資料量太大，請檢查 /boot/firmware/cmdline.txt 是否加入 spidev.bufsiz=65536" << std::endl;
        }
    }
	
	for(int k = 0; k < 100; k++) {
    spi_buffer.push_back(0x00);
    }
	
    usleep(300);  // WS2812B 需要至少 300us 的低電平作為 Reset 訊號
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


int main(int argc, char *argv[]) {
	
	// 這行告訴系統：當收到 SIGINT (Ctrl+C) 時，請執行 signalHandler 函式
   signal(SIGINT, LED_System_Cleanup);
	
    // 呼叫你的 SPI 初始化 (請確保保留原有的 RPi_SPI_Init 實作)
    RPi_SPI_Init(); 
	
	std::cout << "ARGB NUMBER " << LED_NUM << " 燈示範程式啟動..." << std::endl; 
	srand(static_cast<unsigned int>(time(NULL)));

	

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "off") {
            for (int i = 0; i < LED_NUM; i++) set_led_color(i, 0, 0, 0);
            ws2812_show();
            return 0;
        }

        try {
            int id = std::stoi(arg);
            if (effect_help_map.count(id)) {
                std::cout << ">>> 執行特效: " << effect_help_map[id].name << std::endl;
                while (true) { run_effect_by_id(id); }
            } else {
                std::cout << "錯誤：無效的 ID " << id << std::endl;
                print_detailed_help(argv[0]);
			    eff_999_system_error_alert(); 
				return 1;
            }
        } catch (...) {
            print_detailed_help(argv[0]);
			eff_999_system_error_alert(); 
			return 1;
        }
    } else {
        // 預設展示模式
        while (true) {
            demo_all_effections();
        }
    }
    return 0;
}





// --- 主程式 ---
int main_v001(int argc, char *argv[]) {
    // 1. 初始化 SPI
	
    RPi_SPI_Init();
	
	
	
	// 2. 檢查命令行參數
    if (argc > 1) {
        std::string cmd = argv[1];
        if (cmd == "--off" || cmd == "off" || cmd == "shutdown") {
            std::cout << "接收到關閉指令：清空所有 LED 並退出。" << std::endl;
            // 將所有 LED 設為黑色 (0,0,0)
            for (int i = 0; i < LED_NUM; i++) {
                set_led_color(i, 0, 0, 0);
            }
            ws2812_show();
            
            // 關閉 SPI 並結束程式
            if (spi_fd >= 0) close(spi_fd);
            return 0;
        }
    }
	
	
	
	
	
	
	
	
	
	
	
	std::cout << "ARGB NUMBER " << LED_NUM << " 燈示範程式啟動..." << std::endl; 
	srand(static_cast<unsigned int>(time(NULL)));

    try {
        while (true) {
			
	

// --- 展示：能量脈衝碰撞 (10s) ---
        auto start = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count() < 10.0f) {
            float t = std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count();
            eff_30_pulse_collision_tick(255, 0, 0, 0, 0, 255, 0.5f, t); 
            delay_ms(10);
        }

        // --- 展示：電漿流動 (10s) ---
        start = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count() < 10.0f) {
            float t = std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count();
            eff_31_plasma_flow_tick(t);
            delay_ms(16);
        }





	
		// --- 展示 1: [ID 34] 強化 Tetris ---
        // --- 展示：Tetris 直到觸發慶典 ---
    std::cout << "Step: 開始堆疊 Tetris..." << std::endl;
    bool finished = false;
    while (!finished) {
        // 它會一顆一顆掉，直到填滿 46 顆觸發彩虹後回傳 true
        finished = eff_33_stack_tetris_enhanced(0, 255, 128, 2); 
    }
    
    // 慶典結束後，可以停一下
    std::cout << "Step: 慶典結束，休息 1 秒" << std::endl;
    delay_ms(1000);
			
			
		// --- 特效展示 1: [ID 33] Tetris Stack (隨機 1-3 顆) ---
        std::cout << "Running: [33] Tetris Stack (Random 1-3)..." << std::endl;
        // 跑 30 次掉落約可填滿大半條
        for(int i = 0; i < 30; i++) {
            eff_33_stack_tetris_random(0, 255, 128, 4); 
        }		
		
			
			
		std::cout << "RUN: [33] Stack Tetris Effect..." << std::endl;
		// 跑 LED_NUM 次代表填滿整條燈管
		for(int i = 0; i < LED_NUM + 1; i++) {
		eff_33_stack_tetris(0, 255, 255, 5); // 青色方塊，掉落速度 5ms
		} 	
			
			
			
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
			  std::cout << "執行特效 : 淺藍色呼吸" << std::endl;
    for(int i=0; i<10; i++) 
		eff_breathe_sync(0, 150, 255, 30); // 淺藍色呼吸
    
    // 執行一段時間的彗星
	  std::cout << "執行特效 : 執行一段時間的彗星" << std::endl;
    for(int i=0; i<100; i++) 
		eff_soft_comet(64, 20, 128, 80); // 紅色彗星
	
	
	
			 
// --- [目標]：30秒展示，每次心跳 1-3 次，顏色鎖定，隨機休息 1-4 秒 ---

         // 呼叫生物脈衝，展示 30 秒
		  std::cout << "執行：呼叫生物脈衝 (限時展示 30 秒，含隨機停頓)..." << std::endl;
        eff_bio_pulse_suite(60);
			
			
			
			
			
			// --- 特效 3: 極致閃頻 RGB 融合 (限時展示 30 秒) ---
            std::cout << "執行：極致閃頻 RGB 融合 (限時展示 30 秒，含隨機停頓)..." << std::endl;

			auto demo_start_timer = std::chrono::high_resolution_clock::now();
			while (true) {
			// 1. 隨機決定這次要「亮多久」 (例如 0.5 秒到 2 秒)
			float active_duration = ((rand() % 15) + 5) / 10.0f; 
			auto active_start = std::chrono::high_resolution_clock::now();
    
			// --- 進入「亮燈」階段 ---
			while (true) {
				
				eff_hyper_flash_rgb_fade(); // 執行特效

				auto now = std::chrono::high_resolution_clock::now();
				if (std::chrono::duration<float>(now - active_start).count() >= active_duration) break;
				if (std::chrono::duration<float>(now - demo_start_timer).count() >= 30.0f) goto end_demo2;
			}

			// --- 進入「全黑」階段 (核心需求：隨機 1 到 4 秒) ---
			// 讓所有燈熄滅
			for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);
			ws2812_show();

			float dark_duration = (rand() % 31 + 10) / 10.0f; // 產生 1.0 ~ 4.0 秒的隨機浮點數
			std::cout << "  [停頓] 全黑休息 " << dark_duration << " 秒..." << std::endl;
    
			auto dark_start = std::chrono::high_resolution_clock::now();
			while (true) {
				auto now = std::chrono::high_resolution_clock::now();
				// 檢查熄滅時間是否到達
				if (std::chrono::duration<float>(now - dark_start).count() >= dark_duration) break;
				// 同時也要檢查 30 秒總時間是否到達，避免在全黑時卡住
				if (std::chrono::duration<float>(now - demo_start_timer).count() >= 30.0f) goto end_demo2;
        
				delay_ms(10); // 休息時不需要全速運算，節省一點 CPU
			}
		}

end_demo2:
	std::cout << "30 秒展示結束，切換至下一個特效。" << std::endl;
			
			// 在 main.cpp 的 while(true) 中
			std::cout << "執行：極限閃頻生物心跳 (約 30 秒)..." << std::endl;
			  start = std::chrono::high_resolution_clock::now();
			while (true) {
			eff_heartbeat_extreme(0, 255, 128); // 呼叫一次約 3-4 秒
    
				auto now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> elapsed = now - start;
			if (elapsed.count() >= 30.0f) break; // 滿 30 秒就跳出
			}
			
			
			std::cout << "執行：慢速生物心跳 (預計 30 秒)..." << std::endl;
			auto start_time = std::chrono::high_resolution_clock::now();

			while (true) {
				eff_heartbeat_biological_slow(0, 255, 128); // 跑一次約 3.3 秒
    
				auto current_time = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> elapsed = current_time - start_time;
    
				if (elapsed.count() >= 30.0f) { // 滿 30 秒就換下一個特效
				break;
				}
			}
			
			std::cout << "執行：不規則生物心跳特效  (計時 15 秒) (Biological Heartbeat)..." << std::endl; 
			auto startTime = std::chrono::high_resolution_clock::now();
			while (true) {
				// 執行一次特效
			eff_heartbeat_biological(0, 255, 128, 4); 

			// 檢查是否超過 15 秒
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed = now - startTime;
    
			if (elapsed.count() >= 15.0f) {
				std::cout << "時間到，切換下一個特效！" << std::endl;
				break; 
			}
			}
			
			std::cout << "執行：生理心跳特效 (Heartbeat)..." << std::endl;
			for(int i = 0; i < 10; i++) { // 執行 10 次心跳週期
			eff_heartbeat_sync(0, 255, 255, 5); // 稍微加快 wait，讓跳動更有力
			}
			
			std::cout << "執行：生理曲線呼吸燈 (手動迴圈版)..." << std::endl;
			 start = std::chrono::high_resolution_clock::now();
while(true) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - start;
    
    // 只負責傳入「現在是第幾秒」
    eff_breathe_pro_tick(0, 255, 128, elapsed.count());
    
    if (elapsed.count() > 10.0f) break; // 準時 10 秒結束
    delay_ms(10); // 穩定的 100 FPS
}
			
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
	
	
	// --- 1. 執行 Theater Chase (劇院追逐) ---
    // 效果：白色燈珠每三顆亮一顆，像劇院門口的跑馬燈
    std::cout << "執行：劇院追逐 (白色)..." << std::endl;
    eff_theater_chase(255, 255, 255, 100); 

    // --- 2. 執行 Gradient Scanner (漸變掃描) ---
    // 效果：在青色 (Cyan) 與 桃紅色 (Magenta) 之間平滑漸變流動
    std::cout << "執行：漸變掃描 (青色 -> 桃紅)..." << std::endl;
    for(int i = 0; i < 300; i++) { // 執行 300 幀
        eff_gradient_scanner(0, 255, 255, 255, 0, 150, 20);
    }
	
	
	std::cout << "執行：閃電風暴..." << std::endl;
for(int i=0; i<5; i++) eff_lightning(255, 255, 255, 50);

std::cout << "執行：多重反彈粒子..." << std::endl;
for(int i=0; i<300; i++) eff_multi_bounce(20);
	
	
	
	// --- 執行：隨機流星 ---
std::cout << "執行：白色流星劃過..." << std::endl;
for(int i = 0; i < LED_NUM * 2; i++) {
    eff_meteor(255, 255, 255, 64, true, 30);
}

// --- 執行：彩色彈珠 ---
std::cout << "執行：RGB 彈珠交錯..." << std::endl;
for(int i = 0; i < 500; i++) {
    eff_marbles(20);
}



std::cout << "執行：理髮店旋轉燈柱..." << std::endl;
    for(int i = 0; i < 200; i++) {
        // 紅色條紋，寬度 6，延遲 50ms
        eff_barber_pole(255, 0, 0, 6, 50); 
    }
	
    std::cout << "執行：三色理髮店燈柱 (紅、白、藍)..." << std::endl;
    // 執行 300 幀 (Frame)，每幀寬度 4 顆燈，延遲 40 毫秒
    for(int i = 0; i < 300; i++) {
        eff_barber_pole_multi(4, 40); 
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
    for(int i=0; i<100; i++) eff_soft_comet(64, 20, 128, 80); // 紅色彗星
			
			
        }
    } catch (...) {
        // 異常處理
        close(spi_fd);
    }

    close(spi_fd);
    return 0;
}


void demo_all_effections(void){
	
	    try {
        while (true) {
			
	

// --- 展示：能量脈衝碰撞 (10s) ---
        auto start = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count() < 10.0f) {
            float t = std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count();
            eff_30_pulse_collision_tick(255, 0, 0, 0, 0, 255, 0.5f, t); 
            delay_ms(10);
        }

        // --- 展示：電漿流動 (10s) ---
        start = std::chrono::high_resolution_clock::now();
        while(std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count() < 10.0f) {
            float t = std::chrono::duration<float>(std::chrono::high_resolution_clock::now()-start).count();
            eff_31_plasma_flow_tick(t);
            delay_ms(16);
        }





	
		// --- 展示 1: [ID 34] 強化 Tetris ---
        // --- 展示：Tetris 直到觸發慶典 ---
    std::cout << "Step: 開始堆疊 Tetris..." << std::endl;
    bool finished = false;
    while (!finished) {
        // 它會一顆一顆掉，直到填滿 46 顆觸發彩虹後回傳 true
        finished = eff_33_stack_tetris_enhanced(0, 255, 128, 2); 
    }
    
    // 慶典結束後，可以停一下
    std::cout << "Step: 慶典結束，休息 1 秒" << std::endl;
    delay_ms(1000);
			
			
		// --- 特效展示 1: [ID 33] Tetris Stack (隨機 1-3 顆) ---
        std::cout << "Running: [33] Tetris Stack (Random 1-3)..." << std::endl;
        // 跑 30 次掉落約可填滿大半條
        for(int i = 0; i < 30; i++) {
            eff_33_stack_tetris_random(0, 255, 128, 4); 
        }		
		
			
			
		std::cout << "RUN: [33] Stack Tetris Effect..." << std::endl;
		// 跑 LED_NUM 次代表填滿整條燈管
		for(int i = 0; i < LED_NUM + 1; i++) {
		eff_33_stack_tetris(0, 255, 255, 5); // 青色方塊，掉落速度 5ms
		} 	
			
			
			
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
			  std::cout << "執行特效 : 淺藍色呼吸" << std::endl;
    for(int i=0; i<10; i++) 
		eff_breathe_sync(0, 150, 255, 30); // 淺藍色呼吸
    
    // 執行一段時間的彗星
	  std::cout << "執行特效 : 執行一段時間的彗星" << std::endl;
    for(int i=0; i<100; i++) 
		eff_soft_comet(64, 20, 128, 80); // 紅色彗星
	
	
	
			 
// --- [目標]：30秒展示，每次心跳 1-3 次，顏色鎖定，隨機休息 1-4 秒 ---

         // 呼叫生物脈衝，展示 30 秒
		  std::cout << "執行：呼叫生物脈衝 (限時展示 30 秒，含隨機停頓)..." << std::endl;
        eff_bio_pulse_suite(60);
			
			
			
			
			
			// --- 特效 3: 極致閃頻 RGB 融合 (限時展示 30 秒) ---
            std::cout << "執行：極致閃頻 RGB 融合 (限時展示 30 秒，含隨機停頓)..." << std::endl;

			auto demo_start_timer = std::chrono::high_resolution_clock::now();
			while (true) {
			// 1. 隨機決定這次要「亮多久」 (例如 0.5 秒到 2 秒)
			float active_duration = ((rand() % 15) + 5) / 10.0f; 
			auto active_start = std::chrono::high_resolution_clock::now();
    
			// --- 進入「亮燈」階段 ---
			while (true) {
				
				eff_hyper_flash_rgb_fade(); // 執行特效

				auto now = std::chrono::high_resolution_clock::now();
				if (std::chrono::duration<float>(now - active_start).count() >= active_duration) break;
				if (std::chrono::duration<float>(now - demo_start_timer).count() >= 30.0f) goto end_demo2;
			}

			// --- 進入「全黑」階段 (核心需求：隨機 1 到 4 秒) ---
			// 讓所有燈熄滅
			for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);
			ws2812_show();

			float dark_duration = (rand() % 31 + 10) / 10.0f; // 產生 1.0 ~ 4.0 秒的隨機浮點數
			std::cout << "  [停頓] 全黑休息 " << dark_duration << " 秒..." << std::endl;
    
			auto dark_start = std::chrono::high_resolution_clock::now();
			while (true) {
				auto now = std::chrono::high_resolution_clock::now();
				// 檢查熄滅時間是否到達
				if (std::chrono::duration<float>(now - dark_start).count() >= dark_duration) break;
				// 同時也要檢查 30 秒總時間是否到達，避免在全黑時卡住
				if (std::chrono::duration<float>(now - demo_start_timer).count() >= 30.0f) goto end_demo2;
        
				delay_ms(10); // 休息時不需要全速運算，節省一點 CPU
			}
		}

end_demo2:
	std::cout << "30 秒展示結束，切換至下一個特效。" << std::endl;
			
			// 在 main.cpp 的 while(true) 中
			std::cout << "執行：極限閃頻生物心跳 (約 30 秒)..." << std::endl;
			  start = std::chrono::high_resolution_clock::now();
			while (true) {
			eff_heartbeat_extreme(0, 255, 128); // 呼叫一次約 3-4 秒
    
				auto now = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> elapsed = now - start;
			if (elapsed.count() >= 30.0f) break; // 滿 30 秒就跳出
			}
			
			
			std::cout << "執行：慢速生物心跳 (預計 30 秒)..." << std::endl;
			auto start_time = std::chrono::high_resolution_clock::now();

			while (true) {
				eff_heartbeat_biological_slow(0, 255, 128); // 跑一次約 3.3 秒
    
				auto current_time = std::chrono::high_resolution_clock::now();
				std::chrono::duration<float> elapsed = current_time - start_time;
    
				if (elapsed.count() >= 30.0f) { // 滿 30 秒就換下一個特效
				break;
				}
			}
			
			std::cout << "執行：不規則生物心跳特效  (計時 15 秒) (Biological Heartbeat)..." << std::endl; 
			auto startTime = std::chrono::high_resolution_clock::now();
			while (true) {
				// 執行一次特效
			eff_heartbeat_biological(0, 255, 128, 4); 

			// 檢查是否超過 15 秒
			auto now = std::chrono::high_resolution_clock::now();
			std::chrono::duration<float> elapsed = now - startTime;
    
			if (elapsed.count() >= 15.0f) {
				std::cout << "時間到，切換下一個特效！" << std::endl;
				break; 
			}
			}
			
			std::cout << "執行：生理心跳特效 (Heartbeat)..." << std::endl;
			for(int i = 0; i < 10; i++) { // 執行 10 次心跳週期
			eff_heartbeat_sync(0, 255, 255, 5); // 稍微加快 wait，讓跳動更有力
			}
			
			std::cout << "執行：生理曲線呼吸燈 (手動迴圈版)..." << std::endl;
			 start = std::chrono::high_resolution_clock::now();
while(true) {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - start;
    
    // 只負責傳入「現在是第幾秒」
    eff_breathe_pro_tick(0, 255, 128, elapsed.count());
    
    if (elapsed.count() > 10.0f) break; // 準時 10 秒結束
    delay_ms(10); // 穩定的 100 FPS
}
			
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
	
	
	// --- 1. 執行 Theater Chase (劇院追逐) ---
    // 效果：白色燈珠每三顆亮一顆，像劇院門口的跑馬燈
    std::cout << "執行：劇院追逐 (白色)..." << std::endl;
    eff_theater_chase(255, 255, 255, 100); 

    // --- 2. 執行 Gradient Scanner (漸變掃描) ---
    // 效果：在青色 (Cyan) 與 桃紅色 (Magenta) 之間平滑漸變流動
    std::cout << "執行：漸變掃描 (青色 -> 桃紅)..." << std::endl;
    for(int i = 0; i < 300; i++) { // 執行 300 幀
        eff_gradient_scanner(0, 255, 255, 255, 0, 150, 20);
    }
	
	
	std::cout << "執行：閃電風暴..." << std::endl;
for(int i=0; i<5; i++) eff_lightning(255, 255, 255, 50);

std::cout << "執行：多重反彈粒子..." << std::endl;
for(int i=0; i<300; i++) eff_multi_bounce(20);
	
	
	
	// --- 執行：隨機流星 ---
std::cout << "執行：白色流星劃過..." << std::endl;
for(int i = 0; i < LED_NUM * 2; i++) {
    eff_meteor(255, 255, 255, 64, true, 30);
}

// --- 執行：彩色彈珠 ---
std::cout << "執行：RGB 彈珠交錯..." << std::endl;
for(int i = 0; i < 500; i++) {
    eff_marbles(20);
}



std::cout << "執行：理髮店旋轉燈柱..." << std::endl;
    for(int i = 0; i < 200; i++) {
        // 紅色條紋，寬度 6，延遲 50ms
        eff_barber_pole(255, 0, 0, 6, 50); 
    }
	
    std::cout << "執行：三色理髮店燈柱 (紅、白、藍)..." << std::endl;
    // 執行 300 幀 (Frame)，每幀寬度 4 顆燈，延遲 40 毫秒
    for(int i = 0; i < 300; i++) {
        eff_barber_pole_multi(4, 40); 
    }	
			
			
            std::cout << "--- 循環結束，重新開始 ---" << std::endl;
        }
    } catch (...) {
        if (spi_fd >= 0) close(spi_fd);
    }
	
	
}