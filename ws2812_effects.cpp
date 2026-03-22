#include "ws2812_effects.h"
#include <stdlib.h>
#include <iostream>  // 加入這一行
#include <chrono>
#include <cmath>
#include <cmath>
#include <cstdlib>
#include <math.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#include <map>
#include <functional> // 必須包含，用於 std::function
#include <chrono>
// 1. 正式定義變數實體
std::chrono::high_resolution_clock::time_point base_time;


// --- 全域特效數據表 ---
std::map<int, EffectInfo> effect_help_map = {
    // 基礎類 (Category 1)
    {1,  {"Color Wipe", "依次填滿顏色，建議 wait=20。", 1}},
    {3,  {"Rainbow Cycle", "經典全燈條彩虹循環。", 1}},
    {23, {"Color Flow", "顏色像水流一樣平移。", 1}},
    {27, {"Neon Chase", "模擬霓虹燈管逐個點亮的追逐感。", 1}},
    {38, {"Theater Chase", "老電影院招牌風格的間隔跑馬燈。", 1}},
    {39, {"Grad Scanner", "雙色漸層段在燈條上來回掃描。", 1}},
    {40, {"Barber Pole", "理髮店螺旋燈柱旋轉特效。", 1}},
    {41, {"Barber Pole Multi", "紅、白、藍三色經典理髮店燈柱。", 1}},

    // 生物類 (Category 2)
    {13, {"Cyberpulse", "賽博風格亮度規律起伏。", 2}},
    {14, {"Breathe Sync", "所有人造燈光同步平滑淡入淡出。", 2}},
    {15, {"Breathe Pro", "進階數學 Sine 波呼吸，過渡最細膩。", 2}},
    {16, {"Heartbeat Sync", "快速閃動兩下後停頓，模擬心跳。", 2}},
    {17, {"Bio Heartbeat", "模擬真實生物不規則心律。", 2}},
    {18, {"Bio Slow", "極度緩慢的深層呼吸，安靜不刺眼。", 2}},
    {19, {"Extreme Heart", "高頻率、緊張的狂飆心跳閃爍。", 2}},
    {20, {"Bio Pulse Suite", "自動循環播放一系列生物脈衝特效。", 2}},

    // 物理類 (Category 3)
    {24, {"Soft Comet", "一顆流星帶著柔和的長尾巴劃過。", 3}},
    {28, {"Color Bounce", "一顆光點在燈條兩端來回彈跳。", 3}},
    {29, {"Multi Bounce", "多顆光點同時彈跳碰撞。", 3}},
    {30, {"Pulse Collision", "兩道能量波對撞，中心產生高亮爆炸。", 3}},
    {33, {"Tetris Enhanced", "方塊堆疊，填滿後觸發彩虹慶典。", 3}},
    {34, {"Tetris Random", "方塊長度隨機 (1-3) 的堆疊模式。", 3}},
    {35, {"Meteor", "帶有餘輝衰減的隕石，拖尾隨機。", 3}},
    {36, {"Marbles", "幾顆彈珠在燈條上優雅地滾動碰撞。", 3}},

    // 隨機類 (Category 4)
    {4,  {"Fire", "模擬火堆燃燒，隨機向上跳動。", 4}},
    {21, {"Hyper Flash", "RGB 高頻爆閃，派對氣氛最強。", 4}},
    {25, {"Twinkle", "隨機位置、亮度的星星閃爍。", 4}},
    {31, {"Plasma Flow", "電漿流動感 (淡紫色底色)。", 4}},
    {32, {"Aurora", "模擬極光在夜空中擺動的色彩。", 4}},
    {37, {"Lightning", "隨機間隔強白光閃電爆發。", 4}},
	
	// --- 新增：科技與自然系列 ---
    {42, {"Cyber Scanner",    "科技藍光：在燈條上來回掃描的雷射感效果", 1}},
    {43, {"Forest Mist",     "森林之霧：綠色與青色的交錯呼吸動態背景", 2}},
    {44, {"Star Burst",      "隨機星爆：閃爍暖白星光並伴隨優雅的淡出拖尾", 4}},
	// --- 確保這行存在 ---
    {45, {"Pride 2015",    "經典彩虹：多層正弦波疊加的絲滑織物感", 3}},
	{46, {"Sinelon",       "孤寂擺鐘：帶有長尾巴的往復運動亮點", 1}}, // 新增這行
	{47, {"Sinelon Rainbow", "彩色變幻擺鐘：結合 HSV 變色邏輯", 1}},
	{48, {"Aurora Sinelon", "極光擺鐘：自帶色彩基因的動態拖尾", 1}},
	{49, {"Dual Sinelon", "雙重交錯流星。兩顆不同顏色的流星在燈條上對稱碰撞，視覺衝擊力強。", 3}},
	{50, {"Collision Sparkle", "雙重鏡像流星 + 碰撞火花。撞擊瞬間會產生亮度爆發與隨機白閃。", 3}},
	{51, {"Static Rainbow", "靜態全燈彩虹切換。每個顏色停留固定時間 (預設 2秒)。", 1}},
	{52, {"Hyper Flash", "高頻指數型呼吸閃爍。中心擴散視覺，隨時間變換色相。", 1}},
	
};
// 建立 ID 與特效資訊的映射表
std::map<int, EffectInfo> effect_help_mapxxx = {
    {1,  {"Color Wipe", "顏色依次擦除填滿，像刷漆一樣。適合測試燈珠順序。", 1}},
    {3,  {"Rainbow Cycle", "整個燈條呈現連續變化的彩虹。最經典的展示模式。", 1}},
    {4,  {"Fire", "模擬火堆燃燒，底部亮紅/橙色並隨機向上跳動。營造溫暖感。", 4}},
    {13, {"Cyberpulse", "全域亮度像呼吸一樣規律起伏，具備未來科技感。", 2}},
    {14, {"Breathe Sync", "所有人造燈光同步平滑淡入淡出，適合睡眠背景。", 2}},
    {15, {"Breathe Pro", "基於數學 Sine 波的進階呼吸，過渡最細膩平滑。", 2}},
    {16, {"Heartbeat Sync", "模擬心跳，快速閃動兩下後停頓，適合警告提醒。", 2}},
    {17, {"Bio Heartbeat", "模擬生物真實心律，帶有輕微的不規則生命感。", 2}},
    {18, {"Bio Slow", "極度緩慢的深層呼吸心跳，安靜不刺眼。", 2}},
    {19, {"Extreme Heart", "高頻率、緊張的狂飆心跳閃爍。", 2}},
    {20, {"Bio Pulse Suite", "自動循環播放一系列生物脈衝特效的組合包。", 2}},
    {21, {"Hyper Flash", "RGB 三原色隨機高頻爆閃並淡出，派對氣氛最強。", 4}},
    {23, {"Color Flow", "顏色像水流一樣在燈條上平移，適合邊角流光。", 1}},
    {24, {"Soft Comet", "一顆流星帶著柔和的長尾巴劃過燈條。", 3}},
    {25, {"Twinkle", "隨機位置、隨機亮度的星星閃爍，適合夜燈。", 4}},
    {26, {"Dual Fade", "在兩種自定義顏色之間不斷進行平滑切換。", 1}},
    {27, {"Neon Chase", "模擬霓虹燈管逐個點亮的追逐感，經典復古。", 1}},
    {28, {"Color Bounce", "一顆光點在燈條兩端之間來回彈跳。", 3}},
    {29, {"Multi Bounce", "多顆光點同時彈跳，碰撞時有視覺交錯感。", 3}},
    {30, {"Pulse Collision", "兩道能量波從兩端對撞，中心點產生高亮碰撞。", 3}},
    {31, {"Plasma Flow", "多重波形疊加的電漿感，搭配淡紫色底色。", 4}},
    {32, {"Aurora", "模擬極光在夜空中擺動的繽紛繽紛流動。", 4}},
    {33, {"Tetris Enhanced", "方塊掉落堆疊，填滿後自動觸發彩虹慶典。", 3}},
    {34, {"Tetris Random", "方塊長度隨機 (1-3) 的堆疊模式，更具遊戲感。", 3}},
    {35, {"Meteor", "帶有餘輝衰減的隕石，拖尾消失速度隨機。", 3}},
    {36, {"Marbles", "模擬幾顆彈珠在燈條上優雅地滾動與碰撞。", 3}},
    {37, {"Lightning", "隨機間隔、隨機亮度的強白光閃電爆發。", 4}},
	
    {38, {"Theater Chase", "老電影院招牌風格的間隔跑馬燈。", 1}},
    {39, {"Grad Scanner", "雙色漸層段在燈條上來回掃描，適合狀態顯示。", 1}},
    {40, {"Barber Pole", "理髮店螺旋燈柱旋轉特效。", 1}},
    {41, {"Barber Pole Multi", "紅、白、藍三色經典理髮店燈柱旋轉。", 1}},
	{42, {"Cyber Scanner", "科技感藍色往復掃描", 1}},
	{43, {"Forest Mist", "森林風綠色交錯呼吸", 2}},
	{44, {"Star Burst", "隨機星光閃爍後消散", 4}},
	{45, {"Pride 2015", "經典 Pride 2015：多層正弦波疊加的絲滑彩虹", 3}},
	
};


void print_detailed_help(char* prog_name) {
    printf("\n==============================================================================\n");
    printf("   RPi 5 ARGB 控制器說明書 (LED: %d)\n", LED_NUM);
    printf("==============================================================================\n");
    printf(" 使用方式: sudo %s [ID/off]\n\n", prog_name);
    
    const char* cats[] = {"", "【基礎類】", "【生物類】", "【物理類】", "【隨機類】, 【科技與自然系列】"};
    for (int i = 1; i <= 4; i++) {
        printf("%s\n", cats[i]);
        for (auto const& [id, info] : effect_help_map) {
            if (info.category == i) {
                printf("  ID %2d : %-18s - %s\n", id, info.name.c_str(), info.description.c_str());
            }
        }
        printf("\n");
    }
    printf(" 指令: off (關燈) | --help (說明)\n");
    printf("==============================================================================\n");
}

