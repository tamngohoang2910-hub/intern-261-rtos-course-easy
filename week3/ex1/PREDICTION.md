# Dự đoán — Week 3 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____

1. Với `ISR_NVIC_PRIO` của bạn và nhóm priority `NVIC_PRIORITYGROUP_4`, số này
   nằm trong khoảng cho phép (`configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` ..
   `configLIBRARY_LOWEST_INTERRUPT_PRIORITY`) không? Trình bày phép so sánh.
- Với `ISR_NVIC_PRIO` là 7 và nhóm priority `NVIC_PRIORITYGROUP_4`, số này thỏa mãn khoảng cho phép vì `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY` =5 và  `configLIBRARY_LOWEST_INTERRUPT_PRIORITY`=15. Vì ISR phải gọi API (có dạng FromISR) phải có ưu tiên thấp hơn hoặc bằng `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`, do số càng nhỏ = ưu tiên phần cứng càng cao. Và ưu tiêng của `ISR_NVIC_PRIO` phải cao hơn ưu tiên thấp nhất của ISR là `configLIBRARY_LOWEST_INTERRUPT_PRIORITY`. Qua đó ta có 5<=7<=15 đã thoả mãn điều kiện.
2. Khi đặt priority EXTI cao hơn ngưỡng cho phép rồi gọi
   `vTaskNotifyGiveFromISR`, bạn dự đoán hành vi quan sát được là gì (reset?
   treo? thông báo gì trên UART/debugger)?
- Theo dự đoán thì hệ thống sẽ bị treo và không có dòng thông báo gì trên UART. Vì khi gọi `vTaskNotifyGiveFromISR` có hàm vPortValidateInterruptPriority(). Hàm này sẽ đọc priority của ngắt đang chạy so với `configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY`, nếu đặt priority EXTI cao hơn ngưỡng này thì `configASSERT` tắt ngắt và lặp vô hạn `for(;;)`. Hệ thống sẽ bị treo. 
3. Dự đoán latency ISR → task (có `portYIELD_FROM_ISR`) theo đơn vị us, dựa
   trên tần số CPU 72 MHz và số lệnh ước lượng trong đường đi ISR → context
   switch. So với `LATENCY_BUDGET_US`: đạt hay không?
- Dự đoán latency ISR → task (có `portYIELD_FROM_ISR`) khoảng 10 µs. Đường đi gồm vào ngắt (~15 chu kỳ), ISR + HAL callback (~40), `vTaskNotifyGiveFromISR` (~200), yield qua PendSV (~170 gồm lưu/nạp contextvà chọn task), thoát ngắt và chạy tiếp `ulTaskNotifyTake` (~90).
Tổng ~520 chu kỳ, nhân hệ số ~1,5 cho wait state Flash được ~780 chu kỳ.
Với 72 MHz: 780 / 72 MHz ≈ 10,8 µs < 30 µs, nên đạt.
1. Nếu bỏ `portYIELD_FROM_ISR`, `ack_task` sẽ chạy **khi nào** (ngay lập tức
   hay phải chờ tick ngắt kế tiếp)? Latency tăng thêm cỡ bao nhiêu?
- Nếu bỏ `portYIELD_FROM_ISR`, `ack_task` sẽ phải chờ đến lần scheduler hoạt động tiếp, thường là tick ngắt kế tiếp. Latency sẽ tăng thêm trong khoảng từ 0-1ms khi `configTICK_RATE_HZ` = 1000.