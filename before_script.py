Import("env")
import requests
from pathlib import Path

def pre_build_action(source, target, env):
    # Adresse de ton serveur Hono
    server_url = "http://localhost:3000/firmware/next-id"
    print("------------------------------------")
    print("Fetching next firmware version from server...")
    print("------------------------------------")
    try:
        # Récupère le next_id depuis l'API
        response = requests.get(server_url)
        response.raise_for_status()
        data = response.json()
        next_id = data.get("next_id")

        if not next_id:
            print("Erreur : next_id non fourni par le serveur.")
            env.Exit(1)

        print(f"Next firmware version from server: {next_id}")

        # Génère version.h
        version_h_path = Path("include/version.h")
        version_h_path.parent.mkdir(parents=True, exist_ok=True)
        with open(version_h_path, "w") as f:
            f.write("#ifndef VERSION_H\n#define VERSION_H\n\n")
            f.write(f"#define FIRMWARE_VERSION {next_id}\n")
            f.write("#endif // VERSION_H\n")

        print(f"Generated {version_h_path}")

    except requests.RequestException as e:
        print(f"Erreur lors de la récupération de next_id : {e}")
        env.Exit(1)

env.AddPreAction("$PROGPATH", pre_build_action)

