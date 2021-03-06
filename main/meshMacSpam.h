// ./main/meshMacSpam.h
// Headerfile for all other code files and code splitting simplification

#pragma once

// ===== Imports ===============================================================

/**
 * @brief Simplifies connection to WiFi server and returns once completed
 *
 */
void connectToWiFi();

/**
 * @brief Spams HTTP POSTs to /update/ on the external server with MAC ID
 *
 */
void spamUpdates();