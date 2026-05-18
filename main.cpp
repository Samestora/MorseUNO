#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

#define DOT_BTN 5
#define DASH_BTN 6
#define BUZZER 4

String morse = "";
unsigned long lastPress = 0;

LiquidCrystal_I2C lcd(0x27, 16, 2);

SemaphoreHandle_t morseMutex;

char decodeMorse(String code) {
  if (code == ".-") return 'A';
  if (code == "-...") return 'B';
  if (code == "-.-.") return 'C';
  if (code == "-..") return 'D';
  if (code == ".") return 'E';
  if (code == "..-.") return 'F';
  if (code == "--.") return 'G';
  if (code == "....") return 'H';
  if (code == "..") return 'I';
  if (code == ".---") return 'J';
  if (code == "-.-") return 'K';
  if (code == ".-..") return 'L';
  if (code == "--") return 'M';
  if (code == "-.") return 'N';
  if (code == "---") return 'O';
  if (code == ".--.") return 'P';
  if (code == "--.-") return 'Q';
  if (code == ".-.") return 'R';
  if (code == "...") return 'S';
  if (code == "-") return 'T';
  if (code == "..-") return 'U';
  if (code == "...-") return 'V';
  if (code == ".--") return 'W';
  if (code == "-..-") return 'X';
  if (code == "-.--") return 'Y';
  if (code == "--..") return 'Z';
  if (code == ".----") return '1';
  if (code == "..---") return '2';
  if (code == "...--") return '3';
  if (code == "....-") return '4';
  if (code == ".....") return '5';
  if (code == "-....") return '6';
  if (code == "--...") return '7';
  if (code == "---..") return '8';
  if (code == "----.") return '9';
  if (code == "-----") return '0';
  return '?';
}

void TaskDot(void *pvParameters);
void TaskDash(void *pvParameters);
void TaskDecode(void *pvParameters);

void setup() {
  pinMode(DOT_BTN, INPUT_PULLUP);
  pinMode(DASH_BTN, INPUT_PULLUP);
  pinMode(BUZZER, OUTPUT);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Actual:");
  lcd.setCursor(0, 1);
  lcd.print("Morse:");

  Serial.begin(9600);

  morseMutex = xSemaphoreCreateMutex();

  xTaskCreate(TaskDot, "Dot", 128, NULL, 1, NULL);
  xTaskCreate(TaskDash, "Dash", 128, NULL, 1, NULL);
  xTaskCreate(TaskDecode, "Decode", 128, NULL, 1, NULL);
}

void loop() {}

void TaskDot(void *pvParameters) {
  while (1) {
    if (digitalRead(DOT_BTN) == LOW) {
      if (xSemaphoreTake(morseMutex, portMAX_DELAY)) {
        morse += ".";

        lcd.setCursor(6, 1);
        lcd.print("          ");
        lcd.setCursor(6, 1);
        lcd.print(morse);

        lastPress = millis();
        xSemaphoreGive(morseMutex);
      }

      tone(BUZZER, 1000, 100);
      vTaskDelay(200 / portTICK_PERIOD_MS);
    }
  }
}

void TaskDash(void *pvParameters) {
  while (1) {
    if (digitalRead(DASH_BTN) == LOW) {
      if (xSemaphoreTake(morseMutex, portMAX_DELAY)) {
        morse += "-";

        lcd.setCursor(6, 1);
        lcd.print("          ");
        lcd.setCursor(6, 1);
        lcd.print(morse);

        lastPress = millis();
        xSemaphoreGive(morseMutex);
      }

      tone(BUZZER, 1000, 300);
      vTaskDelay(300 / portTICK_PERIOD_MS);
    }
  }
}

void TaskDecode(void *pvParameters) {
  static bool letterPrinted = false;
  static bool wordPrinted = false;
  static int lcdPos = 7;

  while (1) {
    if (xSemaphoreTake(morseMutex, portMAX_DELAY)) {
      unsigned long pause = millis() - lastPress;

      if (morse.length() > 0 && pause > 500 && !letterPrinted) {
        char decoded = decodeMorse(morse);

        Serial.print(decoded);

        lcd.setCursor(lcdPos, 0);
        lcd.print(decoded);
        lcdPos++;

        if (lcdPos >= 16) {
          lcd.clear();
          lcd.setCursor(0, 0);
          lcd.print("Actual:");
          lcd.setCursor(0, 1);
          lcd.print("Morse:");
          lcdPos = 7;
        }

        morse = "";

        lcd.setCursor(6, 1);
        lcd.print("          ");

        letterPrinted = true;
      }

      if (pause > 1000 && letterPrinted && !wordPrinted) {
        Serial.print(" ");

        lcd.setCursor(lcdPos, 0);
        lcd.print(" ");
        lcdPos++;

        wordPrinted = true;
      }

      if (morse.length() > 0) {
        letterPrinted = false;
        wordPrinted = false;
      }

      xSemaphoreGive(morseMutex);
    }

    vTaskDelay(50 / portTICK_PERIOD_MS);
  }
}
