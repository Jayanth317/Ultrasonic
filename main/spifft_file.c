/* Wear levelling and FAT filesystem example.
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.

   This sample shows how to store files inside a FAT filesystem.
   FAT filesystem is stored in a partition inside SPI flash, using the
   flash wear levelling library.
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

extern void Write_to_file(uint8_t*,uint8_t*,uint8_t*);
extern uint8_t Read_from_file(uint8_t*,uint8_t*,uint8_t*);
extern void remove_file();
static const char *TAG = "example";

// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;

// Mount path for the partition
const char *base_path = "/spiflash";



void remove_file()
{
           vTaskDelay(100/ portTICK_PERIOD_MS);
           printf("remove file\n");
        /**fatfile system**/
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
        return;
    }

    remove("/spiflash/new.txt");
                // Unmount FATFS
        ESP_LOGI(TAG, "Unmounting FAT filesystem");
        ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));

}
void Write_to_file(uint8_t* lw_ssid,uint8_t* lw_pass,uint8_t* lw_client_id)
{

           vTaskDelay(100/ portTICK_PERIOD_MS);
          // remove("/spiflash/new.txt");
   // static const char *TAG = "example";
    /**fatfile system**/
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
    const esp_vfs_fat_mount_config_t mount_config = {
            .max_files = 4,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
        return;
    }

    ESP_LOGI(TAG, "Opening file to write");
    FILE *f = fopen("/spiflash/new.txt", "w");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for writing");
        ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
        return;
    }
    fprintf(f, "%s,%s,%s\n",lw_ssid,lw_pass,lw_client_id);
    fclose(f);

            // Unmount FATFS
        ESP_LOGI(TAG, "Unmounting FAT filesystem");
        ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
    
}
uint8_t Read_from_file(uint8_t* l_ssid,uint8_t* l_pass,uint8_t* l_client_id)
{
    vTaskDelay(100/ portTICK_PERIOD_MS);
   // static const char *TAG = "example";
        /**fatfile system**/
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formated before
    const esp_vfs_fat_mount_config_t mount_config = 
    {
            .max_files = 4,
            .format_if_mount_failed = true,
            .allocation_unit_size = CONFIG_WL_SECTOR_SIZE
    };
    esp_err_t err = esp_vfs_fat_spiflash_mount(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) 
    {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
        return 1;
    }


    ESP_LOGI(TAG, "Reading file");
    vTaskDelay(10/ portTICK_PERIOD_MS);
    FILE *f = fopen("/spiflash/new.txt", "r");
    if (f == NULL) {
        ESP_LOGE(TAG, "Failed to open file for reading");
        ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
        return 1;
    }
    char line[128];
    fgets(line, sizeof(line), f);
    fclose(f);
    // strip newline
    char *pos = strchr(line, '\n');
    if (pos) {
        *pos = '\0';
    }
   // ESP_LOGI(TAG, "Read from file: %s", line);
    vTaskDelay(10/ portTICK_PERIOD_MS);
    int i=0,j=0,IdCount=0;
    while(line[i] != ',')
    {
        l_ssid[i]=line[i];
        i++;
    }
    l_ssid[i]='\0';
    i++;
    while(line[i] != ',')
    {
        l_pass[j] = line[i];
        i++;
        j++;
    }
    l_pass[j]='\0';
    i++;
    while(line[i] != '\0')
    {
        l_client_id[IdCount] = line[i];
        i++;
        IdCount++;
    }
    l_client_id[IdCount]='\0';
    i++;
   ESP_LOGI(TAG, "Read a line from file: %s", line);
   vTaskDelay(10/ portTICK_PERIOD_MS);
   ESP_LOGI(TAG,"Read_ssid : %s",(char*)l_ssid);
   vTaskDelay(10/ portTICK_PERIOD_MS);
   ESP_LOGI(TAG,"Read_pass : %s",(char*)l_pass);
   vTaskDelay(10/ portTICK_PERIOD_MS);
   ESP_LOGI(TAG,"Read_ID : %s",(char*)l_client_id);
   vTaskDelay(10/ portTICK_PERIOD_MS);
   // Unmount FATFS
    ESP_LOGI(TAG, "Unmounting FAT filesystem");
    ESP_ERROR_CHECK( esp_vfs_fat_spiflash_unmount(base_path, s_wl_handle));
    printf("line_length : %d\n",strlen(line));
    if(strlen(line) == 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
    
}