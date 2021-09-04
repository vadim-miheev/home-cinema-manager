#include <IRremote.h>

// Pins numbers
#define PC_L A4
#define PC_R A5

// Array keys
#define OFF_KEY 0
#define PC_KEY 1
#define BT_KEY 2

//Settings
#define TURN_OFF_SEQUENCE 360
#define MEASUREMENT_SENSITIVITY_PC 10
#define WINNING_SEQUENCE 3

IRsend irsend;
int testCount = 0;
int ChanelsStats[3];
int winnersStats[3];
int prevChanel = 0;
int currentCinemaCenterChanel = 0;
unsigned long last_time_winner;
unsigned long last_time_ir;

int prev_pc_l = 0;
int prev_pc_r = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(PC_L, INPUT);
  pinMode(PC_R, INPUT);
  analogReference(INTERNAL);
  Serial.begin(9600);
  int ChanelsStats[3] = {0,0,0};
  int winnersStats[3] = {1,1,1};
  last_time_winner = millis();
  last_time_ir = millis();
}

void loop() {
  ChanelsStats[currentChanel()]++;
  
  if (millis() - last_time_winner > 300){
    last_time_winner = millis();
    
    int winner = checkWinner();
    if(prevChanel != winner) { //Проверка нужно ли сбрасывать статистику побед подряд
      for (int i = 0; i < (sizeof(ChanelsStats) / 2); i++){
        winnersStats[i] = 0;
      }
    }
    winnersStats[winner]++;
    int currentChanelSequence = winnersStats[winner];
    if ((currentChanelSequence == WINNING_SEQUENCE && winner != OFF_KEY) || currentChanelSequence == TURN_OFF_SEQUENCE){ //Условие нужного колличества побед подряд
      sendIrSignal(winner);
    }
    prevChanel = winner; //Обновление "Предыдущего победителя"
    for (int i = 0; i < (sizeof(ChanelsStats) / 2); i++){
        ChanelsStats[i] = 0;
    }
  }
}

int checkWinner () {
  int winner = OFF_KEY;

  if (ChanelsStats[PC_KEY] != 0 && ChanelsStats[OFF_KEY] / ChanelsStats[PC_KEY] < 9){ //Аналогично ^^
      winner = PC_KEY;
  }
  
  Serial.print("Count off: ");
  Serial.println(ChanelsStats[OFF_KEY]);
  Serial.print("Count pc: ");
  Serial.println(ChanelsStats[PC_KEY]);
  Serial.print("Winner: ");
  Serial.println(winner);
  return winner;
}

//Проверка каналов на наличие звука. 
//Возвращает текущий канал.
int currentChanel() {
  int result = OFF_KEY;
  int c_l = analogRead(PC_L);
  int c_r = analogRead(PC_R);
  int sens = MEASUREMENT_SENSITIVITY_PC;
  bool sound_in_pc_l = abs(prev_pc_l - c_l) > sens;
  bool sound_in_pc_r = abs(prev_pc_r - c_r) > sens;
  
  if(sound_in_pc_l || sound_in_pc_r){
    //Serial.println(PC_KEY);
    result = PC_KEY;
  }
  
  prev_pc_l = c_l;
  prev_pc_r = c_r;

  return result;
  
}

//Отправляет на выбранный канал сигнал с повторением 9-10 раз
void sendIrSignal (int channel) {
  if (currentCinemaCenterChanel != channel) {
    currentCinemaCenterChanel = channel;
    
    Serial.print("Send IR: ");
    Serial.println(channel);
    
    last_time_ir = millis();
    while(millis() - last_time_ir < 500){ //время повторения IR сигнала
      if ((millis() - last_time_ir) % 50 == 0) { //пауза 50 миллисекунд между каждой отправкой команды
        switch (channel){
          case 0 : 
            //Serial.println("OFF");
            irsend.sendRC5(0xD0C, 12);
            break;
          case 1 : 
            //Serial.println("PC");
            irsend.sendRC5(0x53F, 12); // CD режим
            // irsend.sendRC5(0x6BF, 12); // CD-R TAPE режим           
        }
      }
    }
  }
}
