# KIỂM TRA & ĐỐI CHIẾU CẤU HÌNH PHẦN CỨNG (.ioc vs CODE THỰC TẾ)

> Tài liệu này đối chiếu ảnh sơ đồ chân bạn gửi với 2 file cấu hình đang có trong repo:
> `STM32F429I_DISCO_REV_D01.ioc` và `backup_STM32F429I_DISCO_REV_D01.ioc`, cùng với code thực tế trong
> `Core/Src/main.c` và `STM32CubeIDE/Application/User/joystick_task.cpp`.

---

## KẾT LUẬN NHANH (đọc phần này trước)

**Ảnh sơ đồ chân bạn gửi KHỚP 100% với 2 file `.ioc` hiện có trong repo** — mọi chân LCD/LTDC, FMC/SDRAM,
SPI5, I2C3, các chân giám sát (`VSYNC_FREQ`, `RENDER_TIME`, `FRAME_RATE`, `MCU_ACTIVE`), và 2 chân
`GPIO_Output` (PD12/PD13) đều trùng khớp chính xác.

**NHƯNG:** có **4 nhóm chức năng đang chạy thật trong firmware lại HOÀN TOÀN KHÔNG được khai báo trong
file `.ioc`** (đó là lý do các chân này hiện trong ảnh dưới dạng ô xám/không nhãn):

| Chức năng | Chân sử dụng | Có trong `.ioc`? |
|---|---|---|
| ADC đọc Joystick 1 & 2 | `PC3, PC1, PA7, PA5` | ❌ Không |
| Nút bấm giao bóng (User Button) | `PA0` | ❌ Không |
| Buzzer (âm thanh) | `PG13` | ❌ Không |
| UART debug (USART1) | `PA9 (TX)`, `PA10 (RX)` | ❌ Không |

➡️ Tức là 4 khối này được bạn/AI **viết tay trực tiếp trong code** (`Core/Src/main.c`), **không đi qua
STM32CubeMX**. Đây là điểm quan trọng nhất cần lưu ý ở phần cảnh báo an toàn bên dưới.

---

## 1. ĐỐI CHIẾU CHI TIẾT TỪNG CHÂN TRONG ẢNH

| Chân | Nhãn trong ảnh/`.ioc` | Khớp code? | Ghi chú |
|---|---|---|---|
| PA3, PA4, PA6, PA11, PA12 | LTDC_B5, VSYNC, G2, R4, R5 | ✅ | LCD, do CubeMX quản lý |
| PA8, PC9 | I2C3_SCL, I2C3_SDA | ✅ | Cảm ứng/IO Expander |
| PA9, PA10 | *(trống trong ảnh)* | ⚠️ Thực tế = USART1 TX/RX | Không khai báo trong `.ioc` |
| PA0 | *(trống trong ảnh)* | ⚠️ Thực tế = Nút bấm giao bóng | Không khai báo trong `.ioc` |
| PA5, PA7 | *(trống trong ảnh)* | ⚠️ Thực tế = Joystick 2 (Y, X) | Không khai báo trong `.ioc` |
| PC1, PC3 | *(trống trong ảnh)* | ⚠️ Thực tế = Joystick 1 (Y, X) | Không khai báo trong `.ioc` |
| PC0, PC2 | FMC_SDNWE, GPIO_Output | ✅ | SDRAM + chân dự phòng khoá cứng |
| PB5, PB6, PB0/1/8/9/10/11 | FMC_SDCKE1/SDNE1, LTDC R/G/B | ✅ | |
| PC6, PC7 | LTDC_HSYNC, LTDC_G6 | ✅ | |
| PD0/1/3/8-15 | FMC_D0-D15, LTDC_G7 | ✅ | |
| PD12, PD13 | GPIO_Output (khoá `Locked=true`) | ✅ | **Chưa dùng vào việc gì trong code hiện tại** (chỉ init rồi để yên) |
| PE0-E15 | FMC_NBL0/1, FMC_D4-D12 | ✅ | |
| PE2-E5 | VSYNC_FREQ, RENDER_TIME, FRAME_RATE, MCU_ACTIVE | ✅ | Chân đo hiệu năng debug của TouchGFX |
| PF0-F5, F7-F15 | FMC_A0-A9, SPI5, LTDC_DE | ✅ | |
| PG0/1/4/5/6/7/8/10/11/12/15 | FMC_A10/A11/BA0/BA1, LTDC, FMC_SDCLK/SDNCAS | ✅ | |
| PG13 | *(trống trong ảnh)* | ⚠️ Thực tế = Buzzer | Không khai báo trong `.ioc` |
| PH0, PH1 | RCC_OSC_IN/OUT | ✅ | Thạch anh 8MHz |

---

## 2. SYSTEM CORE — kiểm tra chuẩn

