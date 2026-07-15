#include "joystick_task.h"          // Header tùy chỉnh cho định nghĩa tác vụ joystick
#include "main.h"                  // Header chính cho thư viện HAL của STM32
#include "cmsis_os.h"              // API CMSIS-RTOS cho FreeRTOS
#include "stm32f4xx_hal.h"         // Thư viện HAL cho STM32F4
#include "stm32f4xx_hal_adc.h"     // Mô-đun HAL ADC
#include "gui/model/Model.hpp"      // Header C++ cho mô hình GUI
#include <cstring>
#include <cstdio>

extern ADC_HandleTypeDef hadc1;    // Handle ADC cho ADC1
extern UART_HandleTypeDef huart1;  // Handle UART cho UART1
extern osMessageQueueId_t joystickQueueHandle; // Handle hàng đợi FreeRTOS

// Định nghĩa kênh ADC
#define JOY1_X_CHANNEL ADC_CHANNEL_13 // Kênh ADC cho trục X của Joystick 1 (PC3)
#define JOY1_Y_CHANNEL ADC_CHANNEL_11 // Kênh ADC cho trục Y của Joystick 1 (PC1 - đổi từ PA0 để nhường chỗ cho nút USER)
#define JOY2_X_CHANNEL ADC_CHANNEL_7 // Kênh ADC cho trục X của Joystick 2 (PA7)
#define JOY2_Y_CHANNEL ADC_CHANNEL_5  // Kênh ADC cho trục Y của Joystick 2 (PA5)

#define USER_BUTTON_PIN  GPIO_PIN_0   // Nút USER on-board (thay cho 2 nút SW không có trên joystick)
#define USER_BUTTON_PORT GPIOA

#define JOY_THRESHOLD_LEFT   1000     // Ngưỡng cho chuyển động trái
#define JOY_THRESHOLD_RIGHT  3000     // Ngưỡng cho chuyển động phải
#define JOY_REPEAT_MS        40U      // Giới hạn tốc độ gửi lệnh để tránh đầy queue


//uint32_t tick_counter = 0;            // Biến đếm thời gian

