const express = require('express');
const fs = require('fs');
const path = require('path');

const app = express();
const port = 3000;

// Middleware pour parser les données brutes
app.use('/audio/upload', express.raw({ type: 'audio/wav', limit: '10mb' }));

// Classe pour le traitement audio en JavaScript pur
class AudioProcessor {
    constructor() {
        this.sampleRate = 22050;
    }

    // Lire l'en-tête WAV et extraire les données audio
    parseWAVFile(buffer) {
        const dataView = new DataView(buffer.buffer, buffer.byteOffset, buffer.byteLength);

        // Vérifier l'en-tête RIFF
        const riff = String.fromCharCode(...buffer.slice(0, 4));
        if (riff !== 'RIFF') {
            throw new Error('Fichier WAV invalide: en-tête RIFF manquant');
        }

        // Trouver le chunk "data"
        let offset = 12; // Après RIFF header
        let dataOffset = 0;
        let dataSize = 0;
        let sampleRate = 22050;
        let channels = 1;
        let bitsPerSample = 16;

        while (offset < buffer.length - 8) {
            const chunkType = String.fromCharCode(...buffer.slice(offset, offset + 4));
            const chunkSize = dataView.getUint32(offset + 4, true);

            if (chunkType === 'fmt ') {
                // Format chunk
                sampleRate = dataView.getUint32(offset + 12, true);
                channels = dataView.getUint16(offset + 10, true);
                bitsPerSample = dataView.getUint16(offset + 18, true);
            } else if (chunkType === 'data') {
                // Data chunk
                dataOffset = offset + 8;
                dataSize = chunkSize;
                break;
            }

            offset += 8 + chunkSize;
        }

        if (dataOffset === 0) {
            throw new Error('Chunk data non trouvé dans le fichier WAV');
        }

        // Extraire les données audio (16-bit PCM)
        const audioData = new Int16Array(dataSize / 2);
        for (let i = 0; i < audioData.length; i++) {
            audioData[i] = dataView.getInt16(dataOffset + i * 2, true);
        }

        return {
            audioData,
            sampleRate,
            channels,
            bitsPerSample,
            duration: audioData.length / sampleRate
        };
    }

    // Filtre passe-haut simple (1er ordre)
    applyHighPassFilter(audioData, cutoffFreq = 85) {
        const dt = 1.0 / this.sampleRate;
        const rc = 1.0 / (2.0 * Math.PI * cutoffFreq);
        const alpha = rc / (rc + dt);

        const filtered = new Int16Array(audioData.length);
        let y1 = 0;
        let x1 = 0;

        for (let i = 0; i < audioData.length; i++) {
            const x = audioData[i] / 32768.0; // Normaliser
            const y = alpha * (y1 + x - x1);

            x1 = x;
            y1 = y;

            // Reconvertir en 16-bit
            filtered[i] = Math.max(-32768, Math.min(32767, Math.round(y * 32768)));
        }

        return filtered;
    }

    // Calculer le RMS (Root Mean Square)
    calculateRMS(audioData) {
        let sum = 0;
        for (let i = 0; i < audioData.length; i++) {
            const normalized = audioData[i] / 32768.0;
            sum += normalized * normalized;
        }
        return Math.sqrt(sum / audioData.length);
    }

    // Normalisation du volume
    normalizeVolume(audioData, targetLevel = 0.7) {
        const rms = this.calculateRMS(audioData);
        if (rms < 0.001) return audioData; // Éviter division par zéro

        const gain = targetLevel / rms;
        const limitedGain = Math.min(8.0, Math.max(0.1, gain)); // Limiter le gain

        const normalized = new Int16Array(audioData.length);
        for (let i = 0; i < audioData.length; i++) {
            const amplified = audioData[i] * limitedGain;
            normalized[i] = Math.max(-32768, Math.min(32767, Math.round(amplified)));
        }

        return normalized;
    }

    // Réduction de bruit simple (filtre moyenneur)
    reduceNoise(audioData, windowSize = 3) {
        const filtered = new Int16Array(audioData.length);
        const halfWindow = Math.floor(windowSize / 2);

        for (let i = 0; i < audioData.length; i++) {
            let sum = 0;
            let count = 0;

            for (let j = Math.max(0, i - halfWindow); j <= Math.min(audioData.length - 1, i + halfWindow); j++) {
                sum += audioData[j];
                count++;
            }

            filtered[i] = Math.round(sum / count);
        }

        return filtered;
    }

    // Égaliseur simple - boost des médiums (clarté vocale)
    applyEqualizer(audioData) {
        // Filtre simple pour boosting 2kHz (approximatif)
        const filtered = new Int16Array(audioData.length);
        let x1 = 0, x2 = 0;
        let y1 = 0, y2 = 0;

        // Coefficients pour un filtre peak à ~2kHz
        const b0 = 1.02, b1 = -1.8, b2 = 0.8;
        const a1 = -1.8, a2 = 0.82;

        for (let i = 0; i < audioData.length; i++) {
            const x0 = audioData[i] / 32768.0;
            const y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;

            x2 = x1; x1 = x0;
            y2 = y1; y1 = y0;

            filtered[i] = Math.max(-32768, Math.min(32767, Math.round(y0 * 32768)));
        }

        return filtered;
    }

