// CC-by-www.Electrosmash.com
// Based on OpenMusicLabs previous works.
// pedalshield_uno_fuzz.ino reads the ADC signal and boost it to the max or min value depending on a threshold value
// pressing the pushbutton_1 or 2 turns the fuzz distortion harder or softer.
// Made by Ava Horwood
 
 
//defining harware resources.
#define LED 13
#define FOOTSWITCH 12
#define TOGGLE 2
#define PUSHBUTTON_1 A5
#define PUSHBUTTON_2 A4

//defining the output PWM parameters
#define PWM_FREQ 0x00FF // pwm frequency - 31.3KHz
#define PWM_MODE 0 // Fast (1) or Phase Correct (0)
#define PWM_QTY 2 // 2 PWMs in parallel

//other variables
int input, distortion_threshold=6000; //initial value adjusted by try and error.
int counter=0;
unsigned int ADC_low, ADC_high;
 
void setup() {
 
  //setup IO
  pinMode(FOOTSWITCH, INPUT_PULLUP);
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(PUSHBUTTON_1, INPUT_PULLUP);
  pinMode(PUSHBUTTON_2, INPUT_PULLUP);
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
 
  // setup ADC
  ADMUX = 0x60; // left adjust, adc0, internal vcc
  ADCSRA = 0xe5; // turn on adc, ck/32, auto trigger
  ADCSRB = 0x07; // t1 capture for trigger
  DIDR0 = 0x01; // turn off digital inputs for adc0
 
  // setup PWM
  TCCR1A = (((PWM_QTY - 1) << 5) | 0x80 | (PWM_MODE << 1)); //
  TCCR1B = ((PWM_MODE << 3) | 0x11); // ck/1
  TIMSK1 = 0x20; // interrupt on capture interrupt
  ICR1H = (PWM_FREQ >> 8);
  ICR1L = (PWM_FREQ & 0xff);
  DDRB |= ((PWM_QTY << 1) | 0x02); // turn on outputs
  sei(); // turn on interrupts - not really necessary with arduino
  }
 
void loop() 
{
    //Turn on the LED if the effect is ON.
  if (digitalRead(FOOTSWITCH)){
    digitalWrite(LED, HIGH); 
//    Serial.println("effect is on");
    } 
     else {
      digitalWrite(LED, LOW);
//      Serial.println("effect is off");
     }
  //nothing here, all happens in the Timer 1 interruption.
}
 
ISR(TIMER1_CAPT_vect) 
{
  // get ADC data
  ADC_low = ADCL; // you need to fetch the low byte first
  ADC_high = ADCH;
  //construct the input sumple summing the ADC low and high byte.
  input = ((ADC_high << 8) | ADC_low) + 0x8000; // make a signed 16b value
 
counter++; //to save resources, the pushbuttons are checked every 1000 times.
if(counter==1000)
{ 
counter=0;
if (!digitalRead(PUSHBUTTON_2)) {
  Serial.println("Increasing Volume");
  if (distortion_threshold<32768)distortion_threshold=distortion_threshold+25; //increase the vol
    digitalWrite(LED, LOW); //blinks the led
    }
 
    if (!digitalRead(PUSHBUTTON_1)) {
        Serial.println("Decreasing Volume");
  if (distortion_threshold>0)distortion_threshold=distortion_threshold-25; //decrease vol
  digitalWrite(LED, LOW); //blinks the led
    }
}
 
  //the input signal is 16bits (values from -32768 to +32768
  //the value is clipped to the distortion_threshold value
    if(input>distortion_threshold) input=32768;
    else if(input<-distortion_threshold) input=-32768;
 
  //write the PWM signal
  OCR1AL = ((input + 0x8000) >> 8); // convert to unsigned, send out high byte
  OCR1BL = input; // send out low byte
}
