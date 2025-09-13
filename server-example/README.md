# Installation et Test du Serveur Audio

## Prérequis
- Node.js (version 14 ou plus récente)
- npm

## Installation
```bash
cd server-example
npm install
```

## Démarrage
```bash
npm start
```
ou pour le développement avec rechargement automatique :
```bash
npm run dev
```

## Test
Le serveur sera accessible sur `http://192.168.0.18:3000`

### Tester manuellement
```bash
# Test de base
curl http://192.168.0.18:3000/

# Upload d'un fichier test
curl -X POST \
  -H "Content-Type: audio/wav" \
  --data-binary @test.wav \
  http://192.168.0.18:3000/audio/upload

# Lister les enregistrements
curl http://192.168.0.18:3000/recordings
```

## Structure des fichiers
```
server-example/
├── server.js          # Serveur principal
├── package.json        # Dépendances
├── uploads/           # Dossier des enregistrements (créé automatiquement)
└── README.md          # Ce fichier
```

## Logs
Le serveur affiche des logs détaillés pour chaque upload :
```
Réception d'un fichier audio...
Taille: 54432 octets
Content-Type: audio/wav
Fichier sauvegardé: ./uploads/recording-2025-09-13T10-30-45-123Z.wav
```
