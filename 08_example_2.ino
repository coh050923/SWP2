// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12   // sonar sensor TRIGGER
#define PIN_ECHO 13   // sonar sensor ECHO

// configurable parameters
#define SND_VEL 346.0       // sound velocity at 24 celsius degree (m/sec)
#define INTERVAL 25         // sampling interval (msec)
#define PULSE_DURATION 10   // ultra-sound Pulse Duration (usec)

#define _DIST_MIN 100.0     // minimum distance (mm)
#define _DIST_MAX 300.0     // maximum distance (mm)

#define TIMEOUT ((INTERVAL / 2) * 1000.0) // maximum echo waiting time (usec)
#define SCALE (0.001 * 0.5 * SND_VEL)

unsigned long last_sampling_time;

void setup() {
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  // Sonar OFF
  digitalWrite(PIN_TRIG, LOW);

  // LED OFF
  analogWrite(PIN_LED, 255);

  // initialize serial port
  Serial.begin(57600);

  last_sampling_time = millis();
}

void loop() {
  float distance;
  int brightness;

  // wait until next sampling time
  if (millis() - last_sampling_time < INTERVAL)
    return;

  // measure distance
  distance = USS_measure(PIN_TRIG, PIN_ECHO);

  /*
   * LED brightness control
   *
   * 100mm 이하 : LED OFF
   * 100~200mm : 밝기 증가
   * 200mm     : 최대 밝기
   * 200~300mm : 밝기 감소
   * 300mm 이상: LED OFF
   *
   * LED는 Active Low
   * analogWrite(pin, 0)   -> 가장 밝음
   * analogWrite(pin, 255) -> 꺼짐
   */

  if (distance == 0.0 || distance >= _DIST_MAX) {
    // 측정 실패 또는 300mm 이상
    brightness = 0;
    analogWrite(PIN_LED, 255);   // LED OFF
  }
  else if (distance <= _DIST_MIN) {
    // 100mm 이하
    brightness = 0;
    analogWrite(PIN_LED, 255);   // LED OFF
  }
  else if (distance <= 200.0) {
    // 100 ~ 200mm
    // 거리 증가 -> 밝기 증가
    brightness = (int)((distance - 100.0) / 100.0 * 255.0);

    // Active Low이므로 밝기 값을 반대로 변환
    analogWrite(PIN_LED, 255 - brightness);
  }
  else {
    // 200 ~ 300mm
    // 거리 증가 -> 밝기 감소
    brightness = (int)((300.0 - distance) / 100.0 * 255.0);

    // Active Low
    analogWrite(PIN_LED, 255 - brightness);
  }

  // Serial Monitor output
  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",distance:");
  Serial.print(distance);

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.print(",brightness:");
  Serial.print(brightness);

  Serial.println("");

  // delay(50);  // 삭제

  // update last sampling time
  last_sampling_time += INTERVAL;
}


// get a distance reading from USS
// return value is in millimeter
float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
