# BÁO CÁO HỆ NHÚNG

Game Pingpong

## GIỚI THIỆU

- **Đề bài:** Thiết kế và xây dựng hệ thống trò chơi Ping Pong cho 2 người chơi trên nền tảng vi điều khiển STM32F429ZIT6.
- **Tính năng yêu cầu:**
  - Mỗi người chơi điều khiển một vợt thông qua một joystick analog riêng biệt.
  - Hiển thị trạng thái trò chơi (vị trí bóng, vị trí vợt, hướng bóng, điểm số) lên màn hình (OLED hoặc LCD).
  - Xử lý va chạm bóng với vợt, tường và cập nhật điểm số.
  - Cho phép bắt đầu/trò chơi lại khi có tín hiệu điều khiển.
  - Đảm bảo tốc độ xử lý mượt mà, không giật lag khi thao tác điều khiển.
  - Hiệu ứng âm thanh (buzzer) khi có điểm.
- **Ý nghĩa:** Dự án giúp sinh viên rèn luyện kỹ năng lập trình nhúng, xử lý ngoại vi (ADC cho joystick, giao tiếp màn hình, ngắt...), thiết kế giao diện điều khiển thực tế, và làm việc nhóm.


- Ảnh chụp minh họa:\
<<<<<<< HEAD
  ![Ảnh minh họa](https://github.com/Hoee1802/MyApp/blob/main/Schematic.jpg)
=======
  ![Ảnh minh họa]((https://github.com/Hoee1802/MyApp/blob/main/Schematic.jpg))
>>>>>>> b1bd0e016881fc297db5f5766d100ff1a73ff20d

## TÁC GIẢ

- Tên nhóm: Chip chip
- Thành viên trong nhóm
  |STT|Họ tên|MSSV|Công việc|
  |--:|--|--|--|
  |1|Vũ Văn Nam|20215623|hiện thị oled, hiệu ứng, và xử lý ngắt|
  |2|Vũ Thị Thanh Hoa|20210356|hiện thị oled, hiệu ứng, và xử lý ngắt|
  |3|Trần Bích Diệp|20215539|hiện thị oled, hiệu ứng, và xử lý ngắt|
  |4|Nguyễn Văn Long|20215610|Vẽ sơ đồ Schematic và viết báo cáo|

## MÔI TRƯỜNG HOẠT ĐỘNG

- **Module CPU / Dev kit sử dụng:**  
  - STM32F429ZIT6 (ARM Cortex-M4 32-bit)

- **Các kit, module, linh kiện sử dụng:**
  - 2 x Joystick analog (điều khiển hai vợt trong game)
  - Buzzer
  - Dây kết nối Dupont

- **Sơ đồ kết nối cơ bản:**

| STM32F429ZIT6 | Joystick 1 | Joystick 2 |   Buzzer   |
|:-------------:|:----------:|:----------:|:----------:|
| 3V3           | VCC        | VCC        |            |
| GND           | GND        | GND        |     GND    |
| PA0           | VRy        |            |            |
| PC3           | VRx        |            |            |
| PG2           | SW         |            |            |
| PA5           |            | VRy        |            |
| PA7           |            | VRx        |            |
| PG3           |            | SW         |            |
| PD12          |            |            |    VCC     |
- **Chức năng từng module:**
  - **STM32F429ZIT6:** Xử lý tín hiệu, đọc dữ liệu joystick, điều khiển logic game ping pong hai người chơi.
  - **Joystick:** Nhập tín hiệu điều khiển từ người chơi (di chuyển vợt).
  - **Buzzer:** Kêu khi người chơi ghi điểm


## SƠ ĐỒ SCHEMATIC
<<<<<<< HEAD
=======

_Cho biết cách nối dây, kết nối giữa các linh kiện_
Ví dụ có thể liệt kê dạng bảng
|STM32F429|Module ngoại vi|
|--|--|
|PA0|Nút bấm điều khiển trên board|
|PE2|MQ3 SCK|
|PE3|MQ3 SDA|

>>>>>>> b1bd0e016881fc297db5f5766d100ff1a73ff20d
![Sơ đồ Schematic](https://github.com/Hoee1802/MyApp/blob/main/Schematic.jpg)

### TÍCH HỢP HỆ THỐNG

#### **Thành phần phần cứng và vai trò**
- **STM32F429ZIT6**: Vi điều khiển trung tâm, tiếp nhận tín hiệu từ joystick, xử lý thuật toán game và xuất dữ liệu hiển thị.
- **Joystick x2**: Thiết bị nhập liệu, mỗi joystick điều khiển một vợt, cho phép 2 người chơi tương tác với trò chơi.
- **Màn hình OLED/LCD** : Hiển thị trạng thái trò chơi: vị trí bóng, vị trí vợt, điểm số của từng người chơi.
- **Buzzer** : Tạo hiệu ứng âm thanh khi ghi điểm.

#### **Thành phần phần mềm và vai trò**
- **Firmware (chạy trên STM32)**: 
  - Nhận và xử lý tín hiệu analog từ 2 joystick.
  - Thực hiện thuật toán di chuyển bóng, phát hiện va chạm, cập nhật điểm số.
  - Điều khiển giao diện hiển thị (OLED/LCD).
  - Xử lý các hiệu ứng phụ như âm thanh.
- **Giao diện người dùng**:  
  - Được hiển thị trực tiếp trên màn hình nối với STM32.
  - Không sử dụng phần mềm ngoài, mọi thao tác điều khiển và hiển thị được thực hiện toàn bộ trên vi điều khiển và các module ngoại vi.

#### **Lưu ý**
- Hệ thống hoạt động hoàn toàn **độc lập, không cần kết nối mạng, không có máy chủ** hay các thành phần IoT phức tạp.
- Tất cả các chức năng từ nhập liệu, xử lý đến hiển thị đều nằm trên **STM32F429ZIT6** và các module ngoại vi kèm theo.


### ĐẶC TẢ HÀM

- Giải thích một số hàm quan trọng: ý nghĩa của hàm, tham số vào, ra

extern "C" void JoystickTask(void *argument)
{
    
    uint32_t adc_values[4] = {2048, 2048, 2048, 2048};
    ADC_ChannelConfTypeDef sConfig = {0};//
    uint8_t channel_rank = 1;
    uint32_t channels[4] = {JOY1_X_CHANNEL, JOY1_Y_CHANNEL, JOY2_X_CHANNEL, JOY2_Y_CHANNEL};
    for (int i = 0; i < 4; i++) {
        sConfig.Channel = channels[i];
        sConfig.Rank = channel_rank++;
        sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
        HAL_ADC_ConfigChannel(&hadc1, &sConfig);
    }

    hadc1.Instance->CR1 |= ADC_CR1_SCAN; 
    hadc1.Instance->SQR1 &= ~(ADC_SQR1_L); 
    hadc1.Instance->SQR1 |= (3 << 20); 

       JoystickCommand_t command;

    
    GPIO_PinState lastJoy1ButtonState = GPIO_PIN_SET;
    GPIO_PinState lastJoy2ButtonState = GPIO_PIN_SET;

    for (;;) {
       
        HAL_ADC_Start(&hadc1);

        for (int i = 0; i < 4; i++) {
            HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
            adc_values[i] = HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);

        if (adc_values[1] < JOY_THRESHOLD_LEFT) {
            command = JOY1_LEFT;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);//lưu vào hàng đợi command
        } else if (adc_values[1] > JOY_THRESHOLD_RIGHT) {
            command = JOY1_RIGHT;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }

       
        if (adc_values[0] < JOY_THRESHOLD_LEFT) {
            command = JOY1_UP;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }else if (adc_values[0] > JOY_THRESHOLD_RIGHT) {
            command = JOY1_DOWN;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }

        // Xử lý nút bấm Joystick 1
        GPIO_PinState joy1ButtonState = HAL_GPIO_ReadPin(JOY_BUTTON_PORT, JOY1_BUTTON_PIN);
        if (joy1ButtonState == GPIO_PIN_RESET && lastJoy1ButtonState == GPIO_PIN_SET) {
            command = JOY1_BUTTON;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }
        lastJoy1ButtonState = joy1ButtonState;

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
        // Xử lý nút bấm Joystick 2
        GPIO_PinState joy2ButtonState = HAL_GPIO_ReadPin(JOY_BUTTON_PORT, JOY2_BUTTON_PIN);
        if (joy2ButtonState == GPIO_PIN_RESET && lastJoy2ButtonState == GPIO_PIN_SET) {
            command = JOY2_BUTTON;
            osMessageQueuePut(joystickQueueHandle, &command, 0, 0);
        }
        lastJoy2ButtonState = joy2ButtonState;
        osDelay(10); 
    }
}

### Giải thích hàm này:
  - Đây là một task FreeRTOS có nhiệm vụ đọc giá trị từ hai joystick (Joystick 1 và Joystick 2) thông qua ADC (Analog-to-Digital Converter) và nút bấm GPIO, sau đó chuyển thành các lệnh điều khiển (JoystickCommand_t) và gửi vào hàng đợi FreeRTOS (joystickQueueHandle).
  - Sử dụng 4 kênh analog 0,5,7,13 theo thứ tự kênh 13->0->7->5 tương ứng với JOY1_X, JOY1_y, JOY2_X, JOY2_Y, mỗi kênh lấy mẫu trong 144 chu kỳ.
  - ADC1 được cấu hình ở chế độ SCAN, lần lượt polling các giá trị từ các kênh rồi so sánh với các ngưỡng, sau đó các command theo giá trị được đẩy vào hàng đợi để xử lý.
  - 2 chân GPIO2,3 được đọc liên tục để kiểm tra sau đó command cũng được đưa vào hàng đợi.

### KẾT QUẢ

- Các ảnh chụp với caption giải thích.
- Hoặc video sản phẩm
