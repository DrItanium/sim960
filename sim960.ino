// we are defining a fake i960Zx series processor, this processor does not exist but will be what I'm calling my simulated design
// It has an SALIGN value of 4 like the i960Kx,i960Sx, and i960MC processors

/// @todo figure out the arcada library to allow for QSPI and SDCard simultaneous interaction and write an example for the Adafruit_SPIFlash library when done

// Adapt this class to the target microcontroller board
// Right now I am targeting a grand central m4
#include <SPI.h>
#include <Wire.h>
#include <SdFat.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SPIFlash.h>
#include <ArduinoJson.h>
#include <OPL3Duo.h>
#include <Adafruit_seesaw.h>
#include <Adafruit_SI5351.h>
#include <array>
#include "CoreTypes.h"
#include "Core.h"
#include "ProcessorMappingConfiguration.h"
#include "ProcessorAddress.h"

constexpr auto OPL3Duo_A2 = 22;
constexpr auto OPL3Duo_A1 = 23;
constexpr auto OPL3Duo_A0 = 24;
constexpr auto OPL3Duo_Latch = 25;
constexpr auto OPL3Duo_Reset = 26;
constexpr auto i960Zx_SALIGN = 4;
Adafruit_NeoPixel onboardNeoPixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
SdFat sdCard; // use SDCARD_SS_PIN for this one
OPL3Duo theOPL3Duo(OPL3Duo_A2,
                   OPL3Duo_A1,
                   OPL3Duo_A0,
                   OPL3Duo_Latch,
                   OPL3Duo_Reset);
Adafruit_seesaw theSoilSensor;
Adafruit_SI5351 clockgen;
union MemoryCell {
    constexpr explicit MemoryCell(Ordinal value = 0) : oval(value) { }
    Ordinal oval = 0;
    Integer ival;
    ByteOrdinal bo[4];
    ByteInteger bi[4];
    ShortOrdinal so[2];
    ShortInteger si[2];

};
const std::string filename = "/config.txt";
i960::MappingConfiguration mapping;
bool
loadConfiguration(const std::string& filename, i960::MappingConfiguration &theMapping) {
    /// @todo fix this
  // Open file for reading
  File file = sdCard.open(filename.c_str());

  // Allocate a temporary JsonDocument
  // Don't forget to change the capacity to match your requirements.
  // Use arduinojson.org/v6/assistant to compute the capacity.
  StaticJsonDocument<16384> doc;

  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, file);
  if (error)
    Serial.println(F("Failed to read file, using default configuration"));

  // Copy values from the JsonDocument to the Config
  theMapping.setName(doc["name"] | "An i960 Processor");
  JsonArray blocks = doc["blocks"];
  for (JsonVariant block : blocks) {
    JsonObject theObject = block.as<JsonObject>();
    auto index = theObject["index"].as<byte>();
    auto& targetBlock = theMapping.get(index);
    targetBlock.setType(theObject["type"].as<std::string>());
    targetBlock.setFilename(theObject["filename"].as<std::string>());
    targetBlock.setDescription(theObject["description"].as<std::string>());
    targetBlock.setPermissions(theObject["perms"].as<std::string>());
  }

  // Close the file (Curiously, File's destructor doesn't close the file)
  file.close();
  return true;
}

