#include <Servo.h>

// Arduino pin assignment
#define PIN_LED   9
#define PIN_TRIG  12
#define PIN_ECHO  13
#define PIN_SERVO 10

// configurable parameters for sonar
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10

#define _DIST_MIN 180.0   // 18 cm
#define _DIST_MAX 360.0   // 36 cm

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

// EMA filter
#define _EMA_ALPHA 0.3

// Servo duty
#define _DUTY_MIN 500
#define _DUTY_NEU 1500
#define _DUTY_MAX 2500

// global variables
float dist_ema;
float dist_prev;

unsigned long last_sampling_time;

Servo myservo;


void setup() {

  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);

  // LED OFF
  digitalWrite(PIN_LED, HIGH);

  // initialize servo
  myservo.attach(PIN_SERVO);
  myservo.writeMicroseconds(_DUTY_MIN);

  // initialize distance variables
  dist_prev = _DIST_MIN;
  dist_ema = _DIST_MIN;

  // initialize serial port
  Serial.begin(57600);

  last_sampling_time = millis();
}


void loop() {

  float dist_raw;
  float dist_filtered;
  float servo_angle;
  int servo_duty;


  // wait until next sampling time
  if (millis() < last_sampling_time + INTERVAL)
    return;


  // get a distance reading from USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);


  // ---------------------------------------
  // Range filter
  // ---------------------------------------

  if ((dist_raw == 0.0) || (dist_raw < _DIST_MIN)) {

    // measurement failure or below minimum
    dist_filtered = dist_prev;

    // LED OFF
    digitalWrite(PIN_LED, HIGH);

  }
  else if (dist_raw > _DIST_MAX) {

    // above maximum
    dist_filtered = dist_prev;

    // LED OFF
    digitalWrite(PIN_LED, HIGH);

  }
  else {

    // measurement is within 18~36 cm
    dist_filtered = dist_raw;

    dist_prev = dist_raw;

    // LED ON
    digitalWrite(PIN_LED, LOW);
  }


  // ---------------------------------------
  // EMA filter
  // ---------------------------------------

  dist_ema =
      _EMA_ALPHA * dist_filtered
      + (1.0 - _EMA_ALPHA) * dist_ema;


  // ---------------------------------------
  // Calculate servo angle
  // ---------------------------------------

  if (dist_ema <= _DIST_MIN) {

    // 18 cm 이하 -> 0°
    servo_angle = 0.0;

  }
  else if (dist_ema >= _DIST_MAX) {

    // 36 cm 이상 -> 180°
    servo_angle = 180.0;

  }
  else {

    // 18~36 cm -> 거리 비례하여 0~180°
    servo_angle =
        (dist_ema - _DIST_MIN)
        * 180.0
        / (_DIST_MAX - _DIST_MIN);
  }


  // ---------------------------------------
  // Convert angle to servo duty
  // ---------------------------------------

  servo_duty =
      _DUTY_MIN
      + (servo_angle / 180.0)
      * (_DUTY_MAX - _DUTY_MIN);


  myservo.writeMicroseconds(servo_duty);


  // ---------------------------------------
  // Serial output
  // ---------------------------------------

  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",dist:");
  Serial.print(dist_raw);

  Serial.print(",ema:");
  Serial.print(dist_ema);

  Serial.print(",Servo:");
  Serial.print(myservo.read());

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");


  // update last sampling time
  last_sampling_time += INTERVAL;
}


// ========================================
// USS distance measurement
// ========================================

float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);

  delayMicroseconds(PULSE_DURATION);

  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
