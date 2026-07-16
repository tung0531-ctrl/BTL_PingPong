# BÁO CÁO KỸ THUẬT HỆ THỐNG
## Dự án: Game Pong trên STM32F429I-DISCOVERY (TouchGFX + FreeRTOS)

---

## 1. THÀNH PHẦN PHẦN CỨNG VÀ CẤU HÌNH `.ioc`

### 1.1. Vi điều khiển
- **MCU:** STM32F429ZITx (dòng STM32F429, gói **LQFP144**)
- **Clock:**
  - Thạch anh ngoài (HSE): **8 MHz** (chân `PH0`/`PH1`)
  - PLL: `PLLM=8, PLLN=360, PLLP=2, PLLQ=4`
  - **SYSCLK = 180 MHz**, **HCLK = 180 MHz**
  - APB1 = 45 MHz (Timer APB1 = 90 MHz), APB2 = 90 MHz (Timer APB2 = 180 MHz)
  - Clock LTDC/SAI ≈ 6 MHz (qua PLLSAI)
- **Priority Group:** `NVIC_PRIORITYGROUP_4` (4 bit preemption, 0 bit sub-priority)

### 1.2. Ngoại vi được bật trong `.ioc`
| Ngoại vi | Vai trò |
|---|---|
| **ADC1** | Đọc 4 kênh analog của 2 joystick |
| **I2C3** | Giao tiếp cảm biến chạm / IO Expander màn hình |
| **SPI5** | Điều khiển LCD driver (ILI9341) |
| **FMC (SDRAM)** | Bộ nhớ ngoài chứa framebuffer LCD |
| **LTDC** | Bộ điều khiển hiển thị LCD (RGB565) |
| **DMA2D** | Tăng tốc vẽ đồ hoạ phần cứng |
| **TIM6** | Timebase hệ thống (thay SysTick), ngắt ưu tiên 0 |
| **TIM7** | Sinh ngắt định kỳ cho buzzer (không dùng PWM) |
| **CRC** | Module kiểm tra CRC (phục vụ HAL/flash) |
| **FreeRTOS (CMSIS-V2)** | Hệ điều hành thời gian thực |

### 1.3. Cấu hình LCD/SDRAM
- **Độ phân giải:** 240 × 320 px, định dạng **RGB565**
- **SDRAM timing:** CAS Latency 3, Load-to-Active 2 chu kỳ, RCD 2, RP 2, Row cycle 7, Write recovery 3
- **LTDC timing:** HSYNC 10, HBP 20, HFP 10, VSYNC 2, VFP 4
- Framebuffer TouchGFX được cấp phát trên SDRAM ngoài (không đủ RAM nội bộ MCU để chứa toàn bộ khung hình)

### 1.4. Bảng đấu chân (Pin mapping) chính

**LCD/LTDC (RGB565):**
- Đỏ: `PA11`(R4), `PB0`(R3), `PB1`(R6), `PG6`(R7)
- Xanh lá: `PA6`(G2), `PB10`(G4), `PB11`(G5), `PC7`(G6), `PD3`(G7), `PG10`(G3)
- Xanh dương: `PA3`(B5), `PB8`(B6), `PB9`(B7), `PG11`(B3), `PG12`(B4)
- Đồng bộ: `PA4`(VSYNC), `PC6`(HSYNC), `PF10`(DE), `PG7`(LTDC_CLK)

**FMC/SDRAM:**
- Địa chỉ: `PF0-5`, `PF12-15`, `PG0-1`, `PG4-5`
- Dữ liệu: `PD0-1`, `PD3`, `PD8-15`, `PE0-15`
- Điều khiển: `PC0`(SDNWE), `PB5`(SDCKE1), `PB6`(SDNE1), `PF11`(SDNRAS), `PG15`(SDNCAS), `PG8`(SDCLK)
- Byte enable: `PE0-1` (NBL0, NBL1)

**SPI5 (LCD controller):** `PF7`(SCK), `PF8`(MISO), `PF9`(MOSI)

**I2C3 (cảm biến/IO Expander):** `PA8`(SCL, pull-up), `PC9`(SDA, pull-up)

