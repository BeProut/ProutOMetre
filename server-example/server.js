const express = require('express');
const multer = require('multer');
const fs = require('fs');
const path = require('path');

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

// Route pour recevoir les fichiers audio
app.post('/audio/upload', (req, res) => {
    try {
        console.log('Réception d\'un fichier audio...');
        console.log('Taille:', req.body.length, 'octets');
        console.log('Content-Type:', req.get('Content-Type'));

        // Créer le dossier uploads s'il n'existe pas
        const uploadDir = './uploads';
        if (!fs.existsSync(uploadDir)) {
            fs.mkdirSync(uploadDir);
        }

        // Générer un nom de fichier unique
        const timestamp = new Date().toISOString().replace(/[:.]/g, '-');
        const filename = `recording-${timestamp}.wav`;
        const filepath = path.join(uploadDir, filename);

        // Sauvegarder le fichier
        fs.writeFileSync(filepath, req.body);

        console.log(`Fichier sauvegardé: ${filepath}`);

        res.status(200).json({
            success: true,
            message: 'Fichier audio reçu avec succès',
            filename: filename,
            size: req.body.length
        });

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
        message: 'Serveur audio ProutOMètre',
        status: 'actif',
        endpoints: {
            upload: 'POST /audio/upload'
        }
    });
});

// Route pour lister les enregistrements
app.get('/recordings', (req, res) => {
    try {
        const uploadDir = './uploads';
        if (!fs.existsSync(uploadDir)) {
            return res.json({ recordings: [] });
        }

        const files = fs.readdirSync(uploadDir)
            .filter(file => file.endsWith('.wav'))
            .map(file => {
                const filepath = path.join(uploadDir, file);
                const stats = fs.statSync(filepath);
                return {
                    filename: file,
                    size: stats.size,
                    created: stats.birthtime,
                    modified: stats.mtime
                };
            })
            .sort((a, b) => b.created - a.created);

        res.json({ recordings: files });
    } catch (error) {
        res.status(500).json({ error: error.message });
    }
});

app.listen(port, '0.0.0.0', () => {
    console.log(`Serveur audio démarré sur http://0.0.0.0:${port}`);
    console.log('Endpoints disponibles:');
    console.log('  GET  / - Informations du serveur');
    console.log('  POST /audio/upload - Upload de fichiers audio');
    console.log('  GET  /recordings - Liste des enregistrements');
});

module.exports = app;
