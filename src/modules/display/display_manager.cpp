#include "display_manager.h"

DisplayManager::DisplayManager()
{
    display = nullptr;
    lastUpdate = 0;
    animationFrame = 0;
    screenOn = true;
}

bool DisplayManager::init()
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

    if (!display->begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS))
    {
        Serial.println("Erreur: Impossible d'initialiser l'écran OLED");
        return false;
    }

    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->setTextSize(1);
    display->display();

    Serial.println("DisplayManager: Écran OLED initialisé");
    return true;
}

void DisplayManager::update(FartDetector::DetectionResult result)
{
    if (!display || !screenOn)
        return;

    display->clearDisplay();

    if (result.isDetected)
    {
        drawHeader();
        drawScore(result.totalScore);
        drawIntensity(result.intensity);
        drawCategory(result.category);
    }
    else
    {
        drawNoDetection();
    }

    display->display();
    lastUpdate = millis();
}

void DisplayManager::drawHeader()
{
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->print("DETECTION GAZ");

    // Ligne séparatrice
    display->drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
}

void DisplayManager::drawScore(int score)
{
    // Titre
    display->setTextSize(1);
    display->setCursor(0, 15);
    display->print("Score:");

    // Score numérique grand
    display->setTextSize(2);
    display->setCursor(50, 13);
    if (score < 10)
        display->print("0");
    display->print(score);
    display->setTextSize(1);
    display->print("/100");

    // Barre de progression
    drawProgressBar(0, 30, SCREEN_WIDTH - 20, 8, score, 100);

    // Pourcentage à droite de la barre
    display->setCursor(SCREEN_WIDTH - 18, 30);
    display->print(score);
    display->print("%");
}

void DisplayManager::drawIntensity(int intensity)
{
    display->setTextSize(1);
    display->setCursor(0, 45);
    display->print("Niveau: ");

    // Affichage des barres d'intensité
    for (int i = 0; i < 5; i++)
    {
        int x = 50 + (i * 12);
        int y = 45;

        if (i < intensity)
        {
            // Barre pleine
            display->fillRect(x, y, 8, 8, SSD1306_WHITE);
        }
        else
        {
            // Barre vide
            display->drawRect(x, y, 8, 8, SSD1306_WHITE);
        }
    }

    // Chiffre à droite
    display->setCursor(115, 45);
    display->print(intensity);
    display->print("/5");
}

void DisplayManager::drawCategory(String category)
{
    display->setTextSize(1);
    display->setCursor(0, 57);
    display->print("Type: ");
    display->print(category);
}

void DisplayManager::drawNoDetection()
{
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->print("MONITEUR GAZ");

    // Animation de point clignotant
    animationFrame++;
    if (animationFrame > 60)
        animationFrame = 0;

    display->setTextSize(2);
    display->setCursor(25, 25);
    display->print("STANDBY");

    if (animationFrame < 30)
    {
        display->fillCircle(100, 32, 3, SSD1306_WHITE);
    }

    display->setTextSize(1);
    display->setCursor(15, 50);
    display->print("En attente...");
}

void DisplayManager::drawProgressBar(int x, int y, int width, int height, int value, int maxValue)
{
    // Bordure
    display->drawRect(x, y, width, height, SSD1306_WHITE);

    // Remplissage
    int fillWidth = map(value, 0, maxValue, 0, width - 2);
    if (fillWidth > 0)
    {
        display->fillRect(x + 1, y + 1, fillWidth, height - 2, SSD1306_WHITE);
    }
}

void DisplayManager::clear()
{
    if (display)
    {
        display->clearDisplay();
        display->display();
    }
}

void DisplayManager::turnOn()
{
    screenOn = true;
    if (display)
    {
        display->ssd1306_command(SSD1306_DISPLAYON);
    }
}

void DisplayManager::turnOff()
{
    screenOn = false;
    if (display)
    {
        display->ssd1306_command(SSD1306_DISPLAYOFF);
    }
}

void DisplayManager::setBrightness(int level)
{
    if (display)
    {
        level = constrain(level, 0, 255);
        display->ssd1306_command(SSD1306_SETCONTRAST);
        display->ssd1306_command(level);
    }
}

void DisplayManager::showCalibration()
{
    if (!display)
        return;

    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(15, 10);
    display->print("CALIBRATION");

    display->setTextSize(2);
    display->setCursor(25, 30);
    display->print("EN COURS");

    // Animation de progression
    for (int i = 0; i < 4; i++)
    {
        int x = 30 + (i * 15);
        if ((millis() / 200) % 4 == i)
        {
            display->fillCircle(x, 50, 3, SSD1306_WHITE);
        }
        else
        {
            display->drawCircle(x, 50, 3, SSD1306_WHITE);
        }
    }

    display->display();
}

void DisplayManager::showStatus(String message)
{
    if (!display)
        return;

    display->clearDisplay();
    display->setTextSize(1);
    display->setCursor(0, 0);
    display->print("STATUS:");

    display->setCursor(0, 20);
    display->print(message);

    display->display();
}