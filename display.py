import struct
import tkinter as tk
from tkinter import filedialog, messagebox
from PIL import Image, ImageTk
import numpy as np
import os

HDR_SIZE = 16  # sizeof(enc_header_t) in dna_en.c (packed)

def read_enc_file(path):
    """
    Đọc file .enc theo header struct trong dna_en.c (little-endian).
    Trả về (PIL.Image, (width,height,channels)) hoặc raise Exception.
    """
    with open(path, "rb") as f:
        hdr = f.read(HDR_SIZE)
        if len(hdr) < HDR_SIZE:
            raise ValueError("File quá nhỏ, không có header hợp lệ.")
        # unpack header: '<I I I B 3s' -> magic, width, height, channels, reserved(3)
        magic, width, height, channels, _ = struct.unpack("<I I I B 3s", hdr)
        # Kiểm tra magic (0x4F414E44)
        if magic != 0x4F414E44:
            # có thể file endian khác hoặc file không phải format; vẫn tiếp tục nhưng cảnh báo
            raise ValueError(f"Magic mismatch: 0x{magic:08X} (không phải 0x4F414E44). Có thể file không hợp lệ.")
        if width <= 0 or height <= 0 or channels not in (1, 3, 4):
            raise ValueError(f"Header không hợp lệ: width={width}, height={height}, channels={channels}")

        expected = width * height * channels
        data = f.read()
        if len(data) < expected:
            raise ValueError(f"Dữ liệu ciphertext không đủ: cần {expected} bytes nhưng file chỉ chứa {len(data)} bytes.")
        # lấy đúng số byte cần thiết
        buf = np.frombuffer(data[:expected], dtype=np.uint8)
        # reshape thành H x W x C
        try:
            arr = buf.reshape((height, width, channels))
        except Exception as e:
            raise ValueError(f"Không thể reshape buffer: {e}")

        # Nếu channels == 1, chuyển thành mode "L"; nếu 3 -> "RGB"; 4 -> "RGBA"
        if channels == 1:
            mode = "L"
            pil_img = Image.fromarray(arr.reshape((height, width)), mode=mode)
        elif channels == 3:
            mode = "RGB"
            pil_img = Image.fromarray(arr, mode=mode)
        else:
            mode = "RGBA"
            pil_img = Image.fromarray(arr, mode=mode)

        return pil_img, (width, height, channels)

class EncViewer:
    def __init__(self, root):
        self.root = root
        root.title("Encrypted .enc Viewer — dna_en format")
        root.geometry("900x600")

        self.img_label = tk.Label(root, text="Open a .enc file to view the encrypted image", bd=2, relief="sunken")
        self.img_label.pack(fill="both", expand=True, padx=8, pady=8)

        frm = tk.Frame(root)
        frm.pack(fill="x", padx=8, pady=6)

        btn_open = tk.Button(frm, text="Open .enc", command=self.open_enc)
        btn_open.pack(side="left", padx=4)

        btn_save = tk.Button(frm, text="Save as .png", command=self.save_png)
        btn_save.pack(side="left", padx=4)

        btn_info = tk.Button(frm, text="File Info", command=self.show_info)
        btn_info.pack(side="left", padx=4)

        self.status = tk.Label(root, text="No file loaded", anchor="w")
        self.status.pack(fill="x", padx=8, pady=(0,8))

        self.current_image = None
        self.current_path = None
        self.current_shape = None
        self.tk_image = None

    def open_enc(self):
        path = filedialog.askopenfilename(filetypes=[("Encrypted files", "*.enc"), ("All files","*.*")])
        if not path:
            return
        try:
            pil_img, shape = read_enc_file(path)
        except Exception as e:
            messagebox.showerror("Error reading .enc", str(e))
            return
        self.current_image = pil_img
        self.current_path = path
        self.current_shape = shape
        self._display_image(pil_img)
        self.status.config(text=f"Loaded: {os.path.basename(path)} — {shape[0]}x{shape[1]} channels={shape[2]}")

    def _display_image(self, pil_img):
        # Fit image into label area while preserving aspect ratio
        lbl_w = self.img_label.winfo_width() or 800
        lbl_h = self.img_label.winfo_height() or 500
        # Compute max size leaving some padding
        max_w = max(100, lbl_w - 20)
        max_h = max(100, lbl_h - 20)
        w, h = pil_img.size
        scale = min(max_w / w, max_h / h, 1.0)
        if scale < 1.0:
            new_size = (int(w * scale), int(h * scale))
            disp = pil_img.resize(new_size, Image.NEAREST)
        else:
            disp = pil_img
        self.tk_image = ImageTk.PhotoImage(disp)
        self.img_label.config(image=self.tk_image, text="")

    def save_png(self):
        if self.current_image is None:
            messagebox.showwarning("No image", "Please open a .enc file first.")
            return
        suggested = os.path.splitext(os.path.basename(self.current_path))[0] + "_enc.png"
        out = filedialog.asksaveasfilename(defaultextension=".png", initialfile=suggested,
                                           filetypes=[("PNG image", "*.png"), ("All files","*.*")])
        if not out:
            return
        try:
            # Save raw encrypted image as PNG for visualization
            self.current_image.save(out, format="PNG")
            messagebox.showinfo("Saved", f"Encrypted image saved as {out}")
        except Exception as e:
            messagebox.showerror("Save error", str(e))

    def show_info(self):
        if not self.current_path:
            messagebox.showinfo("Info", "No file loaded")
            return
        w,h,c = self.current_shape
        messagebox.showinfo("File info", f"Path: {self.current_path}\nSize: {w} x {h}\nChannels: {c}")

if __name__ == "__main__":
    root = tk.Tk()
    app = EncViewer(root)
    # ensure the label has initial size so resizing works
    root.update_idletasks()
    root.minsize(480, 320)
    root.mainloop()
