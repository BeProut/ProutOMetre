#include "audio_recorder.h"
#include <math.h>

AudioRecorder::AudioRecorder()
{
    isRecording = false;
    buttonPressed = false;
    buttonPressTime = 0;
    recordingStartTime = 0;
    i2sBuffer = nullptr;
    processedBuffer = nullptr;
    bytesRead = 0;
    lastButtonState = HIGH;
    lastDebounceTime = 0;

    // Initialisation des variables de traitement audio
    currentGain = AUDIO_GAIN;
    targetGain = AUDIO_GAIN;
    maxSample = 1.0;
    avgPower = 0.0;
    sampleCount = 0;

    // Initialisation du filtre passe-haut
    highPassY1 = 0.0;
    highPassX1 = 0.0;
    highPassAlpha = 0.0;
}

AudioRecorder::~AudioRecorder()
{
    if (i2sBuffer)
    {
        free(i2sBuffer);
    }
    if (processedBuffer)
    {
        free(processedBuffer);
    }
    i2s_driver_uninstall(I2S_PORT);
}

void AudioRecorder::begin()
{
    Serial.println("[AUDIO] Initialisation du module audio...");

    initLittleFS();
    initButton();
    initStatusLED();
    initI2S();
    initAudioProcessing();

    // Allouer les buffers pour les données I2S
    i2sBuffer = (int16_t *)malloc(I2S_READ_LEN * sizeof(int16_t));
    processedBuffer = (int16_t *)malloc(I2S_READ_LEN * sizeof(int16_t));
    if (!i2sBuffer || !processedBuffer)
    {
        Serial.println("[AUDIO] Erreur: Impossible d'allouer les buffers I2S");
        return;
    }

    Serial.println("[AUDIO] Module audio initialisé avec succès");

    // Clignoter la LED pour indiquer que tout est prêt
    if (LED_ENABLED)
    {
        for (int i = 0; i < 3; i++)
        {
            setStatusLED(true);
            delay(100);
            setStatusLED(false);
            delay(100);
        }
    }
}

void AudioRecorder::initLittleFS()
{
    if (!LittleFS.begin(true))
    {
        Serial.println("[AUDIO] Erreur: Impossible de monter LittleFS");
        return;
    }

    // Formater LittleFS au démarrage si demandé
    Serial.println("[AUDIO] LittleFS monté avec succès");

    // Nettoyer les anciens fichiers audio
    if (LittleFS.exists(AUDIO_FILE_PATH))
    {
        LittleFS.remove(AUDIO_FILE_PATH);
        Serial.println("[AUDIO] Ancien fichier audio supprimé");
    }
}

void AudioRecorder::initButton()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    Serial.println("[AUDIO] Bouton initialisé sur le pin " + String(BUTTON_PIN));
}

void AudioRecorder::initStatusLED()
{
    if (LED_ENABLED)
    {
        pinMode(STATUS_LED_PIN, OUTPUT);
        setStatusLED(false);
        Serial.println("[AUDIO] LED de statut initialisée sur le pin " + String(STATUS_LED_PIN));
    }
}

void AudioRecorder::setStatusLED(bool state)
{
    if (LED_ENABLED)
    {
        digitalWrite(STATUS_LED_PIN, state);
    }
}

void AudioRecorder::initI2S()
{
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = I2S_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_FORMAT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = I2S_DMA_BUF_COUNT, // Amélioré pour réduire les dropouts
        .dma_buf_len = I2S_DMA_BUF_LEN,     // Optimisé pour la latence
        .use_apll = false,                  // Désactivé pour éviter problèmes de timing
        .tx_desc_auto_clear = false,
        .fixed_mclk = 0};

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD_IO};

    esp_err_t result = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    if (result != ESP_OK)
    {
        Serial.println("[AUDIO] Erreur: Installation du driver I2S échouée: " + String(result));
        return;
    }

    result = i2s_set_pin(I2S_PORT, &pin_config);
    if (result != ESP_OK)
    {
        Serial.println("[AUDIO] Erreur: Configuration des pins I2S échouée: " + String(result));
        return;
    }

    Serial.println("[AUDIO] I2S initialisé avec succès");
}

