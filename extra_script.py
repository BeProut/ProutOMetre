Import("env")
import requests
from pathlib import Path

def post_program_action(source, target, env):
    print("Program has been built!")

    # Récupère le chemin absolu du firmware.bin
    program_path = Path(str(target[0].get_abspath()))
    bin_path = program_path.with_suffix('.bin')
    print(f"Bin path: {bin_path}")

    if not bin_path.is_file():
        print("Error: firmware binary not found.")
        env.Exit(1)

    # Lecture de la version depuis version.h
    version_h_path = Path("include/version.h")
    firmware_version = None
    if version_h_path.is_file():
        with open(version_h_path, "r") as f:
            for line in f:
                if "#define FIRMWARE_VERSION" in line:
                    firmware_version = int(line.strip().split()[-1])
                    break

    if firmware_version is None:
        print("Error: FIRMWARE_VERSION not found in version.h")
        env.Exit(1)

    print(f"Firmware version: {firmware_version}")

    # Envoi du firmware sur le serveur Hono
    url = f"http://178.16.130.164:3025/firmware/upload/{firmware_version}"
    with open(bin_path, 'rb') as f:
        files = {'file': (bin_path.name, f)}
        try:
            response = requests.post(url, files=files)
            response.raise_for_status()
            print(f"Firmware version {firmware_version} uploaded successfully")
        except requests.RequestException as e:
            print(f"Error uploading firmware: {e}")
            env.Exit(1)


# Ajoute l'action post-compilation sur le programme
env.AddPostAction("$BUILD_DIR/firmware.bin", post_program_action)
