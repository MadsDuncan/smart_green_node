/* 

	DHT22 temperature sensor driver

*/


#ifndef DHT22_H_  
#define DHT22_H_

#define DHT_OK 0
#define DHT_CHECKSUM_ERROR -1
#define DHT_TIMEOUT_ERROR -2

// == function prototypes =======================================

// #ifdef for C++ added!

#ifdef __cplusplus
extern "C"{
#endif 

extern void 	setDHTgpio(int gpio);
extern void 	errorHandler(int response);
extern int 		readDHT();
extern float 	getHumidity();
extern float 	getTemperature();
extern int 		getSignalLevel( int usTimeOut, bool state );

#ifdef __cplusplus
}
#endif 

#endif