bool AudioRecorder::readButton()
{
    bool reading = digitalRead(BUTTON_PIN) == LOW; // Active LOW avec pull-up

    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY)
    {
        if (reading != buttonPressed)
        {
            buttonPressed = reading;
            lastButtonState = reading;
            return true; // État du bouton a changé
        }
    }

    lastButtonState = reading;
    return false; // Pas de changement d'état
}

void AudioRecorder::startRecording()
{
    if (isRecording)
        return;

    Serial.println("[AUDIO] Début de l'enregistrement...");

    // Allumer la LED pendant l'enregistrement
    setStatusLED(true);

    // Ouvrir le fichier pour l'écriture
    audioFile = LittleFS.open(AUDIO_FILE_PATH, "w");
    if (!audioFile)
    {
        Serial.println("[AUDIO] Erreur: Impossible de créer le fichier audio");
        setStatusLED(false);
        return;
    }

    // Écrire l'en-tête WAV (sera mise à jour à la fin)
    writeWAVHeader(audioFile, I2S_SAMPLE_RATE, I2S_SAMPLE_BITS, I2S_CHANNEL_NUM);

    isRecording = true;
    recordingStartTime = millis();

    // Vider le buffer I2S
    i2s_zero_dma_buffer(I2S_PORT);

    Serial.println("[AUDIO] Enregistrement en cours...");
}

void AudioRecorder::stopRecording()
{
    if (!isRecording)
        return;

    Serial.println("[AUDIO] Arrêt de l'enregistrement...");

    isRecording = false;

    // Éteindre la LED
    setStatusLED(false);

    if (audioFile)
    {
        // Mettre à jour l'en-tête WAV avec la taille réelle
        uint32_t fileSize = audioFile.size();
        updateWAVHeader(audioFile, fileSize - 44); // 44 = taille de l'en-tête WAV
        audioFile.close();

        Serial.println("[AUDIO] Fichier audio sauvegardé (" + String(fileSize) + " octets)");

        // Clignoter rapidement pendant l'upload
        if (LED_ENABLED)
        {
            for (int i = 0; i < 6; i++)
            {
                setStatusLED(true);
                delay(50);
                setStatusLED(false);
                delay(50);
            }
        }

        // Envoyer le fichier au serveur
        if (uploadAudioFile())
        {
            Serial.println("[AUDIO] Fichier envoyé avec succès au serveur");
            // Double clignotement pour succès
            if (LED_ENABLED)
            {
                setStatusLED(true);
                delay(200);
                setStatusLED(false);
                delay(100);
                setStatusLED(true);
                delay(200);
                setStatusLED(false);
            }
        }
        else
        {
            Serial.println("[AUDIO] Erreur lors de l'envoi au serveur");
            // Clignotement long pour erreur
            if (LED_ENABLED)
            {
                setStatusLED(true);
                delay(1000);
                setStatusLED(false);
            }
        }
    }
}

void AudioRecorder::writeWAVHeader(File &file, uint32_t sampleRate, uint16_t bitsPerSample, uint16_t channels)
{
    uint32_t byteRate = sampleRate * channels * (bitsPerSample / 8);
    uint16_t blockAlign = channels * (bitsPerSample / 8);

    // En-tête RIFF
    file.write((uint8_t *)"RIFF", 4);
    uint32_t chunkSize = 36; // Sera mis à jour plus tard
    file.write((uint8_t *)&chunkSize, 4);
    file.write((uint8_t *)"WAVE", 4);

    // Sous-chunk fmt
    file.write((uint8_t *)"fmt ", 4);
    uint32_t subchunk1Size = 16;
    file.write((uint8_t *)&subchunk1Size, 4);
    uint16_t audioFormat = 1; // PCM
    file.write((uint8_t *)&audioFormat, 2);
    file.write((uint8_t *)&channels, 2);
    file.write((uint8_t *)&sampleRate, 4);
    file.write((uint8_t *)&byteRate, 4);
    file.write((uint8_t *)&blockAlign, 2);
    file.write((uint8_t *)&bitsPerSample, 2);

    // Sous-chunk data
    file.write((uint8_t *)"data", 4);
    uint32_t subchunk2Size = 0; // Sera mis à jour plus tard
    file.write((uint8_t *)&subchunk2Size, 4);
}

