#ifndef RUNS_H
#define RUNS_H

#include <Robot.hpp>
#include <thread>

bool RUNS::Robot::bluetoothEnabled(){
    return SimpleBLE::Adapter::bluetooth_enabled();
}

RUNS::Robot::Robot(){
    std::thread entry;
    if (!SimpleBLE::Adapter::bluetooth_enabled()){
        entry = std::thread(&Robot::bluetoothOff, this);
    } else {
        entry = std::thread(&Robot::notConnected, this);
    }
    entry.detach();
}

RUNS::Robot::~Robot(){
    run                 = false;
    speed               = 0;
    rotation            = NONE;
    bumpers             = 0;
    microprocessor_temp = 0;

    bt_handle.reset();
}

void RUNS::Robot::bluetoothOff(){
    while (this->run){
        if (SimpleBLE::Adapter::bluetooth_enabled()){
            this->notConnected();
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );
    }
}

void RUNS::Robot::notConnected(){
    bool               match;
    SimpleBLE::Adapter adapter;
    while (this->run){
        if (!SimpleBLE::Adapter::bluetooth_enabled()){
            this->bluetoothOff();
        }
        adapter = SimpleBLE::Adapter::get_adapters()[0];
        for (SimpleBLE::Peripheral peer : adapter.get_paired_peripherals()){
            if (!peer.is_connected()){ continue; }
            match = true;
            for (SimpleBLE::Service serv : peer.services()){
                if (
                    (serv.uuid() != TEMPERATURE_SERVICE) &&
                    (serv.uuid() != PROXIMITY_SERVICE)   &&
                    (serv.uuid() != MOVEMENT_SERVICE)    &&
                    (serv.uuid() != GENERIC_ATTRIBUTE)   &&
                    (serv.uuid() != GENERIC_ACCESS)      &&
                    (serv.uuid() != DEVICE_INFO)
                ){
                    match = false;
                    break;
                }
            }
            if (match){
                this->bt_handle = peer;
                this->connected();
            }
        }
        std::this_thread::sleep_for(
            std::chrono::milliseconds(100)
        );
    }
}

void RUNS::Robot::timedQueueAdder(Command command, int16_t msInterval){
    while (this->isConnected() && this->run){
        this->queue.push_back(command);
        std::this_thread::sleep_for(
            std::chrono::milliseconds(msInterval)
        );
    }
}

void RUNS::Robot::connected(){
    char                data[2]     = {0, 0};
    Command             command;
    precise_time_point  current_time;
    while (this->run){
        if (!this->bt_handle->is_connected()){
            this->bt_handle.reset();
            this->notConnected();
        }
        current_time = std::chrono::system_clock::now();
        if ((current_time - bumper_timer) > MS_20){
            this->queue.push_back({OP_PROX_BUMPERS, 0});
            bumper_timer = current_time;
        }
        if ((current_time - mcu_timer) > MS_50){
            this->queue.push_back({OP_TEMP_MICROPROCESSOR, 0});
            mcu_timer = current_time;
        }
        if ((current_time - ambient_timer) > MS_100){
            this->queue.push_back({OP_TEMP_ENVIRONMENT, 0});
            ambient_timer = current_time;
        }
        if (!this->queue.size()){
            std::this_thread::sleep_for(
                std::chrono::milliseconds(1)
            );
            continue;
        }
        command = this->queue.front();
        switch (command.op){
        case OP_TEMP_MICROPROCESSOR:
            this->microprocessor_temp = bt_handle->read(
                TEMPERATURE_SERVICE,
                TEMP_MICROPROCESSOR
            ).at(0);
            break;
        case OP_TEMP_ENVIRONMENT:
            this->environment_temp = bt_handle->read(
                TEMPERATURE_SERVICE,
                TEMP_ENVIRONMENT
            ).at(0);
            break;
        case OP_PROX_BUMPERS:
            this->bumpers = bt_handle->read(
                PROXIMITY_SERVICE,
                PROX_BUMPERS
            ).at(0);
            if (bumpers){ speed = 0; }
            break;
        case OP_MOVE_VELOCITY:
            data[0] = command.value;
            bt_handle->write_request(
                MOVEMENT_SERVICE,
                MOVE_VELOCITY, 
                SimpleBLE::ByteArray(
                    (const char*)data,
                    2
                )
            );
            break;
        case OP_MOVE_ROTATION:
            data[0] = command.value;
            bt_handle->write_request(
                MOVEMENT_SERVICE,
                MOVE_ROTATION,
                SimpleBLE::ByteArray(
                    (const char*)data,
                    2
                )
            );
            break;
        default:
            break;
        }
        this->queue.pop_front();
    }
}

bool RUNS::Robot::isConnected(){
    return this->bt_handle.has_value();
}

// set fns here
void RUNS::Robot::setVelocity(int8_t speed){
    precise_time_point now = std::chrono::system_clock::now();
    
    if ((speed > 0) && bumpers)   { return; }
    if (speed == this->speed)     { return; }
    if (now - speed_timer < MS_20){ return; }

    this->speed = speed;
    if (speed){
        this->queue.push_back({OP_MOVE_VELOCITY, speed});
    } else {
        this->queue.push_front({OP_MOVE_VELOCITY, speed});
    }

    this->speed_timer = now;
}

void RUNS::Robot::setRotation(Rotation dir){
    precise_time_point now = std::chrono::system_clock::now();

    if (dir == this->rotation)       { return; }
    if (now - rotation_timer < MS_20){ return; }

    this->rotation = dir;
    this->queue.push_front({OP_MOVE_ROTATION, dir});
    this->rotation_timer = now;
}

int8_t RUNS::Robot::getVelocity(){
    return this->speed;
}

RUNS::Rotation RUNS::Robot::getRotation(){
    return this->rotation;
}

int8_t RUNS::Robot::getMicroprocessorTemp(){
    return this->microprocessor_temp;
}

int8_t RUNS::Robot::getEnvironmentTemp(){
    return this->environment_temp;
}

int8_t RUNS::Robot::getBumpers(){
    return this->bumpers;
}

#endif
