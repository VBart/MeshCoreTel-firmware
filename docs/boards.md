# Сравнение устройств

Эта страница поможет вам выбрать устройство, а не просто перечислит все технические детали.

Если вам нужен короткий ответ:

- для установки MQTT-ретранслятора на крышу по принципу «установил и забыл» начните с `heltec_v4`, `heltec_v4_tft`, `Station_G2` или `T_Beam_S3_Supreme_SX1262`
- для Wi-Fi-компаньона, ориентированного на приложение, бездисплейные устройства вполне подходят и зачастую проще
- если вам нужен встроенный экран, которым действительно будут пользоваться, отдавайте предпочтение TFT-платам
- если нужен экономичный экран состояния, выбирайте устройство с электронной бумагой

Таблицы ниже построены на основе метаданных устройств из PlatformIO в репозитории и флагов сборки их вариантов.

- `Устройство` — это короткое имя, используемое на этой странице сравнения. Имена файлов релизов и команды локальной сборки по-прежнему используют полные имена окружений, например `heltec_v4_repeater_mqtt`.
- `Микроконтроллер` показывает только семейство чипа. Фактическая рабочая частота может варьироваться в зависимости от окружения и конфигурации устройства.
- `ОЗУ` — встроенная оперативная память микроконтроллера.
- `PSRAM` — дополнительная память на некоторых устройствах. Больше PSRAM обычно означает больше запаса для интерфейса, MQTT и будущих функций.
- `GPS`: `✅` — присутствует, пусто — отсутствует.
- `SD`: `✅` — устройство в настоящее время поддерживает архивацию на SD в прошивке MeshCoreTel-firmware, `🧪` — аппаратно поддерживает TF/microSD, но интеграция для конкретной платы ещё нуждается в проверке, пусто — информация о поддержке SD/архива отсутствует.

## С чего начать

- Выбирайте устройство `ESP32-S3` с `16 Мб` флеш-памяти и PSRAM, если вам нужен солидный запас как для MQTT, так и для интерфейса: `heltec_v4_tft`, `heltec_v4`, `Station_G2`, `LilyGo_TBeam_1W`.
- Выбирайте устройство с TFT, если устройство будет использоваться как персональный компаньон или полевой узел: `heltec_v4_tft`, `heltec_tracker_v2`, `LilyGo_TDeck`, `Heltec_T190`.
- Выбирайте электронную бумагу, если нужен экран состояния с низким энергопотреблением в режиме ожидания и редким обновлением: `Heltec_E213`, `Heltec_E290`, `Heltec_Wireless_Paper`, `ThinkNode_M5`.
- Выбирайте бездисплейное устройство, если это преимущественно стационарный MQTT-шлюз и экран не нужен: `RAK_3112`, `Generic_E22`, `Meshimi`, `Xiao_C6`.
- Выбирайте бездисплейный Wi-Fi-компаньон, если основным интерфейсом всё равно будет приложение на телефоне: `RAK_3112_companion_radio_wifi`, `Xiao_S3_WIO_companion_radio_wifi`, `Station_G2_companion_radio_wifi`.
- Выбирайте устройство с GPS, если важно определение местоположения / мобильное использование: семейство T-Beam, `heltec_tracker_v2`, `Heltec_v3`, `heltec_v4`, `Station_G2`, `ThinkNode_M5`.
- Если вам нужны самые консервативные, проверенные временем радиомодули, то устройства с `SX1276`: `LilyGo_TLora_V2_1_1_6`, `Tbeam_SX1276` и `Heltec_v2`.

## Дополнительные критерии

Если вы выбираете между в остальном похожими устройствами, эти параметры могут быть часто важнее одной лишь флеш-памяти.

