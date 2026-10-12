# Dự đoán — Week 1 / Ex1

> **Commit file này TRƯỚC khi nạp chương trình lần đầu.** Report sẽ phải giải thích
> mọi chỗ dự đoán lệch so đo. Dự đoán sai không bị trừ điểm; dự đoán sau khi đo thì bị.

`student_id`: 2413062    Ngày/giờ commit: 9:41A.M 12/10/2026

1. Chu kỳ LED tôi sẽ đo được: 96 ms (`BLINK_PERIOD_MS` = 97)
2. Sai số giữa chu kỳ đo được và `BLINK_PERIOD_MS`: 1000 us. Vì sao?
- Vì mỗi lần nháy led sẽ có 2 lần đảo trạng thái ( sáng và tắt ), nên thời gian mỗi lần sáng / tắt là 97/2=48.5. Do trong FreeRTOS, API vTaskDelay dùng số tick nguyên, 1 tick =1ms ( 1 /configTICK_RATE_HZ =1/1000) nên sẽ lấy 48. Vì thế theo dự đoán của em thì chu kỳ Led sẽ đo được là 48*2 = 96ms.
3. Nếu để HAL timebase ở lại SysTick, điều gì sẽ xảy ra khi `vTaskDelay()` được gọi?
(chọn và giải thích: chạy bình thường / treo / tick sai nhịp / hard fault)
   - Khi để HAL timebase ở lại SysTick thì sẽ có thể bị treo.
   - Vì SysTick chỉ có một mà trong HAL và FreeRTOS cùng cần nó. Do đó, khi SysTick bắt đầu hoạt động, nếu HAL giữ SysTick_Handler thì tick bên kernel sẽ không tăng, từ đó sẽ bị treo.
4. `BRINGUP_STACK_WORDS` = 96 words = 384 bytes. Task này có đủ không? Vì sao?
- Task này đủ. Vì:
+ Trong task này sử dụng hàm vTaskDelay() và HAL_GPIO_TogglePin(). Trong file task.c, hàm vTaskDelay có gọi làm khác nên phải cất thanh ghi địa chỉ quay về (lr) vào stack (8 byte), có biến cục bộ và tham số nên sẽ tốn 8 byte. => hàm này có thể tốn 16 byte trong bộ nhớ stack. Ngoài ra hàm này còn gọi một số hàm khác khoảng khoảng 30-60 byte.
+ Hàm bring-up cũng gọi hàm khác nên lưu vào stack địa chỉ quay về (8 byte), các thanh ghi khi có ngắt xảy ra sẽ lưu 8 thanh ghi vào stack (8*4=32 byte). Khi đổi task, ngắt PendSV cất các thanh ghi lưu trạng thái cũ vào stack (32 byte).
+ Vậy task này cần 16+8+32+32+60= 148 byte=37 words. Vì thế uxTaskGetStackHighWaterMark có thể còn trống khoảng 55 words.
5. Trước khi map handler, linker sẽ báo lỗi gì (đoạn nguyên văn thông báo): multiple definition cho cả ba handler. 
- Do sau khi #define đổi tên thì trong file port.c và stm32f1xx_it.c cùng định nghĩa SVC_Handler, PendSV_Handler, SysTick_Handler.