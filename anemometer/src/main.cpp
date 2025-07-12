#include <esp_adc/adc_continuous.h>
#include <esp_adc/adc_cali.h>
#include <esp_adc/adc_cali_scheme.h>
#include <HardwareSerial.h>

// Configuration
#define SAMPLE_RATE_HZ 160000        // 80kHz sampling rate
#define BUFFER_SIZE 1024            // Buffer size for samples
#define ADC_CHANNEL ADC_CHANNEL_0   // GPIO36 on ESP32-DevKitC-V4
#define ADC_ATTEN ADC_ATTEN_DB_12   // 0-3.3V range
#define ADC_BITWIDTH ADC_BITWIDTH_12 // 12-bit resolution (0-4095)

// Global variables
static uint16_t adc_buffer[BUFFER_SIZE];
static volatile bool buffer_ready = false;
static volatile int buffer_index = 0;
static adc_continuous_handle_t adc_handle = NULL;
static adc_cali_handle_t adc_cali_handle = NULL;
static uint8_t adc_raw_data[BUFFER_SIZE * SOC_ADC_DIGI_RESULT_BYTES];

// Global Functions
void send_data_for_processing(uint32_t samples_count);
float raw_to_voltage(uint16_t raw);

// ADC continuous callback function
static bool IRAM_ATTR adc_conv_done_cb(adc_continuous_handle_t handle, const adc_continuous_evt_data_t *edata, void *user_data) {
    BaseType_t mustYield = pdFALSE;
    // Set flag that conversion is done
    buffer_ready = true;
    return (mustYield == pdTRUE);
}

void setup_adc() {
    // Configure ADC continuous mode
    adc_continuous_handle_cfg_t adc_config = {
        .max_store_buf_size = BUFFER_SIZE * SOC_ADC_DIGI_RESULT_BYTES,
        .conv_frame_size = BUFFER_SIZE * SOC_ADC_DIGI_RESULT_BYTES,
    };
    
    ESP_ERROR_CHECK(adc_continuous_new_handle(&adc_config, &adc_handle));
    
    // Configure ADC pattern
    adc_digi_pattern_config_t adc_pattern[1] = {
        {
            .atten = ADC_ATTEN,
            .channel = ADC_CHANNEL,
            .unit = ADC_UNIT_1,
            .bit_width = ADC_BITWIDTH,
        }
    };
    
    adc_continuous_config_t dig_cfg = {
        .pattern_num = 1,
        .adc_pattern = adc_pattern,
        .sample_freq_hz = SAMPLE_RATE_HZ,
        .conv_mode = ADC_CONV_SINGLE_UNIT_1,
        .format = ADC_DIGI_OUTPUT_FORMAT_TYPE1,
    };
    
    ESP_ERROR_CHECK(adc_continuous_config(adc_handle, &dig_cfg));
    
    // Register callback for conversion complete
    adc_continuous_evt_cbs_t cbs = {
        .on_conv_done = adc_conv_done_cb,
    };
    ESP_ERROR_CHECK(adc_continuous_register_event_callbacks(adc_handle, &cbs, NULL));
    
    // Initialize ADC calibration
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };
    
    esp_err_t ret = adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle);
    if (ret == ESP_OK) {
        Serial.println("ADC: Calibration scheme line fitting initialized");
    } else {
        Serial.println("ADC: Calibration scheme not supported, using raw values");
    }
    
    // Start ADC continuous mode
    ESP_ERROR_CHECK(adc_continuous_start(adc_handle));
    Serial.println("ADC: Continuous mode started");
}

void setup() {
    Serial.begin(115200);
    Serial.println("ESP32 High-Speed ADC Sampling for Ultrasonic Anemometer");
    Serial.println("Sample Rate: 160kHz");
    Serial.println("ADC Channel: GPIO36");
    Serial.println("Buffer Size: 1024 samples");
    
    setup_adc();
    
    Serial.println("Sampling started...");
}