**Joystick (ADC1, 12-bit):**
| Tín hiệu | Kênh ADC | Chân |
|---|---|---|
| Joystick 1 - trục X | ADC_CHANNEL_13 | PC3 |
| Joystick 1 - trục Y | ADC_CHANNEL_11 | PC1 |
| Joystick 2 - trục X | ADC_CHANNEL_7  | PA7 |
| Joystick 2 - trục Y | ADC_CHANNEL_5  | PA5 |

**Nút bấm chọn (User Button):** `PA0` (mức RESET = nhả, mức SET = nhấn)

**Buzzer (âm thanh):** `PG13` — cấu hình **GPIO Output Push-Pull thuần túy**, KHÔNG bao giờ chuyển sang chế độ Alternate Function (lý do kỹ thuật trình bày ở mục 3.6)

**Ghi chú lịch sử:** `PD12` từng được dùng làm chân buzzer ban đầu nhưng bị loại bỏ vì đây là chân dùng chung với đường địa chỉ của FMC (SDRAM) — khi SDRAM hoạt động liên tục để phục vụ framebuffer LCD, chân này bị FMC tự động toggle nền, gây ra tiếng kêu không mong muốn dù không có sự kiện game nào.

---

## 2. LUỒNG HOẠT ĐỘNG TOÀN BỘ HỆ THỐNG

### 2.1. Cấu trúc tác vụ FreeRTOS

| Task | Vai trò | Stack | Priority |
|---|---|---|---|
| `defaultTask` | Task mặc định (rỗng, chỉ `osDelay`) | 512 byte | Normal |
| `GUI_Task` (do TouchGFX cấp) | Vòng lặp render TouchGFX (~60 FPS) | 32768 byte | Normal |
| `joystickTask` | Đọc ADC joystick, sinh lệnh điều khiển | 512 byte | Normal |

- **Hàng đợi (Queue):** `joystickQueueHandle` — 32 phần tử, mỗi phần tử là 1 giá trị `JoystickCommand_t`. Task joystick là bên **ghi (producer)**, các màn hình game (Easy/Medium/Hard View) là bên **đọc (consumer)** trong `handleTickEvent()`.
- Không có Semaphore/Mutex tự tạo thêm — TouchGFX tự quản lý đồng bộ nội bộ của nó.

### 2.2. Task Joystick — cơ chế đọc & sinh lệnh
- Đọc lần lượt 4 kênh ADC (quét tuần tự), giá trị 12-bit (0–4095), tâm nghỉ ≈ 2048.
- Ngưỡng phát hiện hướng:
  - Giá trị `< 1000` → hướng "trái/lên" (`JOY_LEFT`/`JOY_UP`)
  - Giá trị `> 3000` → hướng "phải/xuống" (`JOY_RIGHT`/`JOY_DOWN`)
- Có cơ chế chống dội tín hiệu (debounce phần mềm): mỗi trục chỉ gửi lại lệnh nếu trạng thái đổi, hoặc đã qua tối thiểu ~40ms — tránh làm tràn hàng đợi do rung tín hiệu analog.
- Nút bấm `PA0` sinh cả `JOY1_BUTTON` và `JOY2_BUTTON`; màn hình game tự lọc theo `servingPlayer` hiện tại để biết ai vừa bấm.
- Chu kỳ polling: khoảng 10ms/lần.

### 2.3. Luồng màn hình (TouchGFX Model-View-Presenter)

```
StartScreen
   │  (chọn độ khó)
   ▼
EasyReadyScreen / MediumReadyScreen / HardReadyScreen   (đếm ngược 3 giây, 180 tick)
   │
   ▼
EasyScreen / MediumScreen / HardScreen     (màn chơi chính)
   │  (điểm số ≥ 11)
   ▼
EndScreen   (hiển thị người thắng)
   │  (bấm nút)
   ▼
StartScreen
```

- **Model (`Model.hpp/.cpp`):** lưu trạng thái dùng chung — điểm số, người thắng (`winner`), cờ `gameOver`. Có `ModelListener` để các Presenter đăng ký lắng nghe.
- **Presenter:** cầu nối giữa Model và View, gọi `goToEndScreen(winner)` khi kết thúc trận, gọi `resetGame()` khi bắt đầu lại.
- **View:** nơi chứa toàn bộ logic gameplay thực tế (vị trí bóng, vận tốc, va chạm, điểm số, phát âm thanh) — kế thừa từ lớp `*ViewBase` do TouchGFX Designer sinh ra.

