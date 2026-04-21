#include <GyverINA.h>

class ina219 {
public:
  uint8_t address = 0x40;
  float bus_voltage = 0, shunt_voltage = 0, power = 0, current = 0, resistance = 0;
  int PGA = 8;
  uint8_t MODE = 1;
  int BADC = 128, SADC = 128;

  String ConfigRegister, ShuntRegister, BusRegister, PowerRegister, CurrentRegister, CalibrationRegister;
  INA219* ina = nullptr;

  void Begin();
  void Initialize();
  void SetMode();
  void SetPGA();
  void SetBADC();
  void SetSADC();
  void ReadBinaryRegisters();
  void ReadDecimalValues();
  void PrintAll();
  ~ina219() { if (ina) delete ina; }
};

void ina219::Begin() {
  Serial.begin(115200);
  delay(500);
  Serial.println(F("READY"));   // отправляет READY\r\n
}

void ina219::Initialize() {
  // 1. Адрес (шестнадцатеричный) — читаем до \n
  while (!Serial.available()) delay(10);
  String addrStr = Serial.readStringUntil('\n');
  addrStr.trim();
  address = (uint8_t)strtoul(addrStr.c_str(), NULL, 16);

  // 2. Сопротивление шунта (float) — читаем до \n
  while (!Serial.available()) delay(10);
  String shuntStr = Serial.readStringUntil('\n');
  shuntStr.trim();
  resistance = shuntStr.toFloat();

  // Создаём объект INA219
  if (ina) delete ina;
  float maxCurrent = (PGA / resistance);
  ina = new INA219(resistance, maxCurrent, address);
  if (!ina->begin()) {
    Serial.println(address, HEX);
    delete ina;
    ina = nullptr;
    return;
  }

  // Применяем настройки
  SetMode();
  SetPGA();
  SetSADC();
  SetBADC();
}

void ina219::SetMode() {
  if (!ina) return;
  while (!Serial.available()) delay(10);
  String modeStr = Serial.readStringUntil('\n');
  modeStr.trim();
  MODE = (uint8_t)modeStr.toInt();

  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(address, (uint8_t)2);
  uint16_t config = 0;
  if (Wire.available() >= 2) config = (Wire.read() << 8) | Wire.read();
  config = (config & 0xFFF8) | (MODE & 0x07);
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.write((config >> 8) & 0xFF);
  Wire.write(config & 0xFF);
  Wire.endTransmission();
}

void ina219::SetPGA() {
  if (!ina) return;
  while (!Serial.available()) delay(10);
  String pgaStr = Serial.readStringUntil('\n');
  pgaStr.trim();
  PGA = pgaStr.toInt();

  uint8_t bits;
  if (PGA == 1) bits = 0b00;
  else if (PGA == 2) bits = 0b01;
  else if (PGA == 4) bits = 0b10;
  else bits = 0b11;
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.endTransmission(false);
  Wire.requestFrom(address, (uint8_t)2);
  uint16_t config = 0;
  if (Wire.available() >= 2) config = (Wire.read() << 8) | Wire.read();
  config = (config & 0xE7FF) | (bits << 11);
  Wire.beginTransmission(address);
  Wire.write(0x00);
  Wire.write((config >> 8) & 0xFF);
  Wire.write(config & 0xFF);
  Wire.endTransmission();
}

void ina219::SetBADC() {
  if (!ina) return;
  while (!Serial.available()) delay(10);
  String badcStr = Serial.readStringUntil('\n');
  badcStr.trim();
  BADC = badcStr.toInt();

  uint8_t mode;
  switch (BADC) {
    case 1:   mode = INA219_RES_12BIT; break;
    case 2:   mode = INA219_RES_12BIT_X2; break;
    case 4:   mode = INA219_RES_12BIT_X4; break;
    case 8:   mode = INA219_RES_12BIT_X8; break;
    case 16:  mode = INA219_RES_12BIT_X16; break;
    case 32:  mode = INA219_RES_12BIT_X32; break;
    case 64:  mode = INA219_RES_12BIT_X64; break;
    case 128: mode = INA219_RES_12BIT_X128; break;
    default:  mode = INA219_RES_12BIT; break;
  }
  ina->setResolution(INA219_VBUS, mode);
}

