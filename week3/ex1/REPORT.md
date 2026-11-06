# Report — Week 3 / Ex1

`student_id`: __2413062____    Commit của PREDICTION.md: __06/11/2026____

## 1. Bằng chứng configASSERT
Mô tả/log/ảnh lúc priority sai (bước 4 README) + giải thích chính xác **vì sao**
assert được kích hoạt (nêu đúng tên macro và điều kiện).
Log UART:
=== FREERTOS ASSERT ===
File: C:/Users/TAM/intern-261-rtos-course-easy/third_party/FreeRTOS-Kernel/portable/GCC/ARM_CM3/port.c
Line: 803
EXTI Priority: 4
- Giải thích: trong Cortex-M, số nhỏ = ưu tiên cao. EXTI15_10 đặt priority 4, cao hơn ngưỡng `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY = 5`, nên `BASEPRI` (che ngắt ≥ 5) không che được nó, có thể làm hỏng dữ liệu kernel. Vì vậy `vTaskNotifyGiveFromISR()` gọi `vPortValidateInterruptPriority()` để kiểm tra `configASSERT( ucCurrentPriority >= ucMaxSysCallPriority )`. Ở đây `0x40 < 0x50` nên assert thất bại và chương trình dừng.   
## 2. Latency ISR → task
| Cấu hình | Dự đoán (us) | Đo được (us) | Đạt `LATENCY_BUDGET_US`? |
|---|---|---|---|
| Có `portYIELD_FROM_ISR` | 10.8 | 13 | đạt |
| Không có `portYIELD_FROM_ISR` | 500 | 556.227 | không đạt |
Ảnh logic analyzer đính kèm cho cả 2 cấu hình.
- Ảnh có `portYIELD_FROM_ISR`: ![port_yield](image/port_yield.png)
- Ảnh không có `portYIELD_FROM_ISR`: ![ko_port_yield](image/k_port_yield.png)
## 3. Giải thích quan hệ priority
Giải thích bằng lời của bạn: số NVIC priority nhỏ/lớn nghĩa là gì, vì sao
`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` tồn tại, điều gì xảy ra nếu một
ISR ưu tiên cao hơn ngưỡng này gọi API FreeRTOS.
- Số NVIC càng nhỏ thì độ ưu tiên của NVIC càn lớn. `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` tồn tại do khi có ngắt, kernel vào critical section, nó không tắt hết ngắt mà sẽ ghi vào thanh ghi `BASEPRI`, chỉ che các ngắt có priority từ 5 trở xuống.
- Nếu ISR ưu tiên cao hơn ngưỡng (số < 5) gọi API FreeRTOS, `BASEPRI` sẽ không che được các ngắt này. Vì thế khi có ngắt, nó có thể chen vào lúc kernel đang sửa ready list hoặc event group, từ đó sẽ làm hỏng chương trình.
## 4. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý (jitter NVIC, pipeline, v.v.).
| Cấu hình | Dự đoán (µs) | Đo được (µs) | Lệch |
|---|---|---|---|---|
| Có `portYIELD_FROM_ISR` | 10.8 | 13 | +2.2 µs (+20%) | 
| Không có `portYIELD_FROM_ISR` | 500 | 556.227 | +56.2 µs (+11%) |
-Không có `portYIELD_FROM_ISR`: dự đoán 500 µs là giá trị trung bình lý thuyết của phân bố đều trong [0, 1 ms] (task chỉ chạy ở tick kế tiếp) và có thể do debounce và overhead NVIC.
- Có  `portYIELD_FROM_ISR` : dự đoán chỉ đếm lệnh ước lượng, thực tế còn thêm độ trễ đồng bộ EXTI và vào ngắt của NVIC, wait state Flash và pipeline bị flush khi vào/ra ISR và PendSV, cộng với configASSERT đang bật.
## 5. Câu hỏi đánh đổi
Nếu tăng `ISR_NVIC_PRIO` của bạn lên một bậc (ưu tiên thấp hơn) trong khi vẫn
hợp lệ, latency đo được thay đổi bao nhiêu? Trả lời bằng số đo thật, không
đoán.
- Khi tăng `ISR_NVIC_PRIO` lên 1 bậc(ưu tiên thấp hơn) thì latency đo được vẫn khoảng 13us. Do trong bài này chỉ có EXTI là SysTick và PendSV ở priority 15 không thể chen vào, không có EXTI nào khác nên latency không thay đổi.
- Minh chứng: ![minh chung](image/minh_chung.png)