### 2.1 RCC (đọc trực tiếp từ `.ioc`, đối chiếu đúng số trong code)
| Thông số | Giá trị |
|---|---|
| HSE (thạch anh ngoài) | 8 MHz |
| PLLM / PLLN / (PLLP suy ra) | 8 / 360 / 2 |
| VCO Input / VCO Output | 1 MHz / 360 MHz |
| **SYSCLK** | **180 MHz** |
| HCLK (AHB) | 180 MHz |
| APB1 (Peripheral / Timer) | 45 MHz / 90 MHz |
| APB2 (Peripheral / Timer) | 90 MHz / 180 MHz |
| PLLSAI (LCD-TFT clock) | N=192, R=4, DivR=/8 → 6 MHz |
| Nguồn SYSCLK | PLLCLK (không dùng HSI) |

➡️ Khớp hoàn toàn với thông số mình đã đưa vào `BaoCaoHeThong.md` trước đó. Không có sai lệch.

### 2.2 SYS / NVIC
| Ngắt | Preemption Priority | Sub Priority |
|---|---|---|
| DMA2D_IRQn | 5 | 0 |
| LTDC_IRQn | 5 | 0 |
| TIM6_DAC_IRQn (timebase) | 0 | 0 |
| PendSV_IRQn | 15 | 0 |
| SysTick_IRQn | 15 | 0 |
| TIM7_IRQn (buzzer — **thêm bằng tay, không có trong `.ioc`**) | 6 | 0 |

**Priority Group:** `NVIC_PRIORITYGROUP_4` (4 bit preemption / 0 bit subpriority) — chuẩn, không đổi.

**Nhận xét:** Timebase hệ thống dùng `TIM6` (không dùng SysTick mặc định) vì SysTick được TouchGFX/FreeRTOS
dùng cho mục đích khác — đây là cấu hình đúng chuẩn cho project TouchGFX + FreeRTOS, không cần sửa.

---

## 3. ANALOG (ADC1) — kiểm tra chuẩn

**Không có trong `.ioc`** (không thuộc `Mcu.IP` list) — toàn bộ được viết tay trong `Core/Src/main.c`
hàm `MX_ADC1_Init()`.

| Thông số | Giá trị hiện tại |
|---|---|
| Độ phân giải | 12-bit |
| Clock Prescaler | `ADC_CLOCK_SYNC_PCLK_DIV2` |
| Data Align | Right (căn phải) |
| Trigger | Phần mềm (`ADC_SOFTWARE_START`) |
| Số kênh cấu hình qua `HAL_ADC_ConfigChannel` | 4 (rank 1-4) |
| Chế độ Scan | **Bật bằng cách ghi thẳng thanh ghi** `hadc1.Instance->CR1 |= ADC_CR1_SCAN;` và `SQR1` (không qua trường `hadc1.Init.ScanConvMode` chuẩn của HAL) |
| Sampling Time mỗi kênh | 144 chu kỳ |

**Bảng kênh — đối chiếu với `joystick_task.cpp`:**

| Kênh ADC | Chân vật lý | Gán cho |
|---|---|---|
| `ADC_CHANNEL_13` | PC3 | Joystick 1 — trục X |
| `ADC_CHANNEL_11` | PC1 | Joystick 1 — trục Y |
| `ADC_CHANNEL_7`  | PA7 | Joystick 2 — trục X |
| `ADC_CHANNEL_5`  | PA5 | Joystick 2 — trục Y |

**Lưu ý kỹ thuật (không phải lỗi, chỉ cần biết):** trong code, biến `joy1XAxisState` lại đang đọc từ
`adc_values[1]` (là dữ liệu của **trục Y** vật lý — PC1), còn `joy1YAxisState` đọc từ `adc_values[0]`
(dữ liệu trục X — PC3). Đây có thể là chủ đích do hướng lắp module joystick thực tế trên khung máy, không
phải bug — chỉ ghi chú lại để không nhầm khi tra cứu sau này.

**Ngưỡng xử lý hướng:**
- `< 1000` → lệch về hướng "trái/lên"
- `> 3000` → lệch về hướng "phải/xuống"
- Khoảng `[1000, 3000]` quanh tâm ~2048 = vùng chết (deadzone), không sinh lệnh
- Chống dội lệnh: tối thiểu 40ms giữa 2 lần gửi lệnh giống nhau

**Nút bấm (User Button):** `PA0`, chế độ `GPIO_MODE_INPUT`, không pull nội (board có pull-down cứng sẵn),
đọc mức `SET` = đang nhấn.

---

## 4. TIMER — kiểm tra chuẩn

| Timer | Vai trò | Có trong `.ioc`? | Thông số |
|---|---|---|---|
| **TIM6** | Timebase hệ thống thay SysTick | ✅ Có | Ngắt ưu tiên 0/0 |
| **TIM7** | Sinh ngắt định kỳ cho buzzer (không PWM) | ❌ Không có | Prescaler 89 (clock hiệu dụng 1MHz), đếm lên, ngắt ưu tiên 6/0 |

**Không có timer nào khác được dùng** (không có TIM1-5, TIM8-14 nào được bật).

---

## 5. CONNECTIVITY — kiểm tra chuẩn

