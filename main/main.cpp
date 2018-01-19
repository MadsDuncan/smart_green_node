#include "global.h"
#include "green_ble_server.h"
#include "DHT22.h"

#define GPIO_LIGHT_PANEL	GPIO_NUM_17
#define GPIO_PUMP			GPIO_NUM_16	
#define COZIR_TXD	(GPIO_NUM_4)
#define COZIR_RXD	(GPIO_NUM_5)
#define	COZIR_RTS	(UART_PIN_NO_CHANGE)
#define COZIR_CTS	(UART_PIN_NO_CHANGE)

static const int COZIR_RX_BUF_SIZE = 1024;

extern "C"{
	void app_main(void);
}

BLE_node_server *p_BLE_node_server;

void request_task(void *pvParameters);
void light_panel_task(void *pvParameters);
void pump_task(void *pvParameters);
void moist_soil_task(void *pvParameters);
static void COZIR_rx_task(void *pvParameters);
static void COZIR_tx_task(void *pvParameters);
void DHT_task(void *pvParameters);

esp_adc_cal_characteristics_t moist_soil_setup();
void COZIR_UART_init();
int COZIR_sendData(const char* data);
void send_ack(bool ack_nack);

TaskHandle_t request_task_handle = NULL;
TaskHandle_t light_panel_task_handle = NULL;
TaskHandle_t pump_task_handle = NULL;
TaskHandle_t moist_soil_task_handle = NULL;
TaskHandle_t COZIR_rx_task_handle = NULL;
TaskHandle_t COZIR_tx_task_handle = NULL;
TaskHandle_t DHT_task_handle = NULL;


void app_main(void){
	
	p_BLE_node_server = new BLE_node_server(20000);
	p_BLE_node_server->start();
	vTaskDelay(1000/portTICK_PERIOD_MS);

	#ifdef USE_SEMAPHORE_BLE_OCCU
	semaphore_BLE_occu = xSemaphoreCreateBinary();
	xSemaphoreGive(semaphore_BLE_occu);
	#endif

	xTaskCreate(request_task, "request_task", 10000, NULL, 10, &request_task_handle);
	xTaskCreate(light_panel_task, "light_panel_task", 10000, NULL, 20, &light_panel_task_handle);
	xTaskCreate(pump_task, "pump_task", 10000, NULL, 20, &pump_task_handle);

	xTaskCreate(moist_soil_task, "moist_soil_task", 10000, NULL, 10, &moist_soil_task_handle);
	xTaskCreate(COZIR_rx_task, "COZIR_rx_task", 1024*2, NULL, 11, &COZIR_rx_task_handle);
	xTaskCreate(COZIR_tx_task, "COZIR_tx_task", 1024*2, NULL, 10, &COZIR_tx_task_handle);
	xTaskCreate(DHT_task, "DHT_task", 1024*2, NULL, 10, &DHT_task_handle);

} // app_main


void request_task(void *pvParameters){
	uint32_t request_num = 0;

	printf("REQUEST_TASK:\t Ready to receive requests. \n");

	while(1){

		// Wait for a client to connect
		while(p_BLE_node_server->is_client_connected() == 0){
			vTaskDelay(100/portTICK_PERIOD_MS);
		}

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Wait for BLE set resource to become available.
		while( xSemaphoreTake(semaphore_BLE_occu, (TickType_t)10) == pdFALSE ){
			vTaskDelay(50/portTICK_PERIOD_MS);
		}
		#endif

		request_num = p_BLE_node_server->read_request();

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Release BLE resource.
		xSemaphoreGive(semaphore_BLE_occu);
		#endif

		switch(request_num){
			case 0:	// Idle, no request received
				vTaskDelay(2000/portTICK_PERIOD_MS);
				break;
			case 1:	// Measure all sensors and and advertise results on the BLE server
				send_ack(1);
				xTaskNotifyGive(moist_soil_task_handle);
				xTaskNotifyGive(COZIR_tx_task_handle);
				xTaskNotifyGive(DHT_task_handle);
				break;
			/* Room for more request options.
			case 2:	
				send_ack(1);

				break;
			*/
			default:

				send_ack(0);
				vTaskDelay(3000/portTICK_PERIOD_MS);
				break;
		} // switch
	} // while(1)
} // request_task


