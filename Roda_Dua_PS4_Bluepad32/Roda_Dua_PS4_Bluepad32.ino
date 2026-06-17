#include <Bluepad32.h>

// ================= BLUEPAD32 CONTROLLER =================
ControllerPtr ps4 = nullptr;

// ================= PIN MAPPING L298N =================
// Motor kiri  : IN1, IN2
// Motor kanan : IN3, IN4
// ENA dan ENB pada L298N harus dijumper ke 5V agar motor aktif.
const int IN1 = 33;
const int IN2 = 25;
const int IN3 = 26;
const int IN4 = 27;

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

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, LOW);
  stopMotors();

  Serial.println("ESP32 L298N Bluepad32 PS4");
  Serial.print("Firmware Bluepad32: ");
  Serial.println(BP32.firmwareVersion());

  BP32.setup(&onConnectedController, &onDisconnectedController);

  // Aktifkan hanya kalau ingin menghapus pairing lama:
  // BP32.forgetBluetoothKeys();
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
  // Bluepad32 axis umumnya -512 sampai 512.
  int throttle = -ps4->axisY();
  int steering = ps4->axisRX();

  throttle = applyDeadzone(throttle);
  steering = applyDeadzone(steering);

  throttle = map(constrain(throttle, -512, 512), -512, 512, -MAX_SPEED, MAX_SPEED);
  steering = map(constrain(steering, -512, 512), -512, 512, -MAX_SPEED, MAX_SPEED);

  // Mixing tank drive.
  motorKiri = throttle - steering;
  motorKanan = - throttle - steering;

  motorKiri = constrain(motorKiri, -MAX_SPEED, MAX_SPEED);
  motorKanan = constrain(motorKanan, -MAX_SPEED, MAX_SPEED);

  driveMotor(IN1, IN2, motorKiri);
  driveMotor(IN3, IN4, motorKanan);

  Serial.printf(
    "LY:%4d RX:%4d | kiri:%4d kanan:%4d\n",
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
}
