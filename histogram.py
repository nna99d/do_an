import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
import struct
import math

# ====== Đọc file .enc do dna_en.c tạo ra ======
def read_enc(path):
    with open(path,'rb') as f:
        hdr = f.read(16)  # header = 16 bytes (I I I B 3s)
        magic,width,height,channels,_ = struct.unpack('<I I I B 3s', hdr)
        data = f.read()
    arr = np.frombuffer(data, dtype=np.uint8)
    arr = arr[:width*height*channels]
    return arr.reshape((height, width, channels)), (width,height,channels)

# ====== Tính histogram và các chỉ số ======
def hist_and_metrics(img):
    H,W,C = img.shape
    N = H*W
    res = {}
    for ch in range(C):
        vals = img[:,:,ch].ravel()
        Hbins, _ = np.histogram(vals, bins=256, range=(0,256))
        p = Hbins / Hbins.sum()
        uniform = np.ones(256) / 256.0
        E = N / 256.0
        chi2 = ((Hbins - E)**2 / E).sum()
        eps = 1e-12
        kl = np.sum(p * np.log2((p+eps) / uniform))
        tv = 0.5 * np.sum(np.abs(p - uniform))
        res[ch] = {'chi2':chi2, 'kl':kl, 'tv':tv}
    return res

# ====== Vẽ histogram ảnh gốc vs mã hóa ======
def plot_histograms(orig_img, enc_img, save_prefix=None):
    for ch,name in zip([0,1,2], ['R','G','B']):
        plt.figure(figsize=(10,4))
        plt.subplot(1,2,1)
        plt.title(f'Original {name}')
        plt.hist(orig_img[:,:,ch].ravel(), bins=256, range=(0,255), color=name.lower())
        plt.subplot(1,2,2)
        plt.title(f'Encrypted {name}')
        plt.hist(enc_img[:,:,ch].ravel(), bins=256, range=(0,255), color=name.lower())
        plt.tight_layout()
        if save_prefix:
            plt.savefig(f"{save_prefix}_hist_{name}.png")
        plt.show()

# ====== Thí nghiệm ======
# Đọc ảnh gốc
orig = np.array(Image.open("dog.jpg").convert("RGB"))  # thay test.png bằng ảnh của bạn
# Đọc ảnh mã hóa
enc_img, shape = read_enc("en_out.enc")  # thay test.enc bằng ảnh đã mã hóa

# Tính chỉ số
print("Ảnh gốc:")
print(hist_and_metrics(orig))
print("Ảnh mã hóa:")
print(hist_and_metrics(enc_img))

# Vẽ histogram
plot_histograms(orig, enc_img, save_prefix="result")
