#include <simpleble/SimpleBLE.h>

#include <algorithm>
#include <chrono>
#include <cstring>
#include <iomanip>
#include <ios>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <list>

typedef std::chrono::high_resolution_clock::time_point precise_time_point;

const std::chrono::duration MS_20{std::chrono::milliseconds(20)};
const std::chrono::duration MS_50{std::chrono::milliseconds(50)};
const std::chrono::duration MS_100{std::chrono::milliseconds(100)};

namespace RUNS {

// Service UUIDs
const std::string GENERIC_ACCESS      = "00001800-0000-1000-8000-00805f9b34fb";
const std::string GENERIC_ATTRIBUTE   = "00001801-0000-1000-8000-00805f9b34fb";
const std::string DEVICE_INFO         = "0000180a-0000-1000-8000-00805f9b34fb";

// RUNS Specific
const std::string TEMPERATURE_SERVICE = "0000a000-0000-1000-8000-00805f9b34fb";
const std::string PROXIMITY_SERVICE   = "0000b000-0000-1000-8000-00805f9b34fb";
const std::string MOVEMENT_SERVICE    = "0000c000-0000-1000-8000-00805f9b34fb";

// Temperature characteristic UUIDs
const std::string TEMP_MICROPROCESSOR = "0000a001-0000-1000-8000-00805f9b34fb";
const std::string TEMP_ENVIRONMENT    = "0000a002-0000-1000-8000-00805f9b34fb";

// Proximity characteristic UUIDs
const std::string PROX_BUMPERS        = "0000b001-0000-1000-8000-00805f9b34fb";

// Movement characteristic UUIDs
const std::string MOVE_VELOCITY       = "0000c001-0000-1000-8000-00805f9b34fb";
const std::string MOVE_ROTATION       = "0000c002-0000-1000-8000-00805f9b34fb";

enum Target {
    OP_TEMP_MICROPROCESSOR,
    OP_TEMP_ENVIRONMENT,
    OP_PROX_BUMPERS,
    OP_MOVE_VELOCITY,
    OP_MOVE_ROTATION
};

enum Rotation : int8_t {
    NONE  = 0x00,
    LEFT  = 0x10, 
    RIGHT = 0x20
};

typedef struct Command {
    Target       op;
    int8_t       value;
} Command;

class Robot {
private:
    bool                                 run                 = true;
    std::optional<SimpleBLE::Peripheral> bt_handle           = std::nullopt;
    int8_t                               microprocessor_temp = 0;
    int8_t                               environment_temp    = 0;
    int8_t                               bumpers             = 0;
    int8_t                               speed               = 0;
    Rotation                             rotation            = NONE;
    precise_time_point                   mcu_timer           = std::chrono::system_clock::now();
    precise_time_point                   ambient_timer       = std::chrono::system_clock::now();
    precise_time_point                   bumper_timer        = std::chrono::system_clock::now();
    precise_time_point                   speed_timer         = std::chrono::system_clock::now();
    precise_time_point                   rotation_timer      = std::chrono::system_clock::now();
    std::list<Command>                   queue               = {};

    void timedQueueAdder(Command, int16_t);
    void bluetoothOff();
    void notConnected();
    void connected();
public:
    Robot();
    ~Robot();
    static bool bluetoothEnabled();
    bool        isConnected();
    void        setVelocity(int8_t);
    void        setRotation(Rotation);
    int8_t      getVelocity();
    Rotation    getRotation();
    int8_t      getMicroprocessorTemp();
    int8_t      getEnvironmentTemp();
    int8_t      getBumpers();
    void        exit();
};
}