// 透過名稱反查 ID 的工具函式
int get_id_by_name(std::string name) {
    for (auto const& [id, info] : effect_help_map) {
        if (info.name == name) return id;
    }
    return -1; // 找不到則回傳 -1
}


// --- 執行邏輯 ---
// --- 特效執行調度器 (支援 ID 前綴列印與無限循環) ---
void run_effect_loop(int id, float t) {
    // 取得當前精確時間，用於 Tick 類特效
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto get_t = [&]() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float>(now - start_time).count();
    };

    // --- 新增：ID 前綴列印邏輯 ---
    // 使用 static 紀錄上一次的 ID，避免在 while(true) 中重複洗版
    static int last_printed_id = -1;
    if (id != last_printed_id) {
        if (effect_help_map.count(id)) {
            // 格式：[ID] 名稱 - 描述
            printf("EFFECT ID : [%02d] 啟動特效: %s - %s\n", 
                   id, 
                   effect_help_map[id].name.c_str(), 
                   effect_help_map[id].description.c_str());
        }
        last_printed_id = id;
    }

    // --- 特效分發邏輯 ---
    switch (id) {
        // Category 1: 基礎類 (Basic)
        case 1:  eff_1_color_wipe(0, 255, 127, 20); break;
        case 3:  eff_3_rainbow_cycle(10); break;
        case 23: eff_color_flow(30); break;
        case 27: eff_neon_chase(0, 255, 150, 30); break;
        case 28: eff_color_bounce(30); break;
        case 38: eff_theater_chase(255, 255, 255, 100); break;
        case 39: eff_gradient_scanner(0, 255, 255, 255, 0, 150, 20); break;
        case 40: eff_barber_pole(255, 0, 0, 6, 50); break;
        case 41: eff_barber_pole_multi(4, 40); break;

        // Category 2: 生物脈衝類 (Biological)
        case 13: eff_13_cyberpulse(0, 255, 255, 10); break;
        case 14: eff_breathe_sync(200, 0, 255, 30); break;
        case 15: eff_breathe_pro_tick(0, 255, 150, get_t()); delay_ms(16); break;
        case 16: eff_heartbeat_sync(255, 0, 0, 2); break;
        case 17: eff_heartbeat_biological(255, 20, 20, 4); break;
        case 18: eff_heartbeat_biological_slow(100, 0, 0); break;
        case 19: eff_heartbeat_extreme(255, 0, 0); break;
        case 20: eff_bio_pulse_suite(30); break;

        // Category 3: 物理與動態類 (Physical)
        case 24: eff_soft_comet(255, 255, 255, 40); break;
        case 29: eff_multi_bounce(25); break;
        case 30: eff_30_pulse_collision_tick(255,0,0, 0,0,255, 0.8f, get_t()); delay_ms(16); break;
        case 33: // Tetris 邏輯：堆疊滿了會回傳 true，while 確保它跑完一輪
            while(!eff_33_stack_tetris_enhanced(0, 255, 128, 2)); 
            break; 
        case 34: eff_33_stack_tetris_random(255, 128, 0, 5); break;
        case 35: eff_meteor(255, 255, 255, 64, true, 30); break;
        case 36: eff_marbles(30); break;

        // Category 4: 自然與隨機類 (Natural)
        case 4:  eff_4_fire(55, 120, 15); break;
        case 21: eff_hyper_flash_rgb_fade(); break;
        case 25: eff_twinkle(255, 255, 255, 5, 100); break;
        case 31: eff_31_plasma_flow_tick(get_t()); delay_ms(16); break;
        case 32: eff_aurora(0, 255, 150, 40); break;
        case 37: eff_lightning(255, 255, 255, 50); delay_ms(rand() % 1000 + 500); break;
		
		// Category 5: 科技與自然系列 (Natural)
		case 42: eff_42_cyber_scanner(0, 150, 255, 30); break;
		case 43: eff_43_forest_mist(get_t()); delay_ms(16); break;
		case 44: eff_44_star_burst(15); break;
		case 45: eff_45_pride_2015(get_t()); delay_ms(16); /* 保持 60fps 的絲滑感*/    break;
		case 46: /* 這裡我們給它一個酷炫的紫色 (150, 0, 255)*/ eff_46_sinelon(get_t(),0, 120, 150);delay_ms(15); /* 延遲決定了動畫的平滑度*/ break;
		case 47: /* 彩色版：現在只需要一行！*/  eff_47_sinelon_rainbow(get_t()); delay_ms(15); break;
	    case 48: /*Aurora 版本不需要在外層計算顏色，函式內部會自理*/ eff_48_aurora_sinelon(get_t()); delay_ms(12); /* 高更新率讓極光更絲滑*/ break;
		case 49: /* [ID 49] 雙重交錯流星 , 直接傳入全域時間 get_t()，內部會透過 get_local_t(t, 49) 處理重置*/ eff_49_dual_sinelon(get_t()); \
		/*建議幀率設定在 12ms ~ 15ms 之間，畫面會非常流暢*/usleep(12000);  break;
	   
	   case 50: eff_50_dual_sinelon_sparkle(get_t()); usleep(12000); /* 保持流暢的幀率 */ break;
	   case 51: /* 這裡傳入 2.0f 代表每個顏色顯示 2 秒*/ eff_51_static_rainbow_step(get_t(), 2.0f);usleep(50000); /* 靜態切換不需要太高頻率50ms(20fps) 即可*/ 
	   break;
	   case 52: { /* 讓色相每 10 秒轉一圈，或是固定一個顏色*/ float dynamic_hue = fmodf(t * 50.0f, 360.0f); eff_52_hyper_flash_fixed(t, dynamic_hue); break;
	   }
	   // 特別 ID: 錯誤警告
	   case 999: eff_999_system_error_alert(); break;

        default:
            // 如果 ID 不在 switch 內，預設執行警告
            eff_999_system_error_alert();
            break;
    }
}


void run_effect_by_id(int id, float t) {
        // 取得當前精確時間，用於 Tick 類特效
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto get_t = [&]() {
        auto now = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<float>(now - start_time).count();
    };

    // --- 新增：ID 前綴列印邏輯 ---
    // 使用 static 紀錄上一次的 ID，避免在 while(true) 中重複洗版
    static int last_printed_id = -1;
    if (id != last_printed_id) {
        if (effect_help_map.count(id)) {
            // 格式：[ID] 名稱 - 描述
            printf("EFFECT ID : [%02d] 啟動特效: %s - %s\n", 
                   id, 
                   effect_help_map[id].name.c_str(), 
                   effect_help_map[id].description.c_str());
        }
        last_printed_id = id;
    }

    // --- 特效分發邏輯 ---
    switch (id) {
        // Category 1: 基礎類 (Basic)
        case 1:  eff_1_color_wipe(0, 255, 127, 20); break;
        case 3:  eff_3_rainbow_cycle(10); break;
        case 23: eff_color_flow(30); break;
        case 27: eff_neon_chase(0, 255, 150, 30); break;
        case 28: eff_color_bounce(30); break;
        case 38: eff_theater_chase(255, 255, 255, 100); break;
        case 39: eff_gradient_scanner(0, 255, 255, 255, 0, 150, 20); break;
        case 40: eff_barber_pole(255, 0, 0, 6, 50); break;
        case 41: eff_barber_pole_multi(4, 40); break;

        // Category 2: 生物脈衝類 (Biological)
        case 13: eff_13_cyberpulse(0, 255, 255, 10); break;
        case 14: eff_breathe_sync(200, 0, 255, 30); break;
        case 15: eff_breathe_pro_tick(0, 255, 150, get_t()); delay_ms(16); break;
        case 16: eff_heartbeat_sync(255, 0, 0, 2); break;
        case 17: eff_heartbeat_biological(255, 20, 20, 4); break;
        case 18: eff_heartbeat_biological_slow(100, 0, 0); break;
        case 19: eff_heartbeat_extreme(255, 0, 0); break;
        case 20: eff_bio_pulse_suite(30); break;

        // Category 3: 物理與動態類 (Physical)
        case 24: eff_soft_comet(255, 255, 255, 40); break;
        case 29: eff_multi_bounce(25); break;
        case 30: eff_30_pulse_collision_tick(255,0,0, 0,0,255, 0.8f, get_t()); delay_ms(16); break;
        case 33: // Tetris 邏輯：堆疊滿了會回傳 true，while 確保它跑完一輪
            while(!eff_33_stack_tetris_enhanced(0, 255, 128, 2)); 
            break; 
        case 34: eff_33_stack_tetris_random(255, 128, 0, 5); break;
        case 35: eff_meteor(255, 255, 255, 64, true, 30); break;
        case 36: eff_marbles(30); break;

        // Category 4: 自然與隨機類 (Natural)
        case 4:  eff_4_fire(55, 120, 15); break;
        case 21: eff_hyper_flash_rgb_fade(); break;
        case 25: eff_twinkle(255, 255, 255, 5, 100); break;
        case 31: eff_31_plasma_flow_tick(get_t()); delay_ms(16); break;
        case 32: eff_aurora(0, 255, 150, 40); break;
        case 37: eff_lightning(255, 255, 255, 50); delay_ms(rand() % 1000 + 500); break;
		
		// Category 5: 科技與自然系列 (Natural)
		case 42: eff_42_cyber_scanner(0, 150, 255, 30); break;
		case 43: eff_43_forest_mist(get_t()); delay_ms(16); break;
		case 44: eff_44_star_burst(15); break;
		case 45: eff_45_pride_2015(get_t()); delay_ms(16); /* 保持 60fps 的絲滑感*/    break;
		case 46: /* 這裡我們給它一個酷炫的紫色 (150, 0, 255)*/ eff_46_sinelon(get_t(),0, 120, 150);delay_ms(15); /* 延遲決定了動畫的平滑度*/ break;
		case 47: /* 彩色版：現在只需要一行！*/  eff_47_sinelon_rainbow(get_t()); delay_ms(15); break;
	    case 48: /*Aurora 版本不需要在外層計算顏色，函式內部會自理*/ eff_48_aurora_sinelon(get_t()); delay_ms(12); /* 高更新率讓極光更絲滑*/ break;
		case 49: /* [ID 49] 雙重交錯流星 , 直接傳入全域時間 get_t()，內部會透過 get_local_t(t, 49) 處理重置*/ eff_49_dual_sinelon(get_t());/*建議幀率設定在 12ms ~ 15ms 之間，畫面會非常流暢*/usleep(12000);  break;
	   
	   case 50: eff_50_dual_sinelon_sparkle(get_t()); usleep(12000); /* 保持流暢的幀率 */ break;
	   case 51: /* 這裡傳入 2.0f 代表每個顏色顯示 2 秒*/ eff_51_static_rainbow_step(get_t(), 2.0f);usleep(50000); /* 靜態切換不需要太高頻率，50ms (20fps) 即可*/ break;
	    case 52: {/* 讓色相每 10 秒轉一圈，或是固定一個顏色*/ float dynamic_hue = fmodf(t * 50.0f, 360.0f); eff_52_hyper_flash_fixed(t, dynamic_hue); break;
		}
		/* 特別 ID: 錯誤警告  */
        case 999: eff_999_system_error_alert(); break;

        default:
            // 如果 ID 不在 switch 內，預設執行警告
            eff_999_system_error_alert();
            break;
    }
}

 
// 內部使用的靜態變數，用來追蹤特效切換
static int last_effect_id = -1;
static float start_time_offset = 0;

