#include <Wire.h>
#include <RDA5807.h>
#include <LiquidCrystal_I2C.h>
#include <Preferences.h>

#define SDA_PIN 21
#define SCL_PIN 22

#define ENC_CLK 32
#define ENC_DT 33
#define ENC_SW 25

RDA5807 radio;
LiquidCrystal_I2C lcd(0x27, 16, 2);
Preferences prefs;

uint16_t frequency;
uint8_t volume;

int lastCLK;
bool refreshLCD = true;

unsigned long saveTimer = 0;
bool needSave = false;
bool inMenu = false;
bool inVolume = false;

byte menuIndex = 0;

const char *menuItems[] =
{
  "Volume",
  "Scan",
  "Save Station",
  "Load Station",
  "Exit"
};

const byte menuCount = 5;

bool editingVolume = false;
bool savingStation = false;
bool confirmReplace = false;
bool waitingReplace = false;
bool loadingStation = false;

uint16_t tempFrequency = 0;

byte presetIndex = 0;

uint16_t presets[10];
bool scanning = false;

const uint8_t RSSI_LIMIT = 25;
bool lastButton = HIGH;
unsigned long buttonTime = 0;

void drawScreen()
{
    lcd.setCursor(0,0);
    lcd.print("FM ");

    lcd.print(frequency/100.0,1);

    lcd.print(" ");

    if(radio.isStereo())
        lcd.print("ST ");
    else
        lcd.print("MO");

    lcd.print("   ");

    lcd.setCursor(0,1);

    lcd.print("RSSI:");

    uint8_t rssi = radio.getRssi();

    if(rssi<10) lcd.print(" ");

    lcd.print("           ");      
    lcd.setCursor(5,1);     
    lcd.print(rssi);
}
void drawMenu()
{
    lcd.clear();

    byte firstItem = (menuIndex / 2) * 2;

    
    lcd.setCursor(0,0);

    if(menuIndex == firstItem)
        lcd.print(">");
    else
        lcd.print(" ");

    lcd.print(menuItems[firstItem]);

    
    if(firstItem + 1 < menuCount)
    {
        lcd.setCursor(0,1);

        if(menuIndex == firstItem + 1)
            lcd.print(">");
        else
            lcd.print(" ");

        lcd.print(menuItems[firstItem + 1]);
    }
}

void drawSaveScreen()
{
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("Save Station");

    lcd.setCursor(0,1);

    if(presetIndex == 10)
{
    lcd.print("Exit");
}
else
{
    lcd.print("P");
    lcd.print(presetIndex + 1);
    lcd.print(" ");

    if(presets[presetIndex] == 0)
    {
        lcd.print("Empty");
    }
    else
    {
        lcd.print(presets[presetIndex] / 100.0, 1);
        lcd.print("MHz");
    }
}
}
void drawLoadScreen()
{
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("Load Station");

    lcd.setCursor(0,1);

    if(presetIndex == 10)
    {
        lcd.print("Exit");
    }
    else
    {
        lcd.print("P");
        lcd.print(presetIndex + 1);
        lcd.print(" ");

        if(presets[presetIndex] == 0)
        {
            lcd.print("Empty");
        }
        else
        {
            lcd.print(presets[presetIndex] / 100.0,1);
            lcd.print("MHz");
        }
    }
}
void drawReplaceScreen()
{
    lcd.clear();

    lcd.setCursor(0,0);
    lcd.print("Replace ?");

    lcd.setCursor(0,1);

    lcd.print("P");
    lcd.print(presetIndex + 1);
    lcd.print(" ");

    lcd.print(presets[presetIndex] / 100.0, 1);
    lcd.print("MHz");
    lcd.setCursor(13,0);
    lcd.print("OK");
}
void showBootScreen()
{
    lcd.clear();

    lcd.setCursor(2,0);
    lcd.print("CYBER FM");      // Change this text ("CYBER FM") to display your own name or logo during startup.

    lcd.setCursor(5,1);
    lcd.print("v1.0");

    delay(1000);

    lcd.clear();

    lcd.setCursor(4,0);
    lcd.print("Loading");

    lcd.setCursor(0,1);

    for(int i=0;i<16;i++)
    {
        lcd.write(byte(255));   
        delay(200);
    }

    delay(1000);

    lcd.clear();
}
void checkButton()
{
    bool state = digitalRead(ENC_SW);

    if(state == LOW && lastButton == HIGH)
    {
        if(millis() - buttonTime > 250)
        {
            
            buttonTime = millis();
            if(savingStation)
            {
                if(waitingReplace)
                {
                    presets[presetIndex] = frequency;

                    String key = "P" + String(presetIndex);

                    prefs.putUShort(key.c_str(), frequency);

                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("Replaced!");

                    lcd.setCursor(0,1);
                    lcd.print("P");
                    lcd.print(presetIndex+1);

                    delay(1000);

                    waitingReplace = false;
                    savingStation = false;

                    drawScreen();

                    return;
                }
                if(presetIndex == 10)
                {
                    savingStation = false;
                    waitingReplace = false;
                    inMenu = true;

                    drawMenu();

                    return;
                }
                if(presets[presetIndex]==0)
                {
                    presets[presetIndex]=frequency;

                    String key="P"+String(presetIndex);

                    prefs.putUShort(key.c_str(),frequency);

                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("Saved!");

                    lcd.setCursor(0,1);
                    lcd.print("P");
                    lcd.print(presetIndex+1);

                    delay(1000);

                    savingStation=false;

                    drawScreen();

                    lastButton = HIGH;

                    return;
                }
                else
                {
                    waitingReplace=true;

                    drawReplaceScreen();

                    return;
                }
            }
            if(loadingStation)
            {
                if(presetIndex == 10)
                {
                    loadingStation = false;
                    inMenu = true;
                    drawMenu();
                    return;
                }

                if(presets[presetIndex] == 0)
                {
                    lcd.clear();
                    lcd.setCursor(0,0);
                    lcd.print("Empty!");

                    delay(1000);

                    drawLoadScreen();
                    return;
                }

                frequency = presets[presetIndex];

                radio.setFrequency(frequency);

                prefs.putUShort("freq", frequency);

                loadingStation = false;

                drawScreen();

                return;
            }
                
            if(editingVolume)
            {
                editingVolume = false;
                inMenu = true;
                drawMenu();
            }
            else if(!inMenu)
            {
                inMenu = true;
                drawMenu();
            }
            else
            {
                switch(menuIndex)
                {
                    case 0: // Volume
                        editingVolume = true;
                        inMenu = false;

                        lcd.clear();
                        lcd.setCursor(0,0);
                        lcd.print("Volume");

                        lcd.setCursor(0,1);
                        lcd.print(volume);
                        break;

                    case 1: // Scan

                        lcd.clear();
                        lcd.setCursor(0,0);
                        lcd.print("Scanning...");

                        radio.seek(RDA_SEEK_STOP, RDA_SEEK_UP);

                        frequency = radio.getRealFrequency();

                        radio.setFrequency(frequency);

                        refreshLCD = true;

                        prefs.putUShort("freq", frequency);

                        inMenu = false;

                        break;

                    case 2:

                        savingStation=true;
                        inMenu=false;

                        presetIndex=0;

                        drawSaveScreen();

                        break;

                    case 3: // Load

                        loadingStation = true;
                        inMenu = false;

                        presetIndex = 0;

                        drawLoadScreen();

                        break;

                    case 4: // Exit
                        inMenu = false;
                        drawScreen();
                        break;
                }
            }
        }
    }

    lastButton = state;
}
void setup()
{
    Serial.begin(115200);

    Wire.begin(SDA_PIN,SCL_PIN);

    lcd.init();
    lcd.backlight();

    showBootScreen();

    pinMode(ENC_CLK,INPUT_PULLUP);
    pinMode(ENC_DT,INPUT_PULLUP);
    pinMode(ENC_SW,INPUT_PULLUP);

    lastCLK = digitalRead(ENC_CLK);

    prefs.begin("radio",false);
    for(int i=0;i<10;i++)
    {
        String key="P"+String(i);

        presets[i]=prefs.getUShort(key.c_str(),0);
    }

    frequency = prefs.getUShort("freq",10190);
    volume    = prefs.getUChar("vol",8);

    radio.setup();

    radio.setBand(0);
    radio.setSpace(0);

    radio.setVolume(volume);
    radio.setFrequency(frequency);

    drawScreen();
}

