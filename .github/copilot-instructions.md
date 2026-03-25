# Esp32MicFun - context de repositori per a Copilot

## Resum rapid

Aquest repositori te dos subprojectes principals:

- `Esp32MicFun/`: firmware d'ESP32 amb PlatformIO i framework Arduino.
- `AzureFunctions/`: backend en C# (.NET 7) amb Azure Functions.

L'objectiu principal es un panell LED reactiu a l'audio: l'ESP32 llegeix un microfon I2S, calcula l'FFT i renderitza efectes visuals sobre un panell LED. A mes, hi ha integracions de WiFi, MQTT, OTA, IR i consulta de consum electric via Azure/MySQL.

## Punts d'entrada importants

- `Esp32MicFun/src/main.cpp`: setup del maquinari, inicialitzacio de Preferences, I2S i creacio de tasques FreeRTOS.
- `Esp32MicFun/src/GlobalDefines.h`: configuracio de panell i definicions de compilacio. El projecte sembla treballar principalment amb `PANEL_SIZE_96x54`.
- `Esp32MicFun/src/Global.h`: estat global, cues, topics MQTT, preferencies i handles de tasques.
- `Esp32MicFun/src/FftPower.h` i `Esp32MicFun/src/FftPower.cpp`: calcul FFT i agrupacio de bins de frequencia.
- `Esp32MicFun/src/Effects/`: efectes visuals (`Wave`, `LedBars`, `MatrixFFT`, `VertSpectrogram`, `Clock`, etc.).
- `Esp32MicFun/src/Tasks/`: pipeline i serveis en segon pla (`ReaderTask`, `DrawerTask`, `WifiReconnectTask`, `ReceiveIRTask`, etc.).
- `AzureFunctions/ConsultaConsumElectricitat.cs`: Azure Function HTTP que consulta MySQL i retorna dades de consum electric.

## Flux principal del firmware

1. `ReaderTask` llegeix mostres del microfon via I2S.
2. `FftPower` calcula l'espectre i la potencia per bins.
3. La cua `MsgAudio2Draw` passa les dades al `DrawerTask`.
4. `DrawerTask` aplica l'efecte visual actiu (`DRAW_STYLE`) i actualitza el panell LED amb FastLED.

Tasques auxiliars:

- `WifiReconnectTask`: WiFi, MQTT, NTP, OTA i refresc de dades remotes.
- `ReceiveIRTask`: canvis d'estil i parametres per IR.
- `VerticalFireTaskPreCalc`: suport a l'efecte de foc.

## Tecnologies detectades

- PlatformIO
- Arduino framework per ESP32
- FreeRTOS
- FastLED
- U8g2
- PubSubClient (MQTT)
- IRremoteESP8266
- tinyu-zhao/FFT
- Azure Functions Worker v4
- .NET 7
- MySqlConnector

## Comandes habituals

Per executar els tests nadius de l'FFT:

```powershell
cd Esp32MicFun
python -m platformio test -e native
```

Per compilar el firmware ESP32 sense pujar-lo:

```powershell
cd Esp32MicFun
python -m platformio test -e az-delivery-devkit-v4 --without-uploading --without-testing
```

Per compilar Azure Functions:

```powershell
cd AzureFunctions
dotnet build
```

## Criteris de treball recomanats

- Quan toquis el firmware, revisa primer `main.cpp`, `GlobalDefines.h`, `Global.h`, `FftPower.*` i la carpeta `Tasks/`.
- Quan toquis visuals, mira la carpeta `Effects/` i com `DrawerTask` invoca cada mode.
- Si canvies logica FFT o indexacio de bins, valida `Esp32MicFun/test/test_fft_power/test_main.cpp`.
- Si canvies el backend C#, revisa si la signatura HTTP, el format JSON/CSV o les variables d'entorn de MySQL canvien.
- Evita editar secrets o configuracions locals a `mykeys.h` i `AzureFunctions/local.settings.json` tret que sigui explicitament necessari.
- `AzureFunctions/bin` i `AzureFunctions/obj` son sortides generades; no s'han de modificar manualment.

## Context extra ja existent

Hi ha un fitxer amb notes humanes i resum tecnic a `Esp32MicFun/PILOTA_INSTRUCCIONS.txt`. Si cal mes detall de domini o de la historia del projecte, val la pena llegir-lo tambe.
