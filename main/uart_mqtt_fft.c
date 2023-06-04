
#include "inv_webserver.h"

static const char *TAG = "Automation";
static const int RX_BUF_SIZE = 2048;



uint32_t MQTT_CONNECTED = 0;

/**function declaration*/
static void mqtt_app_start(void);
static void Esp_in_ap_mode(void);
uint8_t Convert_HexCharToHexDecimal(char character);
extern void Write_to_file(uint8_t*,uint8_t*,uint8_t*);
extern uint8_t Read_from_file(uint8_t*,uint8_t*,uint8_t*);
extern void remove_file(void);
void Esp_in_sta_mode(void);
void uart_init(void) ;
void ReadCredentials(void);
void WriteCredentials(char* DataBuf);

/*task declarations*/
void Main_task(void * parm);
void Spifft_task(void * parm);
void PasswordErase_task(void * parm);
void UartRx_task(void *arg);
void EspRestart_task(void * parm);


extern enum state current_state;

char WifiSsidBuf[33] = {0};
char WifiPasswordBuf[65] = {0};
char MqttClietIdBuf[65] = {0};
char Read_WifiSsidBuf[33] = {0};
char Read_WifiPasswordBuf[65] = {0};
char Read_MqttClietIdBuf[65] = {0};
char g_TopicNameBuf[10] = {0};
char g_TopicDataBuf[10] = {0};
char *g_Name=NULL;
char *g_Data=NULL;
uint8_t g_topic_length = 0;
uint8_t g_TopicCharCount = 0;
uint8_t g_data_length = 0;
uint8_t g_DataCharCount = 0;
uint8_t g_html_data_sent_flag = 0;
uint8_t g_wifi_connect_flag = 0;
uint8_t g_reset_esp_flag = 0;
uint8_t g_filepresent;
uint8_t g_SecondCount = 0;
uint8_t g_MinuteCount = 0;
uint8_t g_PasswordEraseFlag = 0;
uint32_t  g_PreMicroSecCount = 0;
uint32_t  g_CurrMicroSecCount = 0;
uint32_t g_diffMicroSecCount=0;



/***********wifi ap mode*************/