/**
 * 延時函式 (毫秒)
 */
void delay_ms(uint32_t ms) {
    usleep(ms * 1000); 
}


// 實作獲取時間的輔助函式
float get_elapsed_t() {
    auto now = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = now - base_time;
    return elapsed.count();
}
 
/**
 * 根據目前的特效 ID，回傳相對應的局部時間 (Local Time)
 * 確保每次切換 ID 時，回傳值都會從 0.0f 開始
 */
 
float get_local_t(float t, int id) {
    static int last_id = -1;
    static float start_off = 0;

    if (id != last_id) {
        last_id = id;
        start_off = t; // 紀錄切換瞬間的全域時間
    }
    return t - start_off; // 必須回傳「差值」
} 
 
float get_local_txxx(float global_t, int effect_id) {
    // 如果偵測到 ID 改變了（使用者切換了特效）
    if (effect_id != last_effect_id) {
        last_effect_id = effect_id;
        start_time_offset = global_t; // 紀錄切換當下的全域時間
    }
    return global_t - start_time_offset;
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


// 內部輔助：極致絲滑的衰減運算
uint8_t smooth_fade(uint8_t val, float factor) {
    if (val == 0) return 0;
    
    // 1. 基本乘法衰減
    uint8_t next = static_cast<uint8_t>(val * factor);
    
    // 2. 關鍵修正：解決「捨入陷阱」
    // 如果乘法結果沒變（例如 5 * 0.96 = 4.8，無條件捨去還是 4）
    // 但在視覺上，我們需要它確實地「減 1」來維持流暢感
    if (next == val && val > 0) {
        next = val - 1;
    }
    
    return next;
}



//輔助函數

// FastLED-style HSV to PrideRGB conversion
// ws2812_effects.cpp

PrideRGB hsv2rgb(uint8_t h, uint8_t s, uint8_t v) {
    PrideRGB rgb;
    uint8_t region, remainder, p, q, t;

    if (s == 0) {
        rgb.r = rgb.g = rgb.b = v;
        return rgb;
    }

    region = h / 43;
    remainder = (h % 43) * 6; 

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:  rgb.r = v; rgb.g = t; rgb.b = p; break;
        case 1:  rgb.r = q; rgb.g = v; rgb.b = p; break;
        case 2:  rgb.r = p; rgb.g = v; rgb.b = t; break;
        case 3:  rgb.r = p; rgb.g = q; rgb.b = v; break;
        case 4:  rgb.r = t; rgb.g = p; rgb.b = v; break;
        default: rgb.r = v; rgb.g = p; rgb.b = q; break;
    } // <-- 這裡原本可能誤寫成 } Region，請刪除該單字
    
    return rgb;
}

void render_sinelon_point(float t, uint8_t r, uint8_t g, uint8_t b) {
    float speed = 0.7f; 
    float pos_f = (-cosf(t * speed) + 1.0f) / 2.0f * (LED_NUM - 1);
    int pos_i = static_cast<int>(pos_f);
    float frac = pos_f - pos_i;

    auto add_color = [&](int idx, float intensity) {
        if (idx < 0 || idx >= LED_NUM) return;
        uint8_t tr = (uint8_t)(r * intensity);
        uint8_t tg = (uint8_t)(g * intensity);
        uint8_t tb = (uint8_t)(b * intensity);
        PrideRGB cur = get_led_color(idx);
        set_led_color(idx, std::max(cur.r, tr), std::max(cur.g, tg), std::max(cur.b, tb));
    };

    add_color(pos_i, 1.0f - frac);
    add_color(pos_i + 1, frac);
}

void render_fixed_hue_frame(float hue, float intensity) {
    float r_f, g_f, b_f;
    
    // HSV 轉 RGB 的簡化演算法
    float h_prime = hue / 60.0f;
    float x = (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));
    
    if (0 <= h_prime && h_prime < 1)      { r_f = 1; g_f = x; b_f = 0; }
    else if (1 <= h_prime && h_prime < 2) { r_f = x; g_f = 1; b_f = 0; }
    else if (2 <= h_prime && h_prime < 3) { r_f = 0; g_f = 1; b_f = x; }
    else if (3 <= h_prime && h_prime < 4) { r_f = 0; g_f = x; b_f = 1; }
    else if (4 <= h_prime && h_prime < 5) { r_f = x; g_f = 0; b_f = 1; }
    else                                  { r_f = 1; g_f = 0; b_f = x; }

    int center = LED_NUM / 2;
    for (int j = 0; j < LED_NUM; j++) {
        // 加入空間淡化，讓中心最亮，兩端稍暗，更有能量感
        float dist_factor = 1.0f - (abs(j - center) / (float)center);
        float spatial_fade = pow(dist_factor, 1.2f);

        // 結合傳入的 intensity (生理曲線亮度)
        uint8_t r_out = (uint8_t)(r_f * intensity * 255 * spatial_fade);
        uint8_t g_out = (uint8_t)(g_f * intensity * 255 * spatial_fade);
        uint8_t b_out = (uint8_t)(b_f * intensity * 255 * spatial_fade);
        
        set_led_color(j, r_out, g_out, b_out);
    }
    // 這裡不呼叫 ws2812_show()，留給外部迴圈控制
}


void draw_sinelon_head(float t, uint8_t r, uint8_t g, uint8_t b) {
    float speed = 0.7f; 
    float pos_f = (-cosf(t * speed) + 1.0f) / 2.0f * (LED_NUM - 1);
    
    // 定義流星頭部的「半徑」
    // 0.8f 代表這個頭部會影響中心點前後約 1 顆燈的距離，總長度會很穩定
    float radius = 0.8f; 

    for (int i = -2; i <= 2; i++) { // 檢查中心點前後各 2 顆燈
        int idx = static_cast<int>(pos_f) + i;
        if (idx < 0 || idx >= LED_NUM) continue;

        // 計算燈珠與精確浮點位置的距離
        float dist = fabsf(pos_f - (float)idx);

        if (dist < radius) {
            // 使用線性插值計算亮度：距離越近越亮
            float intensity = 1.0f - (dist / radius);
            
            uint8_t tr = static_cast<uint8_t>(r * intensity);
            uint8_t tg = static_cast<uint8_t>(g * intensity);
            uint8_t tb = static_cast<uint8_t>(b * intensity);

            PrideRGB cur = get_led_color(idx);
            set_led_color(idx, std::max(cur.r, tr), std::max(cur.g, tg), std::max(cur.b, tb));
        }
    }
}

// Returns a value that oscillates between 0-255 at a specific BPM
uint8_t beat8(float bpm, float t) {
    float beat = sinf(t * (bpm / 60.0f) * M_PI);
    return (uint8_t)((beat + 1.0f) * 127.5f);
}

