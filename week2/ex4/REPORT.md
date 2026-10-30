# Report — Week 2 / Ex4

`student_id`: ___2413062___    Commit của PREDICTION.md: ______

## 1. Bảng so sánh RAM
| | Bytes thực tế | Cách tính |
|---|---|---|
| Event group (3 điểm đồng bộ) | 72 | 24*3 |
| Event group (2 điểm) + notification (1 điểm) | 48 | 2 × 24 + 0 |
| Chênh lệch | 24| do notification không tốn RAM nên chênh lệch bằng đúng phần heap bị chiếm của 1 event group |

## 2. Bảng so sánh tốc độ đánh thức
| Cách | Thời gian đánh thức (cycle) | Thời gian đánh thức (us) |
|---|---|---|
| Event-group-bit (producer_1 hoặc 2) | 1137 | 15.8 |
| Task notification (producer_0) | 945 | 13.1 |

Mô tả cách đo (2 điểm DWT: lúc gọi set/notify, lúc sync_task bắt đầu chạy):
- Ta dùng bộ đếm DWT với thời gian t0 là thời gian được đọc tại xTaskNotifyGive() (producer_0) hoặc xEventGroupSetBits() (producer 1-3), t1 là thời gian được đọc trong sync_task: ulTaskNotifyTake() cho notification hoặc xEventGroupWaitBits()` cho event group.
## 3. Khi nào KHÔNG dùng được notification
Tình huống bạn dự đoán ở PREDICTION.md, kiểm chứng lại: đúng hay sai, vì sao.
-  Kiểm chứng lại đúng là event group sẽ phù hợp với tình huống 1 sự kiện và nhiều task chờ cùng lúc. Kiểm chứng: khi tạo 2 task cùng chờ notification, chỉ gửi noti cho task A thì chỉ có task A được đánh thức, task B timeout vẫn chưa được đánh thức. Khi gửi noti cho cả 2 task thì cả 2 task mới được đánh thức.
## 4. Timeout
Log/ảnh một lần `sync_task` bị timeout (cố tình làm 1 producer chậm hơn
`SYNC_TIMEOUT_MS`), và giá trị trả về của `xEventGroupWaitBits` lúc đó.
Log 1 lần sync_task bị timeout:
SYNC #44: t=8537 ms, period=190 ms, lat=1132 cycles
SYNC TIMEOUT: ret=0x7, mask=0xF, thieu=0x8
SYNC #45: t=8740 ms, period=203 ms, lat=1132 cycles
SYNC TIMEOUT: ret=0x7, mask=0xF, thieu=0x8
SYNC #46: t=8942 ms, period=202 ms, lat=1132 cycles
SYNC TIMEOUT: ret=0x7, mask=0xF, thieu=0x8
- Giá trị trả về của `xEventGroupWaitBits` lúc đó là 0x7, khi timeout mà vẫn chưa đủ producer nên event group vẫn giữ các bit trước đó.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân vật lý.
|mục| lí thuyết | đo được|chênh lệch|nguyên nhân|
|---|---|---|---|---|
|RAM event group|24|24|0|dự đoán chính xác các trường trong biến struct của event group|
|RAM thêm khi dùng task notification cho 1 điểm đồng bộ|0|0|0|do notification không tạo thêm vùng nhớ nào mà chúng có sẵn trong TCB của task đó|
|thời gian đánh thức của notification so với event group| nhanh hơn | nhanh hơn 2.7us|---|event group không biết task nào chờ nên phải kiểm tra điều kiện từng task trong khi notification lại không cần mà đi thẳng vào vùng TCB của task|

## 6. Câu hỏi đánh đổi
Với `EVENT_BIT_COUNT` của **bạn**, nếu tăng thêm 1 bit (1 producer nữa) thì chu
kỳ đồng bộ trung bình đo được thay đổi thế nào? Trả lời bằng số từ dữ liệu của
bạn, rồi đo lại để kiểm chứng.
- Khi tăng thêm 1 bit thì chu kì đồng bộ trung bình sẽ tăng lên.
- Kiểm chứng:
|`EVENT_BIT_COUNT`|Chu kỳ trung bình đo được |
|4| 24.8889ms |
|5| 26.26667ms |
=> chênh lệch 1.3778ms, do đó khi tăng thêm 1 producer nữa thì sẽ tăng chu kì đồng bộ trung bình do phải chờ thêm thời gian của 1 producer nữa mới đồng bộ được cả 5.