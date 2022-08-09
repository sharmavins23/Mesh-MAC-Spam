// ./main/updateSpam.c
// Spins and spams update POST requests to a server repetitively

// ===== Imports ===============================================================

#include <stdio.h>
#include <string.h>
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_http_client.h"
#include "cJSON.h"

#define TAG "UPDATE SPAM"

// ===== JSON formatting =======================================================

// // Create a JSON string based on a scraped MAC ID
// char *createJSON() {
//     // Get the ESP's MAC ID
//     uint8_t mac[6];
//     esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
//     // Convert MAC ID to string
//     char macString[18];
//     sprintf(macString,                        // Buffer to place data in
//             "%02x:%02x:%02x:%02x:%02x:%02x",  // Format string for MAC ID
//             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]  // MAC ID fields
//     );
//     // Log success of MAC ID conversion
//     ESP_LOGI(TAG, "Gained device MAC: %s", macString);

//     // Create a JSON object based on this
//     cJSON *json = cJSON_CreateObject();
//     // Create a new JSON array within this
//     cJSON *macArray = cJSON_CreateArray();
//     cJSON_AddItemToObject(json, "deviceIDs", macArray);
//     // Add the device MAC string into the macArray
//     cJSON_AddItemToArray(macArray, cJSON_CreateString(macString));

//     // Print this JSON to a character list
//     char *jsonString = cJSON_Print(json);

//     // Clean up our CJSON object (so as to not lose memory)
//     cJSON_Delete(json);

//     // Return our created JSON object
//     return jsonString;
// }

// Return the ESP's MAC ID as a string
char *getMACString() {
    // Read the ESP's MAC ID
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);

    // Convert this value to a string
    char macString[18];
    sprintf(macString,                        // Buffer to place data in
            "%02x:%02x:%02x:%02x:%02x:%02x",  // Format string for MAC ID
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]  // MAC ID fields
    );

    // Log success of MAC ID conversion
    ESP_LOGI(TAG, "Gained device MAC: %s", macString);

    return macString;
}

// Return the ESP's current temperature
float getESPTemperature() {
    // Generate the temperature sensor
    temperature_sensor_handle_t temperature_sensor_handle = NULL;
    temperature_sensor_config_t temperature_sensor_config = {
        .range_min = 20,
        .range_max = 50,
    };
    ESP_ERROR_CHECK(temperature_sensor_install(&temperature_sensor_config,
                                               &temperature_sensor_handle));

    // Enable the temperature sensor
    ESP_ERROR_CHECK(temperature_sensor_enable(temperature_sensor_handle));

    float temperature = 0;

    // Read the temperature in celsius
    ESP_ERROR_CHECK(temperature_sensor_get_temperature_celsius(
        temperature_sensor_handle, &temperature));

    // Log the temperature value
    ESP_LOGI(TAG, "Temperature: %f (*C)", temperature);

    // Disable the temperature sensor
    ESP_ERROR_CHECK(temperature_sensor_disable(temperature_sensor_handle));

    return temperature;
}

// Given an ESP reset reason, convert it to a string
char *resetReasonToString(esp_reset_reason_t reason) {
    switch (reason) {
        case ESP_RST_UNKNOWN:
            return "Reset reason can not be determined";
        case ESP_RST_POWERON:
            return "Reset due to power-on event";
        case ESP_RST_EXT:
            return "Reset by external pin (not applicable for ESP32)";
        case ESP_RST_SW:
            return "Software reset via esp_restart";
        case ESP_RST_PANIC:
            return "Software reset due to exception/panic";
        case ESP_RST_INT_WDT:
            return "Reset (software or hardware) due to interrupt watchdog";
        case ESP_RST_TASK_WDT:
            return "Reset due to task watchdog";
        case ESP_RST_WDT:
            return "Reset due to other watchdogs";
        case ESP_RST_DEEPSLEEP:
            return "Reset after exiting deep sleep mode";
        case ESP_RST_BROWNOUT:
            return "Brownout reset (software or hardware)";
        case ESP_RST_SDIO:
            return "Reset over SDIO";
        default:
            return "Reset reason is unspecified in documentation";
    }
}

// Create a JSON string, combining the proper parameters and values
char *createJSON() {
    // Create a JSON object
    cJSON *json = cJSON_CreateObject();

    // Add the device's MAC ID
    char *deviceID = getMACString();
    cJSON_AddItemToObject(json, "deviceID", cJSON_CreateString(deviceID));

    // Add the device's temperature
    float temperature = getESPTemperature();
    cJSON_AddItemToObject(json, "temperature", cJSON_CreateNumber(temperature));

    // Add the size of the free heap
    uint32_t freeHeapSize = esp_get_free_heap_size();
    cJSON_AddItemToObject(json, "freeHeapSize",
                          cJSON_CreateNumber(freeHeapSize));

    // Add the maximum contiguous free heap memory size
    // ! Note: This is definitely NOT the same value as the max contig size
    uint32_t maxFreeHeapSize = esp_get_free_internal_heap_size();

    // Get the last reset reason
    esp_reset_reason_t resetReason = esp_reset_reason_get();
    // Convert the reset reason to a string
    char *resetReasonString = resetReasonToString(resetReason);
    // Add the reset reason to the JSON object
    cJSON_AddItemToObject(json, "resetReason",
                          cJSON_CreateString(resetReasonString));

    // Print this JSON to a character list
    char *jsonString = cJSON_Print(json);

    // Clean up our CJSON object (so as to not lose memory)
    cJSON_Delete(json);

    // Return our created JSON object
    return jsonString;
}

// ===== HTTP Client Functionality =============================================

esp_http_client_handle_t setupHTTPClient() {
    // Create a new HTTP client
    esp_http_client_config_t config = {
        .url = "http://vinsdev.ml:8080/update",  // Server endpoint
        .method = HTTP_METHOD_POST,              // POST connection
    };

    // Start the HTTP session
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // Set the request header to specify JSON content towards the server
    esp_http_client_set_header(client, "Content-Type", "application/json");

    ESP_LOGI(TAG, "HTTP client setup complete.");

    // Return the client object to be used in other functions
    return client;
}

void destroyHTTPClient(esp_http_client_handle_t client) {
    // Close the connection
    esp_http_client_close(client);
    // Clean up any other objects
    esp_http_client_cleanup(client);

    ESP_LOGI(TAG, "HTTP client successfully destroyed.");

    return;
}

// Note: You are responsible for cleaning up your own POST data
void performHTTPPOST(esp_http_client_handle_t client) {
    // Get the post data
    char *postData = createJSON();  // Might slow down the HTTP post speed

    // Set the POST field data
    esp_http_client_set_post_field(client, postData, strlen(postData));

    // Perform the HTTP POST
    esp_err_t error = esp_http_client_perform(client);

    // Check for errors
    if (error != ESP_OK) {
        ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(error));
    } else {
        ESP_LOGI(TAG, "Sent HTTP POST request.");
    }

    return;
}

// ===== Main functionality ====================================================

void spamUpdates() {
    // // Create and format the JSON data
    // char *postData = createJSON();

    // Set up the HTTP client
    esp_http_client_handle_t client = setupHTTPClient();

    // Send the data as an HTTP POST, looping until we can't send anymore
    int i = 30;
    while (i--) {
        // Perform the HTTP POST request
        // performHTTPPOST(client, postData);
        performHTTPPost(client);

        // Delay the loop
        // vTaskDelay(TASK_TIME_MS / portTICK_PERIOD_MS);
    }

    // Clean up the HTTP client
    destroyHTTPClient(client);
    // Free the JSON data
    free(postData);

    return;
}