float get_t() {
    static auto start_time = std::chrono::high_resolution_clock::now();
    auto current_time = std::chrono::high_resolution_clock::now();
    
    // 計算與啟動時間的差值，轉換為秒
    std::chrono::duration<float> elapsed = current_time - start_time;
    return elapsed.count(); 
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

void eff_heartbeat_sync(uint8_t r, uint8_t g, uint8_t b, int wait) {
    // 一個完整的心跳週期分為 256 個步驟
    for (int i = 0; i < 256; i++) {
        float angle = (i / 256.0f) * M_PI * 2;
        
        // 心跳曲線公式：結合兩個 sine 波來模擬雙重搏動
        // 第一個波形 (sin(x)) 是主跳，第二個波形 (sin(2x)) 創造第二次輕微搏動
        float heart = pow(sin(angle), 6) * (1.0f + 0.5f * sin(angle * 2 + M_PI/2));
        
        // 確保亮度過渡平滑且具備動態感
        float intensity = heart; 

        for (int j = 0; j < LED_NUM; j++) {
            set_led_color(j, (uint8_t)(r * intensity), 
                             (uint8_t)(g * intensity), 
                             (uint8_t)(b * intensity));
        }
        ws2812_show();
        delay_ms(wait);
    }
}

void eff_breathe_pro_tick(uint8_t r, uint8_t g, uint8_t b, float t) {
    // 讓呼吸頻率固定（例如 0.5Hz，兩秒呼吸一次）
    float freq = 0.5f; 
    float x = t * freq * M_PI * 2;
    
    float intensity = exp(sin(x)) / M_E;
    float final_it = pow(intensity, 2);

    for (int j = 0; j < LED_NUM; j++) {
        // 使用我們之前討論的保底亮度 1，確保前兩顆燈在微弱呼吸時不熄滅
        uint8_t r_out = (final_it > 0.01f) ? std::max((int)1, (int)(r * final_it)) : 0;
        uint8_t g_out = (final_it > 0.01f) ? std::max((int)1, (int)(g * final_it)) : 0;
        uint8_t b_out = (final_it > 0.01f) ? std::max((int)1, (int)(b * final_it)) : 0;
        
        set_led_color(j, r_out, g_out, b_out);
    }
    ws2812_show();
}

void eff_heartbeat_biological(uint8_t r, uint8_t g, uint8_t b, int wait) {
    // 1. 隨機決定這次跳動幾下 (1 或 2 下)
    int beats = (rand() % 2) + 1; 

    for (int b_cnt = 0; b_cnt < beats; b_cnt++) {
        // 2. 隨機產生這一下的亮度衰減 (0.6 ~ 1.0 之間)
        float fading = ((rand() % 40) + 60) / 100.0f;
        
        // 3. 單次搏動動畫 (使用快速上升、緩慢下降的曲線)
        for (int i = 0; i < 100; i++) {
            float angle = (i / 100.0f) * M_PI;
            // 使用 sin 的高次方 (pow 8) 創造出更有力的爆發感
            float intensity = pow(sin(angle), 8) * fading;

            for (int j = 0; j < LED_NUM; j++) {
                set_led_color(j, (uint8_t)(r * intensity), 
                                 (uint8_t)(g * intensity), 
                                 (uint8_t)(b * intensity));
            }
            ws2812_show();
            // 搏動時速度較快
            delay_ms(wait);
        }
        
        // 兩次搏動之間的小停頓
        for(int j=0; j<LED_NUM; j++) set_led_color(j, 0, 0, 0);
        ws2812_show();
        delay_ms(wait * 20);
    }

    // 4. 大循環之間長度不一的「休息時間」，讓節奏不規律
    int rest = (rand() % 500) + 200;
    delay_ms(rest);
}

void eff_heartbeat_extreme(uint8_t r, uint8_t g, uint8_t b) {
    // 1. 隨機決定跳動次數 (1-3 次)
    int beats = (rand() % 3) + 1; 

    for (int b_cnt = 0; b_cnt < beats; b_cnt++) {
        // 2. 判斷是否為多重跳動模式 (興奮狀態)
        bool is_fast = (beats > 1);
        
        // 如果是多重跳動，大幅縮短每一跳的幀數 (原本 150 改為 60)，讓速度變快
        int frames = is_fast ? 60 : 150;
        // 如果是多重跳動，使用更高次方 (pow 16) 產生 Flash 閃爍感，否則用正常的 pow 10
        float power_val = is_fast ? 16.0f : 10.0f;
        
        float fading = ((rand() % 40) + 60) / 100.0f;

        for (int i = 0; i < frames; i++) {
            float angle = (i / (float)frames) * M_PI;
            
            // 核心演算法：透過高次方達成 Flash 效果 (極短時間的高亮度)
            float intensity = pow(sin(angle), power_val) * fading;

            for (int j = 0; j < LED_NUM; j++) {
                set_led_color(j, (uint8_t)(r * intensity), 
                                 (uint8_t)(g * intensity), 
                                 (uint8_t)(b * intensity));
            }
            ws2812_show();
            
            // 如果是快速模式，延遲也要縮短 (2ms)，增加緊迫感
            delay_ms(is_fast ? 2 : 4); 
        }
        
        // 兩次跳動之間的間隔也隨之縮短
        delay_ms(is_fast ? 80 : 200);
    }

    // 3. 結尾的大休息時間：如果是快速心跳，休息時間也隨機縮短，模擬心律不整
    int rest_base = (beats > 1) ? 500 : 1500;
    delay_ms(rest_base + (rand() % 1000));
}

#include <chrono> // 用於精確時間計算
#include <math.h>


void eff_hyper_flash_rgb_fade() {
    // 1. 靜態變數與時間追蹤
    static auto start_time = std::chrono::high_resolution_clock::now();
    static float color_hue = 0.0f;
    
    auto current_time = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> elapsed = current_time - start_time;
    float t = elapsed.count();

    // 2. 亮度生理曲線 (4Hz 心跳感)
    float breathe_freq = 4.0f; 
    float brightness_base = exp(sin(t * breathe_freq * M_PI * 2)) / M_E; 
    float intensity = pow(brightness_base, 8.0f); 

    // 3. 彩虹顏色變換 (HSV 轉 RGB 邏輯)
    color_hue += 0.8f; // 控制彩虹變換速度
    if (color_hue >= 360.0f) color_hue -= 360.0f;
    
    float r_f, g_f, b_f;
    float h_prime = color_hue / 60.0f;
    float x = (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));

    // 標準 HSV 轉 RGB 分段映射
    if (0 <= h_prime && h_prime < 1)      { r_f = 1.0f; g_f = x;    b_f = 0.0f; }
    else if (1 <= h_prime && h_prime < 2) { r_f = x;    g_f = 1.0f; b_f = 0.0f; }
    else if (2 <= h_prime && h_prime < 3) { r_f = 0.0f; g_f = 1.0f; b_f = x;    }
    else if (3 <= h_prime && h_prime < 4) { r_f = 0.0f; g_f = x;    b_f = 1.0f; }
    else if (4 <= h_prime && h_prime < 5) { r_f = x;    g_f = 0.0f; b_f = 1.0f; }
    else                                  { r_f = 1.0f; g_f = 0.0f; b_f = x;    }

    // 4. 超快速閃頻觸發 (Flash Trigger)
    float flash_trigger = pow(sin(t * 25.0f * M_PI), 10.0f); 
    if (intensity > 0.8f && flash_trigger > 0.9f) {
        intensity = 1.0f; // 閃頻瞬間強制最高亮度
    }

    // --- 5. 核心修正：全域同步渲染與保底亮度 ---
    // 這裡我們不再根據 index (j) 計算距離，確保整條燈管數據一致
    for (int j = 0; j < LED_NUM; j++) {
        // 使用 std::max(1, ...) 確保只要 intensity > 0，硬體至少會收到 1 的訊號
        // 這能有效解決 RPi 5 SPI 在長線模式下第一、二顆 LED 因為訊號太弱而不亮的問題
        uint8_t r_out = (intensity > 0.005f) ? std::max((int)1, (int)(r_f * 255 * intensity)) : 0;
        uint8_t g_out = (intensity > 0.005f) ? std::max((int)1, (int)(g_f * 255 * intensity)) : 0;
        uint8_t b_out = (intensity > 0.005f) ? std::max((int)1, (int)(b_f * 255 * intensity)) : 0;

        set_led_color(j, r_out, g_out, b_out);
    }

    // 推送到 SPI
    ws2812_show();
}

 

 

void eff_heartbeat_biological_slow(uint8_t r, uint8_t g, uint8_t b) {
    // 1. 隨機決定跳動次數 (1-2次，模擬心臟房室收縮)
    int beats = (rand() % 2) + 1; 

    for (int b_cnt = 0; b_cnt < beats; b_cnt++) {
        // 2. 隨機亮度衰減 (60% ~ 100%)，讓心跳強弱有變化
        float fading = ((rand() % 40) + 60) / 100.0f;
        
        // 3. 搏動動畫：調慢循環次數 (原本 100 改為 150)
        for (int i = 0; i < 150; i++) {
            float angle = (i / 150.0f) * M_PI;
            
            // 使用更高次方 (pow 10) 讓亮度「爆發」感更集中，暗部過渡更久
            float intensity = pow(sin(angle), 10) * fading;

            for (int j = 0; j < LED_NUM; j++) {
                set_led_color(j, (uint8_t)(r * intensity), 
                                 (uint8_t)(g * intensity), 
                                 (uint8_t)(b * intensity));
            }
            ws2812_show();
            // 延遲稍微增加，讓單次跳動變慢
            delay_ms(4); 
        }
        
        // 兩次搏動間的短暫停頓 (第一跳與第二跳之間)
        delay_ms(150 + (rand() % 100));
    }

    // 4. 關鍵：大幅拉長兩組心跳之間的大休息時間 (1.5秒 ~ 3秒)
    // 這樣才不會像在跑步，而是像平靜的呼吸
    int long_rest = (rand() % 1500) + 1500;
    
    // 休息期間確保燈是完全熄滅的
    for(int j=0; j<LED_NUM; j++) set_led_color(j, 0, 0, 0);
    ws2812_show();
    
    delay_ms(long_rest);
}