esp_err_t get_handler(httpd_req_t *req)
{
    const char resp[] = "<!DOCTYPE HTML><html><head>\
                            <title>ESP Input Form</title>\
                            <meta charset=\"utf-8\">\
                            <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\
                            <style>\
                            .center {\
                                        text-align: center;\
                                            color: red;\
                                    }\
                            </style>\
                            </head><body style=\"background-color:#66ccff;\">\
                            <h1 class=\"center\">Configuration page</h1>\
                            <h2 class=\"center\">User can enter the data:</h2>\
                            <form action=\"/get\">\
                                <div>\
                                  <lable>WIFI SSID &nbsp &nbsp &nbsp &nbsp &nbsp &nbsp&nbsp:</lable>\
                                  <input type=\"text\" name=\"ssid\"><br><br>\
                                </div>\
                                <div>\
                                  <lable>WIFI PASS &nbsp &nbsp &nbsp &nbsp &nbsp&nbsp&nbsp:</lable>\
                                  <input type=\"text\" name=\"pwd\"><br><br>\
                                </div>\
                                &nbsp &nbsp &nbsp &nbsp<input style=\"height:50px;width:80px\" type=\"submit\" value=\"Submit\">\
                            </form><br>\
                            </body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t get_handler_str(httpd_req_t *req)
{
    // Read the URI line and get the host
    uint8_t *buf;
    size_t buf_len;
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        memset(buf,0x00,buf_len);
        if (httpd_req_get_hdr_value_str(req, "Host", (char*)buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Host: %s",(char*) buf);
        }
        free((char*)buf);
    }

    // Read the URI line and get the parameters
     buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1) 
    {
        buf = malloc(buf_len);
        memset(buf,0x00,buf_len);
        if (httpd_req_get_url_query_str(req, (char*)buf, buf_len) == ESP_OK) 
        {
            ESP_LOGI(TAG, "Found URL query: %s", (char*)buf);
            char param[100];
            uint8_t p_count=0;
            uint8_t s_count=0;
            if (httpd_query_key_value((char*)buf, "ssid", param, sizeof(param)) == ESP_OK) 
            {
                ESP_LOGI(TAG, "The ssid string = %s",param);
                for(p_count=0,s_count=0;param[p_count] != '\0'; p_count++,s_count++)
                {
                    if(param[p_count] != '%')
                    {  
                       WifiSsidBuf[s_count] = param[p_count];
                    }
                    else
                    {
                        p_count++;
                       // WifiSsidBuf[s_count] = (param[p_count] - 48)*16 + param[p_count + 1] - 48;
                        if(param[p_count + 1] >= 58)
                        {
                          uint8_t l_deci_value=0; 
                          l_deci_value = Convert_HexCharToHexDecimal(param[p_count + 1]);
                          WifiSsidBuf[s_count] = (param[p_count] - 48)*16 + l_deci_value;
                        }
                        else
                        {
                            WifiSsidBuf[s_count] = (param[p_count] - 48)*16 + param[p_count + 1] - 48;
                        }
                        p_count++;
                    }
                }
                WifiSsidBuf[s_count] = '\0';
                ESP_LOGI(TAG, "the wifissid = %s",WifiSsidBuf);
            }
            if (httpd_query_key_value((char*)buf, "pwd",param, sizeof(param)) == ESP_OK) 
            {
                ESP_LOGI(TAG, "The WIFI password string  = %s",param);
                for(p_count=0,s_count=0;param[p_count] != '\0'; p_count++,s_count++)
                {
                    if(param[p_count] != '%')
                    {  
                       WifiPasswordBuf[s_count] = param[p_count];
                    }
                    else
                    {
                        p_count++;
                       // WifiPasswordBuf[s_count] = (param[p_count] - 48)*16 + param[p_count + 1] - 48;
                        if(param[p_count + 1] >= 58)
                        {
                          uint8_t l_deci_value=0; 
                          l_deci_value = Convert_HexCharToHexDecimal(param[p_count + 1]);
                          WifiPasswordBuf[s_count] = (param[p_count] - 48)*16 + l_deci_value;
                        }
                        else
                        {
                            WifiPasswordBuf[s_count] = (param[p_count] - 48)*16 + param[p_count + 1] - 48;
                        }
                        p_count++;
                    }
                }
                WifiPasswordBuf[s_count] = '\0';
                ESP_LOGI(TAG, "the WifiPassword = %s",WifiPasswordBuf);
            }
 
        }
        free(buf);
    }

    // The response
    const char resp[] = "The data was sent ...";
    g_html_data_sent_flag = 1;
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_get = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = get_handler,
    .user_ctx = NULL};

httpd_uri_t uri_get_input = {
    .uri = "/get",
    .method = HTTP_GET,
    .handler = get_handler_str,
    .user_ctx = NULL};


/* This handler allows the custom error handling functionality to be
 * tested from client side. For that, when a PUT request 0 is sent to
 * URI /ctrl, the /hello and /echo URIs are unregistered and following
 * custom error handler http_404_error_handler() is registered.
 * Afterwards, when /hello or /echo is requested, this custom error
 * handler is invoked which, after sending an error message to client,
 * either closes the underlying socket (when requested URI is /echo)
 * or keeps it open (when requested URI is /hello). This allows the
 * client to infer if the custom error handler is functioning as expected
 * by observing the socket state.
 */
esp_err_t http_404_error_handler(httpd_req_t *req, httpd_err_code_t err)
{
    
    /* For any other URI send 404 and close socket */
    httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "Some 404 error message");
    return ESP_FAIL;
}

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK) {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_get_input);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_stop(server);
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server) {
        ESP_LOGI(TAG, "Stopping webserver");
        stop_webserver(*server);
        *server = NULL;
    }
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    httpd_handle_t* server = (httpd_handle_t*) arg;
    if (*server == NULL) {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}



static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                    int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_AP_STACONNECTED) {
        wifi_event_ap_staconnected_t* event = (wifi_event_ap_staconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" join, AID=%d",
                 MAC2STR(event->mac), event->aid);
    } else if (event_id == WIFI_EVENT_AP_STADISCONNECTED) {
        wifi_event_ap_stadisconnected_t* event = (wifi_event_ap_stadisconnected_t*) event_data;
        ESP_LOGI(TAG, "station "MACSTR" leave, AID=%d",
                 MAC2STR(event->mac), event->aid);
    }
}

