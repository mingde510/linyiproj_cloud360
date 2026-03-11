import cv2
import numpy as np
from pathlib import Path
import queue
import time
from concurrent.futures import ThreadPoolExecutor
from tqdm import tqdm  
import os, stat, shutil
import tkinter as tk
from tkinter import filedialog
import sys  

def ask_input_folder() -> Path:
    """弹出对话框让用户选择监控目录，返回 Path 对象"""
    root = tk.Tk()
    root.withdraw()                       # 不显示主窗口
    root.update()                         # 防止 macOS 无响应
    folder = filedialog.askdirectory(
        title="请选择待检测图片所在文件夹",
        initialdir=os.getcwd()            # 默认打开当前工作目录
    )
    root.destroy()
    if not folder:                        # 用户点了“取消”
        print("未选择目录，程序退出。")
        sys.exit(0)
    return Path(folder)

# ==================== 配置区 ====================
MODEL_PATH = "best_person_fp16.engine"
TILE_SIZE = 640
OVERLAP = 80
CONF_THR = 0.5
# INPUT_FOLDER = r'D:\program\src'
MAX_WORKERS = 4        
BATCH_SIZE = 8
INPUT_FOLDER = ask_input_folder()
print("已选择监控目录：", INPUT_FOLDER)
RESULT_FOLDER = 'results_RT'
# ===============================================

Path(RESULT_FOLDER).mkdir(exist_ok=True)
model = YOLO(MODEL_PATH, task='detect')

pbar = tqdm(
    total=None,              # 总数未知，实时采集模式
    desc="实时检测中",
    unit="张",
    dynamic_ncols=True,
    bar_format="{l_bar}{bar}| {n_fmt}/{total_fmt} 张 | {rate_fmt}{postfix}"
)

def split_image_with_overlap(img, tile_size=640, overlap=80):
    h, w = img.shape[:2]
    tiles, positions = [], []
    stride = tile_size - overlap
    y = 0
    while y < h:
        x = 0
        while x < w:
            x2 = min(x + tile_size, w)
            y2 = min(y + tile_size, h)
            tile = img[y:y2, x:x2]
            pad_info = None
            if tile.shape[0] < tile_size or tile.shape[1] < tile_size:
                pad_tile = np.zeros((tile_size, tile_size, 3), dtype=np.uint8)
                pad_tile[:tile.shape[0], :tile.shape[1]] = tile
                tile = pad_tile
                pad_info = (tile.shape[1], tile.shape[0])
            tiles.append(tile)
            positions.append((x, y, pad_info))
            x += stride
        y += stride
    return tiles, positions
def is_file_stable(path, timeout=2.0, check_every=0.1):
    """文件大小在 timeout 秒内不再变化，认为已写完"""
    deadline = time.time() + timeout
    last = -1
    while time.time() < deadline:
        try:
            cur = path.stat().st_size
            if cur == last:          # 连续两次一样
                return True
            last = cur
        except FileNotFoundError:
            return False
        time.sleep(check_every)
    return False                     # 超时认为不安全

def safe_remove(path, max_retry=3):
    """反复尝试删除，仍然失败就移到 _trash 目录"""
    trash_dir = Path(INPUT_FOLDER) / '_trash'
    trash_dir.mkdir(exist_ok=True)
    for _ in range(max_retry):
        try:
            path.unlink()
            return
        except (PermissionError, OSError):
            time.sleep(0.2)
    # 删不掉就搬走
    try:
        shutil.move(str(path), str(trash_dir / path.name))
    except Exception:
        pass
