const int ledPin = 7;

void setup() {
  pinMode(ledPin, OUTPUT);

  // 1. 처음 1초 동안 LED 켜기
  digitalWrite(ledPin, LOW);
  delay(1000);

  // 2. 5회 깜빡이기
  for (int i = 0; i < 5; i++) {
    digitalWrite(ledPin, LOW);
    delay(100);
    digitalWrite(ledPin, HIGH);
    delay(100);
  }

  // 3. LED 끄기 및 종료
  digitalWrite(ledPin, HIGH);
}

void loop() {
  // 비워둠 (반복 방지)
}
