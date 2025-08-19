import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import time
import os

# Đường dẫn đến file C đã biên dịch (encrypt_decrypt.exe)
C_PROGRAM = "dna_optical.exe"  # đổi nếu tên exe khác

def encrypt_image():
    input_file = filedialog.askopenfilename(title="Chọn ảnh gốc", filetypes=[("Image files", "*.png;*.jpg;*.jpeg")])
    if not input_file:
        return
    output_file = filedialog.asksaveasfilename(title="Lưu ảnh mã hóa", defaultextension=".png", filetypes=[("PNG files", "*.png")])
    if not output_file:
        return
    key_file = filedialog.asksaveasfilename(title="Lưu file key", defaultextension=".txt", filetypes=[("Text files", "*.txt")])
    if not key_file:
        return

    start = time.time()
    try:
        # Chạy exe C với input/output/key
        subprocess.run([C_PROGRAM], input=f"1\n{input_file}\n{output_file}\n{key_file}\n", 
                       text=True, capture_output=True, check=True)
        elapsed = time.time() - start
        messagebox.showinfo("Hoàn tất", f"✅ Mã hóa thành công!\nThời gian: {elapsed:.3f} giây")
    except subprocess.CalledProcessError as e:
        messagebox.showerror("Lỗi", f"Lỗi khi chạy chương trình C:\n{e.stderr}")

def decrypt_image():
    input_file = filedialog.askopenfilename(title="Chọn ảnh đã mã hóa", filetypes=[("Image files", "*.png")])
    if not input_file:
        return
    output_file = filedialog.asksaveasfilename(title="Lưu ảnh giải mã", defaultextension=".png", filetypes=[("PNG files", "*.png")])
    if not output_file:
        return
    key_file = filedialog.askopenfilename(title="Chọn file key", filetypes=[("Text files", "*.txt")])
    if not key_file:
        return

    start = time.time()
    try:
        # Chạy exe C với input/output/key
        subprocess.run([C_PROGRAM], input=f"2\n{input_file}\n{output_file}\n{key_file}\n", 
                       text=True, capture_output=True, check=True)
        elapsed = time.time() - start
        messagebox.showinfo("Hoàn tất", f"✅ Giải mã thành công!\nThời gian: {elapsed:.3f} giây")
    except subprocess.CalledProcessError as e:
        messagebox.showerror("Lỗi", f"Lỗi khi chạy chương trình C:\n{e.stderr}")

# ================= GUI ==================
root = tk.Tk()
root.title("DNA Image Encryption - GUI")
root.geometry("400x200")

tk.Label(root, text="Chương trình Mã hóa Ảnh bằng DNA", font=("Arial", 12, "bold")).pack(pady=10)

tk.Button(root, text="🔒 Encrypt", width=20, height=2, bg="lightblue", command=encrypt_image).pack(pady=10)
tk.Button(root, text="🔓 Decrypt", width=20, height=2, bg="lightgreen", command=decrypt_image).pack(pady=10)

root.mainloop()
