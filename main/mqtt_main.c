
#include "inv_webserver.h"

extern enum state current_state;
void app_main(void)
{
    
    memset(Read_WifiSsidBuf,0x00,sizeof(Read_WifiSsidBuf));
    memset(Read_WifiPasswordBuf,0x00,sizeof(Read_WifiPasswordBuf));
    memset(Read_MqttClietIdBuf,0x00,sizeof(Read_MqttClietIdBuf));
    g_filepresent = Read_from_file((uint8_t*)Read_WifiSsidBuf,(uint8_t*)Read_WifiPasswordBuf,(uint8_t*)Read_MqttClietIdBuf);
    vTaskDelay(2000/portTICK_PERIOD_MS);

    if(g_filepresent == 0)
    {
        current_state = ESP_WIFI_MODE_STA;
        vTaskDelay(500/portTICK_PERIOD_MS);
    }
    else
    {
        current_state = ESP_WIFI_MODE_AP;
        xTaskCreate(Spi+fft_task, "Spifft_task", 1024*4, NULL, 2, NULL);
        vTaskDelay(500/portTICK_PERIOD_MS);  
    }
    xTaskCreate(Main_task, "Main_task", 1024*8, NULL, 1, NULL);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    xTaskCreate(PasswordErase_task, "PassErase_task", 1024*4, NULL, 4, NULL);
    vTaskDelay(1000/portTICK_PERIOD_MS);
    // the main function that takes data and transmits
    xTaskCreate(UartRx_task, "UartRx_task", 1024*4, NULL, 5, NULL); 
    vTaskDelay(1000/portTICK_PERIOD_MS);
    xTaskCreate(EspRestart_task, "EspRestart_task", 1024*4, NULL, 6, NULL);
    vTaskDelay(1000/portTICK_PERIOD_MS);

}