[[noreturn]]
void
somethingBadHappened() {
    while (true) {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
        delay(1000);
    }
}
// the i960 has other registers and tables we need to be aware of so onboard sram will most likely _not_ be exposed to the i960 processor
// directly
namespace i960 {
    /*
     * Memory Map So far (broken up into 16 megabyte blocks)
     * 0x0000'0000 - 0xFEFF'FFFF: FRAM / SD Card memory / memory buffer /etc
     * 0xFF00'0000 - 0xFFFF'FFFF: Internal IO Space (Where I expose the SAMD51 peripherals)
     */
    /*
     * For now, the Grand Central M4 uses an SD Card for its memory with a small portion of the on board sram used for scratchpad / always
     * available memory. On board devices are mapped into the implicit onboard device area of [0xFF00'0000,0xFFFF'FFFF]. The IO device bus
     * exists here as extra "modules" such as:
     * - Clock Generator
     * - RTC
     * - ESP32 Wifi Coprocessor
     * - Display
     * - Sharp Memory Display
     * - GPIOs from the Grand Central
     * - SPI IO Expanders
     * - Speaker
     * - Control Registers
     * - 2 MB Onboard SPI Flash directly mapped into IO Device Space
     * - OPL3 Device
     * - Neopixels
     * - EEPROM
     * - FLASH memory
     * - etc
     */
    class ZxBusInterfaceUnit : public BusInterfaceUnit {
    public:
        using BusInterfaceUnit::BusInterfaceUnit;
        ~ZxBusInterfaceUnit() override = default;
        ByteOrdinal load(Address address, TreatAsByteOrdinal ordinal) override {
            return 0;
        }
        ByteInteger load(Address address, TreatAsByteInteger integer) override {
            return 0;
        }
        ShortOrdinal load(Address address, TreatAsShortOrdinal ordinal) override {
            return 0;
        }
        ShortInteger load(Address address, TreatAsShortInteger integer) override {
            return 0;
        }
        Ordinal load(Address address, TreatAsOrdinal ordinal) override {
            return 0;
        }
        Integer load(Address address, TreatAsInteger integer) override {
            return 0;
        }
        void store(Address address, ByteOrdinal value, TreatAsByteOrdinal ordinal) override {
        }
        void store(Address address, ByteInteger value, TreatAsByteInteger integer) override {
        }
        void store(Address address, ShortOrdinal value, TreatAsShortOrdinal ordinal) override {
        }
        void store(Address address, ShortInteger value, TreatAsShortInteger integer) override {
        }
        void store(Address address, Ordinal value, TreatAsOrdinal ordinal) override {
        }
        void store(Address address, Integer value, TreatAsInteger integer) override {
        }
    };
    /// @todo handle unaligned load/store and loads/store which span multiple sections
    void
    Core::badInstruction(DecodedInstruction inst) {
        Serial.println("BAD INSTRUCTION!");
        std::visit([](auto&& value) {
            using K = std::decay_t<decltype(value)>;
            Serial.print("Instruction opcode: 0x");
            if constexpr (std::is_same_v<K, i960::MEMFormatInstruction>) {
                Serial.print(value.upperHalf(), HEX);
            }
            Serial.println(value.lowerHalf(), HEX);
            auto name = value.decodeName();
            if (!name.empty()) {
                Serial.print("Name: ");
                Serial.println(name.c_str());
            }
        }, inst);
        raiseFault();
    }
    void
    Core::busTestFailed() noexcept {
        somethingBadHappened();
    }
    volatile uint8_t& memory(const uint32_t address, TreatAsByteOrdinal) noexcept { return *reinterpret_cast<uint8_t*>(address); }
    volatile uint32_t& memory(const uint32_t address, TreatAsOrdinal) noexcept { return *reinterpret_cast<uint32_t*>(address); }
    class ZxInternalPeripheralUnit : public InternalPeripheralUnit {
        /*
         * PA - PA31,PA30,PA27,PA25-PA00 (NO PA29, PA28, PA26)
         * PB - PB00-PB31
         * PC - PC31-PC30, PC28 - PC10, PC07 - PC00 (NO PC29, PC9, PC8)
         * PD - PD21-PD20, PD12 - PD08, PD01-PD00 (NO PD31-PD22, PD19-PD13, PD7-PD2)
         * 0xFF00'0F00 : Cortex M Cache Controller
         * 0xFF00'1000 : LED_BUILTIN value
         * 0xFF00'1010 : Neopixel 0 color
         * 0xFF00'1014 : Neopixel 0 show
         * 0xFF00'1018 : Neopixel 0 clear
         * 0xFF00'101C : Neopixel 0 brightness
         * 0xFF00'0100 : A0
         * 0xFF00'0104 : A1
         * 0xFF00'0108 : A2
         * 0xFF00'010C : A3
         * 0xFF00'0110 : A4
         * 0xFF00'0114 : A5
         * 0xFF00'0118 : A6
         * 0xFF00'011C : A7
         * 0xFF00'0120 : A8
         * 0xFF00'0124 : A9
         * 0xFF00'0128 : A10
         * 0xFF00'012C : A11
         * 0xFF00'0130 : A12
         * 0xFF00'0134 : A13
         * 0xFF00'0138 : A14
         * 0xFF00'013C : A15
         */
    private:
        Ordinal
        analogReadPins(const ProcessorAddress& pa) {
            switch (pa.getBlockOffset()) {
                case 0x00: return analogRead(A0);
                case 0x04: return analogRead(A1);
                case 0x08: return analogRead(A2);
                case 0x0c: return analogRead(A3);
                case 0x10: return analogRead(A4);
                case 0x14: return analogRead(A5);
                case 0x18: return analogRead(A6);
                case 0x1c: return analogRead(A7);
                case 0x20: return analogRead(A8);
                case 0x24: return analogRead(A9);
                case 0x28: return analogRead(A10);
                case 0x2c: return analogRead(A11);
                case 0x30: return analogRead(A12);
                case 0x34: return analogRead(A13);
                case 0x38: return analogRead(A14);
                case 0x3c: return analogRead(A15);
                default: return 0;
            }
        }
        void
        analogWritePins(const ProcessorAddress& pa, Ordinal value) {
            switch (pa.getBlockOffset()) {
                case 0x00: analogWrite(A0, value); break;
                case 0x04: analogWrite(A1, value); break;
                case 0x08: analogWrite(A2, value); break;
                case 0x0c: analogWrite(A3, value); break;
                case 0x10: analogWrite(A4, value); break;
                case 0x14: analogWrite(A5, value); break;
                case 0x18: analogWrite(A6, value); break;
                case 0x1c: analogWrite(A7, value); break;
                case 0x20: analogWrite(A8, value); break;
                case 0x24: analogWrite(A9, value); break;
                case 0x28: analogWrite(A10, value); break;
                case 0x2c: analogWrite(A11, value); break;
                case 0x30: analogWrite(A12, value); break;
                case 0x34: analogWrite(A13, value); break;
                case 0x38: analogWrite(A14, value); break;
                case 0x3c: analogWrite(A15, value); break;
                default: break;
            }
        }
        void writeLEDBlock(const ProcessorAddress& pa, Ordinal value) {
            switch (pa.getBlockOffset()) {
                case 0x00: digitalWrite(LED_BUILTIN, value ? HIGH : LOW); break;
                case 0x10: onboardNeoPixel.setPixelColor(0, value); break;
                case 0x14: onboardNeoPixel.show(); break;
                case 0x18: onboardNeoPixel.clear(); break;
                case 0x1C: onboardNeoPixel.setBrightness(value); break;
                default:
                    break;
            }
        }
        Ordinal readLEDBlock(const ProcessorAddress& pa) {
            switch (pa.getBlockOffset()) {
                case 0x10: return onboardNeoPixel.getPixelColor(0);
                case 0x1C: return onboardNeoPixel.getBrightness();
                default: return 0;
            }
        }
        void writeSubsection0(const ProcessorAddress& pa, Ordinal value) {
            switch (pa.getBlockId()) {
                case 0x01: analogWritePins(pa, value); break;
                case 0x10: writeLEDBlock(pa, value); break;
                default: break;
            }
        }
        Ordinal
        readSubsection0(const ProcessorAddress& pa) {
            switch (pa.getBlockId()) {
                case 0x01: return analogReadPins(pa);
                case 0x10: return readLEDBlock(pa);
                default: return 0;
            }
        }
    public:
        Ordinal
        load(Address address, TreatAsOrdinal ) override {
            ProcessorAddress pa(address);
            switch (pa.getSubsectionId()) {
                case 0x00: return readSubsection0(pa);
                default: return 0;
            }
        }

