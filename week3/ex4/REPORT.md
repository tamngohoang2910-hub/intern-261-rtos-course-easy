# Report — Week 3 / Ex4

`student_id`: __2413062____    Commit của PREDICTION.md: __13/11/2026____

## 1. Bảng CPU %
| Task | CPU % (lần đo 1) | CPU % (lần đo 2) |
|---|---|---|
| RuntimeStats | 2% | <1% |
| IDLE   | <1% | 98% |
| Supervisor  | 1% | <1% |
| Alpha  |	1% |<1% |
|Beta | 2% |<1% |

Giải thích Idle task chiếm bao nhiêu % và vì sao.
- Idle task khi đo lần 1 chiếm dưới 1% vì lúc này có 2 task có priority cao hơn thực thi. Lần đo 2 do 2 task ALpha và Beta deadlock nên bị lock và Supervisor task cũng vì thế mà block nên IDLE task sẽ có %CPU lên 98% (do không còn task nào có priority cao hơn ready).
## 2. Bằng chứng tìm deadlock
Log/ảnh `uxTaskGetSystemState` **trước khi sửa**: trạng thái từng task, chỉ
rõ `task_alpha`/`task_beta` đang `eBlocked` và đang chờ mutex nào. Giải thích
chính xác thứ tự lấy mutex nào gây ra deadlock (AB-BA).
- Log:
--- uxTaskGetSystemState ---
RuntimeStats state=Running prio=1
IDLE         state=Ready prio=0
Supervisor   state=Blocked prio=4
Alpha        state=Blocked prio=3
Beta         state=Blocked prio=3
--- mutex holders ---
mutex_A held by: Alpha
mutex_B held by: Beta
- Do task alpha có priority cao nên sẽ lấy mutex A trước, sau đó block 5ms. Task B có CPU sẽ lấy mutex B sau đó block. Task Alpha dậy và muốn lấy mutex B mà task Beta đang giữ, do đó task Alpha block chờ task Beta nhả mutex và task Beta cũng chờ task Alpha nhả mutex A. 
## 3. Sửa lỗi
Mô tả thay đổi bạn làm để hết deadlock (thứ tự lấy mutex thống nhất). Xác
nhận chạy ổn định sau khi sửa (thời gian chạy thử).
- Để hết deadlock, ta phải thống nhất thứ tự lấy mutex. Với bài này, mình cho task A và task B cùng lấy mutex A trước sau đó mới lấy mutex B. Nếu đổi như vậy thì task Alpha có mutex trước sẽ thực thi, task Beta chờ mutex A nên block cho tới khi task Alpha nhả mutex A. Tương tự như vậy với mutex B, từ đó sẽ không còn hiện tượng deadlock nữa.
- Log uart sau khi sửa:
  ===== TASK RUNTIME STATS (lan do 1) =====
Task Name       Abs Time       CPU %
RuntimeStats   1669             2%
IDLE           0                <1%
Alpha          1263             1%
Supervisor     1711             2%
Beta           2523             3%


===== TASK RUNTIME STATS (lan do 2) =====
Task Name       Abs Time       CPU %
RuntimeStats   1442089          <1%
IDLE           143214491        98%
Alpha          309140           <1%
Beta           370134           <1%
Supervisor     106703           <1%


===== TASK RUNTIME STATS (lan do 3) =====
Task Name       Abs Time       CPU %
RuntimeStats   3006077          1%
IDLE           286380970        98%
Alpha          618005           <1%
Beta           737810           <1%
Supervisor     211695           <1%
- Sau 3 lần đo thì ta thấy Alpha và Beta task vẫn đang chạy, thể hiện qua Abs Time của chúng tiếp tục tăng.
## 4. Watchdog
Phép tính reload IWDG. Log/video board tự reset khi deadlock được tái tạo lại
có chủ đích, kèm thời gian từ lúc treo đến lúc reset — so với `IWDG_TIMEOUT_MS`.
- Ta có công thức tính reload value: Reload = (T_ms × f_LSI) / (1000 × Prescaler) − 1, chọn T_ms = `IWDG_TIMEOUT_MS`, Prescaler=16 =>Reload= 2499.
- Log:
    4680.5 ms  START
    4685.5 ms  ===== TASK RUNTIME STATS (lan do 1) =====
    4688.5 ms  Task Name       Abs Time       CPU %
    4691.5 ms  RuntimeStats     1681            1%
    4695.1 ms  IDLE             0               <1%
    4696.1 ms  Alpha            46354           40%
    4698.4 ms  Beta             1289            1%
    4700.4 ms  Supervisor       1730            1%
    5647.4 ms  FW v7 boot
    5648.4 ms  RESET: IWDG
- Thời gian tính từ lúc treo đến lúc reset = thời điểm RESET IWDG - thời điểm START (từ chu kì CPU lần 2, lúc task Alpha xin mutex B )= 4680.5 - 5648.4 = 967.9 ms. Sai số khoảng 3.21% so với `IWDG_TIMEOUT_MS`.
## 5. Lệch so với dự đoán
Mỗi mục lệch + nguyên nhân.
| Mục lệch | Dự đoán | Đo được | Lệch | Sai số| Nguyên nhân|
|---|---|---|---|---|---|
|thời gian từ lúc treo đến lúc reset| 1000| 967.9 ms|32.1ms|3.21%| Do LSI sai lệch |
|thời gian từ lúc treo đến lúc reset| 1000| 967.9 ms|32.1ms|3.21%| Do LSI sai lệch |
## 6. Câu hỏi đánh đổi
Với `SUPERVISOR_BITS` và chu kỳ check-in của bạn, nếu một task thật sự chạy
chậm (không deadlock, chỉ chậm) trong đúng 1 chu kỳ, supervisor có false-trip
(reset oan) không? Trả lời bằng số đo thời gian thực tế của chu kỳ check-in.
- Nếu có 1 task chỉ chạy chậm trong đúng 1 chu kì và không phải deadlock thì supervisor không false-trip do trong 1 chu kì của 2 task khoảng 25ms, còn thời gian `IWDG_TIMEOUT_MS` =1000ms.
- Log:  3147.5 ms  ===== TASK RUNTIME STATS (lan do 5) =====
    3150.1 ms  Task Name       Abs Time       CPU %
    3153.0 ms  RuntimeStats     6962071         1%
    3156.2 ms  IDLE             572645768               98%
    3158.7 ms  Alpha            1398883         <1%
    3162.2 ms  Beta             1394950         <1%
    3163.3 ms  Supervisor       440900          <1%
    3167.2 ms  Alpha checkin: avg=25 ms max=55 ms (n=323)
    5171.9 ms  ===== TASK RUNTIME STATS (lan do 6) =====
    5175.0 ms  Task Name       Abs Time       CPU %
    5177.8 ms  RuntimeStats     8809602         1%
    5180.1 ms  IDLE             715803989               98%
    5182.9 ms  Alpha            1770764         <1%
    5186.0 ms  Beta             1708032         <1%
    5188.1 ms  Supervisor       550185          <1%
    5193.7 ms  Alpha checkin: avg=25 ms max=55 ms (n=404)
- Ta thấy chu kì Alpha task chậm thêm 1 chu kì (55ms) thì không có reset lần nào.