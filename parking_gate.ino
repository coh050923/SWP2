
#include <Servo.h>
#include <math.h>

// Arduino pin assignment
#define PIN_SERVO 10
#define PIN_TRIG 12
#define PIN_ECHO 13

// Configurable parameters
#define DIST_THRESHOLD 200
#define MOVING_TIME 3000
#define INTERVAL 20

#define CLOSED_ANGLE 0
#define OPEN_ANGLE 90

#define SIGMOID_K 10.0

// 1: Sigmoid, 2: Smoothstep
#define CONTROL_MODE 2

Servo myServo;

int currentAngle = CLOSED_ANGLE;
int startAngle = CLOSED_ANGLE;
int targetAngle = CLOSED_ANGLE;

unsigned long moveStartTime = 0;
unsigned long lastUpdateTime = 0;

// Sigmoid 함수
float sigmoid(float x) {
  return 1.0 / (1.0 + exp(-SIGMOID_K * (x - 0.5)));
}

// 정규화된 Sigmoid
float normalizedSigmoid(float x) {
  float minValue = sigmoid(0.0);
  float maxValue = sigmoid(1.0);

  return (sigmoid(x) - minValue) /
         (maxValue - minValue);
}

// NEW: Smoothstep 함수
float smoothstep(float x) {
  return x * x * (3.0 - 2.0 * x);
}

// NEW: 제어 함수 선택
float getEasedProgress(float t) {
  t = constrain(t, 0.0, 1.0);

#if CONTROL_MODE == 1
  return normalizedSigmoid(t);
#elif CONTROL_MODE == 2
  return smoothstep(t);
#else
  return t;
#endif
}

// 초음파센서 거리 측정
float getDistance() {
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);

  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration =
      pulseIn(PIN_ECHO, HIGH, 12000);

  if (duration == 0) {
    return -1;
  }

  return duration * 0.173;
}

void setup() {
  Serial.begin(9600);

  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  myServo.attach(PIN_SERVO);
  myServo.write(CLOSED_ANGLE);

  Serial.println("Parking gate ready");

#if CONTROL_MODE == 1
  Serial.println("Mode: Sigmoid");
#elif CONTROL_MODE == 2
  Serial.println("Mode: Smoothstep");
#endif
}

void loop() {
  // 1. 차량 감지
  float distance = getDistance();

  if (distance >= 0) {
    int newTarget;

    if (distance < DIST_THRESHOLD) {
      newTarget = OPEN_ANGLE;
    } else {
      newTarget = CLOSED_ANGLE;
    }

    // 목표 각도가 변경되면 이동 시작
    if (newTarget != targetAngle) {
      startAngle = currentAngle;
      targetAngle = newTarget;
      moveStartTime = millis();

      if (targetAngle == OPEN_ANGLE) {
        Serial.println("Vehicle detected - OPEN");
      } else {
        Serial.println("Vehicle left - CLOSE");
      }
    }
  }

  // 2. 서보모터 제어
  unsigned long now = millis();

  if (now - lastUpdateTime >= INTERVAL) {
    lastUpdateTime = now;

    unsigned long progress = now - moveStartTime;

    if (progress >= MOVING_TIME) {
      currentAngle = targetAngle;
    } else {
      float t = (float)progress / MOVING_TIME;

      // 선택한 함수로 보간
      float eased = getEasedProgress(t);

      currentAngle = startAngle +
          (targetAngle - startAngle) * eased;
    }

    myServo.write(currentAngle);
  }
}