void loop()
{
    checkButton();
    int clk = digitalRead(ENC_CLK);
    if(scanning)
{
    frequency += 10;

    if(frequency > 10800)
        frequency = 8750;

    radio.setFrequency(frequency);

    delay(80);

    lcd.setCursor(0,1);
    lcd.print("        ");
    lcd.setCursor(0,1);
    lcd.print(frequency/100.0,1);

    if(radio.getRssi() >= RSSI_LIMIT)
    {
        scanning = false;

        refreshLCD = true;

        prefs.putUShort("freq", frequency);
    }

    return;
}
if(savingStation)
{
    if(clk!=lastCLK)
    {
        if(digitalRead(ENC_DT)!=clk)
        {
            if(presetIndex<10)
                presetIndex++;
        }
        else
        {
            if(presetIndex>0)
                presetIndex--;
        }

        drawSaveScreen();
    }

    lastCLK=clk;

    return;
}
if(loadingStation)
{
    if(clk != lastCLK)
    {
        if(digitalRead(ENC_DT) != clk)
        {
            if(presetIndex < 10)
                presetIndex++;
        }
        else
        {
            if(presetIndex > 0)
                presetIndex--;
        }

        drawLoadScreen();
    }

    lastCLK = clk;

    return;
}
    if(editingVolume)
    {
    if(clk != lastCLK)
    {
        if(digitalRead(ENC_DT) != clk)
        {
            if(volume < 15)
                volume++;
        }
        else
        {
            if(volume > 0)
                volume--;
        }

        radio.setVolume(volume);

        lcd.setCursor(0,1);
        lcd.print("   ");
        lcd.setCursor(0,1);
        lcd.print(volume);

        prefs.putUChar("vol", volume);
    }

    lastCLK = clk;
    return;
}
    if(clk!=lastCLK)
{
    if(inMenu)
    {
        if(digitalRead(ENC_DT)!=clk)
        {
            if(menuIndex < menuCount - 1)
                menuIndex++;
        }
        else
        {
            if(menuIndex>0)
                menuIndex--;
        }

        drawMenu();

        lastCLK=clk;

        return;
    }
        if(digitalRead(ENC_DT) != clk)
        {
            frequency += 10;

            if(frequency > 10800)
                frequency = 8750;
        }
        else
        {
            frequency -= 10;

            if(frequency < 8750)
                frequency = 10800;
        }

        radio.setFrequency(frequency);

        refreshLCD = true;

        needSave = true;
        saveTimer = millis();
    }

    lastCLK = clk;

    if(refreshLCD)
    {
        refreshLCD = false;
        drawScreen();
    }

    if(needSave && millis()-saveTimer > 2000)
    {
        needSave = false;

        prefs.putUShort("freq",frequency);
    }

}