/**
 * 生物感脈衝特效套件
 * @param duration_sec 展示總秒數
 */
 void eff_bio_pulse_suite(int duration_sec) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    while (true) {
        auto current_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<float> elapsed = current_time - start_time;
        if (elapsed.count() >= duration_sec) break;

        float t = elapsed.count();

        // 1. 生理呼吸曲線 (全域同步亮度)
        // 使用 exp(sin) 模擬更真實的肺部呼吸或血液脈動感
        float breathe_freq = 0.8f; // 較慢的基礎呼吸
        float brightness_base = exp(sin(t * breathe_freq * M_PI * 2)) / M_E;
        
        // 疊加一個微小的快速震盪，模擬生物的「不安全感」或「生命力」
        float jitter = 0.9f + 0.1f * sin(t * 15.0f);
        float final_intensity = brightness_base * jitter;

        // 2. 顏色選擇 (這裡可以根據需要調整為固定顏色或緩慢漸變)
        // 範例：深紅色脈動
        uint8_t r_target = 255;
        uint8_t g_target = 20;
        uint8_t b_target = 10;

        // 3. 核心修正：全域渲染 (不再計算空間淡化)
        for (int j = 0; j < LED_NUM; j++) {
            // 使用 std::max(1, ...) 確保即使在亮度低谷，前兩顆燈也不會熄滅
            uint8_t r_out = (final_intensity > 0.01f) ? std::max((int)1, (int)(r_target * final_intensity)) : 0;
            uint8_t g_out = (final_intensity > 0.01f) ? std::max((int)1, (int)(g_target * final_intensity)) : 0;
            uint8_t b_out = (final_intensity > 0.01f) ? std::max((int)1, (int)(b_target * final_intensity)) : 0;

            set_led_color(j, r_out, g_out, b_out);
        }

        // 4. 立即顯示
        ws2812_show();

        // 控制幀率，減輕系統負擔 (約 60 FPS)
        delay_ms(16);
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
	if (pos >= LED_NUM) pos = 0; // Add this to Comet, Marbles, and Bounce effects
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
	if (pos >= LED_NUM) pos = 0; 
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



// --- 特效 9: 閃電風暴 (Lightning Storm) ---
void eff_lightning(uint8_t r, uint8_t g, uint8_t b, int wait) {
    // 先將所有燈熄滅
    for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);
    ws2812_show();
    delay_ms(rand() % 1000 + 500); // 隨機等待閃電發生

    // 閃電核心：隨機選一個位置
    int flash_pos = rand() % LED_NUM;
    int flash_len = rand() % 10 + 5; // 閃電影響的長度

    for(int flash = 0; flash < 3; flash++) { // 閃動 3 次
        for(int i=0; i<LED_NUM; i++) {
            if(abs(i - flash_pos) < flash_len) 
                set_led_color(i, r, g, b); // 閃電顏色 (通常建議 255, 255, 255)
            else 
                set_led_color(i, 0, 0, 0);
        }
        ws2812_show();
        delay_ms(20 + rand() % 50);
        
        for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);
        ws2812_show();
        delay_ms(30);
    }
}

// --- 特效 10: 劇院追逐 (Theater Chase) ---
void eff_theater_chase(uint8_t r, uint8_t g, uint8_t b, int wait) {
    for (int j=0; j<10; j++) {  // 滾動 10 次
        for (int q=0; q < 3; q++) {
            for (int i=0; i < LED_NUM; i=i+3) {
                set_led_color(i+q, r, g, b); // 每三顆點亮一顆
            }
            ws2812_show();
            delay_ms(wait);
            for (int i=0; i < LED_NUM; i=i+3) {
                set_led_color(i+q, 0, 0, 0); // 熄滅以便下次移動
            }
        }
    }
}


// --- 特效 11: 多重反彈粒子 ---
void eff_multi_bounce(int wait) {
    static float pos1 = 0, pos2 = LED_NUM - 1;
    static float vel1 = 0.5, vel2 = -0.7;
    
    // 背景稍微暗一點點，產生拖尾感
    for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);

    // 粒子 1: 青色
    set_led_color((int)pos1, 0, 255, 255);
    // 粒子 2: 桃紅
    set_led_color((int)pos2, 255, 0, 150);

    ws2812_show();

    pos1 += vel1;
    pos2 += vel2;

    if(pos1 >= LED_NUM-1 || pos1 <= 0) vel1 *= -1;
    if(pos2 >= LED_NUM-1 || pos2 <= 0) vel2 *= -1;

    delay_ms(wait);
}

// --- 特效 12: 漸變掃描 (Gradient Scanner) ---
void eff_gradient_scanner(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, int wait) {
    static float offset = 0;
    for (int i = 0; i < LED_NUM; i++) {
        float ratio = (sin(i * 0.2f + offset) + 1.0f) / 2.0f;
        uint8_t r = r1 + (r2 - r1) * ratio;
        uint8_t g = g1 + (g2 - g1) * ratio;
        uint8_t b = b1 + (b2 - b1) * ratio;
        set_led_color(i, r, g, b);
    }
    ws2812_show();
    offset += 0.1f;
    delay_ms(wait);
}


// --- 特效 13: 隨機流星 (Random Meteor) ---
void eff_meteor(uint8_t r, uint8_t g, uint8_t b, uint8_t decay, bool random_decay, int wait) {
    // 讓所有燈光稍微暗淡 (模擬拖尾)
    for(int j=0; j<LED_NUM; j++) {
        // 讀取目前的顏色並手動衰減（這裡假設一個簡單的比例縮放）
        // 注意：因為我們沒有讀取 buffer 的函式，所以改用背景覆蓋法或清除法
        // 這裡示範一種直接生成流星移動的方法
    }

    static int pos = 0;
    // 清除背景
    for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);

    // 繪製流星頭部與尾巴
    for(int j=0; j<10; j++) {
        if(pos - j >= 0 && pos - j < LED_NUM) {
            uint8_t factor = 255 - (j * 25); // 越往後越暗
            set_led_color(pos - j, (r * factor) >> 8, (g * factor) >> 8, (b * factor) >> 8);
        }
    }
    
    ws2812_show();
    pos++;
	if (pos >= LED_NUM) pos = 0;
    if(pos >= LED_NUM + 10) pos = 0; // 跑出畫面後重置
    delay_ms(wait);
}

// --- 特效 14: 彩色彈珠 (Color Marbles) ---
// 模擬三顆不同顏色的彈珠在燈條上交錯移動
void eff_marbles(int wait) {
    static float offset = 0;
    for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0); // 清除背景

    for(int m=0; m<3; m++) {
        // 利用正弦波計算位置
        int pos = (int)((LED_NUM - 1) * (sin(offset + m * 2.0f) + 1.0f) / 2.0f);
        if(m == 0) set_led_color(pos, 255, 0, 0);   // 紅彈珠
        if(m == 1) set_led_color(pos, 0, 255, 0);   // 綠彈珠
        if(m == 2) set_led_color(pos, 0, 0, 255);   // 藍彈珠
    }

    ws2812_show();
    offset += 0.05f;
    delay_ms(wait);
}



// --- 特效: 理髮店燈柱 (Barber Pole) ---
// r, g, b: 條紋顏色
// width: 每個顏色區塊的寬度 (例如 4 顆燈亮，4 顆燈暗)
void eff_barber_pole(uint8_t r, uint8_t g, uint8_t b, int width, int wait) {
    static int offset = 0;
    
    for (int i = 0; i < LED_NUM; i++) {
        // 使用取模運算 (%) 來決定這顆燈屬於「亮區」還是「暗區」
        if (((i + offset) % (width * 2)) < width) {
            set_led_color(i, r, g, b);
        } else {
            set_led_color(i, 0, 0, 0); // 或者設定第二種顏色
        }
    }
    
    ws2812_show();
    offset++; // 增加 offset 產生移動感
    
    // 防止 offset 無限增加導致溢出
    if (offset >= (width * 2)) offset = 0;
    
    delay_ms(wait);
}


// --- 特效: 三色理髮店燈柱 (Barber Pole Multi) ---
void eff_barber_pole_multi(int width, int wait) {
    static int offset = 0;
    int section = width * 3; // 三個顏色區段：紅、白、藍

    for (int i = 0; i < LED_NUM; i++) {
        int pos = (i + offset) % section;
        if (pos < width) {
            set_led_color(i, 255, 0, 0);      // 第一段：紅
        } else if (pos < width * 2) {
            set_led_color(i, 255, 255, 255);  // 第二段：白
        } else {
            set_led_color(i, 0, 0, 255);      // 第三段：藍
        }
    }
    
    ws2812_show();
    offset++; // 讓條紋移動
    
    // 防止 offset 超出 section 範圍
    if (offset >= section) offset = 0;
    
    delay_ms(wait);
}



// --- [ID 33] 俄羅斯方塊堆疊特效 (Tetris Stack) ---
// r, g, b: 掉落方塊的顏色
// wait: 掉落的速度 (越小越快)
void eff_33_stack_tetris(uint8_t r, uint8_t g, uint8_t b, int wait) {
    // 使用靜態向量記錄哪些位置已經堆疊了方塊
    static std::vector<bool> stacked(LED_NUM, false);
    static int stacked_count = 0;

    // 如果全部填滿了，閃爍三次後重置
    if (stacked_count >= LED_NUM) {
        for (int times = 0; times < 3; times++) {
            for (int i = 0; i < LED_NUM; i++) set_led_color(i, r, g, b);
            ws2812_show(); delay_ms(200);
            for (int i = 0; i < LED_NUM; i++) set_led_color(i, 0, 0, 0);
            ws2812_show(); delay_ms(200);
        }
        std::fill(stacked.begin(), stacked.end(), false);
        stacked_count = 0;
        return;
    }

    // 開始一次新的「掉落」過程
    int current_pos = 0;
    while (current_pos < LED_NUM) {
        // 檢查下一格是否已經被佔據或是否到底
        if (current_pos + 1 >= LED_NUM || stacked[current_pos + 1]) {
            // 碰撞發生！固定在當前位置
            stacked[current_pos] = true;
            stacked_count++;
            
            // 為了讓視覺更紮實，固定時可以閃一下
            set_led_color(current_pos, r, g, b);
            ws2812_show();
            break;
        }

        // 繪製當前畫面：已堆疊的方塊 + 正在移動的點
        for (int i = 0; i < LED_NUM; i++) {
            if (stacked[i]) {
                set_led_color(i, r, g, b); // 已堆疊方塊
            } else if (i == current_pos) {
                set_led_color(i, r, g, b); // 正在掉落的點
            } else {
                set_led_color(i, 0, 0, 0); // 空位
            }
        }
        
        ws2812_show();
        delay_ms(wait);
        current_pos++;
    }
}


