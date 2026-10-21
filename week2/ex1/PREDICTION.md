# Dự đoán — Week 2 / Ex1

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413062

1. Phép tính kích thước queue tối thiểu (dùng `SAMPLE_PERIOD_MS`, thời gian xử lý
   ước lượng của `filter_task` và `uart_task` ở `UART_BAUD`):
- Ta có công thức: Nmin=[Rmax/Tp], với Rmax là khoảng thời gian mà consumer không nhận dữ liệu và producer tạo ra mỗi phần từ mỗi Tp. Áp dụng với bài này ta có tần số tạo mẫu =1/SAMPLE_PERIOD_MS=1/4=250(mẫu/s). Còn với UART với cấu hình 8N1 sẽ truyền 10 bit/byte.  
 => Tbyte=10/ UART_BAUD=10/57600=173.61 (us). Giả sử cảm biến gửi 3 giá trị, mỗi giá trị 6 byte, 2 dấu phẩy và /r/n tổng cộng 22 byte =>Ttx=3,82(ms)=> Nmin=[3.82/4]=1.

Kích thước tối thiểu dự đoán: `queue_raw` = ___1___ , `queue_filtered` = __1____ .
So với `QUEUE_LENGTH` được cấp: đủ hay không? Vì sao?
- đủ, vì QUEUE LENGTH có 12 phần tử.
2. Khi hạ `UART_BAUD` xuống 9600 (tạm thời), queue nào sẽ đầy trước? Vì sao?
- Khi hạ xuống 9600 thì Ttx=22.91(ms) > 4ms => queue filter_task sẽ đầy trước. 
3. Với `xQueueSend(..., 0)` (timeout = 0) khi queue đầy: điều gì xảy ra với mẫu đó?
- Khi đó, mẫu bị bỏ, xQueueSend sẽ trả về errQUEUE_FULL, không ghi vào queue và task đó cũng không block, code sẽ chạy đến lệnh tiếp theo trong task đó. Các phần tử đã có trong queue giữ nguyên. Nhưng mẫu đó sẽ bị mất khi một mẫu mới xuất hiện.
4. Với `xQueueSend(..., portMAX_DELAY)` khi queue đầy: `sensor_task` sẽ ở trạng thái
   nào? Có nguy cơ gì cho chu kỳ lấy mẫu?
- sensor_task sẽ ở trạng thái block. Khi sensor_task lấy mẫu mà queue_raw đầy thì mẫu đó vẫn được giữ lại cho đến khi queue có chỗ trống. 
- Vì thế sẽ làm tắc nghẽn, thời gian lấy mẫu tiếp theo sẽ bị kéo dài bằng với thời gian queue_raw hết trống. Nếu queue không hết trống thì task sẽ bị block hoàn toàn.
5. Độ đầy `queue_raw` theo thời gian ở cấu hình bình thường sẽ trông như thế nào
   (tăng dần / dao động quanh một mức / luôn gần 0)? Vì sao?
- Khi ở cấu hình bình thường sẽ luôn gần ở 0. Vì thời gian sensor_task sinh 1 mẫu là 4ms còn filter_task sẽ receive theo chu kì máy nên sẽ có thời gian ít hơn, vì thế queue_raw luôn gần 0.