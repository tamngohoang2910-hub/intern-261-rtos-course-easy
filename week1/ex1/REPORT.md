# Report — Week 1 / Ex1

`student_id`: 2413062    Commit của PREDICTION.md: ______

## 1. Số đo
| Đại lượng | Giá trị |
|---|---|
| `BLINK_PERIOD_MS` (từ params.h) | 97|
| Chu kỳ đo được — min |96.005625|
| Chu kỳ đo được — max |96.005667|
| Chu kỳ đo được — trung bình |96.005646 |
| Sai số trung bình vs `BLINK_PERIOD_MS` |1.025% |
| `uxTaskGetStackHighWaterMark()` của bringup_task |25 words |

Ảnh PulseView (>= 10 chu kỳ, đọc được giá trị): ![ảnh pulseView](image/.png)

## 2. Ba handler của kernel
| Tên trong port ARM_CM3 | Tên trong vector table | Nhiệm vụ |
|---|---|---|
| `vPortSVCHandler` |SVC_Handler | Khởi tạo context của task đầu tiên khi bắt đầu scheduler |
| `xPortPendSVHandler` |PendSV_Handler | Chuyển context giữa các task |
| `xPortSysTickHandler` | SysTick_Handler| Xử lý RTOS tick, cập nhật tick count và hỗ trợ đánh thức task đến hạn/lập lịch |

Log lỗi link trước khi sửa (dán nguyên văn):
portable/GCC/ARM_CM3/port.c:230: multiple definition of `SVC_Handler'; Core/Src/stm32f1xx_it.c:146: first defined here
portable/GCC/ARM_CM3/port.c:459: multiple definition of `PendSV_Handler'; Core/Src/stm32f1xx_it.c:172: first defined here
portable/GCC/ARM_CM3/port.c:492: multiple definition of `SysTick_Handler'; Core/Src/stm32f1xx_it.c:185: first defined here

Cách sửa:
- Vì trong FreeRTOS cần trực tiếp sử dụng cả 3 handler này, mà trong file stm32f1xx_it.c cũng có các handler rỗng do CubeMX sinh ra. Vì thế ta sẽ xóa các handler này trong file stm32f1xx_it.c để mỗi handler chỉ có 1 định nghĩa.
## 3. Tại sao HAL timebase phải sang TIM4
Phải trả lời được: `HAL_IncTick()` chạy ở đâu, `configKERNEL_INTERRUPT_PRIORITY`
ảnh hưởng gì đến nó, và `HAL_Delay()` / `vTaskDelay()` khác nhau chỗ nào.
- HAL timebase phải chuyển sang TIM4 vì FreeRTOS cần sử dụng SysTick để gọi handler lập lịch task và cập nhật tick. Trong khi đó HAL cần HAL_IncTick() để tăng bộ đếm uwTick, để làm các hàm như HAL_Delay.
- configKERNEL_INTERRUPT_PRIORITY cấu hình mức ưu tiên cho các ngắt của kernel như PendSV, SysTick ở các ưu tiên mức thấp và nó không gọi HAL_IncTick().
- vTaskDelay sẽ block task hiện tại để scheduler có thể chạy task khác, còn HAL_Delay sẽ chờ bằng cách polling tick HAL. Vì vậy để làm task FreeRTOS thì ta ưu tiên dùng vTaskDelay.
## 4. Lệch so với dự đoán
Với mỗi mục trong `PREDICTION.md` lệch so đo: lệch bao nhiêu, và **nguyên nhân vật lý**
(không được trả lời "do sai số").
- So với dự đoán là 96ms, chu kỳ Led trung bình đo được là 96.005646ms, lệch 0.005646ms. Nguyên nhân có thể do:
+ Sai số clock lấy mẫu của logic analyzer có dung sai cỡ vài chục ppm.
+ Sai số tần số thạch anh HSE 8 MHz của board. Thạch anh có dung sai vài chục ppm, kết hợp logic analyzer nên gây nguyên nhân sai số nhỏ.
## 5. Câu hỏi đánh đổi
`configTICK_RATE_HZ` = 1000 trong bài này. Dựa vào số đo của **bạn**, nếu hạ xuống
100 Hz thì chu kỳ LED và sai số thay đổi thế nào? Trả lời bằng con số, không bằng lý thuyết.
- Nếu hạ xuống 100Hz thì 1 tick= 1/100=10ms, vậy 1 chu kì sáng/tắt sẽ là BLINK_PERIOD_MS/2=97/2=48.5. Do vTaskDelay lấy số nguyên của tick là 10ms nên làm tròn thành 40ms, vậy chu kỳ LED sẽ khoảng 80ms. Từ đó ta thấy sai số lớn hơn nhiều so với chu kỳ BLINK_PERIOD_MS ban đầu là 97ms (chênh lệch 17ms).