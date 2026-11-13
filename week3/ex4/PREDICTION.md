# Dự đoán — Week 3 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____

1. Chạy `app/deadlock_demo.c` y nguyên như được cấp. Dự đoán: hệ thống sẽ
   treo sau khoảng bao lâu (ngay lập tức, hay sau vài chu kỳ)? Vì sao không
   phải ngay lập tức (gợi ý: cần cả 2 task "đua" vào đúng thời điểm)?
- Khi chạy `app/deadlock_demo.c`, hệ thống sẽ treo sau chu kì đầu tiên. Vì hệ thống sẽ bị treo khi cả 2 task lấy được mutex đầu tiên của mình và chờ mutex thứ hai.
- Do đó giả sử cả 2 task đều ready, `task_alpha` thực thi trước và sau khi lấy mutex A rồi block thì `task_beta` cũng lấy mutex B rồi block. Cả 2 task đều chờ mutex kia của đối phương nhưng lại chưa give mutex của mình nên hệ thống sẽ bị treo trong chu kì đầu tiên. Ngoài ra có 1 số trường hợp có thể treo sau vài chu kì là khi có ISR hoặc task khác preempt thì thời gian treo sẽ tới lâu hơn.
2. Khi treo, `uxTaskGetSystemState` sẽ báo trạng thái gì cho `task_alpha` và
   `task_beta`? Dự đoán tên mutex mỗi task đang chờ (dựa trên đọc code, chưa
   chạy công cụ).
- Khi treo,  `uxTaskGetSystemState` sẽ báo trạng thái eBlocked. Vì cả 2 đều xin mutex với xTickToWaits =  portMAX_DELAY nên cả 2 task đều sẽ block cho tới khi có được mutex task kia trước khi trả mutex của chính mình. Task A đang chờ s_mutex_b còn task B đang chờ s_mutex_a. 
3. Tính reload value cho IWDG với `IWDG_TIMEOUT_MS` của bạn (LSI ~40 kHz).
- Ta có công thức: Reload value= (Tout*f_LSI)/(Prescaler)-1, chọn prescaler = 16, ta được: Reload value =(1*40000)/(16)-1=2499.
- Chọn prescaler=16 vì IWDG là bộ đếm 12 bit với giá trị cao nhất là 2^12-1=4096. Do đó không thể chọn prescaler dưới 16 vì sẽ làm vượt giá trị giới hạn của thanh ghi IWDG_RLR, còn nếu cao hơn thì độ phân giải bộ đếm sẽ giảm gây lệch. 
4. Sau khi sửa lỗi deadlock và thêm supervisor + IWDG, nếu bạn cố tình tái
   tạo lại tình huống deadlock cũ (tạm thời), dự đoán board sẽ phản ứng thế
   nào và sau bao lâu.
- Nếu cố tính tái tạo lại tình hướng deadlock cũ thì dự đoán board sẽ reset sau khoảng thời gian tối đa là 1s. Vì supervisor sẽ kiểm tra số bit theo chu kì, vì thế giả sử deadlock xảy ra ngay sau khi supervisor kick, lúc này bộ đếm vừa mới được nạp đầy thì thời gian board reset sẽ đúng bằng `IWDG_TIMEOUT_MS` =1s. Do khi không thấy supervisor phát tín hiệu thì IWDG sẽ đếm tới 0 và reset board.