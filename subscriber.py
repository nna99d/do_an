import paho.mqtt.client as mqtt
import hashlib
import json
import struct

BROKER = "1ce52fa3e7184ba5bd3f13aa4b86e418.s1.eu.hivemq.cloud"
PORT   = 8883
USER   = "esp32cam"
PASS   = "Abc123@x"

TOPIC_META = "esp32cam/image/meta"
TOPIC_DATA = "esp32cam/image/data"

meta = None
img_buf = bytearray()
expected_len = 0

# ROTL/ROTR 
def rotl8(x, r): return ((x << r) & 0xFF) | (x >> (8-r))
def rotr8(x, r): return ((x >> r) | ((x << (8-r)) & 0xFF)) & 0xFF

# DNA encode/decode
def toDNA(v):
    return "ACGT"[v & 3]

def fromDNA(c):
    return {"A":0,"C":1,"G":2,"T":3}[c]

def dnaTransform(b, k):
    dna = [toDNA((b >> (2*i)) & 3) for i in range(4)]
    mode = k & 3
    if mode == 1: # complement
        dna = [{"A":"T","T":"A","C":"G","G":"C"}[c] for c in dna]
    elif mode == 2: # reverse
        dna = dna[::-1]
    elif mode == 3: # swap pairs
        dna = [dna[1],dna[0],dna[3],dna[2]]
    out=0
    for i in range(4):
        out |= (fromDNA(dna[i]) << (2*i))
    return out

def dnaInverse(b, k):
    mode = k & 3
    dna = [toDNA((b >> (2*i)) & 3) for i in range(4)]
    if mode == 1: # complement
        dna = [{"A":"T","T":"A","C":"G","G":"C"}[c] for c in dna]
    elif mode == 2: # reverse
        dna = dna[::-1]
    elif mode == 3: # swap pairs
        dna = [dna[1],dna[0],dna[3],dna[2]]
    out=0
    for i in range(4):
        out |= (fromDNA(dna[i]) << (2*i))
    return out

#  MQTT callbacks
def on_connect(client, userdata, flags, rc):
    print("Connected with rc:", rc)
    client.subscribe(TOPIC_META)
    client.subscribe(TOPIC_DATA)

def on_message(client, userdata, msg):
    global meta, img_buf, expected_len

    if msg.topic == TOPIC_META:
        meta = json.loads(msg.payload.decode())
        expected_len = meta["len"]
        img_buf = bytearray()
        print("[META] len:", expected_len)
        print("       key:", meta["key"])
        print("       hash:", meta["hash"])

    elif msg.topic == TOPIC_DATA and meta:
        img_buf.extend(msg.payload)
        print(f"[DATA] received {len(img_buf)}/{expected_len}")
        if len(img_buf) >= expected_len:
            print("[+] Image received, start decrypt...")

            # convert hex key -> bytes
            key = bytes.fromhex(meta["key"])
            # verify SHA-256
            if hashlib.sha256(key).hexdigest() != meta["hash"]:
                print("[-] Key SHA256 mismatch!")
                return

            # decrypt
            out = bytearray()
            for i, c in enumerate(img_buf):
                k = key[i % len(key)]
                tmp = c ^ k
                d   = rotr8(tmp, k & 7)
                p   = dnaInverse(d, k)
                out.append(p)

            with open("recv.jpg","wb") as f:
                f.write(out)
            print("[+] Image saved as recv.jpg")
            img_buf.clear()

# Run 
client = mqtt.Client()
client.username_pw_set(USER, PASS)
client.tls_set()   # enable TLS
client.on_connect = on_connect
client.on_message = on_message

client.connect(BROKER, PORT, 60)
client.loop_forever()
