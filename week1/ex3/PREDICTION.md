# Dự đoán — Week 1 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`:  2413062      `PERIOD_MS` = 17     `WORK_JITTER_US` =  1800    `SAMPLE_COUNT` =  1000

1. Bảng dự đoán (điền trước khi đo):

| # | Delay | Tick | Chu kỳ tb (ms) | Jitter (us) | Drift tổng sau SAMPLE_COUNT (ms) |
|---|---|---|---|---|---|
| 1 | `vTaskDelay` | 1000 | 17.444 | 1000 | 444 |
| 2 | `xTaskDelayUntil` | 1000 | 17 | 5 | 0 |
| 3 | `vTaskDelay` | 100 | 10 | 5 | 0 |
| 4 | `xTaskDelayUntil` |100 | 10 | 5 | 0 |

2. Cấu hình nào có drift lớn nhất? Vì sao, viết bằng công thức có `WORK_JITTER_US`.
- Cấu hình 1 ( vTaskDelay, f=1kHz) có drift lớn nhất, công thức: drift = SAMPLE_COUNT× (WORK_JITTER_US / 2 − T_tick / 2), drift = SAMPLE_COUNT × (WORK_JITTER_US − T_tick) / WORK_JITTER_US × T_tick. ( khi T_tick<=`WORK_JITTER_US`<2T_tick).
- Vì `WORK_JITTER_US` có thời gian thực hiện là 1.8ms > 1 tick, mà vTaskDelay = thời gian task thực hiện(làm tròn xuống theo tick) +  `PERIOD_MS` (ở đây là 17 tick). Do đó cấu hình 1 sẽ làm chu kì dài ra thêm 1 tick => drift lớn nhất.
- Đối với các cấu hình 2,4 do xTaskDelayUntil dùng mốc tuyệt đối nên không liên quan đến thời gian thực thi của task dài bao nhiêu. Còn cấu hình 3 do 1 tick =1/100=10ms, `WORK_JITTER_US` không dài hơn 10ms nên không gây drift.
3. `PERIOD_MS` = __17__ . Với tick 100 Hz, một tick = __10__ ms, nên chu kỳ thực tế sẽ
   bị làm tròn thành __10__ ms. Sai số hệ thống: __41.18__ %.
4. Jitter ở tick 1000 Hz so với 100 Hz: lớn hơn / nhỏ hơn / bằng? Vì sao?
- Với trường hợp tick 1kHz hay 100 Hz khi dùng xTaskDelayUntil sẽ chỉ có thể lệch vài us do trễ ISR nên bằng nhau.
- Với trường hợp tick 1kHz hay 100 Hz khi dùng vTaskDelay, cấu hình 1 (tick =1ms) sẽ có jitter lớn hơn so với cấu hình 3 ( tick=10ms) với tải từ 1ms < tải < 10ms. Vì khi tải trong khoảng này sẽ làm chu kì dài thêm 1 tick.
5. Nếu task này **không** phải ưu tiên cao nhất, đại lượng nào trong bảng bị ảnh hưởng
   nhiều nhất? 
- Đại lượng jitter bị ảnh hưởng nhiều nhất, do phải chờ các task ưu tiên cao chạy xong, mà thời gian chờ khác nhau.
- Với xTaskDelayUntil, do cách này lấy tick tuyệt đối nên drift vẫn xấp xỉ 0.