void AudioRecorder::updateWAVHeader(File &file, uint32_t dataSize)
{
    // Mettre à jour la taille du chunk principal
    file.seek(4);
    uint32_t chunkSize = 36 + dataSize;
    file.write((uint8_t *)&chunkSize, 4);

    // Mettre à jour la taille des données
    file.seek(40);
    file.write((uint8_t *)&dataSize, 4);
}

bool AudioRecorder::uploadAudioFile()
{
    if (!LittleFS.exists(AUDIO_FILE_PATH))
    {
        Serial.println("[AUDIO] Erreur: Fichier audio introuvable");
        return false;
    }

    File file = LittleFS.open(AUDIO_FILE_PATH, "r");
    if (!file)
    {
        Serial.println("[AUDIO] Erreur: Impossible d'ouvrir le fichier audio");
        return false;
    }

    Serial.println("[AUDIO] Envoi du fichier audio au serveur...");

    http.begin(SERVER_URL);
    http.addHeader("Content-Type", "audio/wav");
    http.addHeader("Content-Length", String(file.size()));

    // Lire et envoyer le fichier par chunks
    int httpResponseCode = http.sendRequest("POST", &file, file.size());

    file.close();

    if (httpResponseCode == 200)
    {
        String response = http.getString();
        Serial.println("[AUDIO] Réponse serveur: " + response);

        // Supprimer le fichier après envoi réussi
        LittleFS.remove(AUDIO_FILE_PATH);

        http.end();
        return true;
    }
    else
    {
        Serial.println("[AUDIO] Erreur HTTP: " + String(httpResponseCode));
        if (httpResponseCode > 0)
        {
            Serial.println("[AUDIO] Réponse: " + http.getString());
        }
        http.end();
        return false;
    }
}

void AudioRecorder::processAudioData()
{
    if (!isRecording || !audioFile)
        return;

    // Vérifier le temps maximum d'enregistrement
    if (millis() - recordingStartTime > MAX_RECORD_TIME)
    {
        Serial.println("[AUDIO] Temps maximum d'enregistrement atteint");
        stopRecording();
        return;
    }

    // Lire les données du microphone I2S
    esp_err_t result = i2s_read(I2S_PORT, i2sBuffer, I2S_READ_LEN * sizeof(int16_t), &bytesRead, portMAX_DELAY);

    if (result == ESP_OK && bytesRead > 0)
    {
        size_t samples = bytesRead / sizeof(int16_t);

        // Copier les données dans le buffer de traitement
        memcpy(processedBuffer, i2sBuffer, bytesRead);

        // Traitement minimal pour éviter altération du pitch
        // Commenté le filtre passe-haut qui peut causer des problèmes de tonalité
        // applyHighPassFilter(processedBuffer, samples);

        if (AUTO_GAIN_ENABLED)
        {
            // AGC très léger
            applyAutoGainControl(processedBuffer, samples);
        }
        else if (AUDIO_GAIN != 1.0)
        {
            // Gain fixe simple seulement si différent de 1.0
            normalizeAudio(processedBuffer, samples);
        }
        // Sinon, pas de traitement = son naturel

        // Écrire les données traitées dans le fichier
        size_t bytesWritten = audioFile.write((uint8_t *)processedBuffer, bytesRead);

        if (bytesWritten != bytesRead)
        {
            Serial.println("[AUDIO] Erreur: Écriture incomplète dans le fichier");
            stopRecording();
        }
    }
    else
    {
        Serial.println("[AUDIO] Erreur lors de la lecture I2S: " + String(result));
    }
}