void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &wifi_event_handler,
                                                        NULL,
                                                        NULL));

    wifi_config_t wifi_config = {
        .ap = {
            .ssid = EXAMPLE_ESP_WIFI_SSID,
            .ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID),
            .channel = EXAMPLE_ESP_WIFI_CHANNEL,
            .password = EXAMPLE_ESP_WIFI_PASS,
            .max_connection = EXAMPLE_MAX_STA_CONN,
            .authmode = WIFI_AUTH_WPA_WPA2_PSK
        },
    };
    if (strlen(EXAMPLE_ESP_WIFI_PASS) == 0) {
        wifi_config.ap.authmode = WIFI_AUTH_OPEN;
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}


static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    switch (event->event_id)
    {
    case SYSTEM_EVENT_STA_START:
        esp_wifi_connect();
        ESP_LOGI(TAG, "Trying to connect with Wi-Fi\n");
        break;

    case SYSTEM_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG, "Wi-Fi connected\n");
        g_wifi_connect_flag = 1;
        break;

    case SYSTEM_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG, "got ip: startibg MQTT Client\n");
        mqtt_app_start();
        break;

    case SYSTEM_EVENT_STA_DISCONNECTED:
        ESP_LOGI(TAG, "disconnected: Retrying Wi-Fi\n");
        esp_wifi_connect();
        break;

    default:
        break;
    }
    return ESP_OK;
}

void wifi_init(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
   // s_wifi_event_group = xEventGroupCreate();
    esp_netif_t *sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_loop_init(event_handler, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_start());
}

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        MQTT_CONNECTED = 1;
        
        msg_id = esp_mqtt_client_subscribe(client, "/Invense", 0);
        ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);

        //msg_id = esp_mqtt_client_subscribe(client, MQTT_TOPIC, 1);
        //ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        MQTT_CONNECTED = 0;
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        // ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        // ESP_LOGI(TAG,"TOPIC=%.*s\r\n", event->topic_len, event->topic);
        // ESP_LOGI(TAG,"DATA=%.*s\r\n", event->data_len, event->data);
        // /****assigning pointers*******/
        // g_Name = event->topic;
        // g_Data = event->data;
        // /******Get length*****/
        // g_topic_length = event->topic_len;
        // g_data_length = event->data_len;
        // memset(g_TopicNameBuf,0x00,sizeof(g_TopicNameBuf));
        // memset(g_TopicDataBuf,0x00,sizeof(g_TopicDataBuf));

        // for(g_TopicCharCount = 0;g_TopicCharCount < g_topic_length;g_TopicCharCount++)
        // {
        //     g_TopicNameBuf[g_TopicCharCount] = g_Name[g_TopicCharCount];
        // }
        // g_TopicNameBuf[g_TopicCharCount]='\0';
        // g_Name = NULL;

        // for(g_DataCharCount = 0;g_DataCharCount < g_data_length;g_DataCharCount++)
        // {
        //     g_TopicDataBuf[g_DataCharCount] = g_Data[g_DataCharCount];
        // }
        // g_TopicDataBuf[g_DataCharCount]='\0';
        // g_Data = NULL;
        // ESP_LOGI(TAG,"g_TopicNameBuf : %s\n",g_TopicNameBuf);
        // ESP_LOGI(TAG,"g_TopicDataBuf : %s\n",g_TopicDataBuf);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t client = NULL;

static void mqtt_app_start(void)
{
    ESP_LOGI(TAG, "STARTING MQTT");
    esp_mqtt_client_config_t mqttConfig = 
    {
        .uri = "mqtt://52.14.168.247:1883"       
    };
   if(Read_MqttClietIdBuf[0] != '\0')
   {
    mqttConfig.client_id = Read_MqttClietIdBuf;
   }
   else
   {
      mqttConfig.client_id = "Invense";
   }

    printf("MqttClietIdBuf:%s\n",mqttConfig.client_id);
    client = esp_mqtt_client_init(&mqttConfig);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, client);
    esp_mqtt_client_start(client);
}

void ReadCredentials()
{
    char CreadBuf[300]={0};
    uint16_t len=0;

    len = sprintf(CreadBuf,"%c%c%s%c%c%s%c%c%s%c\n",UserId1stAddr,UserId2ndAddr,Read_WifiSsidBuf,PassId1stAddr,PassId2ndAddr,Read_WifiPasswordBuf,ClientId1stAddr,ClientId2ndAddr,Read_MqttClietIdBuf,DataEndAddr);
    uart_write_bytes(UART_NUM,CreadBuf, len);
}

