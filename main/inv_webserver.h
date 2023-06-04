#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_netif.h"

#include <stdlib.h>
#include "freertos/event_groups.h"
#include "esp_wpa2.h"
#include "esp_netif.h"
#include "esp_vfs.h"
#include "esp_vfs_fat.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "driver/gpio.h"
#include "sdkconfig.h"

#include "lwip/err.h"
#include "lwip/sys.h"

#include <sys/param.h>
#include <esp_http_server.h>
#include "soc/soc.h"
//uart header files
#include "driver/uart.h"


//pin declaration
#define TXD_PIN   (GPIO_NUM_17)    //(GPIO_NUM_23)u1
#define RXD_PIN   (GPIO_NUM_16)    //(GPIO_NUM_22)
#define UART_NUM  UART_NUM_2


#define CONFIG_ESP_WIFI_SSID  "HARMONIZER_01"
#define CONFIG_ESP_WIFI_PASSWORD  "12345678"
#define CONFIG_ESP_WIFI_CHANNEL  1
#define CONFIG_ESP_MAX_STA_CONN  4
#define EXAMPLE_ESP_WIFI_SSID      CONFIG_ESP_WIFI_SSID
#define EXAMPLE_ESP_WIFI_PASS      CONFIG_ESP_WIFI_PASSWORD
#define EXAMPLE_ESP_WIFI_CHANNEL   CONFIG_ESP_WIFI_CHANNEL
#define EXAMPLE_MAX_STA_CONN       CONFIG_ESP_MAX_STA_CONN

#define MQTT_TOPIC  "HARM/VEHICLE_DATA"
#define DataAddr    "*D1#"

#define CredentialReadAddr  "*R1#"
#define CredentialWriteAddr "*T1#"
#define MqttStatusAddr      "*R2#"
#define UserId1stAddr       0xA5
#define UserId2ndAddr       0xA6
#define PassId1stAddr       0x7A
#define PassId2ndAddr       0x7B
#define ClientId1stAddr     0x7C
#define ClientId2ndAddr     0x7D
#define DataEndAddr         0x7F
#define MqttStatus1stByte   0xA7
#define MqttStatus2ndByte   0xA8

 
#define RELAY1_GPIO         5
#define RELAY2_GPIO         4
#define HIGH                1
#define LOW                 0
#define BOOT_GPIO           GPIO_NUM_0
#define EspRestartCount      40
#define OneMinute            60
#define UartTimeOutMicroSec  5*60*1000*1000 // min*sec*microsec
#define EraseCommond        "*ERASE#"


extern char Read_WifiSsidBuf[33];
extern char Read_WifiPasswordBuf[65];
extern char Read_MqttClietIdBuf[65];
extern uint8_t g_filepresent;

/*
 *Task declarations
 */

extern void Main_task(void * parm);
extern void Spifft_task(void * parm);
extern void PasswordErase_task(void * parm);
extern void UartRx_task(void *arg);
extern void EspRestart_task(void * parm);

/*
 *Function declarations
 */
extern void Write_to_file(uint8_t*,uint8_t*,uint8_t*);
extern uint8_t Read_from_file(uint8_t*,uint8_t*,uint8_t*);
extern void remove_file(void);

/*
 *Variable declarations
 */
enum state
{
    ESP_WIFI_MODE_AP = 0,
    ESP_WIFI_MODE_STA,
    WIFI_CONNECT,
    MQTT_INIT,
    MQTT_RUNNING
};
enum state current_state;