void light_panel_task(void *pvParameters){

	gpio_pad_select_gpio(GPIO_LIGHT_PANEL);
	gpio_set_direction(GPIO_LIGHT_PANEL, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_LIGHT_PANEL, 1);	// Off (active low)

	bool light_panel_current = 0, light_panel_last = 0;

	while(1){
		light_panel_current = p_BLE_node_server->get_light_panel();

		if(light_panel_current == 1 && light_panel_last == 0){
			
			gpio_set_level(GPIO_LIGHT_PANEL, 0);	// Turn on LED (active low)
			printf("LIGHT_PANEL_TASK:\t Light panel ON \n");
			send_ack(1);

		}else if(light_panel_current == 0 && light_panel_last == 1){

			gpio_set_level(GPIO_LIGHT_PANEL, 1);	// Turn off LED (active low)
			printf("LIGHT_PANEL_TASK:\t Light panel OFF \n");
			send_ack(1);
		}
		light_panel_last = light_panel_current;
		vTaskDelay(3000/portTICK_PERIOD_MS);
	} // while(1)
} // light_panel_task


void pump_task(void *pvParameters){
	bool pump_status = 0;

	gpio_pad_select_gpio(GPIO_PUMP);
	gpio_set_direction(GPIO_PUMP, GPIO_MODE_OUTPUT);
	gpio_set_level(GPIO_PUMP, 0);

	uint32_t pump_time = 0;
	while(1){

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Wait for BLE set resource to become available.
		while( xSemaphoreTake(semaphore_BLE_occu, (TickType_t)10) == pdFALSE ){
			vTaskDelay(50/portTICK_PERIOD_MS);
		}
		#endif

		pump_status = p_BLE_node_server->get_pump();

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Release BLE resource.
		xSemaphoreGive(semaphore_BLE_occu);
		#endif

		if(pump_status == 1){
			pump_time = p_BLE_node_server->get_pump_time();
			printf("PUMP_TASK:\t Pump activated for %u ms.\n", pump_time);
			gpio_set_level(GPIO_PUMP, 1);
			vTaskDelay(pump_time/portTICK_PERIOD_MS);
			gpio_set_level(GPIO_PUMP, 0);
			send_ack(1);
		}
	vTaskDelay(3000/portTICK_PERIOD_MS);
	} // while(1)
} // pump_task


void moist_soil_task(void *pvParameters){

	moist_soil_setup();

	while(1){
		// Wait for request task to activate this task.
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);	

		uint32_t moist_soil_val = adc1_get_raw(ADC1_CHANNEL_6);
		printf("MOIST_SOIL_TASK: Soil humidity value: %u. \n", moist_soil_val);

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Wait for BLE set resource to become available.
		while( xSemaphoreTake(semaphore_BLE_occu, (TickType_t)10) == pdFALSE ){
			vTaskDelay(50/portTICK_PERIOD_MS);
		}
		#endif

		p_BLE_node_server->set_moist_soil(moist_soil_val);

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Release BLE resource.
		xSemaphoreGive(semaphore_BLE_occu);
		#endif
	}
}


void COZIR_tx_task(void *pvParameters){
	COZIR_UART_init();

	while(1){
		// Wait for request task to activate this task.
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		COZIR_sendData("Z\r\n");

		xTaskNotifyGive(COZIR_rx_task_handle);
	} // while(1)
} // COZIR_tx_task


