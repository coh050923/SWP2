// Arduino pin assignment
#define PIN_LED  9
#define PIN_TRIG 12
#define PIN_ECHO 13

// configurable parameters
#define SND_VEL 346.0
#define INTERVAL 25
#define PULSE_DURATION 10

#define _DIST_MIN 100
#define _DIST_MAX 300

#define TIMEOUT ((INTERVAL / 2) * 1000.0)
#define SCALE (0.001 * 0.5 * SND_VEL)

// Median filter size
#define N 30

// global variables
unsigned long last_sampling_time;

// 최근 N개의 측정값을 저장
float dist_samples[N];

// 현재까지 저장된 샘플 개수
int sample_count = 0;


// 중위수 계산 함수
float getMedian(float samples[], int size)
{
  float sorted[N];

  // 원본 배열을 변경하지 않기 위해 복사
  for (int i = 0; i < size; i++)
    sorted[i] = samples[i];

  // 오름차순 정렬
  for (int i = 0; i < size - 1; i++)
  {
    for (int j = i + 1; j < size; j++)
    {
      if (sorted[i] > sorted[j])
      {
        float temp = sorted[i];
        sorted[i] = sorted[j];
        sorted[j] = temp;
      }
    }
  }

  // 중위수 반환
  if (size % 2 == 1)
  {
    // N이 홀수인 경우
    return sorted[size / 2];
  }
  else
  {
    // N이 짝수인 경우
    return (sorted[size / 2 - 1] + sorted[size / 2]) / 2.0;
  }
}


void setup()
{
  // initialize GPIO pins
  pinMode(PIN_LED, OUTPUT);
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);

  digitalWrite(PIN_TRIG, LOW);

  // initialize serial port
  Serial.begin(57600);

  // 초기 배열을 0으로 설정
  for (int i = 0; i < N; i++)
    dist_samples[i] = 0;
}


void loop()
{
  float dist_raw;
  float dist_median;

  // wait until next sampling time
  if (millis() < last_sampling_time + INTERVAL)
    return;

  // get a distance reading from USS
  dist_raw = USS_measure(PIN_TRIG, PIN_ECHO);


  // ==========================================
  // 최근 N개의 측정값 저장
  // ==========================================

  // 기존 값들을 한 칸씩 뒤로 이동
  for (int i = N - 1; i > 0; i--)
  {
    dist_samples[i] = dist_samples[i - 1];
  }

  // 가장 최근 측정값 저장
  dist_samples[0] = dist_raw;

  // 아직 N개가 모이지 않은 경우
  if (sample_count < N)
    sample_count++;


  // ==========================================
  // Median Filter
  // ==========================================

  dist_median = getMedian(dist_samples, sample_count);


  // ==========================================
  // Serial Plotter 출력
  // ==========================================

  Serial.print("Min:");
  Serial.print(_DIST_MIN);

  Serial.print(",raw:");
  Serial.print(min(dist_raw, (float)_DIST_MAX + 100));

  Serial.print(",median:");
  Serial.print(min(dist_median, (float)_DIST_MAX + 100));

  Serial.print(",Max:");
  Serial.print(_DIST_MAX);

  Serial.println("");


  // ==========================================
  // LED
  // ==========================================

  if ((dist_raw < _DIST_MIN) || (dist_raw > _DIST_MAX))
    digitalWrite(PIN_LED, HIGH);
  else
    digitalWrite(PIN_LED, LOW);


  // update last sampling time
  last_sampling_time +=  INTERVAL;
}


// ==========================================
// 초음파 거리 측정 함수
// ==========================================

float USS_measure(int TRIG, int ECHO)
{
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(PULSE_DURATION);
  digitalWrite(TRIG, LOW);

  return pulseIn(ECHO, HIGH, TIMEOUT) * SCALE;
}
