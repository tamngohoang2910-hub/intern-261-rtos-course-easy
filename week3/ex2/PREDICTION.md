# Dự đoán — Week 3 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____

1. Với nút nhấn cơ khí thông thường, nảy phím thường kéo dài bao lâu
   (tra cứu/ước lượng)? So với `DEBOUNCE_MS` của bạn: đủ lớn để lọc hết không?
- Theo khảo sát của Jack Ganssle về hiện tượng debounce, thời gian nảy trung bình là 1,6 ms và tối đa là 10-20ms tùy loại, các loại cũ sẽ có thời gian debounce lớn hơn. Vì thế so với `DEBOUNCE_MS`=20ms thì đủ lớn để lọc cho các nút tactile thông thường.
2. Nếu `DEBOUNCE_MS` quá lớn, điều gì xảy ra khi người dùng nhấn nhả rất
   nhanh liên tiếp (double-click)?
- Khi `DEBOUNCE_MS` quá lớn sẽ làm mất đi các lần do lúc nhấn nhả rất nhanh liên tiêp (double-click) sẽ rơi vào thời gian khóa/chờ ổn định nên bị coi là nhiễu và bỏ qua thời gian đó. Từ đó làm hệ thống đọc sai số lần nhấn nút. Từ đó các chức năng phụ thuộc vào số lần nhấn nút sẽ hiển thị sai.
3. Khi callback của `debounce_timer` bị block 50 ms (thử nghiệm bước 4), bạn
   dự đoán `longpress_timer` sẽ bị trễ bao nhiêu? Giải thích bằng cơ chế
   daemon task + queue lệnh, không chỉ đoán số.
- Theo dự đoán thì `longpress_timer` sẽ bị trễ tối đa 50ms. Vì chỉ có daemon task chạy mọi hàm callback của software theo thứ tự hết hạn. Khi `debounce_timer` gọi `vTaskDelay(50)` thì daemon task sẽ bị block và không gọi hàm callback được. Nếu trong lúc này, `longpress_timer` hết hạn thì phải chờ callback return của task kia rồi mới được gọi. Ngoài ra, các lệnh xTimerStart/Reset/Stop không tác động trực tiếp mà được đẩy vào timer command queue, do đó khi queue đầy thì thời gian block sẽ lâu hơn nữa. 
4. `LONGPRESS_MS` của bạn là bao nhiêu? Giữ phím đúng ngưỡng này, bạn dự đoán
   callback `longpress_callback` chạy lần đầu ở thời điểm nào tính từ lúc
   nhấn xuống?
-  `LONGPRESS_MS` của mình là 600ms. Theo dự đoán thì callback  `longpress_callback` sẽ chạy lần đầu tiên lúc 600ms+ thời gian debounce + thời gian sau khi `debounce_timer` xác nhận= 600+20+50=670ms.