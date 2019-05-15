
int cali_button = 9;
float cali_start_time;
float cali_time1 = 10000000; // 10 s
float voltage_off = 300;
float sum_power = 0;
float sum_average_power = 0;
int N = 100; // per 200 samples
int main_loop_counter = 0;

// averaging array
const int CIRAVG_SIZE = 10;
float ciravg_arr[CIRAVG_SIZE];
int ciravg_pt = 0;

int jump_pin = 6;

float average_low, average_high;
float output_averaging = 0;

void setup() {
  // put your setup code here, to run once:
  pinMode(jump_pin, OUTPUT);
  Serial.begin(9600);
  
  calibrate();
}

void loop() {
   if (main_loop_counter < N) {
    int adc = analogRead(0)-voltage_off;
    sum_power += pow(adc,2);
    main_loop_counter++;
   } else {
    main_loop_counter = 0;
    sum_average_power = sum_power / N;
    sum_power = 0;

    ciravg_arr[ciravg_pt] = sum_average_power;//getting the average from CIRAVG_SIZE data
    ciravg_pt = (ciravg_pt + 1) % CIRAVG_SIZE;
    float sum = 0;
    for(int i = 0; i < CIRAVG_SIZE; i++)
    {
        sum += ciravg_arr[i];
     }
    output_averaging = sum / CIRAVG_SIZE;
    
    Serial.println(output_averaging);
   }
   float map_out = map(output_averaging, average_low, average_high, (float)0, (float)100);
   if (map_out > 100) {
    map_out = 100;
   }
   if (map_out < 0) {
    map_out = 0;
   }
   Serial.print("Strengh(percentage): ");
   Serial.print(map_out); Serial.println("%");
   int write_out = map(map_out, (float)0, (float)100, (float)0, (float)255);
   // send data over serial
   write_out = 100;
   Serial.write(write_out / 256);
   Serial.write(write_out % 256);
  // delay(200);
}

void calibrate() {
  int N = 200;
  float sum_low = 0, sum_high = 0;
  float cali1_sum_power = 0;
  float cali1_counter = 0;
  
  Serial.println("Calibration started");
  Serial.println("relax");
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  // wait 2 s
  delay(1500);
  Serial.println("start");
  delay(500);
  //calibrate the lower bound
  for (int i = 0; i < N; i++) {
    cali1_sum_power = cali1_sum_power + pow(analogRead(0)-voltage_off, 2);
    delay(10);
  }
  average_low = cali1_sum_power / N;
  cali1_sum_power = 0;
  Serial.println("Finished.");
  Serial.println("Please hold at maximum strengh");
  for (int i = 0; i < 10; i++) {
    Serial.print(".");
    delay(200);
  }
  Serial.println("");
  // wait 2 s
  delay(1500);
  Serial.println("start");
  delay(500);
  //calibrate the upper bound
  for (int i = 0; i < N; i++) {
    cali1_sum_power = cali1_sum_power + pow(analogRead(0)-voltage_off, 2);
    delay(10);
  }
  average_high = cali1_sum_power / N;
  Serial.println("Finished");
  delay(1000);


  Serial.print("P(average) HIGH: "); Serial.println(average_high);
  Serial.print("P(average) LOW: "); Serial.println(average_low);
}