// --- [ID 33] 俄羅斯方塊堆疊 (隨機長度 1-3) ---
void eff_33_stack_tetris_random(uint8_t r, uint8_t g, uint8_t b, int wait) {
    static std::vector<bool> stacked(LED_NUM, false);
    static int total_stacked = 0;

    // 1. 檢查是否已填滿 (或是剩下的空間不足 1 顆)
    if (total_stacked >= LED_NUM) {
        // 滿了就閃爍三次重置
        for (int times = 0; times < 3; times++) {
            for (int i = 0; i < LED_NUM; i++) set_led_color(i, r, g, b);
            ws2812_show(); delay_ms(150);
            for (int i = 0; i < LED_NUM; i++) set_led_color(i, 0, 0, 0);
            ws2812_show(); delay_ms(150);
        }
        std::fill(stacked.begin(), stacked.end(), false);
        total_stacked = 0;
        return;
    }

    // 2. 隨機決定本次掉落的方塊長度 (1~3)
    int block_len = (rand() % 3) + 1;
    
    // 3. 開始掉落動畫
    int head_pos = 0; // 方塊頭部的位置
    while (head_pos < LED_NUM) {
        // 碰撞偵測：
        // 如果頭部的下一格超出範圍，或者下一格已經有方塊
        bool collision = false;
        if (head_pos + 1 >= LED_NUM || stacked[head_pos + 1]) {
            collision = true;
        }

        if (collision) {
            // 固定這組方塊 (由 head_pos 往回推 block_len)
            for (int k = 0; k < block_len; k++) {
                int target = head_pos - k;
                if (target >= 0 && !stacked[target]) {
                    stacked[target] = true;
                    total_stacked++;
                }
            }
            break; // 結束這顆方塊的掉落
        }

        // 4. 繪製當前幀
        // 先繪製已經堆好的部分
        for (int i = 0; i < LED_NUM; i++) {
            if (stacked[i]) set_led_color(i, r, g, b);
            else set_led_color(i, 0, 0, 0);
        }
        
        // 再繪製正在掉落的這組方塊 (長度 1~3)
        for (int k = 0; k < block_len; k++) {
            int draw_pos = head_pos - k;
            if (draw_pos >= 0 && draw_pos < LED_NUM) {
                set_led_color(draw_pos, r, g, b);
            }
        }

        ws2812_show();
        delay_ms(wait);
        head_pos++;
    }
}


// [ID 33] 俄羅斯方塊堆疊 - 強化變色版
// 回傳值：true 表示剛剛完成了一次慶典並重置
bool eff_33_stack_tetris_enhanced(uint8_t r, uint8_t g, uint8_t b, int wait) {
    static std::vector<bool> stacked(LED_NUM, false);
    static int total_stacked = 0;

    // --- 慶典動畫：當堆疊滿了進入這裡 ---
    if (total_stacked >= LED_NUM) {
        std::cout << ">>> 恭喜！滿分！啟動彩虹慶典！ <<<" << std::endl;
        for (int j = 0; j < 256 * 3; j++) { // 跑三輪彩虹，更久一點
            for (int i = 0; i < LED_NUM; i++) {
                uint32_t color = Wheel(((i * 256 / LED_NUM) + j) & 255);
                set_led_color(i, (color >> 8) & 0xFF, (color >> 16) & 0xFF, color & 0xFF);
            }
            ws2812_show();
            delay_ms(5); // 稍微加快一點更華麗
        }
        // 重置所有狀態
        std::fill(stacked.begin(), stacked.end(), false);
        total_stacked = 0;
        return true; 
    }

    // 隨機決定方塊長度
    int block_len = (rand() % 3) + 1;
    int head_pos = 0;

    while (head_pos < LED_NUM) {
        bool collision = false;
        if (head_pos + 1 >= LED_NUM || stacked[head_pos + 1]) {
            collision = true;
        }

        if (collision) {
            for (int k = 0; k < block_len; k++) {
                int target = head_pos - k;
                if (target >= 0 && !stacked[target]) {
                    stacked[target] = true;
                    total_stacked++;
                }
            }
            break;
        }

        // 渲染畫面
        for (int i = 0; i < LED_NUM; i++) {
            if (stacked[i]) set_led_color(i, r, g, b);
            else set_led_color(i, 0, 0, 0);
        }
        for (int k = 0; k < block_len; k++) {
            int draw_pos = head_pos - k;
            if (draw_pos >= 0 && draw_pos < LED_NUM) set_led_color(draw_pos, r, g, b);
        }
        ws2812_show();
        delay_ms(wait);
        head_pos++;
    }
    return false;
}


// --- [ID 31] 電漿流動 - 淡紫色背景版 ---
void eff_31_plasma_flow_tick(float t) {
    // 定義淡紫色背景 (R:40, G:0, B:80 左右會有不錯的低溫紫色感)
    const uint8_t bg_r = 30;
    const uint8_t bg_g = 5;
    const uint8_t bg_b = 60;

    for (int i = 0; i < LED_NUM; i++) {
        // 多重波形計算
        float v1 = sin((float)i * 0.15f + t);
        float v2 = sin(0.1f * ((float)i * sin(t/2.0f) + t * 1.5f));
        float res = (v1 + v2); 

        // 動態顏色計算
        uint8_t r = (uint8_t)((sin(res * M_PI) * 0.5f + 0.5f) * 150);
        uint8_t g = (uint8_t)((cos(res * M_PI) * 0.5f + 0.5f) * 50);
        uint8_t b = (uint8_t)((sin(res * M_PI + 1.5f) * 0.5f + 0.5f) * 180);
        
        // 疊加背景顏色 (使用 std::max 確保顏色飽和且不熄滅)
        uint8_t final_r = std::max(bg_r, r);
        uint8_t final_g = std::max(bg_g, g);
        uint8_t final_b = std::max(bg_b, b);

        set_led_color(i, final_r, final_g, final_b);
    }
    ws2812_show();
}

// --- [ID 30] 能量脈衝碰撞 ---
void eff_30_pulse_collision_tick(uint8_t r1, uint8_t g1, uint8_t b1, uint8_t r2, uint8_t g2, uint8_t b2, float speed, float t) {
    for(int i=0; i<LED_NUM; i++) set_led_color(i, 0, 0, 0);
    float pos1 = fmod(t * speed * LED_NUM, (float)LED_NUM);
    float pos2 = (float)LED_NUM - fmod(t * speed * LED_NUM, (float)LED_NUM);
    for (int i = 0; i < LED_NUM; i++) {
        float dist1 = fabsf((float)i - pos1);
        float dist2 = fabsf((float)i - pos2);
        if (dist1 < 5.0f) {
            float w = 1.0f - (dist1 / 5.0f);
            set_led_color(i, std::max(1, (int)(r1*w)), std::max(1, (int)(g1*w)), std::max(1, (int)(b1*w)));
        }
        if (dist2 < 5.0f) {
            float w = 1.0f - (dist2 / 5.0f);
            set_led_color(i, std::max(1, (int)(r2*w)), std::max(1, (int)(g2*w)), std::max(1, (int)(b2*w)));
        }
    }
    ws2812_show();
}


 
// --- [ID 999] 系統警告：閃爍三次紅色 ---
void eff_999_system_error_alert() {
    std::cout << ">>> [視覺警告] 偵測到無效 ID，閃爍紅色警示..." << std::endl;
    
    for (int count = 0; count < 3; count++) {
        // 全亮紅燈
        for (int i = 0; i < LED_NUM; i++) {
            set_led_color(i, 255, 0, 0); // 純紅，不帶綠跟藍
        }
        ws2812_show();
        delay_ms(200); // 亮 0.2 秒

        // 全滅
        for (int i = 0; i < LED_NUM; i++) {
            set_led_color(i, 0, 0, 0);
        }
        ws2812_show();
        delay_ms(200); // 暗 0.2 秒
    }
}



void eff_42_cyber_scanner(uint8_t r, uint8_t g, uint8_t b, int wait) {
    static int pos = 0;
    static int dir = 1;

    for (int i = 0; i < LED_NUM; i++) {
        int dist = abs(i - pos);
        if (dist == 0) set_led_color(i, r, g, b); // 主燈
        else if (dist < 5) set_led_color(i, r/(dist*2), g/(dist*2), b/(dist*2)); // 尾跡
        else set_led_color(i, 0, 0, 0);
    }
    
    ws2812_show();
    pos += dir;
    if (pos >= LED_NUM - 1 || pos <= 0) dir *= -1;
    delay_ms(wait);
}



void eff_43_forest_mist(float t) {
    float pulse = (sin(t * 1.5f) + 1.0f) / 2.0f; // 0.0 ~ 1.0
    for (int i = 0; i < LED_NUM; i++) {
        // 偶數燈珠偏翠綠，奇數燈珠偏青色
        if (i % 2 == 0) 
            set_led_color(i, 0, (uint8_t)(150 * pulse), (uint8_t)(20 * pulse));
        else 
            set_led_color(i, 0, (uint8_t)(100 * pulse), (uint8_t)(80 * pulse));
    }
    ws2812_show();
}

void eff_44_star_burst(int density) {
    // 每一幀都有機率產生新星星
    if (rand() % 100 < density) {
        int target = rand() % LED_NUM;
        set_led_color(target, 255, 255, 200); // 暖白光
    }
    
    // 讓所有燈珠緩慢暗下去 (Fading)
    for (int i = 0; i < LED_NUM; i++) {
        
		PrideRGB color = get_led_color(i); 
		set_led_color(i, color.r * 0.92f, color.g * 0.92f, color.b * 0.92f);  
    }
    ws2812_show();
    delay_ms(30);
}