void ReadMqttStatus()
{
   char StatusBuf[20]={0};
   uint8_t len=0;
   len = sprintf(StatusBuf,"%c%c%d%c\n",MqttStatus1stByte,MqttStatus2ndByte,MQTT_CONNECTED,DataEndAddr);
   uart_write_bytes(UART_NUM,StatusBuf, len);
}

void WriteCredentials(char* DataBuf)
{
  uint16_t l_DataBufCount_16=0;
  uint16_t l_BufCount_16=0;

  if(strstr(DataBuf,(char*)CredentialWriteAddr))
  {
    memset(WifiSsidBuf,0x00,sizeof(WifiSsidBuf));
    memset(WifiPasswordBuf,0x00,sizeof(WifiPasswordBuf));
    memset(MqttClietIdBuf,0x00,sizeof(MqttClietIdBuf));
    ESP_LOGI(TAG, "I am in WriteCredentials function\n\r");
      char *DataPtr=strstr(DataBuf,(char*)CredentialWriteAddr);

      if((DataPtr[4] == UserId1stAddr) || (DataPtr[5] == UserId2ndAddr))
      {
        l_DataBufCount_16 = 6;
        ESP_LOGI(TAG, "read user ssid\n\r");
      }
      else
      {
        goto exit;
      }
      l_BufCount_16=0;
      for(;(DataPtr[l_DataBufCount_16] != PassId1stAddr) || (DataPtr[l_DataBufCount_16 + 1] != PassId2ndAddr);l_DataBufCount_16++)
      {
        WifiSsidBuf[l_BufCount_16] = DataPtr[l_DataBufCount_16];
        l_BufCount_16++;
      }
      WifiSsidBuf[l_BufCount_16] = '\0';
      l_BufCount_16=0;
      l_DataBufCount_16=l_DataBufCount_16+2;
       ESP_LOGI(TAG, "read user pass\n\r");
     for(;(DataPtr[l_DataBufCount_16] != ClientId1stAddr) || (DataPtr[l_DataBufCount_16 + 1] != ClientId2ndAddr);l_DataBufCount_16++)
      {
        WifiPasswordBuf[l_BufCount_16] = DataPtr[l_DataBufCount_16];
        l_BufCount_16++;
      }
      WifiPasswordBuf[l_BufCount_16] = '\0';
      l_BufCount_16=0;
      l_DataBufCount_16=l_DataBufCount_16+2;
       ESP_LOGI(TAG, "read user client_id\n\r");
     for(;(DataPtr[l_DataBufCount_16] != DataEndAddr);l_DataBufCount_16++)
      {
        MqttClietIdBuf[l_BufCount_16] = DataPtr[l_DataBufCount_16];
        l_BufCount_16++;
      }
      MqttClietIdBuf[l_BufCount_16] = '\0';
      if(DataPtr[l_DataBufCount_16] == DataEndAddr)
      {
              g_html_data_sent_flag=1;
      }

      // remove_file();
      // ESP_LOGI(TAG, "Erase and Write again\n\r");
      // Write_to_file((uint8_t*)WifiSsidBuf,(uint8_t*)WifiPasswordBuf,(uint8_t*)MqttClietIdBuf);
      // vTaskDelay(100/ portTICK_PERIOD_MS);
      // ESP_LOGI(TAG, "WriteBuf:%s:%s:%s\n\r",WifiSsidBuf,WifiPasswordBuf,MqttClietIdBuf);
      // memset(Read_WifiSsidBuf,0x00,sizeof(Read_WifiSsidBuf));
      // memset(Read_WifiPasswordBuf,0x00,sizeof(Read_WifiPasswordBuf));
      // memset(Read_MqttClietIdBuf,0x00,sizeof(Read_MqttClietIdBuf));
      // Read_from_file((uint8_t*)Read_WifiSsidBuf,(uint8_t*)Read_WifiPasswordBuf,(uint8_t*)Read_MqttClietIdBuf);
      // ESP_LOGI(TAG, "ReadBuf:%s:%s:%s\n\r",Read_WifiSsidBuf,Read_WifiPasswordBuf,Read_MqttClietIdBuf);
exit:
ESP_LOGI(TAG, "Nothing Done\n\r");
  }

}
/********* Uart init***********/
//static const char *TAG = "MQTT_EXAMPLE";


void Esp_in_ap_mode()
{
      static httpd_handle_t server = NULL;
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_AP_STAIPASSIGNED, &connect_handler, &server));
}