void AudioRecorder::initAudioProcessing()
{
    // Calcul du coefficient du filtre passe-haut (RC = 1/(2*pi*fc))
    float dt = 1.0 / I2S_SAMPLE_RATE;
    float rc = 1.0 / (2.0 * M_PI * HIGH_PASS_CUTOFF);
    highPassAlpha = rc / (rc + dt);

    Serial.println("[AUDIO] Traitement audio initialisé");
    Serial.println("[AUDIO] - Filtre passe-haut: " + String(HIGH_PASS_CUTOFF) + " Hz");
    Serial.println("[AUDIO] - Gain audio: " + String(AUDIO_GAIN));
    Serial.println("[AUDIO] - AGC activé: " + String(AUTO_GAIN_ENABLED ? "Oui" : "Non"));
}

void AudioRecorder::applyHighPassFilter(int16_t *buffer, size_t samples)
{
    for (size_t i = 0; i < samples; i++)
    {
        float x = (float)buffer[i] / 32768.0; // Normaliser à [-1, 1]

        // Filtre passe-haut du premier ordre
        // y[n] = alpha * (y[n-1] + x[n] - x[n-1])
        float y = highPassAlpha * (highPassY1 + x - highPassX1);

        highPassX1 = x;
        highPassY1 = y;

        // Reconvertir et saturer
        int32_t result = (int32_t)(y * 32768.0);
        if (result > 32767)
            result = 32767;
        if (result < -32768)
            result = -32768;

        buffer[i] = (int16_t)result;
    }
}

void AudioRecorder::applyAutoGainControl(int16_t *buffer, size_t samples)
{
    // Calculer le RMS du signal
    float rms = calculateRMS(buffer, samples);

    if (rms > 0.001) // Éviter la division par zéro
    {
        // Calculer le gain cible basé sur le niveau RMS
        targetGain = TARGET_VOLUME / rms;

        // Limiter le gain pour éviter la saturation
        if (targetGain > 8.0)
            targetGain = 8.0;
        if (targetGain < 0.1)
            targetGain = 0.1;

        // Appliquer un lissage au gain (attaque/relâchement)
        float alpha = (targetGain > currentGain) ? AGC_ATTACK_TIME : AGC_RELEASE_TIME;
        currentGain = currentGain * (1.0 - alpha) + targetGain * alpha;
    }

    // Appliquer le gain aux échantillons
    for (size_t i = 0; i < samples; i++)
    {
        float sample = (float)buffer[i] * currentGain;

        // Saturation douce (soft clipping)
        if (sample > 32767.0)
            sample = 32767.0;
        if (sample < -32768.0)
            sample = -32768.0;

        buffer[i] = (int16_t)sample;
    }
}

void AudioRecorder::normalizeAudio(int16_t *buffer, size_t samples)
{
    // Appliquer simplement le gain fixe
    for (size_t i = 0; i < samples; i++)
    {
        float sample = (float)buffer[i] * AUDIO_GAIN;

        // Saturation
        if (sample > 32767.0)
            sample = 32767.0;
        if (sample < -32768.0)
            sample = -32768.0;

        buffer[i] = (int16_t)sample;
    }
}

float AudioRecorder::calculateRMS(int16_t *buffer, size_t samples)
{
    float sum = 0.0;
    for (size_t i = 0; i < samples; i++)
    {
        float sample = (float)buffer[i] / 32768.0;
        sum += sample * sample;
    }
    return sqrt(sum / samples);
}

void AudioRecorder::process()
{
    // Gérer l'état du bouton
    if (readButton())
    {
        if (buttonPressed && !isRecording)
        {
            // Bouton pressé - commencer l'enregistrement
            startRecording();
        }
        else if (!buttonPressed && isRecording)
        {
            // Bouton relâché - arrêter l'enregistrement
            stopRecording();
        }
    }

    // Traiter les données audio si on enregistre
    if (isRecording)
    {
        processAudioData();
    }
}