static const PrideRGB pride_colors[] = {
    {255, 0, 0},    // 紅
    {255, 127, 0},  // 橙
    {255, 255, 0},  // 黃
    {0, 255, 0},    // 綠
    {0, 0, 255},    // 藍
    {75, 0, 130},   // 靛
    {148, 0, 211}   // 紫
};


void eff_45_pride_2015(float t) {
    // 這裡模擬 FastLED 的多層疊加邏輯
    // 使用不同的頻率 (0.7, 1.2, 1.7 等) 讓波形產生交錯感
    
    for (int i = 0; i < LED_NUM; i++) {
        // 計算該燈珠的基礎色調偏移
        float hue_offset = (float)i / LED_NUM;
        
        // 疊加三個正弦波來決定最終的顏色位置
        float wave1 = sin(t * 0.6f + hue_offset * 5.0f);
        float wave2 = sin(t * 1.1f + hue_offset * 3.0f);
        float wave3 = sin(t * 1.5f + hue_offset * 2.0f);
        
        float final_hue = (wave1 + wave2 + wave3) / 3.0f;
        
        // 將 -1.0 ~ 1.0 的 hue 映射到 0 ~ 6 (pride_colors 的索引)
        float color_idx_raw = (final_hue + 1.0f) * 3.5f; 
        int idx1 = (int)color_idx_raw % 7;
        int idx2 = (idx1 + 1) % 7;
        float fract = color_idx_raw - (int)color_idx_raw;

        // 線性插值 (Linear Interpolation) 讓顏色過渡更滑順
        uint8_t r = pride_colors[idx1].r + fract * (pride_colors[idx2].r - pride_colors[idx1].r);
        uint8_t g = pride_colors[idx1].g + fract * (pride_colors[idx2].g - pride_colors[idx1].g);
        uint8_t b = pride_colors[idx1].b + fract * (pride_colors[idx2].b - pride_colors[idx1].b);

        set_led_color(i, r, g, b);
    }
    ws2812_show();
}
 
// ws2812_effects.cpp

void eff_46_sinelon(float t, uint8_t r, uint8_t g, uint8_t b) {
    // --- 注意：這裡不要呼叫 get_local_t ---
    
    // 1. 衰減邏輯 (Fade)
    for (int i = 0; i < LED_NUM; i++) {
        PrideRGB col = get_led_color(i);
        if (col.r == 0 && col.g == 0 && col.b == 0) continue;
        uint8_t nr = (uint8_t)(col.r * 0.96f);
        uint8_t ng = (uint8_t)(col.g * 0.96f);
        uint8_t nb = (uint8_t)(col.b * 0.96f);
        if (nr == col.r && col.r > 0) nr--; // 確保能歸零
        if (ng == col.g && col.g > 0) ng--;
        if (nb == col.b && col.b > 0) nb--;
        set_led_color(i, nr, ng, nb);
    }

    // 2. 位置計算 (使用傳進來的 t)
    float pos_f = (-cosf(t * 0.7f) + 1.0f) / 2.0f * (LED_NUM - 1);
    int pos_i = static_cast<int>(pos_f);
    float frac = pos_f - pos_i;

    // 3. 亞像素渲染
    auto add_color = [&](int idx, float intensity) {
        if (idx < 0 || idx >= LED_NUM) return;
        PrideRGB cur = get_led_color(idx);
        set_led_color(idx, 
            std::min(255, (int)(cur.r + r * intensity)),
            std::min(255, (int)(cur.g + g * intensity)),
            std::min(255, (int)(cur.b + b * intensity))
        );
    };
    add_color(pos_i, 1.0f - frac);
    add_color(pos_i + 1, frac);

    ws2812_show();
}
void eff_46_sinelonxxx(float t, uint8_t r, uint8_t g, uint8_t b) {
    // 1. 取得局部時間 (Local Time)
    // 傳入 46 作為 ID，確保切換到此特效時時間重置為 0
    float local_t = get_local_t(t, 46);

    // 2. 衰減邏輯 (保持原本的 smooth_fade 或 0.96f)
    for (int i = 0; i < LED_NUM; i++) {
        PrideRGB col = get_led_color(i);
        if (col.r == 0 && col.g == 0 && col.b == 0) continue;
        
        uint8_t nr = (uint8_t)(col.r * 0.96f);
        uint8_t ng = (uint8_t)(col.g * 0.96f);
        uint8_t nb = (uint8_t)(col.b * 0.96f);
        
        // 修正：避免整數捨入導致無法歸零
        if (nr == col.r && col.r > 0) nr--;
        if (ng == col.g && col.g > 0) ng--;
        if (nb == col.b && col.b > 0) nb--;

        set_led_color(i, nr, ng, nb);
    }

    // 3. 位置計算：使用 local_t 並搭配 -cosf
    // -cosf(0) = -1，所以啟動時 (local_t=0) 位置剛好是 0
    float speed = 0.7f; // 你可以調整這個速度
    float pos_f = (-cosf(local_t * speed) + 1.0f) / 2.0f * (LED_NUM - 1);
    
    int pos_i = static_cast<int>(pos_f);
    float frac = pos_f - pos_i;

    // 4. 亞像素渲染 (Anti-aliasing)
    auto add_color = [&](int idx, float intensity) {
    if (idx < 0 || idx >= LED_NUM) return;
    
    // 不要使用 std::min(255, cur.r + r * intensity)
    // 直接計算目標亮度，並「覆蓋」掉舊的值
    uint8_t target_r = static_cast<uint8_t>(r * intensity);
    uint8_t target_g = static_cast<uint8_t>(g * intensity);
    uint8_t target_b = static_cast<uint8_t>(b * intensity);

    // 只有當新算的顏色比舊的顏色「亮」時才寫入（類似亮度最大值濾鏡）
    // 這樣可以保證拖尾平滑，但不會產生錯誤的混色
    PrideRGB cur = get_led_color(idx);
    set_led_color(idx, 
        std::max(cur.r, target_r),
        std::max(cur.g, target_g),
        std::max(cur.b, target_b)
    );
   };

    add_color(pos_i, 1.0f - frac);
    add_color(pos_i + 1, frac);

    ws2812_show();
}




// ws2812_effects.cpp
void eff_47_sinelon_rainbow(float t) {
    float lt = get_local_t(t, 47); // 這裡管理 47 的時間
    //uint8_t hue = static_cast<uint8_t>(lt * 20.0f);
	//uint8_t hue = static_cast<uint8_t>(lt * 100.0f);
	// 加上 42，讓流星「一出生」就是黃色，避開 Hue=0 的紅色區域
    uint8_t hue = static_cast<uint8_t>(lt * 20.0f) + 42;
	
    PrideRGB c = hsv2rgb(hue, 255, 200); 
	eff_46_sinelon(lt, c.r, c.g, c.b); // 傳入 lt 給純渲染器
}


void eff_48_aurora_sinelon(float t) {
    // 1. 絲滑衰減 (保持我們之前優化的 smooth_fade 邏輯)
    float fade_speed = 0.96f; 
	// 2. 位置計算 (使用 local_t 配合 -cosf)
    // 這樣保證 t=0 (也就是啟動瞬間) 時，pos_f 絕對是 0
    float local_t = get_local_t(t, 48);
	
    for (int i = 0; i < LED_NUM; i++) {
        PrideRGB col = get_led_color(i);
        if (col.r == 0 && col.g == 0 && col.b == 0) continue;
        
        // 使用我們之前的平滑遞減技巧
        auto f = [&](uint8_t v) { 
            uint8_t n = static_cast<uint8_t>(v * fade_speed);
            return (n == v && v > 0) ? v - 1 : n;
        };
        set_led_color(i, f(col.r), f(col.g), f(col.b));
    }

    // 2. 計算位置 (慢速優雅擺動)
    // 1. 使用 -cosf 確保 t=0 時，位置剛好是 0 (最左邊)
	// 2. 0.5f 是速度係數，你可以根據喜好調整
	float pos_f = (-cosf(t * 0.5f) + 1.0f) / 2.0f * (LED_NUM - 1);
	//float pos_f = (sinf(t * 0.5f) + 1.0f) / 2.0f * (LED_NUM - 1);
	
	
    int pos_i = static_cast<int>(pos_f);
    float frac = pos_f - pos_i;

    // 3. 極光色彩計算 (根據時間 t 產生變幻的色相)
    // 我們讓 Hue 隨時間轉動
	// 3. 顏色注入 (使用 local_t)
    uint8_t center_hue = static_cast<uint8_t>(local_t * 30.0f);
    
    // 4. 亞像素渲染 + 彩色注入
    auto inject_aurora = [&](int idx, float intensity) {
        if (idx < 0 || idx >= LED_NUM) return;
        
        // 這裡的秘密：亮點的顏色是動態生成的 HSV
        PrideRGB aurora = hsv2rgb(center_hue, 255, 200);
        PrideRGB current = get_led_color(idx);

        // 飽和加法：讓新顏色疊加在舊的拖尾上
        set_led_color(idx, 
            std::min(255, (int)(current.r + aurora.r * intensity)),
            std::min(255, (int)(current.g + aurora.g * intensity)),
            std::min(255, (int)(current.b + aurora.b * intensity))
        );
    };

    inject_aurora(pos_i, 1.0f - frac);
    inject_aurora(pos_i + 1, frac);

    ws2812_show();
}


