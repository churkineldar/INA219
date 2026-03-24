#include <GyverINA.h>

class ina219 {
 public:
  uint8_t address = 040;
  float bus_voltage = 0;
  float shunt_voltage = 0;
  float power = 0;
  float current = 0;
  float calibration = 0;
  float resistance = 0;

  int PGA = 0;
  uint8_t MODE = 0;
  int BADC = 0;
  int SADC = 0;

  String CalibrationRegister = "";
  String ShuntRegister = "";
  String BusRegister = "";
  String PowerRegister = "";
  String CurrentRegister = "";
  String CalibrationRegister = "";

  INA219 ina;

  void Initialize();
  void Begin();
  void SetMode();
  void SetPGA();
  void SetBADC();
  void SetSADC();
  void ReadBinaryRegisters();
  void ReadDecimalValues();
  void PrintAll();

  void ina219::Initialize() {
    address = Serial.parseInt();
    resistance = Serial.parseInt();
    PGA = Serial.parseInt();
    MODE = (uint8_t)Serial.parseInt();
    BADC = Serial.parseInt();
    SADC = Serial.parseInt();
    ina(resistance, PGA / resistance, address);
    SetMode();
    SetPGA();
    SetBADC();
    SetSADC();
  }

  void ina219::Begin() {
    Serial.begin(9600);
    Serial.print(F("INA219..."));
    if (ina.begin()) {
      Serial.println(F("connected!"));
    } else {
      Serial.println(F("not found!"));
      while (1);
    }
  }

  void ina219::SetMode(uint8_t mode) {
    // Чтение текущего конфигурационного регистра
    Wire.beginTransmission(address);
    Wire.write(0x00);
    Wire.endTransmission(false);
    Wire.requestFrom(address, (uint8_t)2);
    uint16_t config = 0;
    if (Wire.available() >= 2) {
      config = (Wire.read() << 8) | Wire.read();
    } else {
      return;
    }
    // Обнуляем биты MODE (младшие 3 бита), затем устанавливаем новые
    config = (config & 0xFFF8) | (mode & 0x07);
    // Запись обновлённой конфигурации
    Wire.beginTransmission(address);
    Wire.write(0x00);
    Wire.write((config >> 8) & 0xFF);
    Wire.write(config & 0xFF);
    Wire.endTransmission();
  }

  void ina219::setPGA() {
    uint8_t bits;
    if (PGA == 1)
      bits = 0b00;
    else if (PGA == 2)
      bits = 0b01;
    else if (PGA == 4)
      bits = 0b10;
    else
      bits = 0b11;  // /8 (диапазон ±320 мВ)

    Wire.beginTransmission(address);
    Wire.write(0x00);
    Wire.endTransmission(false);
    Wire.requestFrom(address, (uint8_t)2);
    uint16_t config = 0;
    if (Wire.available() >= 2) {
      config = (Wire.read() << 8) | Wire.read();
    } else
      return;

    // Обнуляем биты 12 и 11 (маска 0xE7FF = 1110011111111111)
    config = (config & 0xE7FF) | (bits << 11);
    Wire.beginTransmission(address);
    Wire.write(0x00);
    Wire.write((config >> 8) & 0xFF);
    Wire.write(config & 0xFF);
    Wire.endTransmission();
  }

  void ina219::SetBADC() {
    uint8_t mode;
    switch (BADC) {
      case 1:
        mode = INA219_RES_12BIT;
        break;
      case 2:
        mode = INA219_RES_12BIT_X2;
        break;
      case 4:
        mode = INA219_RES_12BIT_X4;
        break;
      case 8:
        mode = INA219_RES_12BIT_X8;
        break;
      case 16:
        mode = INA219_RES_12BIT_X16;
        break;
      case 32:
        mode = INA219_RES_12BIT_X32;
        break;
      case 64:
        mode = INA219_RES_12BIT_X64;
        break;
      case 128:
        mode = INA219_RES_12BIT_X128;
        break;
      default:
        mode = INA219_RES_12BIT;  // 1 выборка по умолчанию
    }
    ina.setResolution(INA219_VBUS, mode);
  }

  void ina219::SetSADC() {
    uint8_t mode;
    switch (SADC) {
      case 1:
        mode = INA219_RES_12BIT;
        break;
      case 2:
        mode = INA219_RES_12BIT_X2;
        break;
      case 4:
        mode = INA219_RES_12BIT_X4;
        break;
      case 8:
        mode = INA219_RES_12BIT_X8;
        break;
      case 16:
        mode = INA219_RES_12BIT_X16;
        break;
      case 32:
        mode = INA219_RES_12BIT_X32;
        break;
      case 64:
        mode = INA219_RES_12BIT_X64;
        break;
      case 128:
        mode = INA219_RES_12BIT_X128;
        break;
      default:
        mode = INA219_RES_12BIT;
    }
    ina.setResolution(INA219_VSHUNT, mode);
  }

