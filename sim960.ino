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
#include <array>
#include "CoreTypes.h"
#include "Core.h"
#include "ProcessorMappingConfiguration.h"
#include "ProcessorAddress.h"

constexpr bool doCPUComputation = true;
constexpr auto i960Zx_SALIGN = 4;
Adafruit_NeoPixel onboardNeoPixel(1, PIN_NEOPIXEL, NEO_GRB + NEO_KHZ800);
Adafruit_FlashTransport_QSPI flashTransport;
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;
SdFat sdCard; // use SDCARD_SS_PIN for this one
/// the base configuration
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
}
i960::ZxBusInterfaceUnit zxBXU;
constexpr Address zxBootBase = 0xFEFF'FF00;
i960::Core cpuCore(zxBXU, zxBootBase, i960Zx_SALIGN);
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
    setupMappingConfiguration();

    // last thing to do is do the post
    if constexpr (doCPUComputation) {
        cpuCore.post();
    }
}

void loop() {
    if constexpr (doCPUComputation) {
        digitalWrite(LED_BUILTIN, HIGH);
        cpuCore.cycle(); // put a single cycle through
        delay(10);
        digitalWrite(LED_BUILTIN, LOW);
        delay(10);
    }
}