void loop() {
  static unsigned long loopStartTime = micros();

    if (buffer_ready) {
        buffer_ready = false;
        
        // Read ADC continuous data
        uint32_t bytes_read = 0;
        esp_err_t ret = adc_continuous_read(adc_handle, adc_raw_data, 
                                           BUFFER_SIZE * SOC_ADC_DIGI_RESULT_BYTES, 
                                           &bytes_read, 0);
        
        if (ret == ESP_OK) {
            // Convert raw data to 12-bit ADC values
            uint32_t samples_read = bytes_read / SOC_ADC_DIGI_RESULT_BYTES;
            
            for (int i = 0; i < samples_read && i < BUFFER_SIZE; i++) {
                adc_digi_output_data_t *p = (adc_digi_output_data_t*)&adc_raw_data[i * SOC_ADC_DIGI_RESULT_BYTES];
                if (p->type1.channel == ADC_CHANNEL) {
                    adc_buffer[i] = p->type1.data;
                }
            }
            
            // Process the buffer
            Serial.println("=== New Buffer ===");
            Serial.printf("Samples read: %ld\n", samples_read);
            
            // Calculate some basic statistics
            uint32_t sum = 0;
            uint16_t min_val = 4095;
            uint16_t max_val = 0;
            
            for (int i = 0; i < samples_read && i < BUFFER_SIZE; i++) {
                sum += adc_buffer[i];
                if (adc_buffer[i] < min_val) min_val = adc_buffer[i];
                if (adc_buffer[i] > max_val) max_val = adc_buffer[i];
            }
            
            float avg_raw = (float)sum / samples_read;
            float avg_voltage = raw_to_voltage(avg_raw);
            float min_voltage = raw_to_voltage(min_val);
            float max_voltage = raw_to_voltage(max_val);
            float pp_voltage = max_voltage - min_voltage;
            
            Serial.printf("Average: %.0f (%.3fV)\n", avg_raw, avg_voltage);
            Serial.printf("Min: %d (%.3fV)\n", min_val, min_voltage);
            Serial.printf("Max: %d (%.3fV)\n", max_val, max_voltage);
            Serial.printf("Peak-to-Peak: %.3fV\n", pp_voltage);
            
            // Print first 50 samples for waveform visualization
            Serial.println("First 50 samples (Raw, Voltage):");
            for (int i = 0; i < 50 && i < samples_read; i++) {
                float voltage = raw_to_voltage(adc_buffer[i]);
                Serial.printf("%d: %d, %.3fV\n", i, adc_buffer[i], voltage);
            }
            
            // Optional: Send data for external processing
            send_data_for_processing(samples_read);
            
            Serial.println("=================\n");
        } else {
            Serial.printf("ADC read error: %s\n", esp_err_to_name(ret));
        }
    }
    
    Serial.println("Loop time: " + String(micros() - loopStartTime) + " us");
    // Small delay to prevent watchdog timeout
    delay(10);
}

void send_data_for_processing(uint32_t samples_count) {
    // This function can be used to send the sampled data for further processing
    // Examples:
    // - Send via WiFi to a computer for analysis
    // - Store to SD card for later analysis
    // - Process locally for wind speed calculation
    
    // For now, we'll just demonstrate how to access the data
    Serial.println("Data ready for processing...");
    
    // Example: Find zero crossings (useful for frequency analysis)
    int zero_crossings = 0;
    uint16_t threshold = 2048; // Mid-point for 12-bit ADC
    bool above_threshold = adc_buffer[0] > threshold;
    
    for (int i = 1; i < samples_count; i++) {
        bool current_above = adc_buffer[i] > threshold;
        if (current_above != above_threshold) {
            zero_crossings++;
            above_threshold = current_above;
        }
    }
    
    // Estimate frequency from zero crossings
    float buffer_duration = (float)samples_count / SAMPLE_RATE_HZ;
    float estimated_frequency = (zero_crossings / 2.0) / buffer_duration;
    
    Serial.printf("Zero crossings: %d\n", zero_crossings);
    Serial.printf("Estimated frequency: %.1f Hz\n", estimated_frequency);
}

// Function to convert raw ADC value to voltage
float raw_to_voltage(uint16_t raw) {
    if (adc_cali_handle) {
        int voltage_mv;
        esp_err_t ret = adc_cali_raw_to_voltage(adc_cali_handle, raw, &voltage_mv);
        if (ret == ESP_OK) {
            return voltage_mv / 1000.0; // Convert mV to V
        }
    }
    // Fallback to raw conversion if calibration not available
    return (raw / 4095.0) * 3.3;
}

// Function to get current sampling statistics
void print_sampling_stats() {
    Serial.printf("Sampling Rate: %d Hz\n", SAMPLE_RATE_HZ);
    Serial.printf("Buffer Size: %d samples\n", BUFFER_SIZE);
    Serial.printf("Buffer Duration: %.2f ms\n", (float)BUFFER_SIZE / SAMPLE_RATE_HZ * 1000);
    Serial.printf("ADC Resolution: %d bits\n", ADC_BITWIDTH_12);
    Serial.printf("Voltage Range: 0-3.3V\n");
}