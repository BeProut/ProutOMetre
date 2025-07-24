#include "display_manager.h"

#define UPDATE_INTERVAL 100

DisplayManager::DisplayManager()
    : currentStatus(DISPLAY_STATUS_WAITING_CONNECTION),
      lastUpdate(0),
      animationCounter(0),
      deviceName(""),
      firmwareVersion("")
{
    display = new Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
}

DisplayManager::~DisplayManager()
{
    delete display;
}

bool DisplayManager::begin()
{
    if (!display->begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS))
    {
        Serial.println(F("[DisplayManager] Échec de l'initialisation SSD1306"));
        return false;
    }

    display->clearDisplay();
    display->setTextColor(SSD1306_WHITE);
    display->display();

    Serial.println(F("[DisplayManager] Écran initialisé avec succès"));
    return true;
}

void DisplayManager::setStatus(DisplayStatus status)
{
    if (currentStatus != status)
    {
        currentStatus = status;
        animationCounter = 0;
        Serial.printf("[DisplayManager] Changement de statut: %d\n", status);
    }
}

void DisplayManager::setDeviceName(const String &name)
{
    deviceName = name;
}

void DisplayManager::setFirmwareVersion(const String &version)
{
    firmwareVersion = version;
}

void DisplayManager::update()
{
    unsigned long currentTime = millis();

    if (currentTime - lastUpdate < UPDATE_INTERVAL)
    {
        return;
    }

    lastUpdate = currentTime;
    display->clearDisplay();

    switch (currentStatus)
    {
    case DISPLAY_STATUS_WAITING_CONNECTION:
        drawWaitingConnection();
        break;

    case DISPLAY_STATUS_CONNECTED:
        drawConnectedStatus();
        break;

    case DISPLAY_STATUS_ERROR:
        drawErrorStatus();
        break;
    }

    display->display();
}

void DisplayManager::drawWaitingConnection()
{
    drawLoadingAnimation(64, 10, 3); // Animation plus grande, centrée

    drawCenteredText("BeProut", 25, 2);       // Titre
    drawCenteredText("En attente...", 40, 1); // Texte plus petit

    if (firmwareVersion.length() > 0)
    {
        String versionText = "v" + firmwareVersion;
        drawCenteredText(versionText, 50, 1);
    }
}

void DisplayManager::drawConnectedStatus()
{
    display->fillCircle(120, 8, 3, SSD1306_WHITE);
    display->setCursor(100, 5);
    display->setTextSize(1);
    display->print("BLE");

    drawCenteredText("CONNECTE", 15, 2);

    if (deviceName.length() > 0)
    {
        drawCenteredText("Appareil:", 35, 1);

        if (deviceName.length() > 16)
        {
            drawScrollingText(deviceName, 45, 1);
        }
        else
        {
            drawCenteredText(deviceName, 45, 1);
        }
    }
    else
    {
        drawCenteredText("Appareil inconnu", 40, 1);
    }

    display->setCursor(5, 55);
    display->setTextSize(1);
    display->print("Actif");

    int dotCount = (animationCounter / 5) % 4;
    for (int i = 0; i < dotCount; i++)
    {
        display->print(".");
    }
}

void DisplayManager::drawErrorStatus()
{
    drawCenteredText("ERREUR", 15, 2);
    drawCenteredText("Probleme", 35, 1);
    drawCenteredText("d'affichage", 45, 1);
}

void DisplayManager::drawLoadingAnimation(int x, int y, int radius)
{
    int frame = (animationCounter / 3) % 8;

    for (int i = 0; i < 3; i++)
    {
        int brightness = 255;
        if ((frame + i) % 8 < 4)
        {
            brightness = 100;
        }

        int offset = i * (radius * 3);
        if (brightness > 200)
        {
            display->fillCircle(x + offset - radius, y, radius, SSD1306_WHITE);
        }
        else
        {
            display->drawCircle(x + offset - radius, y, radius, SSD1306_WHITE);
        }
    }

    animationCounter++;
}

void DisplayManager::drawCenteredText(const String &text, int y, int textSize)
{
    display->setTextSize(textSize);
    int textWidth = text.length() * 6 * textSize;
    int x = (display->width() - textWidth) / 2;
    display->setCursor(x, y);
    display->print(removeAccents(text));
}

void DisplayManager::drawScrollingText(const String &text, int y, int textSize)
{
    display->setTextSize(textSize);

    int charWidth = 6 * textSize;
    int textWidth = text.length() * charWidth;
    int screenWidth = display->width();

    if (textWidth <= screenWidth)
    {
        drawCenteredText(removeAccents(text), y, textSize);
        return;
    }

    int scrollOffset = (animationCounter / 2) % (textWidth + screenWidth);
    int x = screenWidth - scrollOffset;

    display->setCursor(x, y);
    display->print(removeAccents(text));

    if (x + textWidth < screenWidth)
    {
        display->setCursor(x + textWidth + 10, y);
        display->print(removeAccents(text));
    }

    animationCounter++;
}

void DisplayManager::clear()
{
    display->clearDisplay();
    display->display();
}

void DisplayManager::showMessage(const String &message, int duration)
{
    unsigned long startTime = millis();

    while (millis() - startTime < duration)
    {
        display->clearDisplay();
        drawCenteredText(removeAccents(message), 25, 1);
        display->display();
        delay(50);
    }
}

void DisplayManager::setBrightness(uint8_t brightness)
{
    display->ssd1306_command(SSD1306_SETCONTRAST);
    display->ssd1306_command(brightness);
}

String removeAccents(const String &input)
{
    String output = input;
    output.replace("é", "e");
    output.replace("è", "e");
    output.replace("ê", "e");
    output.replace("à", "a");
    output.replace("ù", "u");
    output.replace("ç", "c");
    output.replace("ô", "o");
    output.replace("ï", "i");
    output.replace("î", "i");
    output.replace("É", "E");
    output.replace("À", "A");
    output.replace("Ç", "C");
    // Ajoute d’autres si besoin
    return output;
}
