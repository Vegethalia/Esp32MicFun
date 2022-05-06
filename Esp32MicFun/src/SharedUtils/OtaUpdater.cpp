#include <ArduinoOTA.h>
#include "OtaUpdater.h"


OtaUpdater::OtaUpdater()
	: _TheStatus(OtaStatus::NOT_STARTED)
	, _UpdatePercent(0)
{

}

OtaUpdater::~OtaUpdater()
{
	ArduinoOTA.end(); //just in case.
}

void OtaUpdater::Setup(const std::string& hostName, const std::string& pass, uint32_t port)
{
	// Port defaults to 3232
	ArduinoOTA.setPort(port);

	// Hostname defaults to esp3232-[MAC]
	if(!hostName.empty()) {
		ArduinoOTA.setHostname(hostName.c_str());
	}
	// No authentication by default
	if(!pass.empty()) {
		ArduinoOTA.setPassword(pass.c_str());
		// Password can be set with it's md5 value as well
		// MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
		// ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");
	}

	//ara configurem el ArduinoOta com a l'exemple oficial
	ArduinoOTA
		.onStart([=]() {
			String type;
			if(ArduinoOTA.getCommand() == U_FLASH)
				type = "sketch";
			else // U_SPIFFS
				type = "filesystem";

			// NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
			log_d("Start updating [%s]", type.c_str());
			_TheStatus = OtaStatus::UPDATING;
			_UpdatePercent=0;
			_LastErrorMessage="";
		})
		.onEnd([=]() {
			log_d("\nOta Update Finished!!");
			_TheStatus = OtaStatus::UPDATE_FINISHED;
		})
		.onProgress([=](unsigned int progress, unsigned int total) {
			auto perc = (progress / (total / 100));
			_TheStatus = OtaStatus::UPDATING;
			if(perc != _UpdatePercent) {
				log_d("Progress: %u%%\r", perc);
				_UpdatePercent = perc;
			}
		})
		.onError([=](ota_error_t error) {
			log_d("Error[%u]: ", error);
			if(error == OTA_AUTH_ERROR) _LastErrorMessage="Auth Failed";
			else if(error == OTA_BEGIN_ERROR) _LastErrorMessage = "Begin Failed";
			else if(error == OTA_CONNECT_ERROR) _LastErrorMessage = "Connect Failed";
			else if(error == OTA_RECEIVE_ERROR) _LastErrorMessage = "Receive Failed";
			else if(error == OTA_END_ERROR) _LastErrorMessage = "End Failed";
			log_d("OTA update error: %s", _LastErrorMessage.c_str());
			_TheStatus = OtaStatus::ERROR;
		});
}

void OtaUpdater::Begin()
{
	log_d("Starting the Ota Updater...");
	ArduinoOTA.begin();
	_TheStatus = OtaStatus::READY;
	_LastErrorMessage="";
}

void OtaUpdater::Process()
{
	ArduinoOTA.handle();
}