uint8_t Convert_HexCharToHexDecimal(char character)
{
    uint8_t value = 0;
    if(character == 'a' || character =='A')
    {
        value = 10;
    }
    else if(character == 'b' || character =='B')
    {
        value = 11;
    }
    else if(character == 'c' || character =='C')
    {
        value = 12;
    }
    else if(character == 'd' || character =='D')
    {
        value = 13;
    }
    else if(character == 'e' || character =='E')
    {
        value = 14;
    }
    else if(character == 'f' || character =='F')
    {
        value = 15;
    } 

    return value;
}

void Esp_in_sta_mode()
{
    wifi_config_t wifi_config;
    printf("i am in wifi_connect state\n");
    bzero(&wifi_config, sizeof(wifi_config_t));
    memcpy(wifi_config.sta.ssid,Read_WifiSsidBuf, sizeof(wifi_config.sta.ssid));
    memcpy(wifi_config.sta.password, Read_WifiPasswordBuf, sizeof(wifi_config.sta.password));
    vTaskDelay(500/portTICK_PERIOD_MS);        
    ESP_ERROR_CHECK( esp_wifi_disconnect() );
    ESP_ERROR_CHECK( esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    esp_wifi_connect(); 
}
//uart initialization
void uart_init(void) 
{
    const uart_config_t uart_config = 
    {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    // We won't use a buffer for sending data.
    uart_driver_install(UART_NUM, RX_BUF_SIZE * 6, 0, 0, NULL, 0);
    uart_param_config(UART_NUM, &uart_config);
    uart_set_pin(UART_NUM, TXD_PIN, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
}


//reciever(RX) function
void UartRx_task(void *arg)
{
    uart_init();
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE+1);
    memset(data,0x00,RX_BUF_SIZE+1);
    while (1) 
    {
        g_CurrMicroSecCount = esp_timer_get_time();

        const int rxBytes = uart_read_bytes(UART_NUM, data, RX_BUF_SIZE, 100 / portTICK_RATE_MS);
        if ((rxBytes > 0)) 
        {
            g_PreMicroSecCount = g_CurrMicroSecCount;
            data[rxBytes] = 0;
            ESP_LOGI(TAG, "I am in rx function\n\r");
            ESP_LOGI(TAG, "%s\n\r",(char*)data);
            uint16_t Distance = 256*data[1]+data[2];
            char DBuf[10]={0};
            sprintf(DBuf,"%d",Distance);
            esp_mqtt_client_publish(client, "Distance",DBuf, 0, 1, 0);
            vTaskDelay(2000/ portTICK_PERIOD_MS);
            if((MQTT_CONNECTED == 1) && (strstr((char*)data,(char*)DataAddr)))
            {
              data = data + 4;
              esp_mqtt_client_publish(client, MQTT_TOPIC,(char*)data, 0, 1, 0);

              ESP_LOGI(TAG, "%s\n\r",(char*)data);
              ESP_LOGI(TAG, "%s\n\r",(char*)DataAddr);
              vTaskDelay(5/ portTICK_PERIOD_MS);  
            }
            else if(strstr((char*)data,EraseCommond))
            {
                g_PasswordEraseFlag = 1;
            }
            else if(strstr((char*)data,(char*)CredentialReadAddr))
            {
                ReadCredentials();
                ESP_LOGI(TAG, "%s\n\r",(char*)CredentialReadAddr);
            }
            else if(strstr((char*)data,(char*)CredentialWriteAddr))
            {
                WriteCredentials((char*)data);
                ESP_LOGI(TAG, "%s\n\r",(char*)CredentialWriteAddr);
            }
            else if(strstr((char*)data,(char*)MqttStatusAddr))
            {
                ReadMqttStatus();
                ESP_LOGI(TAG, "%s\n\r",(char*)MqttStatusAddr);
            }
            //uart_write_bytes(UART_NUM,data, RX_BUF_SIZE);
            memset(data,00,RX_BUF_SIZE+1);
        }
      vTaskDelay(5/ portTICK_PERIOD_MS);
      g_diffMicroSecCount = (g_CurrMicroSecCount - g_PreMicroSecCount);
      if( g_diffMicroSecCount > UartTimeOutMicroSec)
      {
            ESP_LOGI(TAG, "diff:%u",g_diffMicroSecCount);
            ESP_LOGI(TAG, "Uart Timeout Done");
            vTaskDelay(1000/ portTICK_PERIOD_MS);
            fflush(stdout);
            esp_restart(); 
      }
    }
    free(data);
}


void Main_task(void * parm)
{
    uint8_t Do_once = 0;
 while(1)
    {
        switch(current_state)
        {
            case ESP_WIFI_MODE_AP:
            { 
                ESP_LOGI(TAG, "I am in AP mode");
                if(Do_once == 0)
                {
                    Esp_in_ap_mode();
                    Do_once = 1;
                }
                else if(g_reset_esp_flag == 1)
                {
                    fflush(stdout);
                    esp_restart(); 
                }
                vTaskDelay(1000/portTICK_PERIOD_MS);
                break;
            }       
            case ESP_WIFI_MODE_STA:
            { 
                ESP_LOGI(TAG, "I am in STA mode");
                wifi_init();
                Esp_in_sta_mode();
                vTaskDelay(1000/portTICK_PERIOD_MS);
                current_state = WIFI_CONNECT;
                break;
            }
            case WIFI_CONNECT:
            { 
                ESP_LOGI(TAG, "I am in wifi_connect");
                vTaskDelay(1000/portTICK_PERIOD_MS);
                if(g_wifi_connect_flag == 1)
                {
                    current_state= MQTT_INIT;
                }
                break;
            }
            case MQTT_INIT:
            { 
                ESP_LOGI(TAG, "I am in mqtt_init");
                vTaskDelay(1000/portTICK_PERIOD_MS);
                current_state = MQTT_RUNNING;
                break;
            }
            case MQTT_RUNNING:
            { 
                
                vTaskDelay(1000/portTICK_PERIOD_MS);
                
                break;
            }
            default:
            break;

        }
        vTaskDelay(100/portTICK_PERIOD_MS);
    }

}



/*******SPIFFT task******/
void Spifft_task(void * parm)
{
      while(1)
    {
        if(g_html_data_sent_flag == 1)
        {
            vTaskDelay(1000/ portTICK_PERIOD_MS);
            ESP_LOGI(TAG, "html flag 1");
            Write_to_file((uint8_t*)WifiSsidBuf,(uint8_t*)WifiPasswordBuf,(uint8_t*)MqttClietIdBuf);
            vTaskDelay(1000/ portTICK_PERIOD_MS);
            memset(Read_WifiSsidBuf,0x00,sizeof(Read_WifiSsidBuf));
            memset(Read_WifiPasswordBuf,0x00,sizeof(Read_WifiPasswordBuf));
            memset(Read_MqttClietIdBuf,0x00,sizeof(Read_MqttClietIdBuf));
            Read_from_file((uint8_t*)Read_WifiSsidBuf,(uint8_t*)Read_WifiPasswordBuf,(uint8_t*)Read_MqttClietIdBuf);
            g_html_data_sent_flag = 0;
            g_reset_esp_flag = 1;
        }
        vTaskDelay(1000/ portTICK_PERIOD_MS);
    }  
}

 /*******Erase task******/
void PasswordErase_task(void * parm)
{ 

        while (1) 
        {
            
            if( g_PasswordEraseFlag == 1)
            {
                /*1 seconds*/
                vTaskDelay(100/ portTICK_PERIOD_MS);
                ESP_LOGI(TAG, "ESP EraseFlag is Received");
                vTaskDelay(500 / portTICK_PERIOD_MS);
                g_PasswordEraseFlag=0;
                remove_file();
                fflush(stdout);
                esp_restart();      
                
            }
            else
            {
               
                vTaskDelay(100 / portTICK_PERIOD_MS);
            }
        }

}

void EspRestart_task(void * parm)
{
    while(1)
    {
        if(g_MinuteCount >= EspRestartCount)
        {
            fflush(stdout);
            esp_restart();  
        }
        else if(g_SecondCount >= OneMinute)
        {
            g_SecondCount = 0;
            g_MinuteCount++;
            vTaskDelay(1/ portTICK_PERIOD_MS);
            ESP_LOGI(TAG, "g_MinuteCount:%d\n",g_MinuteCount);
           // printf("g_MinuteCount:%d\n",g_MinuteCount);
        }
        else
        {
            g_SecondCount++;
            vTaskDelay(1000/ portTICK_PERIOD_MS);
           // printf("g_SecondCount:%d\n",g_SecondCount);
        }
    }
  
}