def worker():
    while True:
        try:
            img_path = task_queue.get(timeout=1)
            if img_path is None:
                break

            start_time = time.time()
            t0 = time.time()
            t_read0 = time.time()
            img = cv2.imread(str(img_path))
            t_read1 = time.time()
            if img is None:
                task_queue.task_done()
                continue
            t_split0 = time.time()  
            tiles, positions = split_image_with_overlap(img, TILE_SIZE, OVERLAP)
            results = []
            t_split1 = time.time()

            # 单张推理循环（兼容固定batch engine）
            t_infer0 = time.time()

            for i in range(0, len(tiles), BATCH_SIZE):
                t_infer_tile0 = time.time()
                batch = tiles[i: i + BATCH_SIZE]
                r = model(batch, conf=CONF_THR, verbose=False)
                t_infer_tile1 = time.time()
                results.extend(r)
            t_infer1 = time.time()
            
            t_save0 = time.time()
            has_defect = any(len(r.boxes) > 0 for r in results)

            if has_defect:

                raw_save_path = Path(RESULT_FOLDER) / "raw" / f"{img_path.name}"
                raw_save_path.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(img_path, raw_save_path)      # 原图复制

                for (x, y, pad_info), r in zip(positions, results):
                    if len(r.boxes) == 0:
                        continue
                    boxes = r.boxes.xyxy.cpu().numpy()
                    cls = r.boxes.cls.cpu().numpy().astype(int)
                    confs = r.boxes.conf.cpu().numpy()
                    for box, c, cf in zip(boxes, cls, confs):
                        x1, y1, x2, y2 = map(int, box)
                        x1 += x; y1 += y; x2 += x; y2 += y
                        cv2.rectangle(img, (x1-15,y1-15), (x2+15,y2+15), (0,255,0), 2)
                        cv2.putText(img, f"{model.names[c]} {cf:.2f}",
                                    (x1, y1-10), cv2.FONT_HERSHEY_SIMPLEX, 0.9, (0,255,0), 2)

                vis_save_path = Path(RESULT_FOLDER) / "vis" / f"{img_path.name}"
                vis_save_path.parent.mkdir(parents=True, exist_ok=True)
                cv2.imwrite(str(vis_save_path), img)   # 画框图保存

                # ---------- 3. YOLO 标注文件 ----------
                H, W = img.shape[:2]
                label_save_path = Path(RESULT_FOLDER) / "labels" / f"{img_path.stem}.txt"
                label_save_path.parent.mkdir(parents=True, exist_ok=True)
                with open(label_save_path, "w") as f:
                    for (x, y, pad_info), r in zip(positions, results):
                        if len(r.boxes) == 0:
                            continue
                        boxes = r.boxes.xyxy.cpu().numpy()
                        cls = r.boxes.cls.cpu().numpy().astype(int)
                        for box, c in zip(boxes, cls):
                            x1, y1, x2, y2 = box
                            x1 += x; y1 += y; x2 += x; y2 += y
                            # 归一化
                            x_center = ((x1 + x2) / 2) / W
                            y_center = ((y1 + y2) / 2) / H
                            width    = (x2 - x1) / W
                            height   = (y2 - y1) / H
                            f.write(f"{c} {x_center:.6f} {y_center:.6f} {width:.6f} {height:.6f}\n")
            else:
                # 无瑕疵，直接保存原图
                raw_save_path = Path(RESULT_FOLDER) / "Nodefect" / f"{img_path.name}"
                raw_save_path.parent.mkdir(parents=True, exist_ok=True)
                shutil.copy2(img_path, raw_save_path)      # 原图复制
            t_save1 = time.time()
            # 删除原图
            # t_del0 = time.time()
            # try:
            #     safe_remove(img_path)
            # except:
            #     pass
            # t_del1 = time.time()
            print(f"""
------------------ 每张图片耗时统计 ------------------
总耗时:        {(t_save1 - t0)*1000:.1f} ms
读取图像:      {(t_read1 - t_read0)*1000:.1f} ms
切图耗时:      {(t_split1 - t_split0)*1000:.1f} ms
推理耗时:      {(t_infer1 - t_infer0)*1000:.1f} ms
推理切片耗时:  {(t_infer_tile1 - t_infer_tile0)*1000 / 8:.1f} ms
保存文件耗时:  {(t_save1 - t_save0)*1000:.1f} ms
----------------------------------------------------
""")
            elapsed = time.time() - start_time
            pbar.set_postfix({
                "当前": f"{elapsed*1000:5.1f}ms",
                "平均": f"{pbar.format_dict['rate']:.2f} it/s → {1000/pbar.format_dict['rate']:.1f}ms/张" if pbar.format_dict['rate'] else "-",
            })
            pbar.update(1)   # 进度条 +1

            task_queue.task_done()

        except queue.Empty:
            continue
        except Exception as e:
            print(f"\n错误: {e}")
            task_queue.task_done()


task_queue = queue.Queue(maxsize=200)
executor = ThreadPoolExecutor(max_workers=MAX_WORKERS)
for _ in range(MAX_WORKERS):
    executor.submit(worker)
seen = set()
pending = set()


try:
    start_time = time.time()
    while True:
        # ---- 1. 发现新文件 ----
        
        for p in Path(INPUT_FOLDER).rglob('*'):
            if p.suffix.lower() not in {'.jpg','.jpeg','.png','.bmp'}:
                continue
            if p in seen or p in pending:
                continue
            pending.add(p)

        # ---- 2. 检查 pending 是否已写完 ----
        stable = []
        for p in list(pending):
            if is_file_stable(p):
                stable.append(p)
                pending.remove(p)
            elif not p.exists():     # 拷贝过程中被用户撤了
                pending.remove(p)

        # ---- 3. 把稳定的文件送进队列 ----
        for p in stable:
            seen.add(p)
            try:
                task_queue.put(p, block=False)
            except queue.Full:
                pass

        # ---- 4. 清理 seen ----
        seen = {p for p in seen if p.exists()}
        time.sleep(0.5)

except KeyboardInterrupt:
    print("\n\n正在关闭...")
    end_time = time.time()
    print(f"本次运行总耗时: {end_time - start_time:.1f} s")
    for _ in range(MAX_WORKERS):
        task_queue.put(None)
    executor.shutdown(wait=True)
    pbar.close()
    print("已安全退出！")