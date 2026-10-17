"""Tính thống kê chu kỳ toggle từ CSV của Saleae Logic 2.
Xuất đúng 1 hàng bảng markdown:
| min (ms) | max (ms) | tb (ms) | jitter = max-min (us) | drift tổng (ms) |
"""
import argparse
import csv
import sys


def read_edges(path, channel):
    """Trả về danh sách thời điểm (s) mà kênh đổi trạng thái."""
    edges = []
    with open(path, newline="", encoding="utf-8-sig") as f:
        reader = csv.reader(f)
        header = next(reader)
        try:
            ci = header.index(channel)
        except ValueError:
            ci = 1  # mặc định cột thứ 2 sau cột Time
        prev = None
        for row in reader:
            if not row:
                continue
            t = float(row[0])
            v = int(float(row[ci]))
            if prev is not None and v != prev:
                edges.append(t)
            prev = v
    return edges


def main():
    ap = argparse.ArgumentParser(description="Thống kê chu kỳ toggle -> 1 hàng bảng")
    ap.add_argument("csv", help="file CSV xuất từ Logic 2")
    ap.add_argument("--target", type=float, default=17.0, help="PERIOD_MS (mặc định 17)")
    ap.add_argument("--n", type=int, default=0,
                    help="chỉ lấy N khoảng đầu, vd 1000 (0 = lấy hết)")
    ap.add_argument("--keep-first", action="store_true",
                    help="giữ khoảng đầu tiên (mặc định bỏ vì có thể bị cắt dở)")
    ap.add_argument("--channel", default="Channel 0", help="tên cột kênh")
    ap.add_argument("--label", default="", help="nhãn cấu hình, thêm làm cột đầu (tùy chọn)")
    ap.add_argument("--header", action="store_true", help="in kèm dòng tiêu đề bảng")
    args = ap.parse_args()

    edges = read_edges(args.csv, args.channel)
    if len(edges) < 3:
        sys.exit("Không đủ cạnh để tính (cần >= 3 lần chuyển trạng thái).")

    iv = [(edges[i + 1] - edges[i]) * 1e3 for i in range(len(edges) - 1)]  # ms
    if not args.keep_first:
        iv = iv[1:]
    if args.n > 0:
        iv = iv[: args.n]

    n = len(iv)
    mn, mx = min(iv), max(iv)
    mean = sum(iv) / n
    jitter_us = (mx - mn) * 1e3
    drift_ms = sum(iv) - n * args.target

    cols = [f"{mn:.4f}", f"{mx:.4f}", f"{mean:.4f}", f"{jitter_us:.3f}", f"{drift_ms:.2f}"]
    head = ["min (ms)", "max (ms)", "tb (ms)", "jitter = max−min (us)", "drift tổng (ms)"]
    if args.label:
        cols.insert(0, args.label)
        head.insert(0, "cấu hình")

    if args.header:
        print("| " + " | ".join(head) + " |")
        print("|" + "|".join(["---"] * len(head)) + "|")
    print("| " + " | ".join(cols) + " |")
    print(f"(N = {n} khoảng)", file=sys.stderr)


if __name__ == "__main__":
    main()