/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)
 
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
 
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>

#include "nvs_flash.h"

#include "esp_log.h"
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_gap_bt_api.h"
#include "esp_bt_device.h"
#include "esp_spp_api.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
 
#define SPP_TAG "SPP_ACCEPTOR_DEMO"
#define SPP_SERVER_NAME "SPP_SERVER"
#define EXAMPLE_DEVICE_NAME "ESP_SPP_ACCEPTOR"
#define BYTES_PER_LINE 16


 
static bool bWriteAfterOpenEvt = true;
static bool bWriteAfterWriteEvt = false;
static bool bWriteAfterSvrOpenEvt = true;
 
static const esp_spp_mode_t esp_spp_mode = ESP_SPP_MODE_CB;
 
 
static const esp_spp_sec_t sec_mask = ESP_SPP_SEC_AUTHENTICATE;
static const esp_spp_role_t role_slave = ESP_SPP_ROLE_SLAVE;
 
#define SPP_DATA_LEN 20
char charArrayLastReceivedData[SPP_DATA_LEN];
static uint8_t spp_data[SPP_DATA_LEN];
 
 
void saveReceivedData(uint8_t *buffer, uint16_t buff_len){    
    for(int i = 0; i < (int)buff_len; i++){
        charArrayLastReceivedData[i] = (char) buffer[i];
    }   
}

 bool SDefault = false;
 
