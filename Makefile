# 定義編譯器
CXX = g++
CC = gcc

# 編譯選項：啟用最佳化 (-O3) 與所有警告 (-Wall)
CXXFLAGS = -O3 -Wall -std=c++11
LIBS = 

# 目標執行檔名稱
TARGET = argb_demo

# 原始碼檔案 (會自動搜尋目前的 .cpp 與 .c 檔案)
SRCS = $(wildcard *.cpp) $(wildcard *.c)
OBJS = $(SRCS:.cpp=.o)
OBJS := $(OBJS:.c=.o)

# 預設編譯規則
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

# 清除編譯產物
clean:
	rm -f $(OBJS) $(TARGET)

# 執行程式 (需要 sudo 權限存取 SPI)
run: $(TARGET)
	sudo ./$(TARGET)