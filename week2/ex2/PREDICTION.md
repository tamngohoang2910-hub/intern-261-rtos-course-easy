# Dự đoán — Week 2 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____    `WRITER_COUNT` = __3__   `MSG_LEN` = __24__

1. Ở `UART_GUARD_MODE = 0`: dự đoán lỗi lẫn lộn sẽ xuất hiện sau khoảng bao
   nhiêu dòng log? Vì sao MSG_LEN và WRITER_PERIOD_MS của bạn ảnh hưởng đến
   con số này?
- Với khung truyền 8N1 thì số dòng log lỗi lẫn lộn sẽ xuất hiện: N=WRITER_PERIOD_MS/((WRITER_COUNT-1)*T_line), với T_line=MSG_LEN × 10 / baudrate =>N =1. Tức là dòng đầu tiên sẽ bị lỗi.
- MSG_LEN có ảnh hưởng vì khi độ dài message càng lớn sẽ làm tăng khả năng lẫn lộn.
- WRITER_PERIOD_MS cũng có thể gây ảnh hưởng vì chúng quyết định tần suẩt task log dày hay thưa. Khi giá trị này giảm thì tần suất các task gặp nhau sẽ dày hơn dẫn đến tăng khả năng lỗi lẫn lộn hơn.
2. Dự đoán thứ tự **độ trễ in** (latency) từ thấp đến cao giữa mutex / critical
   section / gatekeeper. Vì sao?
- Dự đoán theo mình từ thấp đến cao là critical section, gatekeeper và mutex.
- Với critical section do có khả năng chặn các task cũng như ngắt nên thời gian latency thấp nhất do chỉ chạy đúng lệnh in.
- Với gatekeeper do phải gửi dữ liệu yêu cầu xử lí cho gatekeeper task sau đó mới xử lí nên sẽ lâu hơn critical section.
- Với mutex sẽ có 2 trường hợp: 1 là khi task có mutex trước khi in thì thời gian latency sẽ xấp xỉ thời gian in, 2 là task chưa có mutex khi gọi khi in thì phải chờ khi nào có mutex thì mới có thể thực thi.
3. Với critical section, task nào trong hệ thống có nguy cơ bị ảnh hưởng xấu
   nhất nếu đoạn in quá dài? (gợi ý: nghĩ đến ngắt, không chỉ task)
- Theo mình, task ISR UART RX sẽ có nguy cơ ảnh hưởng xấu nhất. Do uart trong mcu stm32f103 chỉ có 1 thanh ghi DR chứa được 1 byte dữ liệu nên khi lệnh in quá dài trong critical section thì sẽ gây mất dữ liệu nghiêm trọng. Ví dụ với thông số của mình là T_uart=4.17ms, khi lệnh in trong section dài bằng T_byte, ISR của UART TX bị chặn nhưng cứ mỗi 174us thì có 1 byte gửi vào DR. Do lúc này ngắt bị vô hiệu mà thanh ghi DR chỉ có thể chứa 1 byte mà liên tục nhận dữ liệu mỗi 174us thì dữ liệu sẽ bị ghì đè. Dẫn đến sau khi hết thời gian critical sẽ chỉ còn 1 byte cuối trong thanh ghi DR.
4. RAM dùng thêm của gatekeeper so với mutex: dự đoán tăng hay giảm, vì sao?
- Gatekeeper sẽ dùng thêm nhiều RAM hơn so với mutex vì cách này sẽ phải tạo gatekeeper task trong heap gồm (TCB, stack của task, vùng điều khiển queue và vùng lưu các phần tử đang chờ). Còn mutex chỉ cần 1 khối điều khiển semaphore chứ không cần tạo task mới cũng như là vùng lưu dữ liệu.
5. Nếu `WRITER_COUNT` tăng gấp đôi, cách nào trong 3 cách bị ảnh hưởng về độ trễ
   nhiều nhất? Vì sao?
- Khi `WRITER_COUNT` tăng gấp đôi thì cách mutex sẽ bị ảnh hưởng về độ trễ nhiều nhất. Để thực hiện task của mình thì phải chờ có mutex mới làm được, mà càng nhiều task thì càng phải chờ lâu.
