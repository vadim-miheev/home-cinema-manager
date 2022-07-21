#include <IRremote.h>

// Pins numbers
#define SOUND_L A4
#define SOUND_R A5
#define POT_GND A0

//Settings
#define TURN_ON_SEQUENCE_VAL 3 //Количество замеров подряд с положительным результатом необходимое для включения
#define TURN_OFF_SEQUENCE_VAL 360 //Количество замеров подряд с отрицательным результатом необходимое для отключения
#define MEASUREMENT_SENSITIVITY 10


IRsend irsend;
unsigned long last_time_ir;
unsigned long last_time_compare;
unsigned int loop_results_for_ON = 0;
unsigned int loop_results_for_OFF = 0;
unsigned int turn_on_sequence = 0;
unsigned int turn_off_sequence = 0;
bool sound_detected_by_last_check = false;
bool current_system_state_is_on = false;
int prev_s_l = 0;
int prev_s_r = 0;


void setup() {
    // Arduino
    pinMode(SOUND_L, INPUT);
    pinMode(SOUND_R, INPUT);
    pinMode(POT_GND, OUTPUT);
    analogReference(EXTERNAL);
    Serial.begin(9600);
    
    // App
    last_time_ir = millis();
    last_time_compare = millis();
    sendIrSignal(false);
}

void loop() {
    checkSound();
    if (millis() - last_time_compare > 300){
        last_time_compare = millis();
        compareOnAndOf();
        checkSequences();
        
        loop_results_for_ON = 0;
        loop_results_for_OFF = 0;
    }
}

// На основании собранных данных из compareOnAndOf() отправляем IR сигнал
void checkSequences() {
    if (sound_detected_by_last_check) {
        if (turn_on_sequence == TURN_ON_SEQUENCE_VAL && !current_system_state_is_on) {
            current_system_state_is_on = true;
            sendIrSignal(true);
        } else if (turn_on_sequence > 2000) {
            turn_on_sequence = 1000; // защита от переполнения переменной
        }
    } else {
        if (turn_off_sequence == TURN_OFF_SEQUENCE_VAL) {
            current_system_state_is_on = false;
            sendIrSignal(false);
        } else if (turn_off_sequence > 2000) {
            turn_off_sequence = 1000; // защита от переполнения переменной
            sendIrSignal(false); // защита от случайного включения
        }
    }
}

// Сравниваем данные собранные с помощью checkSound() и сохраняем результат в счетчик
void compareOnAndOf() {
    bool sound_detected = loop_results_for_ON != 0 && loop_results_for_OFF / loop_results_for_ON < 9;
    if (sound_detected != sound_detected_by_last_check) {
        turn_on_sequence = 0;
        turn_off_sequence = 0;
        sound_detected_by_last_check = sound_detected;
    }
    if (sound_detected) {
        turn_on_sequence++;
    } else {
        turn_off_sequence++;
    }

    Serial.print("Count OFF: ");
    Serial.println(loop_results_for_OFF);
    Serial.print("Count ON: ");
    Serial.println(loop_results_for_ON);
    Serial.print("Winner: ");
    Serial.println(sound_detected ? "ON" : "OFF");
    Serial.println("----------------");
}

// Проверям есть ли звук в данный момент времени и сохраняем результат в счетчик 
void checkSound() {
    int s_l = analogRead(SOUND_L);
    int s_r = analogRead(SOUND_R);
    int sens = MEASUREMENT_SENSITIVITY;
    bool there_is_sound_in_l = abs(prev_s_l - s_l) > sens;
    bool there_is_sound_in_r = abs(prev_s_r - s_r) > sens;
    prev_s_l = s_l;
    prev_s_r = s_r; 

    if(there_is_sound_in_l || there_is_sound_in_r) {
        loop_results_for_ON++;
    } else {
        loop_results_for_OFF++;
    }
}

// Отправляем IR сигнал (on|off) с повторением 9-10 раз
void sendIrSignal (bool on) {
    Serial.print("Send IR: ");
    Serial.println(on ? "ON" : "OFF");
    last_time_ir = millis();
    while(millis() - last_time_ir < 500){ //время повторения IR сигнала
        if ((millis() - last_time_ir) % 50 == 0) { //пауза 50 миллисекунд между каждой отправкой команды
            if (on){
                irsend.sendRC5(0x53F, 12); // CD режим
                // irsend.sendRC5(0x6BF, 12); // CD-R TAPE режим
            } else {
                irsend.sendRC5(0xD0C, 12);  
            }
        }
    }
}