volatile uint32_t g_debug_adc[4]= {0,0,0,0};
volatile int g_debug_btn = 0;
extern "C" void JoystickTask(void *argument)
{

    // Khởi tạo giá trị ADC ở giữa dải (2048 cho ADC 12-bit)
    uint32_t adc_values[4] = {2048, 2048, 2048, 2048}; // mảng lưu giá trị ADC: [JOY1_X, JOY1_Y, JOY2_X, JOY2_Y]


    ADC_ChannelConfTypeDef sConfig = {0};//đặt các trường về 0
    uint8_t channel_rank = 1;

    // Cấu hình tất cả 4 kênh cho ADC1 ở chế độ Scan
    uint32_t channels[4] = {JOY1_X_CHANNEL, JOY1_Y_CHANNEL, JOY2_X_CHANNEL, JOY2_Y_CHANNEL};
    for (int i = 0; i < 4; i++) {
        sConfig.Channel = channels[i];
        sConfig.Rank = channel_rank++;
        sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;//quét tuần tự 4 kênh, mỗi kênh lấy mẫu trong 144 chu kỳ
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    }

    // Cấu hình ADC1 ở chế độ Scan
    hadc1.Instance->CR1 |= ADC_CR1_SCAN; // Bật chế độ Scan
    hadc1.Instance->SQR1 &= ~(ADC_SQR1_L); // Xóa số lượng kênh
    hadc1.Instance->SQR1 |= (3 << 20); // Thiết lập 4 kênh (L = 3, vì đếm từ 0)

    // Biến lệnh joystick
    JoystickCommand_t command;

    uint8_t lastAxisState[4] = {0, 0, 0, 0};
    uint32_t lastAxisTick[4] = {0, 0, 0, 0};

    // Theo dõi trạng thái nút USER (board có pull-down cứng, nhả = RESET, nhấn = SET)
    GPIO_PinState lastButtonState = GPIO_PIN_RESET;

    auto queueAxisCommand = [&](uint8_t axisIndex, uint8_t state, JoystickCommand_t negativeCommand, JoystickCommand_t positiveCommand) {
        uint32_t now = osKernelGetTickCount();

        if (state == 0) {
            lastAxisState[axisIndex] = 0;
            return;
        }

        if (lastAxisState[axisIndex] != state || (now - lastAxisTick[axisIndex]) >= JOY_REPEAT_MS) {
            command = (state == 1) ? negativeCommand : positiveCommand;
            if (osMessageQueuePut(joystickQueueHandle, &command, 0, 0) == osOK) {
                lastAxisState[axisIndex] = state;
                lastAxisTick[axisIndex] = now;
            }
        }
    };

  for (;;) {
        // Bắt đầu quét tất cả các kênh
        HAL_ADC_Start(&hadc1);

        // Đọc lần lượt giá trị từ 4 kênh, có timeout để tránh treo vĩnh viễn
        HAL_StatusTypeDef adc_ok = HAL_OK;
        for (int i = 0; i < 4; i++) {
            if (HAL_ADC_PollForConversion(&hadc1, 50) != HAL_OK) {
                adc_ok = HAL_ERROR;
                break;
            }
            adc_values[i] = HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);

        // Nếu ADC bị lỗi/timeout, xóa cờ lỗi và bỏ qua vòng này
        if (adc_ok != HAL_OK) {
            __HAL_ADC_CLEAR_FLAG(&hadc1, ADC_FLAG_OVR);
            osDelay(10);
            continue;
        }

        uint8_t joy1XAxisState = (adc_values[1] < JOY_THRESHOLD_LEFT) ? 1 : ((adc_values[1] > JOY_THRESHOLD_RIGHT) ? 2 : 0);
        uint8_t joy1YAxisState = (adc_values[0] < JOY_THRESHOLD_LEFT) ? 1 : ((adc_values[0] > JOY_THRESHOLD_RIGHT) ? 2 : 0);
        uint8_t joy2XAxisState = (adc_values[3] < JOY_THRESHOLD_LEFT) ? 1 : ((adc_values[3] > JOY_THRESHOLD_RIGHT) ? 2 : 0);
        uint8_t joy2YAxisState = (adc_values[2] < JOY_THRESHOLD_LEFT) ? 1 : ((adc_values[2] > JOY_THRESHOLD_RIGHT) ? 2 : 0);

        queueAxisCommand(0, joy1XAxisState, JOY1_LEFT, JOY1_RIGHT);
        queueAxisCommand(1, joy1YAxisState, JOY1_UP, JOY1_DOWN);
        queueAxisCommand(2, joy2XAxisState, JOY2_LEFT, JOY2_RIGHT);
        queueAxisCommand(3, joy2YAxisState, JOY2_UP, JOY2_DOWN);
        // Nút USER dùng chung cho cả 2 người chơi: gửi cả 2 lệnh,
        // màn hình nào đang ở trạng thái chờ serve (waitingForServe && servingPlayer đúng)
        // sẽ tự nhận đúng lệnh của mình, lệnh còn lại bị bỏ qua vô hại.
        GPIO_PinState buttonState = HAL_GPIO_ReadPin(USER_BUTTON_PORT, USER_BUTTON_PIN);
        g_debug_adc[0] = adc_values[0];
        g_debug_adc[1] = adc_values[1];
        g_debug_adc[2] = adc_values[2];
		g_debug_adc[3] = adc_values[3];
		g_debug_btn = (int)buttonState;
        if (buttonState == GPIO_PIN_SET && lastButtonState == GPIO_PIN_RESET) {
            command = JOY1_BUTTON;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
            command = JOY2_BUTTON;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }
        lastButtonState = buttonState;

        osDelay(10); // Tạm dừng 10ms
    }
}
