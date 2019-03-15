/*==============================================================
| Sistema de control para un electrocoagulador de vinazas       |
| Núcleo STM32F42ZI, LCD 16x2, Teclado Matricial 4x4            |
| Proyecto final: Digital II                                    |
| Emmanuel Vanegas Arias y Andrés Saavedra Muñoz                |
| 13 de marzo de 2019                                           |
===============================================================*/

#include "mbed.h"
#include "keypad.h"
#include "TextLCD.h"
#include "QEI.h"

/************************* IO **************************/
DigitalOut E_1(D8,0);               // Control de electrovalvula 1
DigitalOut E_2(D9,0);               // Control de electrovalvula 2
DigitalOut E_3(D10,0);              // Control de electrovalvula 3
DigitalOut E_4(D12,0);              // Control de electrovalvula 4
PwmOut motor_plus(PB_8);            // Variables para el motor
PwmOut motor_cero(PB_9);        
QEI encoder(PD_6,PD_5,NC,4576,QEI::X2_ENCODING);

/******************* INTERRUPCIONES ********************/
Timeout t;
InterruptIn button(USER_BUTTON);    // Cancelar operación

/********************* VARIABLES ***********************/
int tempUser1 = 1;                  // Tiempo de electrocoagulacion (tanque 1)
int tempUser2 = 1;                  // Tiempo de reposo de la vinaza en el tanque 2
int tempUser3 = 1;                  // Tiempo de vaciado del agua del tanque 2
int tempUser4 = 1;                  // Tiempo de vaciado de residuos sólidos del tanque 2
int pwm = 50;                       // Duty cycle del motor

int tiempoLlenado = 5;              // Tiempo de llenado del tanque 1
int timeScale = 60;                 // 1 -> segundos, 60 -> minutos. Afecta los tiempos tempUser1 y tempUser2
 
int a = 0;
int b = 0;
int c = 0;
int running = 0;
int finished = 0;
int canceled = 0;
int campo = 1;

/******************** TECLADO Y LCD ***********************/
// Definición de los IO del teclado y LCD
Serial pc(USBTX, USBRX); //rs, e, d4, d5, d6, d7
TextLCD lcd(PC_8,PC_9,PC_10,PC_11,PC_12,PD_2);
Keypad keypad(D3, D2, D1, D0, D7, D6, D5, D4);

/********************* FUNCIONES **************************/
void printLCD() {                               // Función para la impresión de la pantalla LCD
    while(1) {
        lcd.cls();
        lcd.locate(0,0);
        lcd.printf("t1=%d",tempUser1);
        lcd.locate(6,0);
        lcd.printf("t2=%d",tempUser2);
        lcd.locate(0,1);
        lcd.printf("t3=%d",tempUser3);
        lcd.locate(6,1);
        lcd.printf("t4=%d",tempUser4);
        lcd.locate(12,0);
        lcd.printf("PWM");
        lcd.locate(12,1);
        lcd.printf("%d",pwm);
        wait_ms(150);
        switch(campo) {                         // La variable pendiente de modificar titila con un periodo de 300 ms
            case  1:
                lcd.locate(0,0);
                lcd.printf("t1=   ",tempUser1);
                break;
            case  2:
                lcd.locate(6,0);
                lcd.printf("t2=   ",tempUser2);
                break;
            case  3:
                lcd.locate(0,1);
                lcd.printf("t3=   ",tempUser3);
                break;
            case  4:
                lcd.locate(6,1);
                lcd.printf("t4=   ",tempUser4);
                break;
            case  5:
                lcd.locate(12,1);
                lcd.printf("   ");
                break;
            default: ;
                break;
        }
        wait_ms(150);
    }
}

int getNum(){                                   // Función que retorna un valor entero por cada tecla presionada
    while(1){
        char key = keypad.getKey();
        switch(key){
            case '0': return 0;
            case '1': return 1;
            case '2': return 2;
            case '3': return 3;
            case '4': return 4;
            case '5': return 5;
            case '6': return 6;
            case '7': return 7;
            case '8': return 8;
            case '9': return 9;
            case 'A':
                campo++;
                if(campo == 6) campo = 1;
                return -2;
          //case 'B': return -3;
          //case 'C': return -4;
            case 'D': return -5;
            default: ; break;
        }
    }
}
void reiniciar(){                               // Función para apagar las válvulas y el motor
    E_1 = 0;
    E_2 = 0;
    E_3 = 0;
    E_4 = 0;
    
    motor_plus.write(0.0f);                     
    motor_cero.write(0.0f);
}