- `Конфиг ЦП` — рабочая частота, устанавливаемая MeshCore для данного устройства. `default` означает, что MeshCore не переопределяет стандартную частоту процессора.
- `Конфиг TX` — значение `LORA_TX_POWER`, заданное в MeshCore, не гарантированная мощность на выходе антенны. Платы с усилителем мощности (PA) или ВЧ-трактом могут выдавать значительно больше, чем указано в настройке.
- `Питание` — как плата реализует управление питанием: `ADC batt` (батарея через АЦП), `PMU`, `custom 2S` (свой 2S), `fixed/ext` (фиксированное/внешнее) или `none`.
- `Особенности` — заметные ВЧ или периферийные дополнения, такие как `PA/FEM`, `RF switch`, `BME280`, `NeoPixel` или `ext ant` (внешняя антенна).

| Устройство               | Конфиг ЦП | Конфиг TX     | Питание   | Особенности |
| ------------------------ | --------- | ------------- | --------- | ----------- |
| Ebyte_EoRa-S3            | default   | 22 dBm cfg    | ADC batt  |             |
| Generic_E22_sx1262       | default   | 22 dBm cfg    | ADC batt  | RF switch   |
| Generic_E22_sx1268       | default   | 22 dBm cfg    | ADC batt  | RF switch   |
| Heltec_E213              | default   | 22 dBm cfg    | ADC batt  |             |
| Heltec_E290              | default   | 22 dBm cfg    | ADC batt  |             |
| Heltec_T190              | default   | 22 dBm cfg    | ADC batt  |             |
| heltec_tracker_v2        | 160 MHz   | 9 dBm cfg\*   | ADC batt  | PA/FEM      |
| Heltec_v2                | default   | 20 dBm cfg    | ADC batt  |             |
| Heltec_v3                | 80 MHz    | 22 dBm cfg    | ADC batt  |             |
| heltec_v4                | 80 MHz    | 10 dBm cfg\*  | ADC batt  | PA/FEM      |
| heltec_v4_tft            | 80 MHz    | 10 dBm cfg\*  | ADC batt  | PA/FEM      |
| Heltec_Wireless_Paper    | default   | 22 dBm cfg    | ADC batt  |             |
| Heltec_Wireless_Tracker  | 80 MHz    | 22 dBm cfg    | none      |             |
| Heltec_WSL3              | 80 MHz    | 22 dBm cfg    | ADC batt  |             |
| LilyGo_T3S3_sx1262       | default   | 22 dBm cfg    | ADC batt  |             |
| LilyGo_T3S3_sx1276       | default   | 20 dBm cfg    | ADC batt  | RF switch   |
| LilyGo_TBeam_1W          | default   | 22 dBm cfg\*  | custom 2S | PA/FEM      |
| LilyGo_TDeck             | default   | 22 dBm cfg    | ADC batt  |             |
| LilyGo_Tlora_C6          | default   | 22 dBm cfg    | none      | RF switch   |
| LilyGo_TLora_V2_1_1_6    | default   | 20 dBm cfg    | ADC batt  |             |
| M5Stack_Unit_C6L         | default   | 22 dBm cfg    | none      | RF switch   |
| Meshadventurer_sx1262    | default   | 22 dBm cfg    | ADC batt  | RF switch   |
| Meshadventurer_sx1268    | default   | 22 dBm cfg    | ADC batt  | RF switch   |
| Meshimi                  | default   | 22 dBm cfg    | none      | ext ant     |
| nibble_screen_connect    | default   | 22 dBm cfg    | none      | NeoPixel    |
| RAK_3112                 | 80 MHz    | 22 dBm cfg    | ADC batt  |             |
| Station_G2               | default   | 7 dBm cfg\*   | fixed/ext | PA/FEM      |
| Station_G2_logging       | default   | 7 dBm cfg\*   | fixed/ext | PA/FEM      |
| T_Beam_S3_Supreme_SX1262 | default   | 22 dBm cfg    | PMU       | BME280      |
| Tbeam_SX1262             | default   | 22 dBm cfg    | PMU       |             |
| Tbeam_SX1276             | default   | 20 dBm cfg    | PMU       |             |
| ThinkNode_M2             | default   | 22 dBm cfg    | ADC batt  |             |
| ThinkNode_M5             | default   | 22 dBm cfg    | ADC batt  |             |
| WHY2025_badge            | default   | 22 dBm cfg    | none      |             |
| Xiao_C6                  | default   | 22 dBm cfg    | none      | RF switch   |
| Xiao_S3_WIO              | default   | 22 dBm cfg    | none      | RF switch   |

