// ESP32 BLE class libaries
#include "BLEDevice.h"
#include "BLEServer.h"
#include "BLEUtils.h"
#include "BLECharacteristic.h"
#include "BLE2902.h"


class BLE_node_server: public Task{
    public:
        BLE_node_server(uint16_t stack_size);
    	void run(void *data);
        bool is_client_connected();

        uint32_t read_request();
        void send_ack(bool ack);
        void send_error(uint32_t error_code);

        void set_temp_air(float val);
        void set_light(bool val);
        void set_moist_soil(uint32_t val);
        void set_moist_air(float val);
        void set_co2(uint32_t val);

        bool get_light_panel();
        bool get_pump();
        uint32_t get_pump_time();
    private:
        BLEServer *p_server;

        BLEService *p_service_control;
        BLEService *p_service_temp;
        BLEService *p_service_light;
        BLEService *p_service_moist;
        BLEService *p_service_co2;
        BLEService *p_service_pump;

        BLECharacteristic *p_characteristic_control_request;
        BLECharacteristic *p_characteristic_control_ack;
        BLECharacteristic *p_characteristic_control_error;
        BLECharacteristic *p_characteristic_temp_air;
        BLECharacteristic *p_characteristic_light;
        BLECharacteristic *p_characteristic_light_panel;
        BLECharacteristic *p_characteristic_moist_soil;
        BLECharacteristic *p_characteristic_moist_air;
        BLECharacteristic *p_characteristic_co2;
        BLECharacteristic *p_characteristic_pump;
        BLECharacteristic *p_characteristic_pump_time;

        BLE2902 *p_descriptor_control_ack;
        BLE2902 *p_descriptor_control_error;
        BLE2902 *p_descriptor_temp_air;
        BLE2902 *p_descriptor_light;
        BLE2902 *p_descriptor_moist_soil;
        BLE2902 *p_descriptor_moist_air;
        BLE2902 *p_descriptor_co2;

        BLEAdvertising *p_advertising;
};