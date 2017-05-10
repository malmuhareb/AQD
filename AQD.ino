// This #include statement was automatically added by the Particle IDE.
#include <ThingSpeak.h>

// This #include statement was automatically added by the Particle IDE.
#include <LiquidCrystal.h>

// This #include statement was automatically added by the Particle IDE.

#include <Adafruit_Sensor.h>

#include <DHT.h>
#include <DHT_U.h>



// This #include statement was automatically added by the Particle IDE.
#include <MQ135.h>


 // system defines
#define DHTTYPE  DHT22              // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   D1                 // Digital pin for communications
#define DHT_SAMPLE_INTERVAL   2000  // Sample every two seconds
#define MQ_AD A1


MQ135 mq135_sensor = MQ135(MQ_AD);
float correctedPPM;
float temperature = 0;
float humidity = 0;



DHT_Unified dht(DHTPIN, DHTTYPE);

//-----------------------Dust------------------------------
#define        COV_RATIO                       0.2            //ug/mmm / mv
#define        NO_DUST_VOLTAGE                 40            //mv
#define        SYS_VOLTAGE                     500          


/*
I/O define
*/
#define iled  D7                                            //drive the led of sensor
#define vout  A0                                            //analog input

/*
variable
*/
float density, voltage;
int   adcvalue;


LiquidCrystal lcd(D6,D5,D4,D3,D2,D0);


//----------------Thingspeak-----------
unsigned long myChannelNumber = 270848;
const char * myWriteAPIKey = "XLFA96URI44MF5GN";
TCPClient client;

void setup() {

  pinMode(MQ_AD, INPUT);
  dht.begin();


 //-----------------------dust-----------

   pinMode(iled, OUTPUT);
    digitalWrite(iled,LOW);
    
    
 //-----------LCD-----------
  lcd.begin(16, 2);
  lcd.clear();

  //-----------LCD-----------

  

ThingSpeak.begin(client);

}

void loop() {
 
dhtCode();
mqCode();
dustCode();
show();
pubData();



}

void mqCode(){

 correctedPPM = mq135_sensor.getCorrectedPPM(temperature, humidity);
 
 Particle.publish("CPPM: ", String(correctedPPM));

  delay(1000);
  
}



void dhtCode(){
  
  delay(2000);

  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  float t = event.temperature;
  dht.humidity().getEvent(&event);
  float h = event.relative_humidity;
  
  //if (!(t == 0 && h == 0)){
      temperature = t;
   humidity = h;
 // }
  
   
  Particle.publish("T: ", String(temperature));
  Particle.publish("H: ", String(humidity));
  
  
    
}


int Filter(int m)
{
  static int flag_first = 0, _buff[10], sum;
  const int _buff_max = 10;
  int i;
  
  if(flag_first == 0)
  {
    flag_first = 1;

    for(i = 0, sum = 0; i < _buff_max; i++)
    {
      _buff[i] = m;
      sum += _buff[i];
    }
    return m;
  }
  else
  {
    sum -= _buff[0];
    for(i = 0; i < (_buff_max - 1); i++)
    {
      _buff[i] = _buff[i + 1];
    }
    _buff[9] = m;
    sum += _buff[9];
    
    i = sum / 10.0;
    return i;
  }
}

void dustCode(){
   /*
  get adcvalue
  */
  digitalWrite(iled, HIGH);
  delayMicroseconds(280);
  adcvalue = analogRead(vout);
  delayMicroseconds(40);
  digitalWrite(iled, LOW);
  //Particle.publish("adcvalue: ", String(adcvalue));
  
adcvalue = Filter(adcvalue);
//Particle.publish("adcvalue2: ", String(adcvalue));
  
  /*
  convert voltage (mv)
  */
  voltage = (SYS_VOLTAGE / 1024.0) * adcvalue * 11;
  
  /*
  voltage to density
  */
  if(voltage >= NO_DUST_VOLTAGE)
  {
    voltage -= NO_DUST_VOLTAGE;
    
    density = voltage * COV_RATIO;
    
    density -= 40;
    //density = density * 100;
  }
  else
    density = 0;
    
 

Particle.publish("Dust: ", String(density));

  
  delay(1000);
  
}



void showTH(){
    int t1 = (int) temperature;
    int h1 = (int) humidity;
    lcd.clear();
  lcd.print("Temp.: ");
  lcd.print(t1);
  lcd.print("C");
  lcd.setCursor(0,1);
  lcd.print("Humidity: ");
  lcd.print(h1);
  lcd.print("%");
  delay(3000);
    
}

void showDust(){
    lcd.clear();
  lcd.print("Dust Density:");
  lcd.setCursor(0,1);
  lcd.print(density);
  lcd.print(" ug/m3");
  delay(3000);
   lcd.clear();
   lcd.print("Air Quality Eval");
   lcd.setCursor(0,1);
  if (density < 35){
    lcd.print("Excellent");  
  }
  else if (density < 75){
      lcd.print("Average");
  }
  else if (density < 120){
      lcd.print("Light pollution");
  }
  else if (density < 150){
      lcd.print("Moderate pollution");
  }
  else if (density < 250){
      lcd.print("Heavy pollution");
  }
  else{
      lcd.print("Serious pollution");
  }
    
}

void showGas(){
    lcd.clear();
    lcd.print("Combustible Gas:");
    lcd.setCursor(0,1);
    lcd.print(correctedPPM);
  lcd.print(" PPM");
  delay(3000);
}

void show(){
    showTH();
    showGas();
    showDust();
    
}

void pubData(){
   ThingSpeak.setField(1,temperature);
   ThingSpeak.setField(2,humidity);
   ThingSpeak.setField(3,density);
   ThingSpeak.setField(4,correctedPPM);
   // Write the fields that you've set all at once.
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);  

  delay(3000); // ThingSpeak will only accept updates every 15 seconds. 

}

