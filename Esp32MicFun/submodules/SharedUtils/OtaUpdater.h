///This class handles the OtaUpdate process in ESP32.
///The OTA can be launched using the command line tool "espota.exe"
///    ex: C:\Users\theUserName\.platformio\packages\framework-arduinoespressif32\tools\espota.exe
///        -f .\.pio\build\az-delivery-devkit-v4\firmware.bin -d -r -t 10 -i 192.168.1.168 -p 3232
///Call Setup from setup() and then in each loop() call Process()
class OtaUpdater
{
public:
	enum class OtaStatus {
		NOT_STARTED,    //Begin has not been called yet
		READY,          //Begion called sucessfully. WiFi connected. Waiting for OTA....
		UPDATING,       //OTA update started
		ERROR,          //OTA update finished with error.
		UPDATE_FINISHED //OTA Update finished OK!! (reset is inminent)
	};

public:
	OtaUpdater();
	~OtaUpdater();

	//Call Setup to initialize the underlaying ArduinoOTA object.
	//It is possible to specify the hostname (default=esp3232-[MAC]) and port (default=3232) of the updater.
	//If its needed, set the update password. By default it has no password.
	//WiFi is not necessary yet.
	void Setup(const std::string& hostName = "", const std::string& pass = "", uint32_t port = 3232);

	//Call when WiFi is available and before calling Process.
	void Begin();

	//Call in each loop() to process possible incomming petitions.
	//WiFi must be connected!!!
	void Process();

	//Returns true if Begin has been called.
	bool IsStarted() { return _TheStatus != OtaStatus::NOT_STARTED; }

	//Returns the current status.
	OtaStatus Status() { return _TheStatus; }

	//Returns the current status. If return value is UPDATING, percent returns the % processed.
	OtaStatus Status(uint8_t& percent) { return _TheStatus; }

	//Returns the last error message if Status returns ERROR
	std::string GetLastError() { return _LastErrorMessage; }

private:
	OtaStatus _TheStatus;
	std::string _LastErrorMessage; //Contains the last error message
	uint8_t _UpdatePercent;        //When updating, contains the current %
};
