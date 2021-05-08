

int R1= 460;
int Ra=25;
int ECPin= 0;
float TemperatureCoef = 0.019;
float K=2.88;
float EC=0;
float EC25 =0;
float raw= 0;
float Vin= 5;
float Vdrop= 0;
float Rc= 0;


void setup() {

  R1=(R1+Ra);// Taking into acount Powering Pin Resitance
};

void loop() {

 raw= analogRead(ECPin);
 raw= analogRead(ECPin);// This is not a mistake, First reading will be low beause if charged a capacitor
 Vdrop= (Vin*raw)/1024.0;
 Rc=(Vdrop*R1)/(Vin-Vdrop);
 Rc=Rc-Ra; //acounting for Digital Pin Resitance
 EC = 1000/(Rc*K);
 EC25  =  EC/ (1+ TemperatureCoef*(21.19-25.0));

 Serial.print("Rc: ");
 Serial.print(Rc);
 Serial.print(" EC: ");
 Serial.print(EC25);
 Serial.print(" Simens  ");
}
