#include "MyBME280.h"

#ifdef USE_MYBME280

MyBME280::MyBME280(bool showDebugInfo)
:_TheWire(Wire)
, _AddrBME(I2C_ADDRESS_BME280)
, _ShowDebug(showDebugInfo)
, _OffsetPressure(0)
{

}

MyBME280::ERROR MyBME280::Init()
{
	return Init(_AddrBME, Wire);
}

MyBME280::ERROR MyBME280::Init(uint16_t addrBME, TwoWire &theWire, uint16_t retriesInMs)
{
	auto thistime=millis();

	ShowDebugMessageF("Detecting BME280 on address 0x%X...", I2C_ADDRESS_BME280);
  bool status = _TheBME.begin(I2C_ADDRESS_BME280, &theWire);// &Wire);
  while(!status) {
    ShowDebugMessageF("Could not find a valid BME280 sensor, check wiring!");
		if((millis()-thistime)>retriesInMs) {
			break;
		}
		delay(500);
		status = _TheBME.begin(I2C_ADDRESS_BME280, &theWire);
  }
	if(status) {
		ShowDebugMessageF("BME280 sensor [%d] found!!", _TheBME.sensorID());
		return ERROR::OK;
	}
	return ERROR::BME280_NOT_FOUND;
}

MyBME280::ERROR MyBME280::ReadSensor()
{
	float temp=_TheBME.readTemperature();
	float hum=_TheBME.readHumidity();
	float press=_TheBME.readPressure()/100.0;
	bool err=false;
//	float alt=bme.readAltitude(SEALEVELPRESSURE_HPA);

	if(isnan(temp) || temp<-40 || temp>=100){
		err=true;
	}
	if(isnan(hum) || hum<0 || hum>100){
		err=true;
	}
	if(isnan(press) || press<800 || press>1100){
		err=true;
	}
	if(err) {
		ShowDebugMessageF("Error reading BME280 values");
		return ERROR::READ_ERROR;
	}
	else {
		_LastTemperature=temp;
		_LastPressure=press+_OffsetPressure;
		_LastHumidity=hum;

		ShowDebugMessageF("BME280 sensors: Temperature=%2.1f Humidity=%2.1f Pressure=%2.1f",  _LastTemperature, _LastHumidity, _LastPressure);
	}
	return ERROR::OK;
}

void MyBME280::ShowDebugMessage(const char *pTheMsg, ...)
{
	if(_ShowDebug) {
		char buffer[128];
		char bufferLog[256];
		va_list args;
    va_start(args, pTheMsg);
		vsnprintf(buffer, sizeof(buffer), pTheMsg, args);
    va_end(args);
		snprintf(bufferLog, sizeof(bufferLog), "[%s()]: %s\r\n", __FUNCTION__, buffer); //pathToFileName(__FILE__), __LINE__, __FUNCTION__
		log_printf(bufferLog);
	}
}

void MyBME280::ShowDebugMessage(const char *pFileName, int line, const char *pMethodName, const char *pTheMsg , ...)
{
	if(_ShowDebug) {
		char buffer[128];
		char bufferLog[256];
		va_list args;
    va_start(args, pTheMsg);
		vsnprintf(buffer, sizeof(buffer), pTheMsg, args);
    va_end(args);
		snprintf(bufferLog, sizeof(bufferLog), "[%s:%d] %s(): %s\r\n", pFileName, line, pMethodName, buffer);
		log_printf(bufferLog);
	}
}

#endif
