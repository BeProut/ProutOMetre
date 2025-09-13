# Améliorations Audio ProutOMètre 🔊

Ce projet contient des améliorations significatives pour obtenir un audio plus propre et de meilleure qualité.

## 🎯 Améliorations 2. **Compatibilité** : Le code fonctionne avec les microphones I2S standards (INMP441, MAX9814, etc.)
3. **Mémoire** : Les améliorations utilisent ~2 KB de RAM supplémentaire
4. **Qualité vs Taille** : L'augmentation du sample rate augmente la taille des fichiers de ~37%
5. **JavaScript pur** : Le serveur amélioré fonctionne sans aucune dépendance externe
6. **FFmpeg optionnel** : Le serveur basique et JavaScript fonctionnent sans FFmpeg ESP32

### Configuration I2S optimisée
- **Fréquence d'échantillonnage** : Augmentée à 22,050 Hz (vs 16 kHz)
- **Buffers DMA** : 16 buffers de 256 échantillons (réduction des dropouts)
- **APLL activé** : Améliore la précision du timing

### Traitement audio en temps réel
1. **Filtre passe-haut** (85 Hz)
   - Supprime les bruits de fond basse fréquence
   - Élimine la composante DC parasites

2. **Contrôle automatique de gain (AGC)**
   - Normalise automatiquement le volume
   - Temps d'attaque/relâchement configurables
   - Évite la saturation

3. **Gain fixe** (option alternative)
   - Amplification simple avec gain configurable
   - Plus prévisible que l'AGC

### Configuration modifiable dans `audio_config.h`
```cpp
#define I2S_SAMPLE_RATE 22050        // Fréquence d'échantillonnage
#define AUDIO_GAIN 2.0               // Gain fixe
#define HIGH_PASS_CUTOFF 85          // Filtre passe-haut (Hz)
#define AUTO_GAIN_ENABLED true       // AGC on/off
#define TARGET_VOLUME 0.7            // Volume cible AGC
```

## 🖥️ Améliorations côté serveur

### Option 1: Serveur JavaScript pur (`js-enhanced-server.js`) ⭐ RECOMMANDÉ
**Aucune dépendance externe requise** - Traitement audio en JavaScript pur :

1. **Filtre passe-haut** (85 Hz) - Supprime les parasites basse fréquence
2. **Réduction de bruit** simple - Filtre moyenneur pour lisser le signal
3. **Normalisation du volume** - Volume uniforme et optimal
4. **Égaliseur** - Améliore la clarté vocale (boost ~2 kHz)
5. **Analyse RMS** - Métriques de qualité audio

### Option 2: Serveur avec FFmpeg (`enhanced-server.js`)
Utilise **FFmpeg** pour un traitement audio professionnel (nécessite installation)

### Nouvelles fonctionnalités
- Sauvegarde des fichiers originaux ET traités
- Analyse des métriques audio
- API de téléchargement des fichiers
- Interface REST complète

## 🚀 Installation et utilisation

### Côté ESP32
1. Flashez le code modifié
2. Ajustez les paramètres dans `audio_config.h` si nécessaire
3. Le traitement se fait automatiquement

### Côté serveur

#### Serveur JavaScript pur (RECOMMANDÉ - sans FFmpeg)
```bash
cd server-example
npm install
npm run js-enhanced
```

#### Serveur simple (sans traitement)
```bash
cd server-example
npm install
npm start
```

#### Serveur avec FFmpeg (si disponible)
```bash
# Installer FFmpeg d'abord
# Windows: winget install FFmpeg
# macOS: brew install ffmpeg  
# Linux: sudo apt install ffmpeg

cd server-example
npm install
npm run enhanced
```

## 📊 API Endpoints

### POST `/audio/upload`
Upload et traitement automatique des fichiers audio

### GET `/recordings`
Liste tous les enregistrements (originaux + traités)

### GET `/download/:filename`
Télécharge un fichier original

### GET `/download/processed/:filename`
Télécharge un fichier traité

## 🔧 Configuration avancée

### Paramètres ESP32 dans `audio_config.h`
```cpp
// Qualité audio
#define I2S_SAMPLE_RATE 22050     // 22kHz pour meilleure qualité
#define I2S_DMA_BUF_COUNT 16      // Plus de buffers = moins de dropouts
#define I2S_DMA_BUF_LEN 256       // Taille optimale des buffers

// Traitement audio
#define AUDIO_GAIN 2.0            // Gain fixe (si AGC désactivé)
#define HIGH_PASS_CUTOFF 85       // Fréquence de coupure passe-haut
#define AUTO_GAIN_ENABLED true    // Contrôle automatique de gain

// AGC (si activé)
#define TARGET_VOLUME 0.7         // Volume cible (0.0 à 1.0)
#define AGC_ATTACK_TIME 0.1       // Temps d'attaque (secondes)
#define AGC_RELEASE_TIME 0.5      // Temps de relâchement (secondes)
```

### Traitement JavaScript (serveur)
Le serveur JavaScript utilise des algorithmes de traitement audio purs :
```
Parsing WAV → Filtre passe-haut → Réduction bruit → Égaliseur → Normalisation
```

Avantages :
- ✅ Aucune dépendance externe
- ✅ Installation simple
- ✅ Traitement rapide
- ✅ Contrôle total sur les algorithmes

### Traitement FFmpeg (optionnel)
Le serveur FFmpeg utilise une chaîne de filtres optimisée :
```
highpass=f=85 → loudnorm → afftdn → acompressor → equalizer
```

## 📈 Résultats attendus

### Améliorations audio
- ✅ Suppression des bruits de fond
- ✅ Volume normalisé et constant
- ✅ Meilleure clarté vocale
- ✅ Réduction des parasites électroniques
- ✅ Qualité plus professionnelle

### Performance
- ⚡ Traitement temps réel sur ESP32
- ⚡ Traitement batch côté serveur
- ⚡ Pas d'impact significatif sur la latence

## 🛠️ Troubleshooting

### ESP32
- **Audio trop fort/faible** : Ajustez `AUDIO_GAIN` ou activez l'AGC
- **Trop de bruit** : Vérifiez le câblage I2S et la masse
- **Coupures audio** : Augmentez `I2S_DMA_BUF_COUNT`

### Serveur
- **Audio trop fort/faible** : Le serveur JavaScript normalise automatiquement
- **Pas d'amélioration** : Vérifiez que le fichier WAV est valide
- **Erreurs de parsing** : Assurez-vous que l'ESP32 génère un WAV standard

### FFmpeg (optionnel)
- **FFmpeg non trouvé** : Utilisez le serveur JavaScript (`npm run js-enhanced`)
- **Fichiers non traités** : Vérifiez les logs d'erreur FFmpeg
- **Performance** : Le traitement FFmpeg prend 1-2 secondes par fichier

## 📝 Notes importantes

1. **Compatibilité** : Le code fonctionne avec les microphones I2S standards (INMP441, MAX9814, etc.)
2. **Mémoire** : Les améliorations utilisent ~2 KB de RAM supplémentaire
3. **Qualité vs Taille** : L'augmentation du sample rate augmente la taille des fichiers de ~37%
4. **FFmpeg optionnel** : Le serveur basique fonctionne sans FFmpeg

Profitez de votre audio amélioré ! 🎵
