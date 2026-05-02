# Локальная сборка с помощью uv

В этом репозитории для инструментов Python используется `uv`, а PlatformIO запускается через `uv run`.

Эта страница предназначена для сборки из исходников. Если вам нужна готовая прошивка, начните с [Загрузка и прошивка релизов](./releases.md).

## Настройка

Из корневого каталога репозитория:

```bash
uv sync
```

## Полезные команды

Список вариантов сборки:

```bash
bash build.sh list
```

Обычная сборка PlatformIO для одного варианта:

```bash
uv run pio run -e heltec_v4_repeater_mqtt
uv run pio run -e heltec_v4_companion_radio_wifi
```

Прошивка сборки:

```bash
uv run pio run -e heltec_v4_repeater_mqtt -t upload --upload-port /dev/tty.usbmodemXXXX
```

Монитор последовательного порта:

```bash
uv run pio device monitor --port /dev/tty.usbmodemXXXX --baud 115200
```

## Локальные сборки в стиле релизов

Если вам нужны те же метаданные версий, что и в рабочих процессах релизов, сначала экспортируйте переменные версий.

Companion WiFi:

```bash
export FIRMWARE_VERSION=v1.15.0
bash build.sh build-firmware heltec_v4_companion_radio_wifi
```

Repeater MQTT:

```bash
export FIRMWARE_VERSION=v1.15.0
export MESHCORETEL_VERSION=v1.0.0
bash build.sh build-firmware heltec_v4_repeater_mqtt
```

В результате в `out/` появятся версионированные артефакты.

## Поддерживаемые устройства `repeater_mqtt`

Это полные имена окружений PlatformIO, используемые для локальных сборок из исходников и для имён релизных файлов.

```text
Generic_E22_sx1262_repeater_mqtt
Generic_E22_sx1268_repeater_mqtt
Heltec_E213_repeater_mqtt
Heltec_E290_repeater_mqtt
Heltec_T190_repeater_mqtt
heltec_tracker_v2_repeater_mqtt
Heltec_v2_repeater_mqtt
Heltec_v3_repeater_mqtt
heltec_v4_repeater_mqtt
heltec_v4_tft_repeater_mqtt
Heltec_Wireless_Paper_repeater_mqtt
Heltec_Wireless_Tracker_repeater_mqtt
Heltec_WSL3_repeater_mqtt
LilyGo_T3S3_sx1262_repeater_mqtt
LilyGo_T3S3_sx1276_repeater_mqtt
LilyGo_TBeam_1W_repeater_mqtt
LilyGo_TDeck_repeater_mqtt
LilyGo_Tlora_C6_repeater_mqtt
M5Stack_Unit_C6L_repeater_mqtt
Meshadventurer_sx1262_repeater_mqtt
Meshadventurer_sx1268_repeater_mqtt
Meshimi_repeater_mqtt
nibble_screen_connect_repeater_mqtt
RAK_3112_repeater_mqtt
Station_G2_logging_repeater_mqtt
Station_G2_repeater_mqtt
T_Beam_S3_Supreme_SX1262_repeater_mqtt
Tbeam_SX1262_repeater_mqtt
Tbeam_SX1276_repeater_mqtt
WHY2025_badge_repeater_mqtt
Xiao_C6_repeater_mqtt
Xiao_S3_WIO_repeater_mqtt
```
