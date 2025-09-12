import os
import hashlib
import requests
from SCons.Script import Import

Import("env")

# -------------------
# Pré-build : récupérer la version
# -------------------
version_url = "http://localhost:3000/prout-o-metre/firmwares/new-build-version"
resp = requests.get(version_url)
resp.raise_for_status()
version = resp.json()["version"]
print(f"[INFO] Version récupérée : {version}")

# Injecter dans le code
env.Append(CPPDEFINES=[("FW_VERSION", f'"{version}"')])