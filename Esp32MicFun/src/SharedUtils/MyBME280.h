// 2020 - November - Jordi Carrión
// Wrapper around <Adafruit_BME280.h>
// Just to make life easier

#ifdef USE_MYBME280

#include <Adafruit_BME280.h>

#define I2C_ADDRESS_BME280 0x76
#define DEFAULT_RETRY_TIME 2000

#define ShowDebugMessageF(format , ...) ShowDebugMessage(pathToFileName(__FILE__), __LINE__, __FUNCTION__, format, ##__VA_ARGS__)

class MyBME280
{
public:
	enum class ERROR {OK, BME280_NOT_FOUND, READ_ERROR};

public:
	//default constructor.
	//Uses default BME address ( I2C_ADDRESS_BME280)
	//Uses &Wire as I2C definition (default SDA, SCL pins with default speed (100khz?))
	//If showDebugInfo is true, debug messages will be shown on log_d(...)
	//You can specify your own when calling Init
	MyBME280(bool showDebugInfo=true);

	//Call to init the class and search and initialize the BME280 sensor.
	//This Init uses default values. (&Wire for I2C and default SDA/SCL BME280 address)
	//Returns ERROR
	ERROR Init();
	//Call to init the class and search and initialize the BME280 sensor.
	//This Init allows to specify the BME280 address and a specific TwoWire for the I2C bus.
	//retriesInMs specifies how many time will we wait for the sensor to be available.
	//Returns ERROR
	ERROR Init(uint16_t addrBME, TwoWire &theWire, uint16_t retriesInMs=DEFAULT_RETRY_TIME);

	//Reads all values from sensor (temperature, pressure, humidity)
	//Values can be consulted via GetLatestTemperature/Pressure/Humidity()
	ERROR ReadSensor();

	//Returns last read values. They could be NaN if sensor was not avaible.
	//Call ReadSensor() first and check return value.
	float GetLatestTemperature() {return _LastTemperature;}
	float GetLatestPressure() {return _LastPressure;}
	float GetLatestHumidity() {return _LastHumidity;}

	//Offset to be applied to Pressure readings (positive or negative), to adjust sensor misscalibration. Deafult 0.
	void SetPressureOffset(int8_t pressOffset) {_OffsetPressure=pressOffset;}

protected:
	//Shows the message as debug info if _ShowDebug is true
	void ShowDebugMessage(const char *pTheMsg , ...);
	//This methods allows to add the Methodname and the fileNumber. See macro ShowDebugMessageF
	void ShowDebugMessage(const char *pFileName, int line, const char *pMethodName, const char *pTheMsg , ...);

private:
	Adafruit_BME280 _TheBME; // The BME280 I2C object
	TwoWire &_TheWire;
	uint16_t _AddrBME;
	bool _ShowDebug;
	int8_t _OffsetPressure;

	//last values
	float _LastTemperature;
	float _LastPressure;
	float _LastHumidity;
};

#endif
