#include <Bluepad32.h>

// ================= BLUEPAD32 CONTROLLER =================
ControllerPtr ps4 = nullptr;

// ================= PIN MAPPING L298N (4 MOTOR) =================
// Anda bisa menggunakan dua buah driver L298N atau satu driver 4-channel.
// Pin PWM untuk ESP32 (bisa disesuaikan dengan pinout fisik Anda)

// SISI KIRI (Left Side)
const int IN1 = 33; // Motor Kiri Depan (Forward)
const int IN2 = 25; // Motor Kiri Depan (Backward)
const int IN3 = 26; // Motor Kiri Belakang (Forward)
const int IN4 = 27; // Motor Kiri Belakang (Backward)

// SISI KANAN (Right Side)
const int IN5 = 14; // Motor Kanan Depan (Forward)
const int IN6 = 12; // Motor Kanan Depan (Backward)
const int IN7 = 13; // Motor Kanan Belakang (Forward)
const int IN8 = 15; // Motor Kanan Belakang (Backward)

const int LED = 2;

// ================= SETTING KONTROL =================
const int DEADZONE = 40;
const int MAX_SPEED = 255;

int motorKiri = 0;
int motorKanan = 0;

void onConnectedController(ControllerPtr ctl) {
  if (ps4 == nullptr) {
    ps4 = ctl;
    digitalWrite(LED, HIGH);
    stopMotors();
    Serial.println("PS4 controller terhubung.");
  } else {
    Serial.println("Controller lain terdeteksi, tetapi slot sudah dipakai.");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  if (ps4 == ctl) {
    ps4 = nullptr;
    digitalWrite(LED, LOW);
    stopMotors();
    Serial.println("PS4 controller terputus, motor dimatikan.");
  }
}

void setup() {
  Serial.begin(115200);

  // Set semua pin motor sebagai OUTPUT
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(IN5, OUTPUT);
  pinMode(IN6, OUTPUT);
  pinMode(IN7, OUTPUT);
  pinMode(IN8, OUTPUT);
  
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);
  stopMotors();

  Serial.println("ESP32 L298N Bluepad32 PS4 - 4WD Mode");
  Serial.print("Firmware Bluepad32: ");
  Serial.println(BP32.firmwareVersion());

  BP32.setup(&onConnectedController, &onDisconnectedController);
}

void loop() {
  BP32.update();

  if (ps4 == nullptr || !ps4->isConnected()) {
    stopMotors();
    digitalWrite(LED, LOW);
    delay(20);
    return;
  }

  digitalWrite(LED, HIGH);

  // Stick kiri Y untuk maju/mundur, stick kanan X untuk belok.
  int throttle = -ps4->axisY();
  int steering = ps4->axisRX();

  throttle = applyDeadzone(throttle);
  steering = applyDeadzone(steering);

  throttle = map(constrain(throttle, -512, 512), -512, 512, -MAX_SPEED, MAX_SPEED);
  steering = map(constrain(steering, -512, 512), -512, 512, -MAX_SPEED, MAX_SPEED);

  // Mixing tank drive.
  motorKiri = throttle - steering;
  motorKanan = - throttle - steering; // Mengikuti logika asli (dibalik untuk sisi kanan)

  motorKiri = constrain(motorKiri, -MAX_SPEED, MAX_SPEED);
  motorKanan = constrain(motorKanan, -MAX_SPEED, MAX_SPEED);

  // Drive motor sisi kiri (Depan & Belakang)
  driveMotor(IN1, IN2, motorKiri);
  driveMotor(IN3, IN4, motorKiri);

  // Drive motor sisi kanan (Depan & Belakang)
  driveMotor(IN5, IN6, motorKanan);
  driveMotor(IN7, IN8, motorKanan);

  Serial.printf(
    "LY:%4d RX:%4d | Kiri:%4d Kanan:%4d\n",
    ps4->axisY(),
    ps4->axisRX(),
    motorKiri,
    motorKanan
  );

  delay(20);
}

int applyDeadzone(int value) {
  if (abs(value) < DEADZONE) {
    return 0;
  }
  return value;
}

void driveMotor(int pinA, int pinB, int speed) {
  speed = constrain(speed, -MAX_SPEED, MAX_SPEED);

  if (speed > 0) {
    analogWrite(pinA, speed);
    analogWrite(pinB, 0);
  } else if (speed < 0) {
    analogWrite(pinA, 0);
    analogWrite(pinB, -speed);
  } else {
    analogWrite(pinA, 0);
    analogWrite(pinB, 0);
  }
}

void stopMotors() {
  motorKiri = 0;
  motorKanan = 0;

  analogWrite(IN1, 0);
  analogWrite(IN2, 0);
  analogWrite(IN3, 0);
  analogWrite(IN4, 0);
  analogWrite(IN5, 0);
  analogWrite(IN6, 0);
  analogWrite(IN7, 0);
  analogWrite(IN8, 0);
}
