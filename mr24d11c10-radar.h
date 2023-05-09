#include "esphome.h"
#include "radar.h"
#include "radar.cpp"
#include <string>

#define MESSAGE_HEAD 0x55
#define READ_CONFIG 0x01
#define WRITE_CONFIG 0x02
#define PASSIVE_REPORT 0x03
#define ACTIVE_REPORT 0x04


#define REPORT_RADAR 0x03
#define REPORT_OTHER 0x05

#define HEARTBEAT 0x01
#define ABNOEMAL 0x02
#define ENVIRONMENT 0x05
#define BODYSIGN 0x06
#define CLOSE_AWAY 0x07

#define CA_BE 0x01
#define CA_CLOSE 0x02
#define CA_AWAY 0x03
#define SOMEBODY_BE 0x01
#define SOMEBODY_MOVE 0x01
#define SOMEBODY_STOP 0x00
#define NOBODY 0x00

#define DEVICE_ID 0x01
#define SW_VERSION 0x02
#define HW_VERSION 0x03
#define PROTOCOL_VERSION 0x04
#define SCENE 0x10
#define DATA_OFFSET 4

class MR24D11C10Component : public Component, public UARTDevice
{
public:
    MR24D11C10Component(UARTComponent *parent) : UARTDevice(parent) {}
    radar *seeedRadar;
    uint8_t buffer[64];
    size_t msg_len;


    Sensor *body_movement = new Sensor();
    BinarySensor *target_present = new BinarySensor();

    float get_setup_priority() const override { return esphome::setup_priority::AFTER_WIFI; }
    
    void setup() override
    {
        // This will be called by App.setup()
    }
    
    String bytes_2_string(uint8_t *buff, size_t size){
        return reinterpret_cast<char*>(buff);
    }
  
    void extract_data(size_t data_size, uint8_t* output){
        for (size_t i = 0; i < data_size; i++){
            output[i] = buffer[DATA_OFFSET + i];
        }
    }
  
    void send_command(uint8_t *buff, uint8_t data_length) {
        size_t total_size = 5+data_length;
        unsigned char cmd_buff[total_size];
    
        if(cmd_buff == nullptr) {
          ESP_LOGE("ERROR:", "Can't allocate memory for sending the command");
          return;
        }
    
        cmd_buff[0] = MESSAGE_HEAD;
        cmd_buff[1] = 4 + data_length;
        cmd_buff[2] = 0;
        for (size_t i = 0; i < data_length; i++){
          cmd_buff[3+i] = buff[i];
        }

        unsigned short int crc_data = 0x0000;
        crc_data = us_CalculateCrc16(cmd_buff, 3 + data_length);
        unsigned short int res = crc_data;
        res &= 0xFF00;
        res = res >> 8;
        cmd_buff[total_size-2] = res;
        
        res = crc_data;
        res &= 0x00FF;
        cmd_buff[total_size-1] = res;
    
        ESP_LOGI("Command", "Sending data to the radar");
        uint8_t data_to_send[total_size];
        for (size_t i = 0; i < total_size; i++){
          ESP_LOGI("LOG:", "0x%02x", cmd_buff[i]);
          data_to_send[i] = cmd_buff[i];
        }
        ESP_LOGI("Command", "Sending data to the radar");
        for (size_t i = 0; i < total_size; i++){
          ESP_LOGI("LOG:", "0x%02x", data_to_send[i]);
        }
        ESP_LOGI("Command", "Sending data to the radar END");

        bytes_2_string(data_to_send, total_size);
        write_array(data_to_send, total_size);
        ESP_LOGI("data_to_send", "Data is sent");
    }

    void send_new_scene_settings(uint8_t id){
        uint8_t cmd_list[4] = {0x02, 0x04, 0x10, id};
        send_command(cmd_list, 4);
    }

    void send_new_threshold(uint8_t id){
        uint8_t cmd_list[4] = {0x02, 0x04, 0x0C, id};
        send_command(cmd_list, 4);
    }

    void get_radar_device_id(){
        uint8_t cmd_list[3] = {0x01, 0x01, 0x01};
        send_command(cmd_list, 3);
    }
  
    void printBuffer() {
        switch(msg_len) {
            case 4:
                ESP_LOGD("debbug buffer", "0x%02x 0x%02x 0x%02x 0x%02x",buffer[0],buffer[1],buffer[2],buffer[3]);
                break;
            case 8:
                ESP_LOGD("debbug buffer", "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7]);
                break;
            case 9:
                ESP_LOGD("debbug buffer", "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8]);
                break;
            case 10:
                ESP_LOGD("debbug buffer", "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9]);
                break;
            case 11:
                ESP_LOGD("debbug buffer", "0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x",buffer[0],buffer[1],buffer[2],buffer[3],buffer[4],buffer[5],buffer[6],buffer[7],buffer[8],buffer[9],buffer[10]);
                break;
            default:
                ESP_LOGD("debbug buffer", "UNEXPECTED LENGTH: %d B", msg_len);
                break;
        }
    
    }

    void active_result() {
        switch(buffer[5]) {
            case ENVIRONMENT:
            case HEARTBEAT:
            case CLOSE_AWAY: {
                printBuffer();
                int result = seeedRadar->Situation_judgment(buffer[4], buffer[5], buffer[6], buffer[7], buffer[8]);
                ESP_LOGD("debbug target_present", "%d \n", result != 1);
                target_present->publish_state(result != 1);
                break;}
            case BODYSIGN:{
                float x = seeedRadar->Bodysign_val(buffer[5], buffer[6], buffer[7], buffer[8], buffer[9]);
                // Correction
                if (x > 100.0f) { x = 100.0f; }
                if (x < 0.0f) { x = 0.0f; }
                body_movement->publish_state(x);
                break;}
            default:
                ESP_LOGE("UNKNOWN ADDRESS FUNCTION", "UNEXPECTED VALUE: 0x%02x", buffer[5]);
                break;
        }
    }
    
    void read_configs() {
        ESP_LOGD("reading config", "\n");
    }

    void process_message() {
        switch(buffer[3]) {
            case READ_CONFIG:
                read_configs();
                break;
            case WRITE_CONFIG:
                // write_config();
                break;
            case ACTIVE_REPORT:
                active_result();
                break;
            case PASSIVE_REPORT:
                // get_radar_information();
                break;
            default:
                ESP_LOGE("UNKNOWN FUNCTION", "UNEXPECTED VALUE: 0x%02x", buffer[3]);
                break;
        }

    }
    
    void loop() override
    {
        
        if (available())
        {
            msg_len = 0;
            if (read() == MESSAGE_HEAD)
            {
                buffer[msg_len] = MESSAGE_HEAD;
                msg_len++;
                while (available()) {
                    uint8_t incomingValues = read();
                    buffer[msg_len] = incomingValues;
                    msg_len++;
                }
                process_message();
            }
        }
    }
};