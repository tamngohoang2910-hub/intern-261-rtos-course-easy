# Dự đoán — Week 3 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____

1. Thời gian truyền 1 byte ở `RX_BAUD` của bạn là bao nhiêu us? (8N1, tính cả
   start/stop bit). Trong thời gian đó `parser_task` có kịp xử lý 1 byte từ
   stream buffer không, nếu nó là task ưu tiên cao nhất hệ thống?
- Ta có: Tính cả start bit và stop bit (theo cấu hình 8N1) thì tổng cộng 10bit. Thời gian truyền 1 byte là: 10/115200=86.806(us). Với clock=72MHz thì `parser_task` sẽ kịp xử lí 1 byte từ stream buffer. Vì thời gian để `parser_task` xử lí 1 byte bao gồm: 
  + vào/ra ngắt
  + HAL_UART_IRQHandler(kiểm tra lỗi, đọc thanh ghi DR, lưu byte vào buffer, gọi callback)
  + xStreamBufferSendFromISR (vào critical section ghi vào register BASEPRI,cập nhật chỉ số, đánh thức task đang chờ bằng task notification)
  + HAL_UART_Receive_IT(lock, đặt trạng thái, bật lại cờ ngắt)
  + PendSV chuyển sang parser_task
  + xStreamBufferReceive + cộng byte vào bộ tích luỹ
  + Parser block lại, PendSV chuyển ngược
- Tổng thời gian khoảng 1000-2000 chu kì nên tối đa khoảng 55us (<86.806us) nên sẽ xử lí kịp.
2. `STREAM_BUFFER_BYTES` của bạn chứa tối đa bao nhiêu lệnh dài `MAX_CMD_LEN`
   byte trước khi đầy?
- `STREAM_BUFFER_BYTES`(=128 byte) có thể chứa tối đa 5 lệnh dài `MAX_CMD_LEN` (=24 byte). Khi `parser_task` không chạy lần nào thì `STREAM_BUFFER_BYTES` sẽ đầy sau 86.806*128=11.11us.
3. Nếu `xStreamBufferSendFromISR` trả về 0 (stream buffer đầy) trong
   `uart_rx_isr_handler`, byte đó sẽ bị mất. Theo bạn, trường hợp nào thực tế
   khiến điều này xảy ra (gửi quá nhanh / `parser_task` ưu tiên quá thấp /
   cái khác)?
- Nếu `xStreamBufferSendFromISR` trả về 0 (stream buffer đầy) trong
   `uart_rx_isr_handler` phần lớn là do `parser_task` bị chặn trong khoảng 11.11us (có thể do có task khác có priority cao hơn `parser_task` thực thi liên tục không block). Vì UART gửi tín hiệu 115200 baud là khoảng 86.806us lớn hơn so với thời gian xử lí dữ liệu của parser_task. Do đó chỉ có lí do là  `parser_task` ưu tiên quá thấp là thực tế có thể xảy ra.
4. Dự đoán: gửi lệnh bằng gõ tay bình thường có mất byte không? Gửi bằng
   script gửi liên tục không nghỉ thì sao?
-  Khi gõ lệnh bằng tay bình thường thì sẽ không mất byte vì thời gian gõ tay nhanh nhất từng được ghi nhận là 216WPM (quy ước khoảng 1 từ =5 byte) thì tốc độ là 18byte/s nên 1 khoảng 55.55ms 1 byte. Kết quả trên vẫn nhỏ hơn so với tốc độ xử lí của parser_task.
-  Khi gửi liên tục bằng script không nghỉ cũng không làm mất byte do tốc độ truyền giới hạn là 86.806 us.