void eff_49_dual_sinelon(float t) {
    float lt = get_local_t(t, 49);

    // 1. 統一衰減 (每幀只做一次)
    for (int i = 0; i < LED_NUM; i++) {
        PrideRGB col = get_led_color(i);
        set_led_color(i, (uint8_t)(col.r * 0.95f), (uint8_t)(col.g * 0.95f), (uint8_t)(col.b * 0.95f));
    }

    // 2. 計算第一顆的「精確浮點位置」
    float speed = 0.7f;
    float pos1 = (-cosf(lt * speed) + 1.0f) / 2.0f * (LED_NUM - 1);

    // 3. 關鍵：用「鏡像」算出第二顆的位置，而不是用相位偏移
    // 這樣兩者的亞像素位置 (frac) 會完全互補，長度絕對對稱
    float pos2 = (float)(LED_NUM - 1) - pos1;

    // 4. 定義一個內部的渲染 Lambda (不含 cos 計算)
    auto draw_at = [&](float pos_f, uint8_t r, uint8_t g, uint8_t b) {
        int pos_i = static_cast<int>(pos_f);
        float frac = pos_f - pos_i;

        auto add_color = [&](int idx, float intensity) {
            if (idx < 0 || idx >= LED_NUM) return;
            uint8_t tr = static_cast<uint8_t>(r * intensity);
            uint8_t tg = static_cast<uint8_t>(g * intensity);
            uint8_t tb = static_cast<uint8_t>(b * intensity);
            PrideRGB cur = get_led_color(idx);
            set_led_color(idx, std::max(cur.r, tr), std::max(cur.g, tg), std::max(cur.b, tb));
        };
        add_color(pos_i, 1.0f - frac);
        add_color(pos_i + 1, frac);
    };

    // 5. 執行渲染
    draw_at(pos1, 0, 255, 255);   // 第一顆：青色
    draw_at(pos2, 255, 0, 150);   // 第二顆：桃紅 (完美鏡像)

    ws2812_show();
}

void eff_50_dual_sinelon_sparkle(float t) {
    float lt = get_local_t(t, 50);
    static float sparkle_intensity = 0.0f; 

    // 1. 基礎衰減 (調低一點點，讓背景更乾淨，突顯煙火)
    for (int i = 0; i < LED_NUM; i++) {
        PrideRGB col = get_led_color(i);
        set_led_color(i, (uint8_t)(col.r * 0.88f), (uint8_t)(col.g * 0.88f), (uint8_t)(col.b * 0.88f));
    }

    // 2. 統一位置 (Speed 0.7f)
    float speed = 0.7f; 
    float pos1 = (-cosf(lt * speed) + 1.0f) / 2.0f * (LED_NUM - 1);
    float pos2 = (float)(LED_NUM - 1) - pos1;

    // 3. 碰撞觸發
    float dist = fabsf(pos1 - pos2);
    if (dist < 1.2f) {
        sparkle_intensity = 1.0f; // 撞擊瞬間充能
    }

    // 4. 渲染流星主體
    auto draw_at = [&](float pos_f, uint8_t r, uint8_t g, uint8_t b, float bst) {
        int pos_i = static_cast<int>(pos_f);
        float frac = pos_f - pos_i;
        auto add_color = [&](int idx, float intensity) {
            if (idx < 0 || idx >= LED_NUM) return;
            uint8_t tr = (uint8_t)std::min(255.0f, r * intensity * bst);
            uint8_t tg = (uint8_t)std::min(255.0f, g * intensity * bst);
            uint8_t tb = (uint8_t)std::min(255.0f, b * intensity * bst);
            PrideRGB cur = get_led_color(idx);
            set_led_color(idx, std::max(cur.r, tr), std::max(cur.g, tg), std::max(cur.b, tb));
        };
        add_color(pos_i, 1.0f - frac);
        add_color(pos_i + 1, frac);
    };

    draw_at(pos1, 0, 255, 255, 1.0f + sparkle_intensity); 
    draw_at(pos2, 255, 0, 150, 1.0f + sparkle_intensity); 

    // 5. 【核心改善】閃爍煙火邏輯 (Strobe Sparkles)
    if (sparkle_intensity > 0.0f) {
        int mid = (int)((pos1 + pos2) / 2.0f);
        
        // 增加煙火範圍
        for (int i = -4; i <= 4; i++) {
            // 隨機判定：只有部分燈珠會閃爍
            // (rand() % 100) < (sparkle_intensity * 80) 讓煙火隨強度減弱而變稀疏
            if ((rand() % 100) < (sparkle_intensity * 70)) { 
                int spark_idx = mid + i;
                if (spark_idx >= 0 && spark_idx < LED_NUM) {
                    
                    // 隨機閃爍控制：利用 rand() 讓每一幀的亮度都在跳動
                    // 這會產生「交錯閃爍」的視覺效果
                    if ((rand() % 10) > 3) { 
                        // 設定為最大亮度純白
                        set_led_color(spark_idx, 255, 255, 255); 
                    } else {
                        // 模擬熄滅瞬間的火星 (微弱紅/橘色)
                        set_led_color(spark_idx, 40, 20, 0); 
                    }
                }
            }
        }
        
        // 減慢衰減速度，讓煙火留久一點 (0.015f 約可持續 1-2 秒)
        sparkle_intensity -= 0.015f; 
        if (sparkle_intensity < 0) sparkle_intensity = 0;
    }

    ws2812_show();
} 
 
 
 


/**
 * @brief [ID 51] 靜態彩虹步進 (最終優化版)
 * @param t 全域時間 (秒)
 * @param display_time 每個顏色顯示時間，預設為 2.0 秒
 */
void eff_51_static_rainbow_step(float t, float display_time) {
    // 經過多次調校後的黃金比例色表
    static const std::vector<PrideRGB> rainbow_colors = {
        {255, 0, 0},    // 0: 紅 (最亮)
        {255, 30, 0},   // 1: 橙 (已調降綠色，區隔黃色)
        {255, 180, 0},  // 2: 黃 (已調降綠色，避免偏白)
        {0, 255, 0},    // 3: 綠
        {0, 0, 255},    // 4: 藍 (純藍)
        
        // --- 結尾深色區：交換位置並壓低亮度 ---
        {4, 0, 40},     // 5: Dark Purple (神祕感，藍味重)
        {15, 0, 30}     // 6: Indigo (深邃結尾，紅藍比 1:2)
    };

    if (display_time <= 0) display_time = 2.0f;

    // 根據時間計算索引
    int color_index = (int)(t / display_time) % rainbow_colors.size();
    PrideRGB c = rainbow_colors[color_index];

    // 執行全燈渲染
    for (int i = 0; i < LED_NUM; i++) {
        set_led_color(i, c.r, c.g, c.b);
    }
    
    ws2812_show();
}


void eff_51_static_rainbow_step_OK(float t, float display_time) {
    // 重新排列並校正後的顏色清單
    static const std::vector<PrideRGB> rainbow_colors = {
        {255, 0, 0},      // 0: 紅
        {255, 30, 0},     // 1: 橙
        {255, 180, 0},    // 2: 黃
        {0, 255, 0},      // 3: 綠
        {0, 0, 255},      // 4: 藍
        
        // --- 交換位置與細調 ---
        {4, 0, 40},      // 5: Dark Purple (先出現，帶一點神祕感)
        {15, 0, 30}       // 6: Indigo (最後出現，最深最暗的結尾)
    };

    if (display_time <= 0) display_time = 2.0f;

    // 計算索引
    int color_index = (int)(t / display_time) % rainbow_colors.size();
    PrideRGB c = rainbow_colors[color_index];

    // 全燈條顯示
    for (int i = 0; i < LED_NUM; i++) {
        set_led_color(i, c.r, c.g, c.b);
    }
    ws2812_show();
}

/**
 * @brief [ID 52] 超高速閃頻 - 固定色呼吸
 * @param t 全域時間
 * @param current_hue 指定的色相 (0-360)
 */
void eff_52_hyper_flash_fixed(float t, float current_hue) {
	 
    // 1. 亮度與閃頻邏輯 (保持原本的高速呼吸感，直接使用傳入的 t)
    float breathe_freq = 4.5f; 
    // 這個公式會產生極短暫的高亮與長週期的黑暗，形成 Hyper Flash 效果
    float brightness = pow(exp(sin(t * breathe_freq * M_PI * 2)) / M_E, 10.0f); 
    
    // 2. 顏色轉換 (HSV to RGB)
    float r_f, g_f, b_f;
    float h_prime = fmodf(current_hue, 360.0f) / 60.0f;
    float x = (1.0f - fabsf(fmodf(h_prime, 2.0f) - 1.0f));
    
    if (h_prime < 1)      { r_f = 1; g_f = x; b_f = 0; }
    else if (h_prime < 2) { r_f = x; g_f = 1; b_f = 0; }
    else if (h_prime < 3) { r_f = 0; g_f = 1; b_f = x; }
    else if (h_prime < 4) { r_f = 0; g_f = x; b_f = 1; }
    else if (h_prime < 5) { r_f = x; g_f = 0; b_f = 1; }
    else                  { r_f = 1; g_f = 0; b_f = x; }

    // 3. 渲染至燈條 (加入中心擴散感)
    int center = LED_NUM / 2;
    for (int j = 0; j < LED_NUM; j++) {
        // 計算離中心的距離係數 (0.0 ~ 1.0)
        float dist_factor = 1.0f - (abs(j - center) / (float)center);
        float spatial_fade = pow(dist_factor, 1.2f); // 稍微讓中心更集中
        
        uint8_t r_out = (uint8_t)(r_f * brightness * 255 * spatial_fade);
        uint8_t g_out = (uint8_t)(g_f * brightness * 255 * spatial_fade);
        uint8_t b_out = (uint8_t)(b_f * brightness * 255 * spatial_fade);
        
        set_led_color(j, r_out, g_out, b_out);
    }
    ws2812_show();
}