  void ina219::ReadBinaryRegisters() {
    // Массив адресов регистров INA219
    const uint8_t regs[6] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05};
    uint16_t values[6] = {0, 0, 0, 0, 0, 0};

    // Чтение всех шести регистров
    for (uint8_t i = 0; i < 6; i++) {
      Wire.beginTransmission(address);
      Wire.write(regs[i]);
      if (Wire.endTransmission(false) != 0) {
        values[i] = 0;
        continue;
      }
      Wire.requestFrom(address, (uint8_t)2);
      if (Wire.available() >= 2) {
        values[i] = (Wire.read() << 8) | Wire.read();
      } else {
        values[i] = 0;
      }
    }

    // Преобразование в двоичные строки и запись в поля класса
    // Используем имена, максимально близкие к вашему списку
    ConfigRegister = "";  // для регистра конфигурации (0x00)
    for (int b = 15; b >= 0; b--)
      ConfigRegister += (values[0] >> b) & 1 ? '1' : '0';

    ShuntRegister = "";  // регистр шунта (0x01)
    for (int b = 15; b >= 0; b--)
      ShuntRegister += (values[1] >> b) & 1 ? '1' : '0';

    BusRegister = "";  // регистр шины (0x02)
    for (int b = 15; b >= 0; b--)
      BusRegister += (values[2] >> b) & 1 ? '1' : '0';

    PowerRegister = "";  // регистр мощности (0x03)
    for (int b = 15; b >= 0; b--)
      PowerRegister += (values[3] >> b) & 1 ? '1' : '0';

    CurrentRegister = "";  // регистр тока (0x04)
    for (int b = 15; b >= 0; b--)
      CurrentRegister += (values[4] >> b) & 1 ? '1' : '0';

    CalibrationRegister = "";  // регистр калибровки (0x05)
    for (int b = 15; b >= 0; b--)
      CalibrationRegister += (values[5] >> b) & 1 ? '1' : '0';
  }

  void ina219::ReadDecimalValues() {
    bus_voltage = ina.getVoltage();  // напряжение шины (В)
    current = ina.getCurrent();      // ток (А)
    power = ina.getPower();          // мощность (Вт)
    shunt_voltage = ina.getShuntVoltage();  // напряжение на шунте (В)
  }

  void ina219::PrintAll() {
    s Serial.println(F("=== INA219 Status ==="));
    Serial.print(F("Address: 0x"));
    Serial.println(address, HEX);
    Serial.print(F("Bus Voltage: "));
    Serial.print(bus_voltage, 5);
    Serial.println(F(" V"));
    Serial.print(F("Shunt Voltage: "));
    Serial.print(shunt_voltage, 6);
    Serial.println(F(" V"));
    Serial.print(F("Current: "));
    Serial.print(current, 5);
    Serial.println(F(" A"));
    Serial.print(F("Power: "));
    Serial.print(power, 3);
    Serial.println(F(" W"));
    Serial.print(F("Shunt Resistance: "));
    Serial.print(resistance, 6);
    Serial.println(F(" Ohm"));
    Serial.print(F("Calibration Value: "));
    Serial.println(calibration);

    Serial.println(F("--- Configuration ---"));
    Serial.print(F("PGA Gain: "));
    Serial.println(PGA);
    Serial.print(F("Mode: "));
    Serial.println(MODE);
    Serial.print(F("BADC Averaging: "));
    Serial.println(BADC);
    Serial.print(F("SADC Averaging: "));
    Serial.println(SADC);

    Serial.println(F("--- Registers (16-bit binary) ---"));
    Serial.print(F("Config:     "));
    Serial.println(ConfigRegister);
    Serial.print(F("Shunt:      "));
    Serial.println(ShuntRegister);
    Serial.print(F("Bus:        "));
    Serial.println(BusRegister);
    Serial.print(F("Power:      "));
    Serial.println(PowerRegister);
    Serial.print(F("Current:    "));
    Serial.println(CurrentRegister);
    Serial.print(F("Calibration:"));
    Serial.println(CalibrationRegister);
    Serial.println(F("==========================="));
  }
}


void setup() {
  ina219 test;
  test.Initialise();
  test.Begin();
}

void loop() {
  test.ReadBinaryRegisters();
  test.ReadDecimalValues();
  test.PrintAll();

  Serial.println("");
  delay(1000);
}