void COZIR_rx_task(void *pvParameters){
	uint8_t* data_str = (uint8_t*) malloc(COZIR_RX_BUF_SIZE+1);
	uint32_t data = 0;
	int rxBytes = 0;

	while(1){
		// Wait for COZIR tx task to activate this task.
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		while(rxBytes == 0){
			rxBytes = uart_read_bytes(UART_NUM_1, data_str, COZIR_RX_BUF_SIZE, 1000 / portTICK_RATE_MS);
			vTaskDelay(50/portTICK_PERIOD_MS);
		}
		data_str[rxBytes] = 0;

		sscanf((const char*)data_str, " Z %u\r\n", &data);
		printf("COZIR_RX_TASK:\t CO2 value: %u ppm. \n", data);

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Wait for BLE set resource to become available.
		while( xSemaphoreTake(semaphore_BLE_occu, (TickType_t)10) == pdFALSE ){
			vTaskDelay(50/portTICK_PERIOD_MS);
		}
		#endif

		p_BLE_node_server->set_co2(data);

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Release BLE resource.
		xSemaphoreGive(semaphore_BLE_occu);
		#endif
		
		data = 0;
		rxBytes = 0;

	} // while(1){
} // COZIR_rx_task


void DHT_task(void *pvParameters){
	float humidity = 0, temperature_air = 0;
	int ret = 0;
	setDHTgpio(0);	// GPIO_NUM_0

	while(1){
		// Wait for request task to activate this task.
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

		ret = readDHT();
		errorHandler(ret);
		humidity = getHumidity();
		temperature_air = getTemperature();
		
		printf( "DHT_task:\t Humidity value: %.1f%%. \n", humidity);
		printf( "DHT_task:\t Air temperature value: %.1f C. \n", temperature_air);

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Wait for BLE set resource to become available.
		while( xSemaphoreTake(semaphore_BLE_occu, (TickType_t)10) == pdFALSE ){
			vTaskDelay(50/portTICK_PERIOD_MS);
		}
		#endif

		p_BLE_node_server->set_moist_air(humidity);
		p_BLE_node_server->set_temp_air(temperature_air);

		#ifdef USE_SEMAPHORE_BLE_OCCU
		// Release BLE resource.
		xSemaphoreGive(semaphore_BLE_occu);
		#endif

		vTaskDelay( 3000 / portTICK_RATE_MS ); 	// Allow for sensor to take new measurement.
	} // while(1)
} // DHT_task


esp_adc_cal_characteristics_t moist_soil_setup(){
	esp_adc_cal_characteristics_t characteristics;
    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_11db);	// meas_max = V * (3.3 / 3.6) @ ADC_ATTEN_11db
    esp_adc_cal_get_characteristics(1100, ADC_ATTEN_11db, ADC_WIDTH_BIT_12, &characteristics);
	return characteristics;
} // moist_soil_setup


void COZIR_UART_init(){
	uart_config_t uart_config = {
				.baud_rate	= 9200,
				.data_bits	= UART_DATA_8_BITS,
				.parity		= UART_PARITY_DISABLE,
				.stop_bits	= UART_STOP_BITS_1,
				.flow_ctrl	= UART_HW_FLOWCTRL_DISABLE
		};

		uart_param_config(UART_NUM_1, &uart_config);
		uart_set_pin(UART_NUM_1, COZIR_TXD, COZIR_RXD, COZIR_RTS, COZIR_CTS);
		uart_driver_install(UART_NUM_1, COZIR_RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}


int COZIR_sendData(const char* data){
	const int len = strlen(data);
	const int txBytes = uart_write_bytes(UART_NUM_1, data, len);

	return txBytes;
} // sendData


void send_ack(bool ack_nack){

	#ifdef USE_SEMAPHORE_BLE_OCCU
	// Wait for BLE set resource to become available.
	while(xSemaphoreTake(semaphore_BLE_occu, (TickType_t)10) == pdFALSE){	
		vTaskDelay(50/portTICK_PERIOD_MS);
	}
	#endif

	p_BLE_node_server->send_ack(ack_nack);	

	#ifdef USE_SEMAPHORE_BLE_OCCU
	// Release BLE resource.
	xSemaphoreGive(semaphore_BLE_occu);
	#endif
}