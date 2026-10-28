# Dự đoán — Week 2 / Ex3

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: __2413062____    `LOW_HOLD_MS` = __8__  `MID_WORK_MS` = __20__
`HIGH_PERIOD_MS` = __25__  `DEADLINE_MS` = __15__

1. Cấu hình binary semaphore: thời gian `task_high` bị block dự đoán = __28____ ms.
   Phép tính dựa trên đâu (liệt kê các thành phần cộng lại)?
- Khi cấu hình binary semaphore, sau khi task low take semaphore và task high take nên sẽ bị block. Trước khi task low give thì bị task mid preempt nên phải sau khoảng `MID_WORK_MS`, L có lại CPU và give thì khi đó task_high mới hết bị block. Vì thế ta có:
  task_high bị block= `LOW_HOLD_MS`+ `MID_WORK_MS`=20+8=28ms.
2. `task_high` có đạt `DEADLINE_MS` ở cấu hình binary semaphore không? Vì sao?
- Với cấu hình binary semaphore, `task_high` không thể đạt `DEADLINE_MS` được vì thời gian phản hồi của task_high= thời gian chờ + thời gian thực thi. Mà task_high bị block là 28ms >`DEADLINE_MS` rồi, chưa kể thời gian thực thi của task. Do đó không thể đạt `DEADLINE_MS` với cấu hình binary semaphore.
3. Cấu hình mutex: thời gian `task_high` bị block dự đoán = __8____ ms. Vì sao
   ngắn hơn (hoặc không) so với câu 1?
- Với cấu hình mutex thì task_high sẽ có thời gian bị block ngắn hơn so với binary semaphore. Vì cấu hình này, khi task_low take mutex thì độ ưu tiên của task này sẽ tạm thời được nâng lên, bằng với priority của task high. Do đó, task_mid sẽ không thể preempt do lúc này task có độ ưu tiên thấp hơn. Vì thế task_low vẫn sẽ thực thi và give, lúc đó task_high sẽ hết bị block. Thời gian block của task_high = `LOW_HOLD_MS`=8ms.    
4. Trên ảnh logic analyzer ở cấu hình binary semaphore, kênh `task_mid` sẽ ở trạng thái nào trong lúc `task_high` bị chặn — chạy liên tục, chạy ngắt quãng, hay không chạy? Vì sao?
-  Trong cấu hình binary semaphore, kênh  `task_mid` sẽ hoạt động liên tục trong lúc `task_high` bị chặn, vì khi đó task_high bị block. Trong lúc `task_mid` preempt từ task_low và chạy thì không có task nào có priority cao hơn mà ready nên task_mid sẽ thực thi task của mình liên tục.
5. Nếu đổi `task_mid` sang ưu tiên **thấp hơn** `task_low`, inversion còn xảy ra không? Vì sao (đây là cách phân biệt priority inversion với việc chỉ đơn giản là "task_low" chạy lâu").
- Khi đó priority inversion biến mất, vì lúc này task H chỉ phải chờ task_low chạy `LOW_HOLD_MS` thôi. Priority inversion là trong lúc tash high bị block, có 1 task có priority thấp hơn task_high, cao hơn task_low preempt.