# Dự đoán — Week 2 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____    `EVENT_BIT_COUNT` = ____  `SYNC_TIMEOUT_MS` = ____

1. RAM event group chiếm (bytes) dự đoán: ______ . Dựa vào đâu (cấu trúc dữ
   liệu nào trong FreeRTOS)?
2. RAM thêm khi dùng task notification cho 1 điểm đồng bộ: dự đoán = ______
   bytes. Vì sao?
3. Dự đoán thời gian đánh thức (producer gọi → sync_task chạy) của notification
   so với event-group-bit: nhanh hơn / chậm hơn / xấp xỉ? Vì sao (nghĩ đến số
   lớp hàm bên dưới mỗi API)?
4. Nếu `sync_task` bị timeout (`SYNC_TIMEOUT_MS` hết hạn) vì một producer
   không set bit kịp, `xEventGroupWaitBits` trả về gì? Làm sao phân biệt được
   "đủ bit" với "timeout"?
5. Nêu trước một tình huống bạn nghĩ **không thể** thay toàn bộ event group
   bằng notification — sẽ kiểm chứng lại ở REPORT.md.
