// Include libary for this file 
#include "global.h"
#include "green_ble_server.h"

// Server UUIDs, (one for each service and characteristic)
#define UUID_SERVICE_CONTROL	"91bad492-b950-4226-aa2b-f00000000000"
#define UUID_SERVICE_TEMP		"91bad492-b950-4226-aa2b-000000000000"
#define UUID_SERVICE_LIGHT		"91bad492-b950-4226-aa2b-000000000001"
#define UUID_SERVICE_MOIST		"91bad492-b950-4226-aa2b-000000000002"
#define UUID_SERVICE_CO2		"91bad492-b950-4226-aa2b-000000000003"
#define UUID_SERVICE_PUMP		"91bad492-b950-4226-aa2b-000000000004"

#define UUID_CHARACTERISTIC_CONTROL_REQUEST	"0d563a58-196a-48ce-ace2-f00000000000"
#define UUID_CHARACTERISTIC_CONTROL_ACK		"0d563a58-196a-48ce-ace2-f00000000001"
#define UUID_CHARACTERISTIC_CONTROL_ERROR	"0d563a58-196a-48ce-ace2-f00000000002"
#define UUID_CHARACTERISTIC_TEMP_AIR		"0d563a58-196a-48ce-ace2-000000000000"
#define UUID_CHARACTERISTIC_LIGHT			"0d563a58-196a-48ce-ace2-000000000001"
#define UUID_CHARACTERISTIC_LIGHT_PANEL		"0d563a58-196a-48ce-ace2-000000000002"
#define UUID_CHARACTERISTIC_MOIST_SOIL		"0d563a58-196a-48ce-ace2-000000000003"
#define UUID_CHARACTERISTIC_MOIST_AIR		"0d563a58-196a-48ce-ace2-000000000004"
#define UUID_CHARACTERISTIC_CO2				"0d563a58-196a-48ce-ace2-000000000005"
#define UUID_CHARACTERISTIC_PUMP			"0d563a58-196a-48ce-ace2-000000000006"
#define UUID_CHARACTERISTIC_PUMP_TIME		"0d563a58-196a-48ce-ace2-000000000007"


static bool client_connected = 0;


class server_callback: public BLEServerCallbacks{
	void onConnect(BLEServer* p_server){
		client_connected = 1;
		printf("BLE_SERVER:\t Client connected to BLE server. \n");
	}
	void onDisconnect(BLEServer* p_server){
		client_connected = 0;
		printf("BLE_SERVER:\t Client disconnected to BLE server. \n");
	}
};

BLE_node_server::BLE_node_server(uint16_t stack_size){
	this->setStackSize(stack_size);	//20000
}

bool BLE_node_server::is_client_connected(){
	return client_connected;
}

void BLE_node_server::run(void *data) {

	BLEDevice::init("ESP32_node2");
	p_server = BLEDevice::createServer();
	p_server->setCallbacks(new server_callback);

	p_service_control = p_server->createService(UUID_SERVICE_CONTROL);
	p_service_temp    = p_server->createService(UUID_SERVICE_TEMP);
	p_service_light   = p_server->createService(UUID_SERVICE_LIGHT);
	p_service_moist   = p_server->createService(UUID_SERVICE_MOIST);
	p_service_co2     = p_server->createService(UUID_SERVICE_CO2);
	p_service_pump    = p_server->createService(UUID_SERVICE_PUMP);


	p_characteristic_control_request = p_service_control->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_CONTROL_REQUEST),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_WRITE
	);

	p_characteristic_control_ack = p_service_control->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_CONTROL_ACK),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_NOTIFY
	);

	p_characteristic_control_error = p_service_control->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_CONTROL_ERROR),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_NOTIFY
	);

	p_characteristic_temp_air = p_service_temp->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_TEMP_AIR),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_NOTIFY
	);

	p_characteristic_light = p_service_light->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_LIGHT),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_NOTIFY
	);

	p_characteristic_light_panel = p_service_light->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_LIGHT_PANEL),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_WRITE
	);

	p_characteristic_moist_soil = p_service_moist->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_MOIST_SOIL),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_NOTIFY
	);

	p_characteristic_moist_air = p_service_moist->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_MOIST_AIR),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_NOTIFY
	);

	p_characteristic_co2 = p_service_co2->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_CO2),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_NOTIFY
	);
	
	p_characteristic_pump = p_service_pump->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_PUMP),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_WRITE
	);
	
	p_characteristic_pump_time = p_service_pump->createCharacteristic(
		BLEUUID(UUID_CHARACTERISTIC_PUMP_TIME),
		BLECharacteristic::PROPERTY_READ  |	BLECharacteristic::PROPERTY_WRITE
	);

	p_characteristic_control_request->setValue("0");
	p_characteristic_pump->setValue("OFF");
	p_characteristic_light->setValue("OFF");
	
	p_descriptor_control_ack = new BLE2902();
	p_descriptor_control_error = new BLE2902();
	p_descriptor_temp_air = new BLE2902();
	p_descriptor_light = new BLE2902();
	p_descriptor_moist_soil = new BLE2902();
	p_descriptor_moist_air = new BLE2902();
	p_descriptor_co2 = new BLE2902();

	p_descriptor_control_ack->setNotifications(true);
	p_descriptor_control_error->setNotifications(true);
	p_descriptor_temp_air->setNotifications(true);
	p_descriptor_light->setNotifications(true);
	p_descriptor_moist_soil->setNotifications(true);
	p_descriptor_moist_air->setNotifications(true);
	p_descriptor_co2->setNotifications(true);

	p_characteristic_control_ack->addDescriptor(p_descriptor_control_ack);
	p_characteristic_control_error->addDescriptor(p_descriptor_control_error);
	p_characteristic_temp_air->addDescriptor(p_descriptor_temp_air);
	p_characteristic_light->addDescriptor(p_descriptor_light);
	p_characteristic_moist_soil->addDescriptor(p_descriptor_moist_soil);
	p_characteristic_moist_air->addDescriptor(p_descriptor_moist_air);
	p_characteristic_co2->addDescriptor(p_descriptor_co2);

	p_service_control->start();
	p_service_temp->start();
	p_service_light->start();
	p_service_moist->start();
	p_service_co2->start();
	p_service_pump->start();

	p_advertising = p_server->getAdvertising();
	p_advertising->addServiceUUID(BLEUUID(p_service_control->getUUID()));
	p_advertising->addServiceUUID(BLEUUID(p_service_temp->getUUID()));
	p_advertising->addServiceUUID(BLEUUID(p_service_light->getUUID()));
	p_advertising->addServiceUUID(BLEUUID(p_service_moist->getUUID()));
	p_advertising->addServiceUUID(BLEUUID(p_service_co2->getUUID()));
	p_advertising->addServiceUUID(BLEUUID(p_service_pump->getUUID()));
	p_advertising->start();

	printf("BLE_SERVER:\t Server setup done.\n");
}

