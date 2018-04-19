void setup(){
Serial.begin(115200);
pinMode(23, INPUT_PULLUP);
pinMode(22, INPUT_PULLUP);
pinMode(19, INPUT_PULLUP);
pinMode(18, INPUT_PULLUP);

attachInterrupt(digitalPinToInterrupt(23), int1, RISING);
attachInterrupt(digitalPinToInterrupt(22), int2, RISING);
attachInterrupt(digitalPinToInterrupt(19), int3, RISING);
attachInterrupt(digitalPinToInterrupt(18), int4, RISING);
}

void int1(){
 Serial.println("1");
}

void int2(){
 Serial.println("2");
}

void int3(){
 Serial.println("3");
}

void int4(){
 Serial.println("4");
}
void loop() {
  // put your main code here, to run repeatedly:

}