void init(){                                    // Función de iniciación del programa
    
    keypad.enablePullUp();
    wait(1);
    lcd.locate(0,0);
    lcd.printf("  Control para\n");
    lcd.locate(0,1);
    lcd.printf("electrocoagular\n");
    wait(1.5);
    
    return;
}

void setUserTimes(){                            // Algoritmo para la definición de las variables tempUser1, tempUser2,
    while(1){                                   // tempUser3, tempUser4 y PWM
    setvalue:                                   // Presionar la tecla 'A' cambia el campo a modificar
        while(keypad.getKey()!='*'){}           // Presionar la tecla 'D' inicia el proceso de electrocuoagulación
        a = 100*getNum();
            if(a == -200) goto setvalue;
            if(a == -500) break;
        while(keypad.getKey()!='*'){}
        b = 10*getNum();
            if(b == -20) goto setvalue;
            if(b == -50) break;
        while(keypad.getKey()!='*'){}
        c = getNum();
            if(c == -2) goto setvalue;
            if(c == -5) break;

        switch(campo){
            case 1:
                tempUser1 = a+b+c;
                break;
            case 2:
                tempUser2 = a+b+c;
                break;
            case 3:
                tempUser3 = a+b+c;
                break;
            case 4:
                tempUser4 = a+b+c;
                break;
            case 5:
                pwm=a+b+c;
                if(pwm < 5)   pwm = 5;
                if(pwm > 100) pwm = 100;
                break;
            default: ; break;
        }
    }
    return;
}

/********************* CONTROL INTERRUPCIONES (TICKER) *************************/
void vaciadoSolido(){                           // Fin del procedimiento
    E_4 = 0;                                    // Se cierra la EV 4 y se alza la bandera "finished"
    finished = 1;
}

void vaciadoLiquido(){                          // Inicio del vaciado del sólido
    E_3 = 0;                                    // Se cierra la EV 3 y se abre la EV 4
    E_4 = 1;
    t.attach(&vaciadoSolido, tempUser4);
}

void reposo(){                                  // Inicio del vaciado del líquido
    E_3 = 1;                                    // Se abre la EV 3
    t.attach(&vaciadoLiquido, tempUser3);
}

void vaciado1(){                                // Inicio de la etapa de reposo en el tanque 2
    E_2 = 0;                                    // Se cierra la EV2
    t.attach(&reposo, timeScale*tempUser2);
}

void electro(){                                 // Inicio del vaciado del primer tanque al segundo
    motor_plus.write(0.0f);                     // Se apaga el motor y se abre la EV 2
    motor_cero.write(0.0f);
    E_2 = 1;
    t.attach(&vaciado1, tiempoLlenado);
}

void tiempoValvula1(){                          // Inicio del proceso de electrocoagulación
    E_1 = 0;                                    // Cierre de la EV 1 y encendido del motor 
    motor_plus.write((float)pwm*0.01f);

    t.attach(&electro, timeScale*tempUser1);
}

void cancel_op(){                               // Llamado por la interrupción externa "button"
    canceled = 1;                               // Cancela la operación en curso
    t.detach();
}

/**************************** MAIN ********************************/

int main(){
    motor_plus.period(0.015f);
    motor_cero.write(0.0f);

    button.rise(&cancel_op);                    // Habilita (masks) la interrupción externa para cancelar la operación
    while(1){
        if(running == 0){                       // Llamado cada vez que se reinicia el sistema
            running = 1;
            init();                             // Función de iniciación del sistema
            Thread lcdprint;                    // Inicia un hilo independiente para la impresión de la LCD
            lcdprint.start(printLCD);    

            campo = 1;
            setUserTimes();                     // Función en loop para definir las variables del programa

            campo = 0;
            finished = 0;
            canceled = 0;

            E_1 = 1;
            t.attach(&tiempoValvula1, tiempoLlenado); // Inicio del proceso de electrocuagulación
        }
        if(finished == 1){                      // Llamado si el proceso termina exitosamente
            reiniciar();
            lcd.cls();
            lcd.locate(0,0);
            lcd.printf("Finalizado\n");
            running = 0;
            wait(2);
        }
        if(canceled == 1){                      // Llamado si el proceso es cancelado
            reiniciar();
            lcd.cls();
            lcd.locate(0,0);
            lcd.printf("Operacion\n");
            lcd.locate(0,1);
            lcd.printf("cancelada\n");
            running = 0;
            wait(2);
        }
    }
}
