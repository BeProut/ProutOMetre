""" Remove version increment guard file if present """
import os
import requests
import hashlib
from SCons.Script import Import


Import("env")

def upload_firmware(source, target, env):
    firmware_path = str(target[0])
    # Calcul SHA256
    with open(firmware_path, "rb") as f:
        data = f.read()
        sha256 = hashlib.sha256(data).hexdigest()
    print(f"[INFO] SHA256 : {sha256}")
    VERSION_FILE = 'version'
    try:
        with open(VERSION_FILE, 'r') as f:
            version = f.read().strip()
    except Exception as e:
        print(f"[ERROR] Impossible de lire le fichier version: {e}")
        return
    # Upload vers le serveur
    post_url = "http://localhost:3000/prout-o-metre/firmwares"
    files = {"firmwareFile": open(firmware_path, "rb")}
    payload = {"version": version, "sha256": sha256}
    r = requests.post(post_url, data=payload, files=files)
    if r.status_code == 200:
        print("[SUCCESS] Firmware uploadé")
    else:
        print(f"[ERROR] Upload échoué : {r.status_code} {r.text}")

# Ajouter le hook post-build
env.AddPostAction("$BUILD_DIR/firmware.bin", upload_firmware)