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
  Serial.begin(9600);
  delay(500);
  Serial.print(F("READY"));
}

void ina219::Initialize() {
  // 1. Адрес (шестнадцатеричный)
  while (!Serial.available()) delay(10);
  String addrStr = Serial.readStringUntil('\n');
  addrStr.trim();
  address = (uint8_t)strtoul(addrStr.c_str(), NULL, 16);

  // 2. Сопротивление шунта (float)
  while (!Serial.available()) delay(10);
  resistance = Serial.parseFloat();

  // Создаём объект INA219
  if (ina) delete ina;
  float maxCurrent = (PGA / resistance);
  ina = new INA219(resistance, maxCurrent, address);
  if (!ina->begin()) {
    //Serial.print(F("ERROR: INA219 not found at 0x"));
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
  MODE = (uint8_t)Serial.parseInt();
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
  PGA = Serial.parseInt();
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
  BADC = Serial.parseInt();
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
  SADC = Serial.parseInt();
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

void ina219::PrintAll() { //НЕ ЛАБВЬЮШНАЯ, ДЛЯ ЧЕЛОВЕКОВ
  if (!ina) { Serial.println(F("Not initialized")); return; }
  Serial.print(F("Address: 0x")); Serial.println(address, HEX);
  Serial.print(F("Bus Voltage: ")); Serial.print(bus_voltage, 5); Serial.println(F(" V"));
  Serial.print(F("Shunt Voltage: ")); Serial.print(shunt_voltage, 6); Serial.println(F(" V"));
  Serial.print(F("Current: ")); Serial.print(current, 5); Serial.println(F(" A"));
  Serial.print(F("Power: ")); Serial.print(power, 3); Serial.println(F(" W"));
  Serial.print(F("Shunt Resistance: ")); Serial.print(resistance, 6); Serial.println(F(" Ohm"));

  Serial.print(F("PGA Gain: ")); Serial.println(PGA);
  Serial.print(F("Mode: ")); Serial.println(MODE);
  Serial.print(F("BADC Averaging: ")); Serial.println(BADC);
  Serial.print(F("SADC Averaging: ")); Serial.println(SADC);

  Serial.print(F("Config:     ")); Serial.println(ConfigRegister);
  Serial.print(F("Shunt:      ")); Serial.println(ShuntRegister);
  Serial.print(F("Bus:        ")); Serial.println(BusRegister);
  Serial.print(F("Power:      ")); Serial.println(PowerRegister);
  Serial.print(F("Current:    ")); Serial.println(CurrentRegister);
  Serial.print(F("Calibration:")); Serial.println(CalibrationRegister);
}

ina219 test;

void setup() {
  test.Begin();
}

void loop() {
  if (Serial.available()) {
    String cmd = Serial.readString();
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
    } else if (cmd == "PRINTALL") {
      test.ReadBinaryRegisters();
      test.ReadDecimalValues();
      test.PrintAll();
    } else if (cmd == "GETADR") {
      Serial.println(test.address);
    } else if (cmd == "GETBUS") {
      test.ReadDecimalValues();
      Serial.println(test.bus_voltage, 6);
    } else if (cmd == "GETSHUNT") {
      test.ReadDecimalValues();
      Serial.println(test.shunt_voltage, 6);
    } else if (cmd == "GETCURR") {
      test.ReadDecimalValues();
      Serial.println(test.current, 6);
    } else if (cmd == "GETPWR") {
      test.ReadDecimalValues();
      Serial.println(test.power, 6);
    } else if (cmd == "GETRES") {
      Serial.println(test.resistance, 6);
    } else if (cmd == "GETPGA") {
      Serial.println(test.PGA);
    } else if (cmd == "GETBADC") {
      Serial.println(test.BADC);
    } else if (cmd == "GETSADC") {
      Serial.println(test.SADC);
    } else if (cmd == "GETMODE") {
      Serial.println(test.MODE);
    } else if (cmd == "GETCONFREG") {
      test.ReadBinaryRegisters();
      Serial.println(test.ConfigRegister);
    } else if (cmd == "GETSHUNTREG") {
      test.ReadBinaryRegisters();
      Serial.println(test.ShuntRegister);
    } else if (cmd == "GETBUSREG") {
      test.ReadBinaryRegisters();
      Serial.println(test.BusRegister);
    } else if (cmd == "GETPWRREG") {
      test.ReadBinaryRegisters();
      Serial.println(test.PowerRegister);
    } else if (cmd == "GETCURREG") {
      test.ReadBinaryRegisters();
      Serial.println(test.CurrentRegister);
    } else if (cmd == "GETCALREG") {
      test.ReadBinaryRegisters();
      Serial.println(test.CalibrationRegister);
    }
  }
}