static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    ESP_LOGI(SPP_TAG, "Start of: static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)");
    switch (event) {
    case ESP_SPP_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_INIT_EVT");
        ESP_LOGI(SPP_TAG, "Call esp_bt_dev_set_device_name(EXAMPLE_DEVICE_NAME)");
        esp_bt_dev_set_device_name(EXAMPLE_DEVICE_NAME);
        ESP_LOGI(SPP_TAG, "Call esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE)");
        esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
        ESP_LOGI(SPP_TAG, "Call esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME)");
        esp_spp_start_srv(sec_mask,role_slave, 0, SPP_SERVER_NAME);
        break;
    case ESP_SPP_DISCOVERY_COMP_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_DISCOVERY_COMP_EVT");
        break;
    case ESP_SPP_OPEN_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_OPEN_EVT");
       
        ESP_LOGI(SPP_TAG, "bWriteAfterOpenEvt: %d: ", bWriteAfterOpenEvt);
        if (bWriteAfterOpenEvt){
            ESP_LOGI(SPP_TAG, "bWriteAfterOpenEvt = true");
            ESP_LOGI(SPP_TAG, "Call esp_spp_write(param->srv_open.handle, SPP_DATA_LEN, spp_data)");
 
            esp_spp_write(param->srv_open.handle, SPP_DATA_LEN, spp_data);
 
            char * c = "Hello"; 
            uint8_t * u = (uint8_t *)c;
            esp_spp_write(param->srv_open.handle, 6, u);
 
           
        }
        else{
            ESP_LOGI(SPP_TAG, "bWriteAfterOpenEvt = false");
        }
 
        break;
    case ESP_SPP_CLOSE_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CLOSE_EVT");
        break;
    case ESP_SPP_START_EVT:                                        
        ESP_LOGI(SPP_TAG, "ESP_SPP_START_EVT");
        break;
    case ESP_SPP_CL_INIT_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CL_INIT_EVT");
        break;
    case ESP_SPP_DATA_IND_EVT:                                      
        ESP_LOGI(SPP_TAG, "ESP_SPP_DATA_IND_EVT len=%d handle=%d", param->data_ind.len, param->data_ind.handle);
        ESP_LOGI(SPP_TAG, "Call esp_log_buffer_hex("",param->data_ind.data,param->data_ind.len)");
 
        esp_log_buffer_char("Received String Data",param->data_ind.data,param->data_ind.len);
        
        saveReceivedData(param->data_ind.data,param->data_ind.len);

        if(strncmp(charArrayLastReceivedData, "010C0D",6) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "AT@1",4) == 0)
        {
            char * c = "OBDII to RS232 Interpreter>\r";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "AT@2",4) == 0)
        {
            char * c = "?\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATPC",4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATCAF",5) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATDPN", 5) == 0)
        {
            char * c = "A6\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATE0", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATH0", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATH1", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATI", 3) == 0)
        {
            char * c = "\r\rELM327 v1.5\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATL", 3) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATRV", 4) == 0)
        {
            char * c = "12V\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATS0", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATS1", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATSH", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATSP", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATTP", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATWS", 4) == 0)
        {
            char * c = "\r\rELM327 v1.5\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATZ", 3) == 0)
        {
            char * c = "\r\rELM327 v1.5\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)

        }else if (strncmp(charArrayLastReceivedData, "ATST", 4) == 0)
        {
            char * c = "OK\r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0100", 4) == 0)
        {
            if(SDefault == false){
                char * c = "SEARCHING...";
                esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
                char * d = "\r7EA 06 41 00 98 3A 80 13 \r7E8 06 41 00 BE 3F A8 13 \r\r>";
                esp_spp_write(param->srv_open.handle, strlen(d), (uint8_t *)d)
                SDefault = true;
            }else
            {
                char * d = "\r7EA 06 41 00 98 3A 80 13 \r7E8 06 41 00 BE 3F A8 13 \r\r>";
                esp_spp_write(param->srv_open.handle, strlen(d), (uint8_t *)d)
            }
            
        }else if (strncmp(charArrayLastReceivedData, "0103", 4) == 0)
        {
            char * c = "7E8 04 41 03 00 00 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0104", 4) == 0)
        {
            char * c = "7E8 03 41 04 00 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0105", 4) == 0)
        {
            char * c = "7E8 05 41 05 7B \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0106", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "010B", 4) == 0)
        {
            char * c = "7E8 03 41 0B 73 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "010C", 4) == 0)
        {
            char * c = "7E8 04 41 0C 0E 10 \r7EA 04 41 0C 0E 10 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "010D", 4) == 0)
        {
            char * c = "7E8 03 41 0D 38 \r7EA 03 41 0D 38 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "010E", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "010F", 4) == 0)
        {
            char * c = "7E8 03 41 0F 44 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0110", 4) == 0)
        {
            char * c = "7E8 04 41 10 05 1F \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "011F", 4) == 0)
        {
            char * c = "7E8 04 41 1F 00 8C \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0120", 4) == 0)
        {
            char * c = "7EA 06 41 20 80 01 A0 01 \r7E8 06 41 20 90 15 B0 15 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0121", 4) == 0)
        {
            char * c = "7E8 04 41 21 00 00 \r00 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "012C", 4) == 0)
        {
            char * c = "7E8 03 41 2C 0D \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0131", 4) == 0)
        {
            char * c = "7E8 04 41 31 C8 1F \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0133", 4) == 0)
        {
            char * c = "7E8 03 41 33 65 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0140", 4) == 0)
        {
            char * c = "7EA 06 41 40 44 CC 00 21 \r7E8 06 41 40 7A 1C 80 00 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0142", 4) == 0)
        {
            char * c = "7E8 04 41 42 39 D6 \r00 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0143", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0147", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "014D", 4) == 0)
        {
            char * c = "7E8 04 41 4D 00 00 \r00 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "014E", 4) == 0)
        {
            char * c = "7E8 04 41 4E 4C 69 \r00 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0160", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0180", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "01A0", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0101", 4) == 0)
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else if (strncmp(charArrayLastReceivedData, "0902", 4) == 0)
        {
            char * c = "7E8 10 14 49 02 01 57 50 30 \r7E8 21 5A 5A 5A 39 39 5A 54 \r7E8 22 53 33 39 30 30 30 30 \r\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }else
        {
            char * c = "\r>";
            esp_spp_write(param->srv_open.handle, strlen(c), (uint8_t *)c)
        }
        break;
    case ESP_SPP_CONG_EVT:
        ESP_LOGI(SPP_TAG, "ESP_SPP_CONG_EVT");
        break;
    case ESP_SPP_WRITE_EVT:
       ESP_LOGI(SPP_TAG, "ESP_SPP_WRITE_EVT len=%d cong=%d", param->write.len , param->write.cong);
 
        ESP_LOGI(SPP_TAG, "if param->write.cong ...");
        if (param->write.cong == 0) {
            ESP_LOGI(SPP_TAG, "param->write.cong == 0");
            if (bWriteAfterWriteEvt){
                ESP_LOGI(SPP_TAG, "bWriteAfterWriteEvt = true");
                ESP_LOGI(SPP_TAG, "Call esp_spp_write(param->write.handle, SPP_DATA_LEN, spp_data)");
                esp_spp_write(param->write.handle, SPP_DATA_LEN, spp_data);
            }
            else{
                ESP_LOGI(SPP_TAG, "bWriteAfterWriteEvt = false");
            }
        }
        else {
            ESP_LOGI(SPP_TAG, "param->write.cong <> 0");
        }
 
        break;
    case ESP_SPP_SRV_OPEN_EVT:                                      
        ESP_LOGI(SPP_TAG, "ESP_SPP_SRV_OPEN_EVT");
 
        ESP_LOGI(SPP_TAG, "bWriteAfterOpenEvt: %d: ", bWriteAfterSvrOpenEvt);
        if (bWriteAfterSvrOpenEvt){
            ESP_LOGI(SPP_TAG, "bWriteAfterSvrOpenEvt = true");
            char * c = "Hello";             
            uint8_t * u = (uint8_t *)c;
            ESP_LOGI(SPP_TAG, "Call esp_spp_write(param->srv_open.handle, 5, Hello)");
            esp_spp_write(param->srv_open.handle, 5, u);   
 
        }
        else{
            ESP_LOGI(SPP_TAG, "bWriteAfterSvrOpenEvt = false");
        } 
        break;
    default:
        break;
    }
    ESP_LOGI(SPP_TAG, "End of: static void esp_spp_cb(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)");
}
 