// Control methods
uint32_t BLE_node_server::read_request(){
	uint32_t request = 0;
	sscanf(p_characteristic_control_request->getValue().c_str(), "%u", &request);
	if(request != 0){
		p_characteristic_control_request->setValue("0");
		printf("BLE_SERVER:\t Request %u received. \n", request);
	}
	return request;
}

void BLE_node_server::send_ack(bool ack){
	if(ack == 1){
		p_characteristic_control_ack->setValue("ACK");
		printf("BLE_SERVER:\t ACK sent. \n");
	}else{
		p_characteristic_control_ack->setValue("NACK");
		printf("BLE_SERVER:\t NACK sent. \n");
	}
	p_characteristic_control_ack->notify();
}

void BLE_node_server::send_error(uint32_t error_code){
	char error_code_str[10];
	sprintf(error_code_str, "%u", error_code);
	p_characteristic_control_error->setValue(error_code_str);
	p_characteristic_control_error->notify();
	printf("BLE_SERVER:\t Error code %u sent. \n", error_code);
}

// Set methods
void BLE_node_server::set_temp_air(float val){
	char val_str[10];
	sprintf(val_str, "%.0f", val);
	p_characteristic_temp_air->setValue(val_str);
	p_characteristic_temp_air->notify();
	printf("BLE_SERVER:\t Characteristic air temperature air set to %.1f C. \n", val); 
}
 
void BLE_node_server::set_light(bool val){
	if(val){
		p_characteristic_light->setValue("ON");
		printf("BLE_SERVER:\t Characteristic light set to ON. \n");
	}else{
		p_characteristic_light->setValue("OFF");
		printf("BLE_SERVER:\t Characteristic light set to OFF. \n");
	}	
	p_characteristic_light->notify();
}

void BLE_node_server::set_moist_soil(uint32_t val){
	char val_str[10];
	sprintf(val_str, "%u", val);
	p_characteristic_moist_soil->setValue(val_str);
	p_characteristic_moist_soil->notify();
	printf("BLE_SERVER:\t Characteristic soil humidity set to %u. \n", val);
}

void BLE_node_server::set_moist_air(float val){
	char val_str[10];
	sprintf(val_str, "%.0f", val);
	p_characteristic_moist_air->setValue(val_str);
	p_characteristic_moist_air->notify();
	printf("BLE_SERVER:\t Characteristic air humidity set to %.1f%%. \n", val);
}

void BLE_node_server::set_co2(uint32_t val){
	char val_str[10];
	sprintf(val_str, "%u", val);
	p_characteristic_co2->setValue(val_str);
	p_characteristic_co2->notify();
	printf("BLE_SERVER:\t Characteristic CO2 set to %u ppm.\n", val);
}

// Get methods
bool BLE_node_server::get_light_panel(){

	if(strcmp(p_characteristic_light_panel->getValue().c_str(), "ON") == 0){
		return 1;
	}else{
		return 0;
	}
}

bool BLE_node_server::get_pump(){

	if(strcmp(p_characteristic_pump->getValue().c_str(), "ON") == 0){
		p_characteristic_pump->setValue("OFF");
		return 1;
	}else{
		return 0;
	}
}

uint32_t BLE_node_server::get_pump_time(){
	uint32_t val = 0;
	sscanf(p_characteristic_pump_time->getValue().c_str(), "%u", &val);
	return val;
}