    // Traitement complet
    processAudio(audioData, options = {}) {
        console.log(`  - Signal original: RMS = ${this.calculateRMS(audioData).toFixed(4)}`);

        let processed = audioData;

        // 1. Filtre passe-haut
        if (options.highPass !== false) {
            processed = this.applyHighPassFilter(processed, options.highPassCutoff || 85);
            console.log(`  - Après filtre passe-haut: RMS = ${this.calculateRMS(processed).toFixed(4)}`);
        }

        // 2. Réduction de bruit légère
        if (options.noiseReduction !== false) {
            processed = this.reduceNoise(processed, 3);
            console.log(`  - Après réduction de bruit: RMS = ${this.calculateRMS(processed).toFixed(4)}`);
        }

        // 3. Égaliseur
        if (options.equalizer !== false) {
            processed = this.applyEqualizer(processed);
            console.log(`  - Après égaliseur: RMS = ${this.calculateRMS(processed).toFixed(4)}`);
        }

        // 4. Normalisation (toujours en dernier)
        if (options.normalize !== false) {
            processed = this.normalizeVolume(processed, options.targetLevel || 0.7);
            console.log(`  - Après normalisation: RMS = ${this.calculateRMS(processed).toFixed(4)}`);
        }

        return processed;
    }

    // Créer un nouveau fichier WAV
    createWAVFile(audioData, sampleRate = 22050, channels = 1, bitsPerSample = 16) {
        const byteRate = sampleRate * channels * (bitsPerSample / 8);
        const blockAlign = channels * (bitsPerSample / 8);
        const dataSize = audioData.length * 2; // 16-bit = 2 bytes per sample
        const fileSize = 44 + dataSize;

        const buffer = new ArrayBuffer(fileSize);
        const view = new DataView(buffer);
        const uint8View = new Uint8Array(buffer);

        // En-tête RIFF
        uint8View.set([82, 73, 70, 70], 0); // "RIFF"
        view.setUint32(4, fileSize - 8, true);
        uint8View.set([87, 65, 86, 69], 8); // "WAVE"

        // Chunk fmt
        uint8View.set([102, 109, 116, 32], 12); // "fmt "
        view.setUint32(16, 16, true); // Chunk size
        view.setUint16(20, 1, true); // Audio format (PCM)
        view.setUint16(22, channels, true);
        view.setUint32(24, sampleRate, true);
        view.setUint32(28, byteRate, true);
        view.setUint16(32, blockAlign, true);
        view.setUint16(34, bitsPerSample, true);

        // Chunk data
        uint8View.set([100, 97, 116, 97], 36); // "data"
        view.setUint32(40, dataSize, true);

        // Données audio
        for (let i = 0; i < audioData.length; i++) {
            view.setInt16(44 + i * 2, audioData[i], true);
        }

        return Buffer.from(buffer);
    }
}

// Instance du processeur audio
const audioProcessor = new AudioProcessor();

// Route pour recevoir les fichiers audio avec traitement JavaScript
app.post('/audio/upload', async (req, res) => {
    try {
        console.log('Réception d\'un fichier audio...');
        console.log('Taille:', req.body.length, 'octets');

        // Créer les dossiers nécessaires
        const uploadDir = './uploads';
        const processedDir = './uploads/processed';

        if (!fs.existsSync(uploadDir)) {
            fs.mkdirSync(uploadDir);
        }
        if (!fs.existsSync(processedDir)) {
            fs.mkdirSync(processedDir);
        }

        // Générer un nom de fichier unique
        const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
        const originalFilename = `recording-${timestamp}.wav`;
        const enhancedFilename = `enhanced-${timestamp}.wav`;

        const originalPath = path.join(uploadDir, originalFilename);
        const enhancedPath = path.join(processedDir, enhancedFilename);

        // Sauvegarder le fichier original
        fs.writeFileSync(originalPath, req.body);
        console.log(`Fichier original sauvegardé: ${originalPath}`);

        let response = {
            success: true,
            message: 'Fichier audio reçu avec succès',
            original: {
                filename: originalFilename,
                size: req.body.length,
                path: originalPath
            }
        };

        // Traitement audio en JavaScript
        try {
            console.log('Traitement audio en cours...');

            // Parser le fichier WAV
            const audioInfo = audioProcessor.parseWAVFile(req.body);
            console.log(`  - Durée: ${audioInfo.duration.toFixed(2)}s`);
            console.log(`  - Échantillons: ${audioInfo.audioData.length}`);
            console.log(`  - Fréquence: ${audioInfo.sampleRate} Hz`);

            // Traiter l'audio
            const processedAudio = audioProcessor.processAudio(audioInfo.audioData, {
                highPass: true,
                highPassCutoff: 85,
                noiseReduction: true,
                equalizer: true,
                normalize: true,
                targetLevel: 0.7
            });

            // Créer le nouveau fichier WAV
            const enhancedWAV = audioProcessor.createWAVFile(
                processedAudio,
                audioInfo.sampleRate,
                audioInfo.channels,
                audioInfo.bitsPerSample
            );

            // Sauvegarder le fichier traité
            fs.writeFileSync(enhancedPath, enhancedWAV);

            response.enhanced = {
                filename: enhancedFilename,
                size: enhancedWAV.length,
                path: enhancedPath,
                processing: {
                    originalRMS: audioProcessor.calculateRMS(audioInfo.audioData),
                    enhancedRMS: audioProcessor.calculateRMS(processedAudio),
                    duration: audioInfo.duration,
                    sampleRate: audioInfo.sampleRate
                }
            };

            console.log(`Audio traité sauvegardé: ${enhancedPath}`);

        } catch (processError) {
            console.error('Erreur de traitement audio:', processError.message);
            response.warning = `Traitement audio échoué: ${processError.message}`;
        }

        res.status(200).json(response);

    } catch (error) {
        console.error('Erreur lors de la réception:', error);
        res.status(500).json({
            success: false,
            message: 'Erreur lors de la sauvegarde du fichier',
            error: error.message
        });
    }
});

