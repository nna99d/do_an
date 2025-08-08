import tkinter as tk
from tkinter import filedialog, messagebox
import subprocess
import time
import os

# Đường dẫn tới file .exe (chỉnh lại cho phù hợp)
EXE_PATH = "dna_optical.exe"

def encrypt():
    input_file = filedialog.askopenfilename(title="Chọn ảnh PGM gốc", filetypes=[("PGM files", "*.pgm")])
    if not input_file:
        return
    output_file = filedialog.asksaveasfilename(title="Lưu ảnh mã hóa", defaultextension=".pgm", filetypes=[("PGM files", "*.pgm")])
    if not output_file:
        return
    key_file = filedialog.asksaveasfilename(title="Lưu file key", defaultextension=".txt", filetypes=[("Text files", "*.txt")])
    if not key_file:
        return

    start_time = time.time()
    subprocess.run([EXE_PATH], input=f"1\n{input_file}\n{output_file}\n{key_file}\n", text=True)
    elapsed = time.time() - start_time

    messagebox.showinfo("Mã hóa hoàn tất", f"Ảnh mã hóa đã lưu: {output_file}\nKey: {key_file}\nThời gian: {elapsed:.4f} giây")

def decrypt():
    input_file = filedialog.askopenfilename(title="Chọn ảnh PGM đã mã hóa", filetypes=[("PGM files", "*.pgm")])
    if not input_file:
        return
    output_file = filedialog.asksaveasfilename(title="Lưu ảnh giải mã", defaultextension=".pgm", filetypes=[("PGM files", "*.pgm")])
    if not output_file:
        return
    key_file = filedialog.askopenfilename(title="Chọn file key", filetypes=[("Text files", "*.txt")])
    if not key_file:
        return

    start_time = time.time()
    subprocess.run([EXE_PATH], input=f"2\n{input_file}\n{output_file}\n{key_file}\n", text=True)
    elapsed = time.time() - start_time

    messagebox.showinfo("Giải mã hoàn tất", f"Ảnh giải mã đã lưu: {output_file}\nThời gian: {elapsed:.4f} giây")

# Giao diện Tkinter
root = tk.Tk()
root.title("DNA-Optical Image Encryption/Decryption")
root.geometry("400x200")

tk.Label(root, text="Chương trình Mã hóa & Giải mã Ảnh DNA-Optical", font=("Arial", 12, "bold")).pack(pady=10)

tk.Button(root, text="Mã hóa ảnh", font=("Arial", 11), command=encrypt, width=20).pack(pady=10)
tk.Button(root, text="Giải mã ảnh", font=("Arial", 11), command=decrypt, width=20).pack(pady=10)

tk.Button(root, text="Thoát", font=("Arial", 10), command=root.quit, width=10).pack(pady=10)

root.mainloop()
