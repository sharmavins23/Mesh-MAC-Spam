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

// ===== POST functionality ====================================================

void performHTTPPOST(char *postData) {
    // Create a new HTTP client
    esp_http_client_config_t config = {
        .url = "http://vinsdev.ml:8080",  // Server endpoint
        .method = HTTP_METHOD_POST        // POST connection
    };
    // Start the HTTP session
    esp_http_client_handle_t client = esp_http_client_init(&config);
    // Set the request header to specify JSON content towards the server
    esp_http_client_set_header(client, "Content-Type", "application/json");
    // Set the request body to the JSON data string
    esp_http_client_set_post_field(client, postData, strlen(postData));

    // Loop until we get an error
    while (1) {
        // Perform the HTTP POST
        esp_err_t error = esp_http_client_perform(client);

        // If we get an error, break out of the loop
        if (error != ESP_OK) {
            ESP_LOGE(TAG, "HTTP POST failed: %s", esp_err_to_name(error));
            break;
        }
    }

    // Clean up the HTTP client
    esp_http_client_close(client);
    esp_http_client_cleanup(client);
    free(postData);  // Free the memory allocated for the JSON data string

    return;
}

// ===== JSON formatting =======================================================

// Create a JSON string based on a scraped MAC ID
char *createJSON() {
    // Get the ESP's MAC ID
    uint8_t mac[6];
    esp_wifi_get_mac(ESP_IF_WIFI_STA, mac);
    // Convert MAC ID to string
    char macString[18];
    sprintf(macString,                        // Buffer to place data in
            "%02x:%02x:%02x:%02x:%02x:%02x",  // Format string for MAC ID
            mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]  // MAC ID fields
    );
    // Log success of MAC ID conversion
    ESP_LOGI(TAG, "Gained device MAC: %s", macString);

    // Create a JSON object based on this
    cJSON *json = cJSON_CreateObject();
    // Create a new JSON array within this
    cJSON *macArray = cJSON_CreateArray();
    cJSON_AddItemToObject(json, "deviceIDs", macArray);
    // Add the device MAC string into the macArray
    cJSON_AddItemToArray(macArray, cJSON_CreateString(macString));

    // Print this JSON to a character list
    char *jsonString = cJSON_Print(json);

    // Clean up our CJSON object (so as to not lose memory)
    cJSON_Delete(json);

    // Return our created JSON object
    return jsonString;
}

// ===== Main functionality ====================================================

void spamUpdates() {
    // Create and format the JSON data
    char *postData = createJSON();

    // Send the data as an HTTP POST, looping until we can't send anymore
    performHTTPPOST(postData);

    return;
}