void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param){
    ESP_LOGI(SPP_TAG, "Start of: void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)");
    switch (event) {
    case ESP_BT_GAP_AUTH_CMPL_EVT:{
        if (param->auth_cmpl.stat == ESP_BT_STATUS_SUCCESS) {
            ESP_LOGI(SPP_TAG, "authentication success: %s", param->auth_cmpl.device_name);
            esp_log_buffer_hex(SPP_TAG, param->auth_cmpl.bda, ESP_BD_ADDR_LEN);
        } else {
            ESP_LOGE(SPP_TAG, "authentication failed, status:%d", param->auth_cmpl.stat);
        }
        break;
    }
    case ESP_BT_GAP_PIN_REQ_EVT:{
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_PIN_REQ_EVT min_16_digit:%d", param->pin_req.min_16_digit);
        if (param->pin_req.min_16_digit) {
            ESP_LOGI(SPP_TAG, "Input pin code: 0000 0000 0000 0000");
            esp_bt_pin_code_t pin_code = {0};
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 16, pin_code);
        } else {
            ESP_LOGI(SPP_TAG, "Input pin code: 1234");
            esp_bt_pin_code_t pin_code;
            pin_code[0] = '1';
            pin_code[1] = '2';
            pin_code[2] = '3';
            pin_code[3] = '4';
            esp_bt_gap_pin_reply(param->pin_req.bda, true, 4, pin_code);
        }
        break;
    }
 
    case ESP_BT_GAP_CFM_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_CFM_REQ_EVT Please compare the numeric value: %d", param->cfm_req.num_val);
        esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_NOTIF_EVT passkey:%d", param->key_notif.passkey);
        break;
    case ESP_BT_GAP_KEY_REQ_EVT:
        ESP_LOGI(SPP_TAG, "ESP_BT_GAP_KEY_REQ_EVT Please enter passkey!");
        break;
 
 
    default: {
        ESP_LOGI(SPP_TAG, "event: %d", event);
        break;
    }
    }
    return;
    ESP_LOGI(SPP_TAG, "End of: void esp_bt_gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)");
}
 
void startClassicBtSpp(void){
    ESP_LOGI(SPP_TAG, "void startClassicBtSpp(void) - Start");

    for (int i = 0; i < SPP_DATA_LEN; ++i) {
        spp_data[i] = i + 65;
    }
 
    esp_err_t ret = nvs_flash_init(); 
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK( ret );
 
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));        
 
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_LOGI(SPP_TAG, "Call esp_bt_controller_init(&bt_cfg)");
    if ((ret = esp_bt_controller_init(&bt_cfg)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
        ESP_LOGI(SPP_TAG, "Initialize controller ok");
    }
   
    ESP_LOGI(SPP_TAG, "Call esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)");
    if ((ret = esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable controller failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
        ESP_LOGI(SPP_TAG, "Enable controller ok");
    }
   
    ESP_LOGI(SPP_TAG, "Call esp_bluedroid_init()");
    if ((ret = esp_bluedroid_init()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s initialize bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
        ESP_LOGI(SPP_TAG, "Initialize bluedroid ok");
    }
   
    ESP_LOGI(SPP_TAG, "Call esp_bluedroid_enable()");
    if ((ret = esp_bluedroid_enable()) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s enable bluedroid failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
        ESP_LOGI(SPP_TAG, "Enable bluedroid ok");
    }
   
    ESP_LOGI(SPP_TAG, "Call esp_bt_gap_register_callback(esp_bt_gap_cb)");
    if ((ret = esp_bt_gap_register_callback(esp_bt_gap_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s gap register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
        ESP_LOGI(SPP_TAG, "Gap register ok");
    }
   
    ESP_LOGI(SPP_TAG, "Call esp_spp_register_callback(esp_spp_cb)");
    if ((ret = esp_spp_register_callback(esp_spp_cb)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp register failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
        ESP_LOGI(SPP_TAG, "spp register ok");
    }
   
    ESP_LOGI(SPP_TAG, "Call esp_spp_init(esp_spp_mode)");
    if ((ret = esp_spp_init(esp_spp_mode)) != ESP_OK) {
        ESP_LOGE(SPP_TAG, "%s spp init failed: %s\n", __func__, esp_err_to_name(ret));
        return;
    }
    else{
        ESP_LOGI(SPP_TAG, "spp init ok");
    }
   
    ESP_LOGI(SPP_TAG, "CONFIG_BT_SSP_ENABLED == true");
    ESP_LOGI(SPP_TAG, "Set default parameters for Secure Simple Pairing");
    esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
    esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
    esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));
 
    ESP_LOGI(SPP_TAG, "Set default parameters");
    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_VARIABLE;
    esp_bt_pin_code_t pin_code;
    esp_bt_gap_set_pin(pin_type, 0, pin_code);
 
    ESP_LOGI(SPP_TAG, "void startClassicBtSpp(void) - End");
}
 
void app_main(void){
    ESP_LOGI(SPP_TAG, "void app_main(void) - Start");
 
    startClassicBtSpp();
    ESP_LOGI(SPP_TAG, "void app_main(void) - End\n");
}