\* Эти платы используют усилитель мощности (PA), поэтому эффективная выходная мощность может быть значительно выше настроенного значения `LORA_TX_POWER`.

## Устройства `repeater_mqtt`

В этой таблице перечислены все сборки MQTT-ретранслятора, определённые в `variants/eastmesh_mqtt/platformio.ini`.

| Устройство               | Микроконтроллер | ОЗУ    | PSRAM | Флеш   | LoRa   | Дисплей          | GPS | SD  |
| ------------------------ | --------------- | ------ | ----- | ------ | ------ | ---------------- | --- | --- |
| Ebyte_EoRa-S3            | ESP32S3         | 512 Кб | 2 Мб  | 4 Мб   | SX1262 | OLED (SSD1306)   |     |     |
| Generic_E22_sx1262       | ESP32           | 520 Кб | Нет   | 4 Мб   | SX1262 | Нет              |     |     |
| Generic_E22_sx1268       | ESP32           | 520 Кб | Нет   | 4 Мб   | SX1268 | Нет              |     |     |
| Heltec_E213              | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб  | SX1262 | E-paper (2.13")  |     |     |
| Heltec_E290              | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб  | SX1262 | E-paper (2.9")   |     |     |
| Heltec_T190              | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб  | SX1262 | TFT (ST7789)     |     |     |
| heltec_tracker_v2        | ESP32S3         | 512 Кб | Нет   | 8 Мб   | SX1262 | TFT (ST7735)     | ✅  |     |
| Heltec_v2                | ESP32           | 520 Кб | Нет   | 8 Мб   | SX1276 | OLED (SSD1306)   |     |     |
| Heltec_v3                | ESP32S3         | 512 Кб | Нет   | 8 Мб   | SX1262 | OLED (SSD1306)   | ✅  |     |
| heltec_v4                | ESP32S3         | 512 Кб | 2 Мб  | 16 Мб  | SX1262 | OLED (SSD1306)   | ✅  |     |
| heltec_v4_tft            | ESP32S3         | 512 Кб | 2 Мб  | 16 Мб  | SX1262 | TFT (ST7789)     | ✅  |     |
| Heltec_Wireless_Paper    | ESP32S3         | 512 Кб | Нет   | 8 Мб   | SX1262 | E-paper (2.13")  |     |     |
| Heltec_Wireless_Tracker  | ESP32S3         | 512 Кб | Нет   | 8 Мб   | SX1262 | TFT (ST7735)     | ✅  |     |
| Heltec_WSL3              | ESP32S3         | 512 Кб | Нет   | 8 Мб   | SX1262 | Нет              | ✅  |     |
| LilyGo_T3S3_sx1262       | ESP32S3         | 512 Кб | 2 Мб  | 4 Мб   | SX1262 | OLED (SSD1306)   |     | 🧪  |
| LilyGo_T3S3_sx1276       | ESP32S3         | 512 Кб | 2 Мб  | 4 Мб   | SX1276 | OLED (SSD1306)   |     | 🧪  |
| LilyGo_TBeam_1W          | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб  | SX1262 | OLED (SH1106)    | ✅  | ✅  |
| LilyGo_TDeck             | ESP32S3         | 512 Кб | Нет   | 16 Мб  | SX1262 | TFT (ST7789)     | ✅  | 🧪  |
| LilyGo_Tlora_C6          | ESP32C6         | 512 Кб | Нет   | 4 Мб   | SX1262 | Нет              |     |     |
| M5Stack_Unit_C6L         | ESP32C6         | 512 Кб | Нет   | 4 Мб   | SX1262 | Нет              | ✅  |     |
| Meshadventurer_sx1262    | ESP32           | 520 Кб | Нет   | 4 Мб   | SX1262 | OLED (SSD1306)   | ✅  |     |
| Meshadventurer_sx1268    | ESP32           | 520 Кб | Нет   | 4 Мб   | SX1268 | OLED (SSD1306)   | ✅  |     |
| Meshimi                  | ESP32C6         | 512 Кб | Нет   | 4 Мб   | SX1262 | Нет              |     |     |
| nibble_screen_connect    | ESP32S3         | 512 Кб | Нет   | 4 Мб   | SX1262 | OLED (SSD1306)   |     |     |
| RAK_3112                 | ESP32S3         | 512 Кб | Нет   | 8 Мб   | SX1262 | Нет              | ✅  |     |
| Station_G2               | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб  | SX1262 | OLED (SH1106)    | ✅  |     |
| Station_G2_logging       | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб  | SX1262 | OLED (SH1106)    | ✅  |     |
| T_Beam_S3_Supreme_SX1262 | ESP32S3         | 512 Кб | 8 Мб  | 8 Мб   | SX1262 | OLED (SH1106)    | ✅  | ✅  |
| Tbeam_SX1262             | ESP32           | 520 Кб | Нет   | 4 Мб   | SX1262 | OLED (SSD1306)   | ✅  |     |
| Tbeam_SX1276             | ESP32           | 520 Кб | Нет   | 4 Мб   | SX1276 | OLED (SSD1306)   | ✅  |     |
| ThinkNode_M2             | ESP32S3         | 512 Кб | Нет   | 4 Мб   | SX1262 | OLED (SH1106)    |     |     |
| ThinkNode_M5             | ESP32S3         | 512 Кб | Нет   | 4 Мб   | SX1262 | E-paper (GxEPD)  | ✅  |     |
| WHY2025_badge            | ESP32C6         | 512 Кб | Нет   | 4 Мб   | SX1262 | Нет              |     |     |
| Xiao_C6                  | ESP32C6         | 512 Кб | Нет   | 4 Мб   | SX1262 | Нет              |     |     |
| Xiao_S3_WIO              | ESP32S3         | 512 Кб | Нет   | 8 Мб   | SX1262 | Нет              | ✅  |     |

Примечания по SD:

- `✅` — устройство в настоящее время поддерживает архивацию на SD в прошивке MeshCoreTel-firmware.
- `🧪` — аппаратно устройство, вероятно, поддерживает TF/microSD, но интеграция в прошивке MeshCoreTel-firmware ещё нуждается в проверке, прежде чем считать функцию поддерживаемой.
- Для текущего архива статистики достаточно карты microSD `4 ГБ` с `FAT32`. Карты большего объёма также поддерживаются.

## Устройства `companion_radio_wifi` с дисплеем

Это варианты сборки Wi-Fi-компаньона, у которых настроен дисплей для локального статуса, помощи при настройке или периодического прямого взаимодействия.

| Устройство                | Микроконтроллер | ОЗУ    | PSRAM | Флеш  | LoRa   | Дисплей          | GPS |
| ------------------------- | --------------- | ------ | ----- | ----- | ------ | ---------------- | --- |
| Heltec_v2                 | ESP32           | 520 Кб | Нет   |  8 Мб | SX1276 | OLED (SSD1306)   |     |
| Heltec_v3                 | ESP32S3         | 512 Кб | Нет   |  8 Мб | SX1262 | OLED (SSD1306)   | ✅  |
| heltec_tracker_v2         | ESP32S3         | 512 Кб | Нет   |  8 Мб | SX1262 | TFT (ST7735)     | ✅  |
| heltec_v4                 | ESP32S3         | 512 Кб | 2 Мб  | 16 Мб | SX1262 | OLED (SSD1306)   | ✅  |
| heltec_v4_tft             | ESP32S3         | 512 Кб | 2 Мб  | 16 Мб | SX1262 | TFT (ST7789)     | ✅  |
| LilyGo_TBeam_1W           | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб | SX1262 | OLED (SH1106)    | ✅  |
| LilyGo_TLora_V2_1_1_6     | ESP32           | 520 Кб | Нет   |  4 Мб | SX1276 | OLED (SSD1306)   | ✅  |
| nibble_screen_connect     | ESP32S3         | 512 Кб | Нет   |  4 Мб | SX1262 | OLED (SSD1306)   |     |
| Station_G2                | ESP32S3         | 512 Кб | 8 Мб  | 16 Мб | SX1262 | OLED (SH1106)    | ✅  |
| T_Beam_S3_Supreme_SX1262  | ESP32S3         | 512 Кб | 8 Мб  |  8 Мб | SX1262 | OLED (SH1106)    | ✅  |
| ThinkNode_M2              | ESP32S3         | 512 Кб | Нет   |  4 Мб | SX1262 | OLED (SH1106)    |     |
| ThinkNode_M5              | ESP32S3         | 512 Кб | Нет   |  4 Мб | SX1262 | E-paper (GxEPD)  | ✅  |

## Бездисплейные устройства `companion_radio_wifi`

Это варианты сборки Wi-Fi-компаньона, которые полагаются на сопутствующее приложение как на основной интерфейс. Для многих пользователей это лучший повседневный вариант.

| Устройство  | Микроконтроллер | ОЗУ    | PSRAM | Флеш | LoRa   | GPS |
| ----------- | --------------- | ------ | ----- | ---- | ------ | --- |
| Heltec_WSL3 | ESP32S3         | 512 Кб | Нет   | 8 Мб | SX1262 | ✅  |
| RAK_3112    | ESP32S3         | 512 Кб | Нет   | 8 Мб | SX1262 | ✅  |
| Xiao_S3_WIO | ESP32S3         | 512 Кб | Нет   | 8 Мб | SX1262 | ✅  |

## Практические рекомендации

- Лучший универсальный MQTT-ретранслятор с экраном и хорошим запасом ресурсов: `heltec_v4_repeater_mqtt`, `heltec_v4_tft_repeater_mqtt`, `Station_G2_repeater_mqtt`, `LilyGo_TBeam_1W_repeater_mqtt`.
- Лучший MQTT-ретранслятор для простой бездисплейной установки: `RAK_3112_repeater_mqtt`, `Generic_E22_sx1262_repeater_mqtt`, `Meshimi_repeater_mqtt`, `Xiao_C6_repeater_mqtt`.
- Лучшие сборки MQTT с экономичным дисплеем: `Heltec_E213_repeater_mqtt`, `Heltec_E290_repeater_mqtt`, `Heltec_Wireless_Paper_repeater_mqtt`, `ThinkNode_M5_Repeater_mqtt`.
- Лучший выбор компаньона для максимально богатого локального интерфейса: `heltec_v4_tft_companion_radio_wifi` и `heltec_tracker_v2_companion_radio_wifi`.
- Лучший выбор компаньона с максимальным запасом памяти: `T_Beam_S3_Supreme_SX1262_companion_radio_wifi`, `Station_G2_companion_radio_wifi`, `LilyGo_TBeam_1W_companion_radio_wifi`.
- Лучший выбор компаньона, если вас устраивает работа в основном через приложение и бездисплейная конфигурация: `RAK_3112_companion_radio_wifi`, `Xiao_S3_WIO_companion_radio_wifi`, `Station_G2_companion_radio_wifi`.
- Лучший выбор компаньона, если вы предпочитаете OLED вместо TFT: `T_Beam_S3_Supreme_SX1262_companion_radio_wifi`, `heltec_v4_companion_radio_wifi`, `LilyGo_TBeam_1W_companion_radio_wifi`.
