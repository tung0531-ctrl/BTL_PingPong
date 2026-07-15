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

    // Theo dõi trạng thái nút USER (board có pull-down cứng, nhả = RESET, nhấn = SET)
    GPIO_PinState lastButtonState = GPIO_PIN_RESET;

    for (;;) {
        // Bắt đầu quét tất cả các kênh
        HAL_ADC_Start(&hadc1);

        // Đọc lần lượt giá trị từ 4 kênh
        for (int i = 0; i < 4; i++) {
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);//polling giá trị adc
            adc_values[i] = HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);

        // Xử lý Joystick 1
        // Trục X
        if (adc_values[1] < JOY_THRESHOLD_LEFT) {
            command = JOY1_LEFT;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);//lưu vào hàng đợi command
        } else if (adc_values[1] > JOY_THRESHOLD_RIGHT) {
            command = JOY1_RIGHT;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }

        // Trục Y
        if (adc_values[0] < JOY_THRESHOLD_LEFT) {
            command = JOY1_UP;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }else if (adc_values[0] > JOY_THRESHOLD_RIGHT) {
			command = JOY1_DOWN;
			osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
		}

        // Xử lý Joystick 2
        // Trục X
        if (adc_values[3] < JOY_THRESHOLD_LEFT) {
            command = JOY2_LEFT;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        } else if (adc_values[3] > JOY_THRESHOLD_RIGHT) {
            command = JOY2_RIGHT;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }

        // Trục Y
        if (adc_values[2] < JOY_THRESHOLD_LEFT) {
            command = JOY2_UP;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }
        else if (adc_values[2] > JOY_THRESHOLD_RIGHT) {
		   command = JOY2_DOWN;
		   osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
	   }
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
