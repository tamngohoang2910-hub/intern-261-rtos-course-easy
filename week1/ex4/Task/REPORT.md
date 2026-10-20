# Report — Week 1 / Ex4

`student_id`: 2413062    Commit của PREDICTION.md: ______

## 1. Ngân sách RAM từ `.map`
| Mục | Symbol / section trong `.map` | Bytes |
|---|---|---|
| `.data` | .data | 96 |
| `.bss` (không kể heap) |.bss | 2856 |
| Heap FreeRTOS (`ucHeap`) | .bss.ucHeap | 9216 |
| Stack của main (MSP) | _Min_Heap_Size | 1024 |
| Còn lại / chưa dùng | — | 7288 |
| **Tổng** | | **20480** |

`.map` commit tại: `Task.map`

## 2. Số lúc chạy (dán nguyên văn log UART)
--- FREERTOS RAM REPORT ---
HEAP free=7568 B, min_ever=7568 B
Heap target >= 4096 B: PASS
TCB size: 76 bytes
dyn HWM: 131 words, 524 bytes
stat HWM: 132 words, 528 bytes
ram_report HWM: 44 words, 176 bytes
idle HWM: 109 words, 436 bytes
timer HWM: 85 words, 340 bytes

| Đại lượng | Words | Bytes |
|---|---|---|
| `xPortGetFreeHeapSize()` | — | 7568  |
| `xPortGetMinimumEverFreeHeapSize()` | — | 7568 |
| HWM `dyn_task` | 131 |524 |
| HWM `stat_task` | 132 | 528 |
| HWM `ram_report_task` | 44 | 176 |
| HWM idle task | 109 | 436 |
| HWM timer task | 85 | 340 |

Heap còn trống >= `TARGET_FREE_HEAP_BYTES` (__4096__)? __Từ kết quả của Uart thì free heap là 7568 byte.__

## 3. Đối chiếu `.map` vs runtime
Heap trong `.map` = __9216__ ; `xPortGetFreeHeapSize()` = __7568__ ; lệch = __1648__ bytes.
Lệch này gồm những gì (liệt kê từng mục kèm số bytes):
- 640 
- 
## 4. Dynamic vs static
HWM của `dyn_task` và `stat_task` lệch __1__ words dù làm cùng một việc. Vì sao?
- Sau lệch này là do căn lề stack 8 byte, do dyn_task có stack động nằm trong heap, do đó địa chỉ căn 8 nên đỉnh bị làm tròn mất 1 word so với stat_task.
`xTaskCreate` và `xTaskCreateStatic` khác nhau ở đâu về **nơi** TCB và stack nằm?
- Do trong hàm xTaskCreate có hàm pvPortMalloc() nên TCB và stack được cấp từ ở vùng heap của RTOS trong khi xTaskCreateStatic thì stack và TCB đều nằm trong vùng .bss do stack và TCB là biến toàn cục do mình khai báo.
## 5. Tràn stack
Log/ảnh chứng minh hook đã chạy:
```
--- FREERTOS RAM REPORT ---
HEAP free=7568 B, min_ever=7568 B
Heap target >= 4096 B: PASS
TCB size: 76 bytes
dyn HWM: 150 words, 600 bytes
stat HWM: 151 words, 604 bytes
ram_report HWM: 44 words, 176 bytes
idle HWM: 118 words, 472 bytes
timer HWM: 85 words, 340 bytes
STACK OVERFLOW, name: dyn task
```
Tại sao trong `vApplicationStackOverflowHook` không được gọi hàm UART blocking
(hoặc: nếu bạn vẫn gọi được, giải thích tại sao nó không crash — và tại sao đó là may):
- Không nên gọi hàm Uart blocking trong hàm vApplicationStackOverflowHook vì lúc vào hàm này là khi task đang bị tràn, hệ thống nên ưu tiên báo lỗi, khắc phục sự cố rồi dừng thay vì chạy tiếp. 
- Mình vẫn gọi được UART blocking trong vApplicationStackOverflowHook có thể phần tràn không chạm đến phần 'huart2'.
## 6. Stack nhỏ nhất an toàn
| Lần thử | Stack (words) | HWM (words) | Kết quả |
|---|---|---|---|
| 1 | 160 | 150 | overflow |
| 2 | 161 | 152 | overflow |
| 2 | 163 | 5 | normal |

Giá trị cuối cùng chọn: __163__ words. Biên an toàn: __163__ words. Lý do chọn biên đó:
- Vì 163 words là giá trị tối thiểu khi có mảng HOG_DEPTH_BYTE mà không bị tràn overflow. 

## 7. Lệch so với dự đoán
Mỗi dòng lệch + nguyên nhân.
| Mục      | Dự đoán   | Thực tế   | Lệch     |Nguyên nhân|
|----------|-----------|-----------|----------|-----------|
|.data|100|96|4|lệch 4 bytes do có một biến global/static được khởi tạo, chiếm 4 bytes RAM|
|.bss (không kể heap FreeRTOS)|4300|2856| mình không dự đoán được các biến và các hàm trong các file |
| Heap FreeRTOS (`ucHeap`) |9216|9216|0|cùng bằng configTOTAL_HEAP_SIZE|
| Stack của main (MSP) |1024|1024|0|cùng bằng_Min_Stack_Size|
| Còn lại|5840| 7288 | sai lệch chủ yếu từ vùng.bss |

## 8. Câu hỏi đánh đổi
Với `HEAP_SIZE_BYTES` = __9216__ của bạn: nếu phải thêm một task nữa có stack 128 words,
**lấy RAM từ đâu**? Dựa vào bảng ngân sách của bạn, đưa ra 2 phương án kèm chi phí
cụ thể (bytes), rồi chọn một và **thực hiện, đo lại để chứng minh**.
- Có 2 phương án khi thêm 1 task là cấp phát động và cấp phát tĩnh. Nếu dùng cấp phát tĩnh thì sẽ dùng vùng nhớ .bss gồm 76 byte + 128 word byte cho stack của task.
- Nếu dùng cấp phát động thì sẽ lấy RAM trong vùng heap. Chi phí trong heap: stack align8(512 + 8) = 520 B với 76 byte .bss.

--- FREERTOS RAM REPORT ---
HEAP free=6960 B, min_ever=6960 B
Heap target >= 4096 B: PASS
TCB size: 76 bytes
dyn HWM: 131 words, 524 bytes
stat HWM: 132 words, 528 bytes
ram_report HWM: 44 words, 176 bytes
idle HWM: 118 words, 472 bytes
timer HWM: 85 words, 340 bytes

- Ta thấy vùng heap sẽ có thêm 20 byte so với dự đoán ( 96 so với 76). Lệch ấy là do khi dùng heap 4 thì sẽ có thêm vùng overhead để quản lí các vùng nhớ.