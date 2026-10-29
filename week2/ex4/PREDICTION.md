# Dự đoán — Week 2 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____    `EVENT_BIT_COUNT` = __4__  `SYNC_TIMEOUT_MS` = __3__

1. RAM event group chiếm (bytes) dự đoán: __24____ . Dựa vào đâu (cấu trúc dữ
   liệu nào trong FreeRTOS)?
- event group là 1 biến struct nằm trong RAM nên ta đếm kích thước các trường trong đó.
- Trong struct EventGroupDef_t gồm uxEventBits chứa các bit sự kiện và cùng kiểu với TickType_t nên sẽ có 4 byte.
- xTasksWaitingForBits chiêm 20 byte bao gồm: uxNumberOfItems 4 byte ( đếm lúc này có bao nhiêu task đang chờ), pxIndex 4 byte (con trỏ đang trỏ đang trỏ vào đâu trong List_t),xListEnd 12 byte (phần tử đánh dấu cuối danh sách). Tổng cộng tối thiểu 24byte
2. RAM thêm khi dùng task notification cho 1 điểm đồng bộ: dự đoán = __0____
   bytes. Vì sao?
- Vì vùng nhớ cho notification nằm sẵn trong TCB của mọi task từ khi task được tạo nên sẽ không tốn thêm dung lượng nào nữa do không tạo thêm đối tượng nào. 
3. Dự đoán thời gian đánh thức (producer gọi → sync_task chạy) của notification
   so với event-group-bit: nhanh hơn / chậm hơn / xấp xỉ? Vì sao (nghĩ đến số
   lớp hàm bên dưới mỗi API)?
- Thời gian đánh thức của notification sẽ nhanh hơn event-group-bit vì khi dùng notification sẽ đi thẳng vào task nhận sau đó sẽ ghi vào TCB của sync_task.
- Event-group-bit sẽ chậm hơn do không biết task nào chờ cả nên phải kiểm tra điều kiện của từng task trong danh sách chờ => tốn bước hơn nên sẽ có thời gian dài hơn.
4. Nếu `sync_task` bị timeout (`SYNC_TIMEOUT_MS` hết hạn) vì một producer
   không set bit kịp, `xEventGroupWaitBits` trả về gì? Làm sao phân biệt được
   "đủ bit" với "timeout"?
- Khi sync_task timeout, `xEventGroupWaitBits` sẽ trả về các giá trị bit đang có trong event group tại thời điểm timeout.
- Để phân biệt được đủ bit và timeout (tức đã timeout nhưng chưa đủ bit), ta sẽ lấy giá trị trả về & với mask bit rồi so sánh, nếu bằng với mask bit thì "đủ bit", không thì "timeout".
5. Nêu trước một tình huống bạn nghĩ **không thể** thay toàn bộ event group
   bằng notification — sẽ kiểm chứng lại ở REPORT.md.
- Em nghĩ tình huống là 1 sự kiện và có nhiều task chờ cùng lúc. Vì notification chỉ đánh thức đúng 1 task còn event group thì nhiều hơn. Không thể thay toàn bộ bằng notification vì nếu làm vậy thì bên producer(task gửi sẽ phải biết mọi task nhận), event group thì tách hẳn giữa producer và task nhận.
- Event group thì bit vẫn bật nên task tới sau vẫn có thể thấy và thực thi tiếp trong khi noti chỉ gửi task tại thời điểm gọi, sau khi task đọc xong sẽ mất. 