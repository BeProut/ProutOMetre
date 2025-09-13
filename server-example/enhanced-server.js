const express = require('express');
const multer = require('multer');
const fs = require('fs');
const path = require('path');
const { spawn } = require('child_process');

const app = express();
const port = 3000;

// Configuration de multer pour gérer les uploads
const storage = multer.diskStorage({
    destination: (req, file, cb) => {
        const uploadDir = './uploads';
        if (!fs.existsSync(uploadDir)) {
            fs.mkdirSync(uploadDir);
        }
        cb(null, uploadDir);
    },
    filename: (req, file, cb) => {
        const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
        cb(null, `recording-${timestamp}.wav`);
    }
});

const upload = multer({ storage: storage });

// Middleware pour parser les données brutes
app.use('/audio/upload', express.raw({ type: 'audio/wav', limit: '10mb' }));

// Fonction pour améliorer l'audio avec FFmpeg
async function enhanceAudio(inputPath, outputPath) {
    return new Promise((resolve, reject) => {
        // Commande FFmpeg pour améliorer l'audio
        const ffmpeg = spawn('ffmpeg', [
            '-i', inputPath,
            '-y', // Overwrite output file
            // Filtre passe-haut pour supprimer les basses fréquences parasites
            '-af', 'highpass=f=85',
            // Normalisation du volume
            '-af', 'loudnorm=I=-16:LRA=11:TP=-1.5',
            // Réduction de bruit légère
            '-af', 'afftdn=nr=10:nf=-25',
            // Compresseur pour égaliser les niveaux
            '-af', 'acompressor=threshold=0.089:ratio=9:attack=0.003:release=0.086:makeup=2',
            // Amélioration de la clarté
            '-af', 'equalizer=f=2000:width_type=h:width=1000:g=2',
            // Format de sortie
            '-ar', '22050',  // Fréquence d'échantillonnage
            '-ac', '1',      // Mono
            '-c:a', 'pcm_s16le', // Codec audio
            outputPath
        ]);

        let stderr = '';

        ffmpeg.stderr.on('data', (data) => {
            stderr += data.toString();
        });

        ffmpeg.on('close', (code) => {
            if (code === 0) {
                resolve(outputPath);
            } else {
                reject(new Error(`FFmpeg failed with code ${code}: ${stderr}`));
            }
        });

        ffmpeg.on('error', (err) => {
            reject(err);
        });
    });
}

// Fonction pour analyser l'audio et fournir des métriques
async function analyzeAudio(filePath) {
    return new Promise((resolve, reject) => {
        const ffprobe = spawn('ffprobe', [
            '-v', 'quiet',
            '-show_entries', 'stream=sample_rate,channels,duration,bit_rate',
            '-of', 'json',
            filePath
        ]);

        let stdout = '';
        let stderr = '';

        ffprobe.stdout.on('data', (data) => {
            stdout += data.toString();
        });

        ffprobe.stderr.on('data', (data) => {
            stderr += data.toString();
        });

        ffprobe.on('close', (code) => {
            if (code === 0) {
                try {
                    const analysis = JSON.parse(stdout);
                    resolve(analysis);
                } catch (err) {
                    reject(err);
                }
            } else {
                reject(new Error(`FFprobe failed with code ${code}: ${stderr}`));
            }
        });
    });
}

// Route pour recevoir les fichiers audio avec traitement amélioré
app.post('/audio/upload', async (req, res) => {
    try {
        console.log('Réception d\'un fichier audio...');
        console.log('Taille:', req.body.length, 'octets');
        console.log('Content-Type:', req.get('Content-Type'));

        // Créer le dossier uploads s'il n'existe pas
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

        // Essayer d'améliorer l'audio si FFmpeg est disponible
        try {
            console.log('Amélioration de l\'audio en cours...');
            await enhanceAudio(originalPath, enhancedPath);

            const enhancedStats = fs.statSync(enhancedPath);

            // Analyser l'audio amélioré
            const analysis = await analyzeAudio(enhancedPath);

            response.enhanced = {
                filename: enhancedFilename,
                size: enhancedStats.size,
                path: enhancedPath,
                analysis: analysis
            };

            console.log(`Audio amélioré sauvegardé: ${enhancedPath}`);

        } catch (enhanceError) {
            console.warn('Amélioration audio échouée (FFmpeg requis):', enhanceError.message);
            response.warning = 'Audio enhancement failed - FFmpeg required for processing';
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
        message: 'Serveur audio ProutOMètre Enhanced',
        status: 'actif',
        features: [
            'Audio enhancement with FFmpeg',
            'Noise reduction',
            'Volume normalization',
            'High-pass filtering',
            'Audio analysis'
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
    console.log(`Serveur audio Enhanced démarré sur http://0.0.0.0:${port}`);
    console.log('Fonctionnalités:');
    console.log('  ✓ Réception audio brute');
    console.log('  ✓ Amélioration audio avec FFmpeg');
    console.log('  ✓ Réduction de bruit');
    console.log('  ✓ Normalisation du volume');
    console.log('  ✓ Filtrage passe-haut');
    console.log('  ✓ Analyse des métriques audio');
    console.log('');
    console.log('Endpoints disponibles:');
    console.log('  GET  / - Informations du serveur');
    console.log('  POST /audio/upload - Upload de fichiers audio');
    console.log('  GET  /recordings - Liste des enregistrements');
    console.log('  GET  /download/:filename - Téléchargement fichier original');
    console.log('  GET  /download/processed/:filename - Téléchargement fichier traité');
    console.log('');
    console.log('Note: FFmpeg requis pour l\'amélioration audio');
});

module.exports = app;
