import tkinter as tk
import math
import colorsys
import random

# 1. 初始化主窗口并隐藏
root = tk.Tk()
root.withdraw()

# 2. 获取屏幕分辨率
screen_w = root.winfo_screenwidth()
screen_h = root.winfo_screenheight()

# ---------- 核心参数（按你的要求调整） ----------
win_w, win_h = 125, 65 
num_heart_points = 100       # 爱心窗口数量
num_fullscreen_points = 380  # 【调整】满屏弹窗数量增加到180，更加饱满！

scale_x = (screen_w / 2) / 17
scale_y = (screen_h / 2) / 12
scale = int(min(scale_x, scale_y) * 0.55) 
# ----------------------------------------

# 3. 【调整】更贴近爱情与温情的四字祝福语
phrases = [
    "白头偕老", "满心欢喜", "偏爱是你", "共度余生",
    "心跳为你", "相思入骨", "温柔以待", "好好生活",
    "顺顺利利", "你是唯一", "怦然心动", "朝朝暮暮",
    "爱你老己", "相濡以沫", "满眼是你", "何其有幸",
    "携手余生", "我喜欢你", "温暖相伴", "眷恋如初"
]

# 4. 计算爱心坐标
heart_points = []
for i in range(num_heart_points):
    t = math.pi * 2 * i / num_heart_points
    x = 16 * math.sin(t) ** 3
    y = 13 * math.cos(t) - 5 * math.cos(2 * t) - 2 * math.cos(3 * t) - math.cos(4 * t)
    pos_x = int(screen_w / 2 + x * scale - win_w / 2)
    pos_y = int(screen_h / 2 - (y + 6) * scale - win_h / 2)
    heart_points.append((pos_x, pos_y, i))

# 5. 全局窗口列表和安全退出机制
heart_windows = []
fullscreen_windows = []

def close_all(event=None):
    for w in heart_windows + fullscreen_windows:
        try: w.destroy()
        except: pass
    root.destroy()

root.bind("<Escape>", close_all)

# 6. 通用创建窗口函数
def create_window(pos_x, pos_y, text, color_hex, is_center=False):
    win = tk.Toplevel(root)
    win.geometry(f"{win_w}x{win_h}+{pos_x}+{pos_y}")
    win.configure(bg=color_hex)
    win.resizable(False, False)    
    win.attributes("-topmost", True)
    win.title("") 
    
    font_size = 15 if is_center else 11
    label = tk.Label(win, text=text, bg=color_hex, fg="white", 
                     font=("Microsoft YaHei", font_size, "bold"))
    label.pack(expand=True)

    win.protocol("WM_DELETE_WINDOW", close_all)
    return win

# ==================== 阶段 1：一笔画爱心（放慢） ====================
def draw_heart(index=0):
    if index >= len(heart_points):
        # 爱心画完，停留 1.25 秒，然后开始消散
        root.after(1250, dissolve_heart)
        return
    
    pos_x, pos_y, i = heart_points[index]
        # --- 修改：随机低饱和颜色 ---
    hue = random.random()               # 随机色相
    sat = random.uniform(0.4, 0.6)      # 低饱和度（0.4~0.6，越低越柔和）
    val = random.uniform(0.8, 0.95)     # 高亮度（保证白色文字清晰可见）
    r, g, b = colorsys.hsv_to_rgb(hue, sat, val)
    color_hex = f'#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}'
    # ----------------------------

    if abs(pos_x - screen_w / 2) < win_w / 2 and abs(pos_y - screen_h / 2) < win_h / 2:
        win = create_window(pos_x, pos_y, "好好爱己", color_hex, is_center=True)
    else:
        win = create_window(pos_x, pos_y, random.choice(phrases), color_hex)
        
    heart_windows.append(win)
    
    # 【调整】延迟从 20-40 改为 45-75，爱心生成更慢，更优雅
    root.after(random.randint(35, 65), draw_heart, index + 1)

# ==================== 阶段 2：爱心快速消散 ====================
def dissolve_heart():
    if not heart_windows:
        root.after(300, start_fullscreen)
        return
    
    win = heart_windows.pop(random.randint(0, len(heart_windows)-1))
    try: win.destroy()
    except: pass
    
    # 【调整】延迟从 30 改为 12，消散更快，干脆利落
    root.after(12, dissolve_heart)

# ==================== 阶段 3：满屏弹窗（更饱满） ====================
def start_fullscreen():
    for _ in range(num_fullscreen_points):
        rx = random.randint(0, screen_w - win_w)
        ry = random.randint(0, screen_h - win_h)
        
        # 满屏弹窗使用低饱和度的温柔配色
        hue = random.random()
        r, g, b = colorsys.hsv_to_rgb(hue, 0.5, 0.9) 
        color_hex = f'#{int(r*255):02x}{int(g*255):02x}{int(b*255):02x}'
        
        fullscreen_windows.append((rx, ry, color_hex))

    # 【调整】弹出间隔从 10 毫秒改为 4 毫秒，瞬间铺满屏幕，显得更饱满
    pop_next_fullscreen_window(0)

def pop_next_fullscreen_window(index):
    if index >= len(fullscreen_windows):
        root.after(1000, dissolve_fullscreen) # 停留 4 秒
        return
    
    rx, ry, color_hex = fullscreen_windows[index]
    win = create_window(rx, ry, random.choice(phrases), color_hex)
    fullscreen_windows[index] = win
    
    root.after(10, pop_next_fullscreen_window, index + 1)

# ==================== 阶段 4：满屏弹窗缓缓散去（最终单次收尾） ====================
def dissolve_fullscreen():
    if not fullscreen_windows:
        # 【调整】只需要一次循环：全部散去后，延迟1秒直接退出程序，不循环了！
        root.after(1000, root.destroy)
        return
    
    win = fullscreen_windows.pop(random.randint(0, len(fullscreen_windows)-1))
    if isinstance(win, tk.Toplevel):
        try: win.destroy()
        except: pass
    
    # 10 毫秒消散一个，缓缓散去
    root.after(4, dissolve_fullscreen)

# 启动程序！
root.after(300, draw_heart)

# 运行
root.mainloop()