| Ngoại vi | Có trong `.ioc`? | Thông số |
|---|---|---|
| **I2C3** | ✅ Có | `Analog_Filter = DISABLE`, chân `PA8(SCL)/PC9(SDA)` có pull-up nội |
| **SPI5** | ✅ Có | Full-Duplex Master, chân `PF7(SCK)/PF8(MISO)/PF9(MOSI)` |
| **USART1** | ❌ Không có (viết tay) | 115200 baud, 8N1, không flow control, chân `PA9(TX)/PA10(RX)`, AF7 |
| **FMC (SDRAM)** | ✅ Có | Đã kiểm ở báo cáo trước, khớp `.ioc` |

---

## 6. HƯỚNG DẪN "KẾT NỐI CHUẨN" (checklist phần cứng thực tế)

Dựa trên toàn bộ đối chiếu ở trên, đây là bảng đấu nối chuẩn bạn nên dùng khi kiểm tra lại mạch:

| Thiết bị | Chân MCU | Kiểu tín hiệu |
|---|---|---|
| Joystick 1 — trục X | PC3 | Analog (ADC1_IN13) |
| Joystick 1 — trục Y | PC1 | Analog (ADC1_IN11) |
| Joystick 2 — trục X | PA7 | Analog (ADC1_IN7) |
| Joystick 2 — trục Y | PA5 | Analog (ADC1_IN5) |
| Nút giao bóng (2 người dùng chung) | PA0 (User Button on-board) | Digital Input, có pull-down cứng trên board |
| Buzzer | PG13 | Digital Output (GPIO thuần, không PWM) — **đồng thời là LED xanh LD3 có sẵn trên board, sẽ nhấp nháy theo tiếng bíp, không phải lỗi** |
| UART debug (nếu bạn dùng cổng USB ST-LINK VCP để xem log) | PA9 (TX) / PA10 (RX) | UART 115200-8N1 |
| Màn hình LCD, cảm ứng, SDRAM | Theo đúng bảng chân gốc trong `.ioc` (đã khớp ảnh, không đổi) | — |

---

## 7. CẢNH BÁO AN TOÀN — PHẦN NÀO KHÔNG NÊN ĐỘNG VÀO

⚠️ **Quan trọng nhất:** vì ADC1 (joystick), nút bấm PA0, Buzzer (PG13/TIM7), và USART1 **không được khai
báo trong file `.ioc`**, nếu bạn (hoặc ai đó) **mở lại file `.ioc` này bằng STM32CubeMX / trình Device
Configuration Tool trong STM32CubeIDE và bấm "Generate Code"**, công cụ đó **không biết** các chân
`PA0, PA5, PA7, PC1, PC3, PG13, PA9, PA10` đang được dùng. Rủi ro cụ thể:

1. **Rủi ro cao nhất:** Nếu bạn vô tình kéo-thả một chức năng khác (ví dụ thêm 1 ngoại vi mới) đúng vào
   một trong các chân này trên giao diện đồ hoạ rồi Generate Code, code tự viết tay cho joystick/buzzer/UART
   có thể bị **ghi đè hoặc xung đột chân**, gây mất chức năng mà không có thông báo lỗi rõ ràng.
2. Các hàm `MX_ADC1_Init()` và `MX_USART1_UART_Init()` hiện nằm ở vị trí giống chỗ CubeMX thường đặt code
   tự sinh, nhưng **không được bọc trong cặp `/* USER CODE BEGIN ... / USER CODE END ... */` riêng** —
   nghĩa là chúng không được cơ chế "giữ code" chuẩn của CubeMX bảo vệ chắc chắn 100%.

➡️ **Khuyến nghị:**
- **KHÔNG mở file `.ioc` này bằng STM32CubeMX/Device Configuration Tool rồi bấm "Generate Code"** trong
  giai đoạn này, trừ khi bạn đã backup toàn bộ `Core/Src/main.c` trước.
- Nếu chỉ cần sửa giao diện TouchGFX (màn hình, hình ảnh, layout) thì vẫn an toàn như bình thường —
  rủi ro chỉ nằm ở việc regenerate phần **MCU/Peripheral (Device Configuration Tool)**, không liên quan
  đến việc chỉnh sửa TouchGFX Designer.
- Nếu sau này thực sự cần thêm/sửa ngoại vi qua CubeMX, nên **khai báo lại đầy đủ ADC1 (4 kênh),
  TIM7, USART1, và các chân GPIO joystick/buzzer/nút bấm ngay trong giao diện `.ioc`** trước, để công cụ
  "biết" và khoá (`Locked`) các chân này lại — tránh bị cấp phát nhầm về sau.
- Các phần **an toàn tuyệt đối** để chỉnh sửa mà không lo mất code: toàn bộ `TouchGFX/gui/**`,
  `Core/Src/buzzer_music.h` cách gọi hàm, các file `View.cpp` — vì đây là code C/C++ thuần, không liên
  quan đến quy trình sinh code của CubeMX.

---

*Tài liệu đối chiếu trực tiếp từ nội dung thật của `STM32F429I_DISCO_REV_D01.ioc`,
`backup_STM32F429I_DISCO_REV_D01.ioc`, `Core/Src/main.c` và
`STM32CubeIDE/Application/User/joystick_task.cpp` tại thời điểm hiện tại.*
