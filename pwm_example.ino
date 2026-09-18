int led = 7;              // GPIO 7번 핀에 LED 연결
int current_period = 1000; // 주기 설정 (단위: us)
int current_duty = 0;      // 듀티비 설정 (단위: %, 0 ~ 100)

void setup() {
  pinMode(led, OUTPUT);
}

// 주기를 설정하는 함수
void set_period(int period) {
  if (period >= 100 && period <= 10000) {
    current_period = period;
  }
}

// 듀티비를 설정하는 함수
void set_duty(int duty) {
  if (duty >= 0 && duty <= 100) {
    current_duty = duty;
  }
}

// 소프트웨어 PWM 출력 함수 (오버플로우 방지를 위해 연산 형변환 적용)
void customPWM(int duty, int period) {
  unsigned long high_time = (unsigned long)period * duty / 100;
  unsigned long low_time = period - high_time;

  if (high_time > 0) {
    digitalWrite(led, HIGH);
    delayMicroseconds(high_time);
  }
  if (low_time > 0) {
    digitalWrite(led, LOW);
    delayMicroseconds(low_time);
  }
}

void loop() {
  // 100 (0.1ms), 1000 (1ms), 10000 (10ms) 중 하나로 테스트
  set_period(100); 

  // 1초(1000ms) 동안 0 -> 100 -> 0 패턴을 만들기 위한 단계 계산
  int total_steps = 100; // 단계 수를 간소화하여 안정화
  int step_delay_ms = 1000 / (total_steps * 2); // 각 단계당 유지 시간

  // 1. 밝기 증가 (0 -> 100)
  for (int i = 0; i <= 100; i++) {
    set_duty(i);
    
    // 현재 지정한 period로 한 주기 PWM을 돌리되, 
    // 각 단계의 유지 시간(step_delay_ms)동안 반복해서 실행
    unsigned long elapsed = 0;
    while (elapsed < (step_delay_ms * 1000UL)) {
      customPWM(current_duty, current_period);
      elapsed += current_period; // 1번 PWM 돌 데 걸린 시간(us) 합산
    }
  }

  // 2. 밝기 감소 (100 -> 0)
  for (int i = 100; i >= 0; i--) {
    set_duty(i);
    
    unsigned long elapsed = 0;
    while (elapsed < (step_delay_ms * 1000UL)) {
      customPWM(current_duty, current_period);
      elapsed += current_period;
    }
  }
}
