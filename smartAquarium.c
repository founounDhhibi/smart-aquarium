sbit LCD_RS at RC0_bit;
sbit LCD_EN at RC1_bit;
sbit LCD_D4 at RC2_bit;
sbit LCD_D5 at RC3_bit;
sbit LCD_D6 at RC4_bit;
sbit LCD_D7 at RC5_bit;

sbit LCD_RS_Direction at TRISC0_bit;
sbit LCD_EN_Direction at TRISC1_bit;
sbit LCD_D4_Direction at TRISC2_bit;
sbit LCD_D5_Direction at TRISC3_bit;
sbit LCD_D6_Direction at TRISC4_bit;
sbit LCD_D7_Direction at TRISC5_bit;

sbit greenLED  at RD3_bit;
sbit blueLED   at RD0_bit;
sbit redLED    at RD4_bit;
sbit lamp1     at RD6_bit;
sbit relay     at RD5_bit;
sbit lamp2     at RD7_bit;
sbit heater    at RA0_bit;
sbit foodMotor at RD1_bit;
sbit buzzer    at RD2_bit;
sbit niveau    at RB5_bit;
sbit lumin     at RB6_bit;
sbit buttonConsult  at RB1_bit;

#define SetHigh(pin) (pin = 1)
#define SetLow(pin)  (pin = 0)

#define CRITICAL_TEMP 20
#define QUALITY_THRESHOLD 70

void waterLevel() {
    SetLow(greenLED);
    SetHigh(redLED);
    SetHigh(buzzer);
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Out(1, 1, "Niveau d'eau bas");
    Delay_ms(1000);
}

void food() {
    int i;
    SetLow(greenLED);
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Out(1, 1, "Distribution");
    Lcd_Out(2, 1, "de nourriture");
    SetHigh(foodMotor);
    for (i = 0; i < 3; i++) {
        SetHigh(redLED);
        Delay_ms(500);
        SetLow(redLED);
        Delay_ms(500);
    }
    Delay_ms(2000);
}

void log_alert_to_eeprom(unsigned short address, unsigned short value) {
    EEPROM_Write(address, value);
    Delay_ms(20);
}

void consult_alerts() {
    unsigned short temp_alert = EEPROM_Read(0x00);
    unsigned short quality_alert = EEPROM_Read(0x01);
    unsigned short level_alert = EEPROM_Read(0x02);


    if (temp_alert == 1) {
        Lcd_Cmd(_LCD_CLEAR);
        Lcd_Out(1, 1, "EEPROM:");
        Lcd_Out(2, 1, "Temp Critique");
        Delay_ms(150);
        log_alert_to_eeprom(0x00, 0);
    }
    if (quality_alert == 2) {
        Lcd_Cmd(_LCD_CLEAR);
        Lcd_Out(1, 1, "EEPROM:");
        Lcd_Out(2, 1, "Qualite Delicate");
        Delay_ms(150);
        log_alert_to_eeprom(0x01, 0);
    }
    if (level_alert == 3) {
        Lcd_Cmd(_LCD_CLEAR);
        Lcd_Out(1, 1, "EEPROM:");
        Lcd_Out(2, 1, "Niveau Bas");
        Delay_ms(150);
        log_alert_to_eeprom(0x02, 0);
    }
     else{
        Lcd_Cmd(_LCD_CLEAR);
        Lcd_Out(1, 1, "EEPROM:");
        Lcd_Out(2, 1, "Aucune Alerte");
    }
    Delay_ms(1000);
}

int read, m, NB = 183;
unsigned char mm;

void main() {
    int quality, tempC;

    TRISD = 0;
    TRISB = 0xFF;
    TRISC = 0;

    ADC_Init();
    INTCON.GIE = 1;
    INTCON.INTE = 1;
    OPTION_REG.INTEDG = 0;
    INTCON.RBIE = 1;
    OPTION_REG.T0CS = 0;
    OPTION_REG.PSA = 0;
    OPTION_REG.PS0 = 1;
    OPTION_REG.PS1 = 1;
    OPTION_REG.PS2 = 1;
    TMR0 = 0;

    SetLow(buzzer);
    SetLow(foodMotor);
    SetLow(lamp1);
    SetLow(lamp2);
    SetLow(heater);
    SetLow(redLED);
    SetLow(blueLED);

    SetHigh(greenLED);

    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    Lcd_Out(1, 1, "etatNormal");
    Lcd_Out(2, 1, "modeJour");

    while (1) {
        tempC = ADC_Read(0) * 0.496;
        quality = ADC_Read(1) * 0.496;

        if (m == 1) {
            waterLevel();
            SetHigh(redLED);
            SetHigh(buzzer);
            log_alert_to_eeprom(0x02,3);
            m = 0;
        }
        if (buttonConsult) {
            consult_alerts();
        }
        if (m == 2) {
            SetLow(greenLED);
            lamp1 = 1;
            lamp2 = 1;
            Lcd_Cmd(_LCD_CLEAR);
            Lcd_Out(1, 1, "Mode nuit");
            Delay_ms(2000);
            m = 0;
        }
        if (m == 3) {
            food();
            m = 0;
        }
        if (tempC < CRITICAL_TEMP) {
            SetLow(greenLED);
            SetHigh(redLED);
            relay=1;
            SetHigh(buzzer);
            Lcd_Cmd(_LCD_CLEAR);
            Lcd_Out(1, 1, "Temp Critical!");
            log_alert_to_eeprom(0x00,1);
            Delay_ms(1000);
        } else if (quality < QUALITY_THRESHOLD) {
            SetLow(greenLED);
            SetHigh(blueLED);
            Lcd_Cmd(_LCD_CLEAR);
            Lcd_Out(2, 1, "quality low");
            log_alert_to_eeprom(0x01,2);
            Delay_ms(1000);
        } else {
            SetLow(buzzer);
            SetLow(foodMotor);
            SetLow(lamp1);
            relay=0;
            SetLow(lamp2);
            SetLow(heater);
            SetLow(redLED);
            SetLow(blueLED);
            SetHigh(greenLED);
            Lcd_Cmd(_LCD_CLEAR);
            Lcd_Out(1, 1, "etatNormal");
            Lcd_Out(2, 1, "modeJour");
            Delay_ms(1000);
        }
    }
}

void interrupt() {
    if (INTCON.INTF) {
        m = 3;
        INTCON.INTF = 0;
    }
    if (INTCON.T0IF) {
        NB--;
        if (NB == 0) {
            NB = 183;
            SetHigh(redLED);
            INTCON.T0IE = 0;
        }
        INTCON.T0IF = 0;
    }
    if (INTCON.RBIF) {
        if (niveau == 1) {
            m = 1;
            INTCON.T0IE = 1;
        }
        if (lumin == 0) {
            m = 2;
        }
        INTCON.RBIF = 0;
    }
}