### 2.4. Chi tiết gameplay theo từng độ khó

| Đặc điểm | Easy | Medium | Hard |
|---|---|---|---|
| Tốc độ bóng ban đầu | 1.4 | 1.4 | 1.6 (nhanh hơn) |
| Tốc độ khi phát bóng | 2.0 | 2.0 | 2.3 |
| Vùng ghi điểm | Toàn bộ cạnh trái/phải | Giới hạn Y ∈ [60,181] (có `goal1`/`goal2`) | Giống Medium |
| Vật cản | Không | Không | 4 ảnh gai (`image1`, `image1_1`, `image2`, `image2_2`), va chạm bị tính là mất điểm |
| Đường ngắm khi giao bóng | Có (`line1`, `line1_1`) | Có | Có |

**Cơ chế chung cho cả 3 màn:**
- **Giao bóng:** sau khi mất điểm, bóng ẩn 60 tick (~1 giây); người giao bóng chỉnh góc bằng joystick (JOY_UP/DOWN, bước ±0.2, giới hạn [-2,2]); nhấn nút để phát bóng.
- **Phản xạ ở thanh chắn (paddle):** dùng hàm `applyPaddleBounce()` — tính điểm chạm tương đối so với **tâm** paddle (chuẩn hoá về [-1,1]); càng lệch tâm thì thành phần vận tốc dọc (Y) càng lớn (tối đa 85% tổng tốc độ), càng gần tâm thì bóng đi càng thẳng. Đây là cơ chế **thay thế** cho kiểu phản xạ "góc tới = góc phản xạ" cũ vốn dễ khiến bóng bị "kẹt" quỹ đạo thẳng đứng.
- **Va chạm tường trên/dưới:** đảo chiều vận tốc Y.
- **Thắng trận:** điểm số đầu tiên đạt 11.

### 2.5. Hệ thống âm thanh Buzzer (trạng thái hiện tại — bản mới nhất)

- **Chân xuất:** `PG13`, cấu hình GPIO Output Push-Pull thuần túy, **không** dùng Alternate Function.
- **Cơ chế phát âm:** không dùng kênh PWM phần cứng. Timer `TIM7` chỉ sinh **ngắt định kỳ**; trong ngắt đó, phần mềm tự đảo mức chân (`HAL_GPIO_TogglePin`) để tạo sóng vuông đúng tần số mong muốn — cách làm này đảm bảo chân buzzer không bao giờ bị các ngoại vi khác (FMC, LTDC) âm thầm điều khiển.
- **Cấu hình `TIM7`:** Prescaler 89 (clock hiệu dụng 1 MHz), chế độ đếm lên, ngắt ưu tiên NVIC mức 6.
- **Các nhóm âm thanh (`BuzzerStep[] = {tần số Hz, số tick}`):**
  - `scoreAccent` (ghi điểm): 659 Hz → 784 Hz → 988 Hz → nghỉ
  - `paddleBounceEffect` (chạm thanh chắn): 523 Hz → 659 Hz → nghỉ
  - `wallBounceEffect` (chạm tường): 392 Hz → nghỉ
  - `backgroundLoop` (nhạc nền, giai điệu **tự soạn riêng cho dự án**, không sao chép từ bài hát nào): chuỗi nốt ở các tần số 349–784 Hz, nhịp độ đã được chỉnh chậm và có khoảng nghỉ giữa các nốt để tránh cảm giác "kêu đúp/nháy đúp".
- **Cơ chế ưu tiên phát âm:** hiệu ứng va chạm/ghi điểm luôn được ưu tiên phát **đè lên** nhạc nền; phát xong tự động quay lại đúng vị trí tiếp theo của nhạc nền (không lặp lại nốt cũ). Có cơ chế "guard tick" (2–3 tick) chống kích hoạt lặp lại quá nhanh cùng một hiệu ứng.
- **Khởi tạo sớm:** `BuzzerMusic_Init()` được gọi ngay sau `MX_FMC_Init()` trong `main()` để giành quyền điều khiển chân buzzer về GPIO thuần càng sớm càng tốt, đảm bảo im lặng ngay từ lúc cấp nguồn.

