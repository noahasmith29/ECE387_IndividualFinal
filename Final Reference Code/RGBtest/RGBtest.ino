const int blueLEDPin = 4;    // LED connected to digital pin 11
const int greenLEDPin = 5;    // LED connected to digital pin 9
const int redLEDPin = 6;     // LED connected to digital pin 10

void setup() {
  // put your setup code here, to run once:
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);
  pinMode(blueLEDPin, OUTPUT);
}

void loop() {
  setColor(0,0,255);

}

void setColor(int redValue, int greenValue, int blueValue) {
  analogWrite(redLEDPin, redValue);
  analogWrite(greenLEDPin, greenValue);
  analogWrite(blueLEDPin, blueValue);
}
