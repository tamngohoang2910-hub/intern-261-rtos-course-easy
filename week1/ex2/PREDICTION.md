# Dự đoán — Week 1 / Ex2

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413062    Ưu tiên của tôi: A=__ 1 __ B=__ 3 __ C=__ 4 __

1. Bước 2 (`configUSE_PREEMPTION`=1, ưu tiên khác nhau): task nào chạy? task nào
   **không bao giờ** chạy? CPU share dự đoán:

   | Task | CPU share dự đoán (%) |
   |---|---|
   | A | 0 |
   | B | 0 |
   | C | 100 |

2. Bước 3 (`configUSE_PREEMPTION`=0): dự đoán thay đổi gì so với bước 2? Vì sao?
- So với bước 2 thì sẽ khác ở điểm là task đang chạy sẽ không bị tự động preempt chỉ vì có task ưu tiên cao hơn ready. Nếu Task C đang chạy và không yield, block thì vẫn chiếm CPU, task A,B vẫn bị starvation.
3. Bước 4 (3 task cùng ưu tiên): time slice dự đoán = __1____ ms. Dựa vào đâu?
- Dựa vào 1 tick =1/configTICK_RATE_HZ. Phải set configUSE_PREEMPTION = 1 và configUSE_TIME_SLICING = 1.
- Với cơ chế mỗi 1 tick interupt,scheduler sẽ chuyển sang task khác có cùng priority đang ready ( round-robin).
4. Bước 5: tôi sẽ sửa starvation bằng cách: __cho Task C và Task B gọi vTaskDelay____ . Cách này đánh đổi gì?
- Với cách làm này thì có thể làm cả 3 task thực hiện.
-Đánh đổi là task C và task B không chạy liên tục, sẽ có độ trễ do độ phân giải 1 tick, nên thời gian dậy của 2 task lệch tối đa 1 tick.
- Ngoài ra, khi B và C sau 1 khoảng thời gian block, khi 2 task này ready thì sẽ preempt task A ngay, vì thế task A sẽ không đảm bảo thời gian cứng.
5. Chân nào sẽ không bao giờ đổi trạng thái trên logic analyzer ở bước 2? ______
- Chân GPIO của Task A (PA0), Task B (PA5) sẽ không đổi trạng thái.