void ina219::SetSADC() {
  if (!ina) return;
  while (!Serial.available()) delay(10);
  String sadcStr = Serial.readStringUntil('\n');
  sadcStr.trim();
  SADC = sadcStr.toInt();

  uint8_t mode;
  switch (SADC) {
    case 1:   mode = INA219_RES_12BIT; break;
    case 2:   mode = INA219_RES_12BIT_X2; break;
    case 4:   mode = INA219_RES_12BIT_X4; break;
    case 8:   mode = INA219_RES_12BIT_X8; break;
    case 16:  mode = INA219_RES_12BIT_X16; break;
    case 32:  mode = INA219_RES_12BIT_X32; break;
    case 64:  mode = INA219_RES_12BIT_X64; break;
    case 128: mode = INA219_RES_12BIT_X128; break;
    default:  mode = INA219_RES_12BIT; break;
  }
  ina->setResolution(INA219_VSHUNT, mode);
}

void ina219::ReadBinaryRegisters() {
  if (!ina) return;
  const uint8_t regs[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
  uint16_t values[6] = {0};
  for (uint8_t i = 0; i < 6; i++) {
    Wire.beginTransmission(address);
    Wire.write(regs[i]);
    if (Wire.endTransmission(false) != 0) continue;
    Wire.requestFrom(address, (uint8_t)2);
    if (Wire.available() >= 2) values[i] = (Wire.read() << 8) | Wire.read();
  }
  ConfigRegister = ""; for (int b=15; b>=0; b--) ConfigRegister += (values[0]>>b)&1 ? '1':'0';
  ShuntRegister = ""; for (int b=15; b>=0; b--) ShuntRegister += (values[1]>>b)&1 ? '1':'0';
  BusRegister = "";   for (int b=15; b>=0; b--) BusRegister   += (values[2]>>b)&1 ? '1':'0';
  PowerRegister = ""; for (int b=15; b>=0; b--) PowerRegister += (values[3]>>b)&1 ? '1':'0';
  CurrentRegister = ""; for (int b=15; b>=0; b--) CurrentRegister += (values[4]>>b)&1 ? '1':'0';
  CalibrationRegister = ""; for (int b=15; b>=0; b--) CalibrationRegister += (values[5]>>b)&1 ? '1':'0';
}

void ina219::ReadDecimalValues() {
  if (!ina) return;
  bus_voltage = ina->getVoltage();
  current = ina->getCurrent();
  power = ina->getPower();
  shunt_voltage = ina->getShuntVoltage();
}

ina219 test;

void setup() {
  test.Begin();
}

void loop() {
  if (Serial.available()) {
    // Читаем команду строго до символа \n
    String cmd = Serial.readStringUntil('\n');
    cmd.trim();

    if (cmd == "INIT") {
      test.Initialize();
    } else if (cmd == "PGA") {
      test.SetPGA();
    } else if (cmd == "BADC") {
      test.SetBADC();
    } else if (cmd == "SADC") {
      test.SetSADC();
    } else if (cmd == "MODE") {
      test.SetMode();
    } else if (cmd == "GETALLDATA") {
      test.ReadBinaryRegisters();
      test.ReadDecimalValues();
      delay(10);
      Serial.print("0x");
      Serial.print(test.address, HEX);
      Serial.print(" ");
      Serial.print(test.bus_voltage, 6);
      Serial.print(" ");
      Serial.print(test.shunt_voltage, 6);
      Serial.print(" ");
      Serial.print(test.current, 6);
      Serial.print(" ");
      Serial.print(test.power, 6);
      Serial.print(" ");
      Serial.print(test.resistance, 6);
      Serial.print(" ");
      Serial.print(test.PGA);
      Serial.print(" ");
      Serial.print(test.BADC);
      Serial.print(" ");
      Serial.print(test.SADC);
      Serial.print(" ");
      Serial.print(test.ConfigRegister);
      Serial.print(" ");
      Serial.print(test.ShuntRegister);
      Serial.print(" ");
      Serial.print(test.BusRegister);
      Serial.print(" ");
      Serial.print(test.PowerRegister);
      Serial.print(" ");
      Serial.print(test.CurrentRegister);
      Serial.print(" ");
      Serial.println(test.CalibrationRegister);
    }
  }
}