---

## 3. HẠN CHẾ / ĐIỂM CẦN LƯU Ý TRONG LUỒNG CODE

1. **Trùng lặp code xử lý va chạm giữa 3 màn (Easy/Medium/Hard):**
   Logic va chạm paddle, va chạm tường, tính điểm gần như giống hệt nhau nhưng được viết lặp lại riêng trong từng file `View.cpp`. Chưa có lớp game dùng chung (base class) hay module vật lý chia sẻ. Hệ quả: sửa lỗi/vật lý ở một nơi phải nhớ sửa cả 3 nơi, dễ phát sinh sai lệch giữa các màn theo thời gian.

2. **Code cũ còn sót lại (đã comment nhưng chưa xoá) trong `Model.hpp`/`Model.cpp`/`ModelListener.hpp`:**
   Các hàm `movePaddle1Left/Right`, `movePaddle2Left/Right`, `paddle1Action/paddle2Action` bị comment out — đây là thiết kế điều khiển bóng qua Model từ giai đoạn đầu, đã được thay bằng xử lý trực tiếp trong View. Nên dọn dẹp để tránh gây nhầm lẫn cho người đọc code sau này.

3. **Nhiều số "ma thuật" (magic number) chưa được đặt tên hằng số:**
   Ví dụ: giới hạn Y của paddle `[2, 198]`, kích thước màn hình `320×240` lặp lại nhiều chỗ, vùng khung thành `[60, 181]`, thời gian chờ giao bóng `60` tick, ngưỡng thắng `11` điểm. Nên gom thành các `#define`/hằng số dùng chung để dễ chỉnh sửa và tránh sai lệch giữa các file.

4. **Thiếu một số kiểm tra an toàn (error handling):**
   - Không kiểm tra tràn hàng đợi joystick (`joystickQueueHandle`) — nếu lệnh tới nhanh hơn tốc độ xử lý, lệnh cũ bị âm thầm loại bỏ mà không có cảnh báo.
   - Không có cơ chế phát hiện lỗi phần cứng (ADC treo, mất giao tiếp I2C/SPI) để đưa hệ thống về trạng thái an toàn.

5. **Không lưu trạng thái lâu dài:**
   Model không lưu lịch sử điểm cao (high score) hay số liệu thống kê giữa các ván chơi; mọi thứ reset khi vào lại màn chơi.

6. **Ưu tiên ngắt buzzer thấp hơn LTDC/DMA2D (mức 6 so với mức 5):**
   Về lý thuyết `TIM7` có thể bị trễ một chút nếu ngắt LTDC/DMA2D đang xử lý, nhưng với chu kỳ ngắt ở mức micro-giây thì ảnh hưởng thực tế lên chất lượng âm thanh là không đáng kể.

7. **Chân `PG13` dùng chung với LED xanh (LD3) có sẵn trên board STM32F429I-DISCOVERY:**
   Đây không phải lỗi, nhưng cần lưu ý: mỗi khi buzzer phát âm, LED xanh trên board cũng sẽ nhấp nháy theo — là hiệu ứng phụ vô hại về điện, không ảnh hưởng chức năng.

---

## 4. TÓM TẮT SỐ LIỆU NHANH

| Hạng mục | Giá trị |
|---|---|
| Tổng số màn hình TouchGFX | 8 |
| Số task FreeRTOS | 3 |
| Số hàng đợi (queue) | 1 (32 phần tử) |
| Số kênh ADC dùng cho joystick | 4 |
| Số loại lệnh joystick | 10 (5 lệnh × 2 người chơi) |
| Số nhóm hiệu ứng âm thanh | 3 hiệu ứng sự kiện + 1 nhạc nền |
| Độ phân giải LCD | 240×320, RGB565 |
| Xung nhịp hệ thống | 180 MHz (từ HSE 8 MHz qua PLL) |
| Điều kiện thắng | Điểm số đạt 11 trước |

---

*Báo cáo được tổng hợp từ mã nguồn thực tế của dự án tại thời điểm hiện tại (không suy đoán), phục vụ mục đích tài liệu hoá kỹ thuật.*