        void
        store(Address address, Ordinal value, TreatAsOrdinal ordinal) override {
            ProcessorAddress pa(address);
            switch (pa.getSubsectionId()) {
                case 0x00:
                    writeSubsection0(pa, value);
                    break;
                default:
                    break;
            }
        }
        void store(Address address, ByteOrdinal value, TreatAsByteOrdinal ordinal) override { }
        void store(Address address, ByteInteger value, TreatAsByteInteger integer) override { }
        void store(Address address, ShortOrdinal value, TreatAsShortOrdinal ordinal) override { }
        void store(Address address, ShortInteger value, TreatAsShortInteger integer) override { }
        void store(Address address, Integer value, TreatAsInteger integer) override { store(address, value, TreatAsOrdinal{}); }
    public:
        using InternalPeripheralUnit::InternalPeripheralUnit;
        ~ZxInternalPeripheralUnit() override = default;
        ByteOrdinal
        load(Address address, TreatAsByteOrdinal ordinal) override {
            return 0;
        }
        ByteInteger
        load(Address address, TreatAsByteInteger integer) override {
            return 0;
        }
        ShortOrdinal load(Address address, TreatAsShortOrdinal ordinal) override {
            return 0;
        }
        ShortInteger load(Address address, TreatAsShortInteger integer) override {
            return 0;
        }
        Integer load(Address address, TreatAsInteger integer) override {
            return 0;
        }
        void begin() noexcept override {

        }
    };
}
i960::ZxBusInterfaceUnit zxBXU;
i960::ZxInternalPeripheralUnit zxIPU;
i960::Core cpuCore(zxBXU, zxIPU, 0, i960Zx_SALIGN);
void setupSerial() {
    Serial.begin(9600);
    while (!Serial) {
        delay(100);
    }
}
void setupSPI() {
    Serial.print("Starting up SPI...");
    SPI.begin();
    Serial.println("Done");
}
void setupSDCard() {
    Serial.print("Starting up onboard QSPI Flash...");
    flash.begin();
    Serial.println("Done");
    Serial.println("Onboard Flash information");
    Serial.print("JEDEC ID: 0x");
    Serial.println(flash.getJEDECID(), HEX);
    Serial.print("Flash size: ");
    Serial.print(flash.size() / 1024);
    Serial.println(" KB");
    Serial.print("Initializing fileysstem on external flash...");
    if (!fatfs.begin(&flash)) {
        Serial.println("Error: filesystem does not exist! Please try SdFat_format example to make one!");
        somethingBadHappened();
    }
    Serial.println("Done");
    Serial.print("Starting up SD Card...");
    if (!sdCard.begin(SDCARD_SS_PIN)) {
        Serial.println("No card found");
    } else {
        Serial.println("Card found!");
    }
}
void setupNeoPixel() {
    Serial.print("Initialzing onboard NeoPixel...");
    onboardNeoPixel.begin();
    onboardNeoPixel.show();
    Serial.println("Done");
}
void setupOPL3Duo() {
    Serial.print("Starting up OPL3Duo...");
    theOPL3Duo.begin();
    Serial.println("Done");
}
void setupSoilSensor() {
    Serial.print("Bringing up the soil sensor...");
    if (!theSoilSensor.begin(0x36)) {
        Serial.println("Soil Sensor Not Found!");
    } else {
        Serial.println("Soil Sensor Found!");
        Serial.println(theSoilSensor.getVersion(), HEX);
    }
}
void setupClockChip() {
    Serial.print("Bringing up Si5351 Clock Chip...");
    if (clockgen.begin() != ERROR_NONE) {
        Serial.println("No Clock Chip Detected ... forget to hook it up?");
    } else {
        Serial.println("Done");
    }

}
void setupMappingConfiguration() {
    Serial.println(F("Loading mapping configuration..."));
    if (!loadConfiguration(filename, mapping)) {
        Serial.println("Unable to find mapping configuration, stopping!");
        somethingBadHappened();
    }
}
/// @todo implement the register frames "in hardware"
void setup() {
    setupSerial();
    Serial.println("i960 Simulator Starting up");
    pinMode(LED_BUILTIN, OUTPUT);
    setupSPI();
    setupSDCard();
    setupNeoPixel();
    setupOPL3Duo();
    setupSoilSensor();
    setupClockChip();
    setupMappingConfiguration();

    // last thing to do is do the post
    cpuCore.post();
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    cpuCore.cycle(); // put a single cycle through
    delay(100);
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
}
