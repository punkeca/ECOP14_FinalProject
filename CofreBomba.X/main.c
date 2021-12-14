#include <pic18f4520.h>
#include "config.h"
#include "lcd.h"
#include "keypad.h"
#include "ssd.h"
#include "timer.h"
#include "atraso.h"
#include "rgb.h"
#include "pwm.h"


#define L_ON  0x0F
#define L_OFF 0x08
#define L_CLR 0x01
#define L_L1  0x80
#define L_L2  0xC0

static char password[8];
static char input[8];
char try = 0;

int checkPassword() {
    int correct;
    char i = 0;
    unsigned int newkey = 0;
    lcdCommand(L_CLR);
    lcdString("Password:");
    lcdCommand(L_L2);
    correct = 1;
    while (i < 8) {
        kpDebounce();
        if ((kpReadKey() != 0) && (kpRead() != newkey)) {
            newkey = kpRead();
            input[i] = kpReadKey();
            lcdChar('*');
            if (input[i] != password[i]) {
                correct = 0;
            }//end if
            i++;
        }//end if
    }//end while
    atraso_ms(1000);
    lcdCommand(L_CLR);

    if (correct) {
        lcdString("Access Granted");
        try = 0; //reset attempts to avoid explosion
        turnOn(2);
        atraso_ms(10000);
        turnOff(2);
    } else {
        lcdString("Access Denied");
        try++; //increase attempts to explode
        turnOn(1);
        atraso_ms(3000);
        turnOff(1);
    }//end if


    return correct;
}//end void

int explosion() {
    unsigned int cont = 1000;
    int i;
    if (try >= 3) {
        lcdCommand(L_CLR);
        lcdString("Too many tries");
        atraso_ms(1000);
        lcdCommand(L_CLR);
        lcdString("EXPLODING...");

        while(cont!=0) {
            //cont++;
            i--;
            if (i == 0) {
                cont -= 100;
                i = 60;
            }//end if
            ssdDigit(((cont / 1000) % 10), 0); //100s
            ssdDigit(((cont / 100) % 10), 1); //10s
            ssdDigit(((i / 10) % 10), 2); //1s
            ssdDigit(((i / 1) % 10), 3); //0.1s
            ssdUpdate();
            atraso_ms(20);
        }//end while
        lcdCommand(L_CLR);
        lcdString("BOOM!!");
        (*(volatile __near unsigned char*) 0xF83) = 0xFF; //turn on all LEDS 
        pwmFrequency(300); //defines buzzer frequency.
        for(;;){
            ssdDigit(13, 0); //100s
            ssdDigit(1, 1); //10s
            ssdDigit(14, 2); //1s
            ssdDigit(13, 3); //100s
            ssdUpdate();
            atraso_ms(20);
            pwmSet(100);//set buzzer to high
        }
    }//end if
    return 0;
}//end void

void setPassword() {
    char i = 0;
    unsigned int key = 0;
    lcdCommand(L_L2);
    while (i < 8) {
        kpDebounce();
        if ((kpReadKey() != 0) && (kpRead() != key)) {
            key = kpRead();
            password[i] = kpReadKey();
            lcdChar('*');
            i++;
        }//end if
    }//end while
    atraso_ms(1000);
    lcdCommand(L_CLR);

    //lcdString(password);
}//end void
    

void main(void) {
    char slot;
    lcdInit();
    kpInit();
    ssdInit();
    timerInit();
    rgbInit();
    pwmInit();
    (*(volatile __near unsigned char*) 0xF95) = 0x00; //sets as output

    lcdCommand(L_L1);
    lcdString("Configure:");
    lcdCommand(L_L2);
    setPassword();

    for (;;) {
        timerReset(5000);
        switch (slot) {
            case 0:
                checkPassword();
                slot = 1;
                break;
            case 1:
                turnOff(1);
                turnOff(2);
                turnOff(4);
                slot = 2;
                break;
            case 2:
                explosion();
                slot = 0;
                break;
            default:
                slot = 0;
                break;
        }//end switch
        ssdUpdate();
        timerWait();
    }//end for
}//end main