// Route de test
app.get('/', (req, res) => {
    res.json({
        message: 'Serveur audio ProutOMètre - JavaScript Enhanced',
        status: 'actif',
        features: [
            'Audio enhancement en JavaScript pur',
            'Filtre passe-haut (85 Hz)',
            'Réduction de bruit simple',
            'Normalisation du volume',
            'Égaliseur pour clarté vocale',
            'Aucune dépendance externe'
        ],
        endpoints: {
            upload: 'POST /audio/upload',
            recordings: 'GET /recordings',
            download: 'GET /download/:filename'
        }
    });
});

// Route pour lister les enregistrements
app.get('/recordings', (req, res) => {
    try {
        const uploadDir = './uploads';
        const processedDir = './uploads/processed';

        let recordings = [];

        // Fichiers originaux
        if (fs.existsSync(uploadDir)) {
            const originalFiles = fs.readdirSync(uploadDir)
                .filter(file => file.endsWith('.wav'))
                .map(file => {
                    const filepath = path.join(uploadDir, file);
                    const stats = fs.statSync(filepath);
                    return {
                        filename: file,
                        type: 'original',
                        size: stats.size,
                        created: stats.birthtime,
                        modified: stats.mtime,
                        downloadUrl: `/download/${file}`
                    };
                });
            recordings = recordings.concat(originalFiles);
        }

        // Fichiers traités
        if (fs.existsSync(processedDir)) {
            const processedFiles = fs.readdirSync(processedDir)
                .filter(file => file.endsWith('.wav'))
                .map(file => {
                    const filepath = path.join(processedDir, file);
                    const stats = fs.statSync(filepath);
                    return {
                        filename: file,
                        type: 'enhanced',
                        size: stats.size,
                        created: stats.birthtime,
                        modified: stats.mtime,
                        downloadUrl: `/download/processed/${file}`
                    };
                });
            recordings = recordings.concat(processedFiles);
        }

        recordings.sort((a, b) => b.created - a.created);
        res.json({ recordings });

    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

// Route pour télécharger les fichiers
app.get('/download/:filename', (req, res) => {
    const filename = req.params.filename;
    const filepath = path.join('./uploads', filename);

    if (fs.existsSync(filepath)) {
        res.download(filepath);
    } else {
        res.status(404).json({ error: 'Fichier non trouvé' });
    }
});

app.get('/download/processed/:filename', (req, res) => {
    const filename = req.params.filename;
    const filepath = path.join('./uploads/processed', filename);

    if (fs.existsSync(filepath)) {
        res.download(filepath);
    } else {
        res.status(404).json({ error: 'Fichier traité non trouvé' });
    }
});

app.listen(port, '0.0.0.0', () => {
    console.log(`Serveur audio JavaScript Enhanced démarré sur http://0.0.0.0:${port}`);
    console.log('Fonctionnalités (JavaScript pur):');
    console.log('  ✓ Réception audio brute');
    console.log('  ✓ Filtre passe-haut (85 Hz)');
    console.log('  ✓ Réduction de bruit simple');
    console.log('  ✓ Normalisation du volume');
    console.log('  ✓ Égaliseur pour clarté vocale');
    console.log('  ✓ Aucune dépendance externe');
    console.log('');
    console.log('Endpoints disponibles:');
    console.log('  GET  / - Informations du serveur');
    console.log('  POST /audio/upload - Upload et traitement audio');
    console.log('  GET  /recordings - Liste des enregistrements');
    console.log('  GET  /download/:filename - Téléchargement fichier original');
    console.log('  GET  /download/processed/:filename - Téléchargement fichier traité');
});

module.exports = app;
