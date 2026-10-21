# Dự đoán — Week 1 / Ex4

> **Commit TRƯỚC khi nạp chương trình.**

`student_id`: 2413062    `HEAP_SIZE_BYTES` = 9216   `DYN_TASK_STACK_WORDS` = 160
`STATIC_TASK_STACK_WORDS` = 160     `HOG_DEPTH_BYTES` = 512    `TARGET_FREE_HEAP_BYTES` = 4096

1. Ngân sách RAM dự đoán (tổng phải = 20480 bytes):

| Mục | Bytes dự đoán |
|---|---|
| `.data` | 100 |
| `.bss` (không kể heap FreeRTOS) | 4300 |
| Heap FreeRTOS (`ucHeap`) | 9216 |
| Stack của main (MSP) | 1024  |
| Còn lại / chưa dùng | 5840 |
| **Tổng** | **20480** |

1. `xPortGetFreeHeapSize()` ngay sau khi scheduler chạy: __8456__ bytes.
   `HEAP_SIZE_BYTES` − số này = __760__ . Số đó là của những gì?
   -stack của dyn_task : align8(160x4+8)=648
   -TCB của dyn_task: align8(92 + 8) = 104.
   -Khi dùng cách heap 4 sẽ cần 8 byte cuối để marker giúp allocator nhận biết đến cuối list. 
   - Vì chỉ có dyn_task dùng heap, idle, timer, stat_task không dùng vì cấp phát tĩnh.
2. High water mark dự đoán (words): dyn __125__ , stat __125__ , idle __100__ , timer __205__ .
3. `dyn_task` có stack __160__ words = __640__ bytes. Mảng `HOG_DEPTH_BYTES` = __512__ bytes.
   Hook tràn stack **có** chạy không? Vì sao?
   - Hook tràn stack có thể chạy vì lượng stack dyn_task sử dụng sau khi thực hiện mảng HOG thì vẫn còn các phần như vTaskDelay() hay dữ liệu về ngữ cảnh của CPU lưu vào stack khi context switch nên có thể tàn stack. Khi context switch mà stack vẫn bị tràn thì hook báo tràn sẽ chạy. Sẽ có trường hợp là lỗi CPU hoặc bị treo trước đó thì hook có thể không chạy.
4. Heap trong `.map` và `xPortGetFreeHeapSize()` sẽ lệch nhau. Lệch vì:
   - Heap trong .map cho biết kích thước vùng RAM dành cho heap là bao nhiêu. Còn khi gọi hàm xPortGetFreeHeapSize() sẽ cho biết vùng heap còn trống bao nhiêu tại thời điểm gọi hàm. 
5. Stack nhỏ nhất an toàn của `dyn_task` dự đoán: __200__ words. Dựa vào đâu?
   - Mảng HOG của task và các thanh ghi của mảng này lưu vào stack : 512+16 (các thanh ghi lr,r7,..),có thể là các hàm như vTaskDelay() khoảng 32-80 byte. Khi context switch cũng cần lưu lại trạng thái vào ngắt PendSV ( 16 thanh ghi ) nên khoảng 64 byte nên khoảng 160 word. Nhưng để an toàn nhất thì ta nên gap 1 khoảng nên sẽ là từ 160*1.25=200 word.
