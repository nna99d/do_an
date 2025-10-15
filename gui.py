import tkinter as tk
from tkinter import filedialog, messagebox
from PIL import Image, ImageTk
import subprocess
import time
import os

# Paths to compiled C executables
DNA_ENC = "en.exe"
DNA_DEC = "de.exe"

class DNAEncryptGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("DNA-Optical Hybrid Mã hóa/Giải mã")
        self.img_label = tk.Label(root)
        self.img_label.pack(padx=5, pady=5)

        tk.Button(root, text="Tải ảnh", command=self.load_image).pack(pady=2)
        tk.Button(root, text="Mã hóa", command=self.encrypt_image).pack(pady=2)
        tk.Button(root, text="Giải mã", command=self.decrypt_image).pack(pady=2)
        self.status_label = tk.Label(root, text="Status: Idle")
        self.status_label.pack(pady=5)

        self.img_path = None

    def load_image(self):
        path = filedialog.askopenfilename(filetypes=[("Dạng ảnh","*.png;*.jpg;*.jpeg")])
        if path:
            self.img_path = path
            img = Image.open(path)
            img.thumbnail((400,400))
            self.tkimg = ImageTk.PhotoImage(img)
            self.img_label.config(image=self.tkimg)
            self.status_label.config(text=f"Loaded {os.path.basename(path)}")

    def encrypt_image(self):
        if not self.img_path:
            messagebox.showwarning("Warning","No image loaded")
            return
        enc_file = filedialog.asksaveasfilename(defaultextension=".enc", filetypes=[("Encrypted","*.enc")])
        if not enc_file: return
        key_file = filedialog.asksaveasfilename(defaultextension=".bin", filetypes=[("Key","*.bin")])
        if not key_file: return

        start = time.time()
        proc = subprocess.run([DNA_ENC], input=f"{self.img_path}\n{enc_file}\n{key_file}\n", text=True, capture_output=True)
        end = time.time()
        self.status_label.config(text=f"Encrypted in {end-start:.3f}s")
        print(proc.stdout)
        messagebox.showinfo("Done","Encryption finished")

    def decrypt_image(self):
        enc_file = filedialog.askopenfilename(filetypes=[("Encrypted","*.enc")])
        if not enc_file: return
        key_file = filedialog.askopenfilename(filetypes=[("Key","*.bin")])
        if not key_file: return
        out_file = filedialog.asksaveasfilename(defaultextension=".png", filetypes=[("PNG","*.png")])
        if not out_file: return

        start = time.time()
        proc = subprocess.run([DNA_DEC], input=f"{enc_file}\n{key_file}\n{out_file}\n", text=True, capture_output=True)
        end = time.time()
        self.status_label.config(text=f"Decrypted in {end-start:.3f}s")
        print(proc.stdout)

        # Hiển thị ảnh giải mã
        if os.path.exists(out_file):
            img = Image.open(out_file)
            img.thumbnail((400,400))
            self.tkimg = ImageTk.PhotoImage(img)
            self.img_label.config(image=self.tkimg)
        messagebox.showinfo("Done","Decryption finished")

if __name__=="__main__":
    root = tk.Tk()
    gui = DNAEncryptGUI(root)
    root.mainloop()
