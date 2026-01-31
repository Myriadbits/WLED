
// void BLEConfig::gattClientEventHandler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t* param) 
// {
// 	BLECONFIG_LOG("gattClientEventHandler [esp_gatt_if: %d] ... %s",
// 		gattc_if, BLEUtils::gattClientEventTypeToString(event).c_str());
// 	BLEUtils::dumpGattClientEvent(event, gattc_if, param);

// 	switch(event) 
//     {
// 		case ESP_GATTC_CONNECT_EVT: 
//         {
// #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
// 			if(BLEDevice::m_securityLevel)
//             {
// 				esp_ble_set_encryption(param->connect.remote_bda, BLEDevice::m_securityLevel);
// 			}
// #endif	// CONFIG_BLE_SMP_ENABLE
// 			break;
// 		} // ESP_GATTS_CONNECT_EVT

// 		default:
// 			break;
// 	} // switch
// 	for(auto &myPair : BLEDevice::getPeerDevices(true)) 
//     {
// 		// conn_status_t conn_status = (conn_status_t)myPair.second;
// 		// if(((BLEClient*)conn_status.peer_device)->getGattcIf() == gattc_if || ((BLEClient*)conn_status.peer_device)->getGattcIf() == ESP_GATT_IF_NONE || gattc_if == ESP_GATT_IF_NONE){
// 		// 	((BLEClient*)conn_status.peer_device)->gattClientEventHandler(event, gattc_if, param);
// 		// }
// 	}

// 	// if(m_customGattcHandler != nullptr) {
// 	// 	m_customGattcHandler(event, gattc_if, param);
// 	// }

// } // gattClientEventHandler


// /**
//  * @brief Handle GATT server events.
//  *
//  * @param [in] event The event that has been newly received.
//  * @param [in] gatts_if The connection to the GATT interface.
//  * @param [in] param Parameters for the event.
//  */
// void BLEConfig::gattServerEventHandler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t* param) 
// {
// 	BLECONFIG_LOG("gattServerEventHandler [esp_gatt_if: %d] ... %s",
// 		gatts_if,
// 		BLEUtils::gattServerEventTypeToString(event).c_str());

// 	BLEUtils::dumpGattServerEvent(event, gatts_if, param);

// 	switch (event) 
//     {
// 		case ESP_GATTS_CONNECT_EVT: 
//         {
// #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
// 			if(BLEDevice::m_securityLevel)
//             {
// 				esp_ble_set_encryption(param->connect.remote_bda, BLEDevice::m_securityLevel);
// 			}
// #endif	// CONFIG_BLE_SMP_ENABLE
// 			break;
// 		} // ESP_GATTS_CONNECT_EVT

// 		default: 
// 			break;
// 	} // switch


// 	// if (BLEDevice::m_pServer != nullptr) 
//     // {
// 	// 	BLEDevice::m_pServer->handleGATTServerEvent(event, gatts_if, param);
// 	// }

// 	// if(m_customGattsHandler != nullptr) {
// 	// 	m_customGattsHandler(event, gatts_if, param);
// 	// }

// } // gattServerEventHandler



// /**
//  * @brief Handle GAP events.
//  */
// void BLEConfig::gapEventHandler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) 
// {
// 	BLEUtils::dumpGapEvent(event, param);

// 	switch(event) 
//     {
//         case ESP_GAP_BLE_OOB_REQ_EVT:                                /* OOB request event */
//             BLECONFIG_LOG("ESP_GAP_BLE_OOB_REQ_EVT");
//             break;
//         case ESP_GAP_BLE_LOCAL_IR_EVT:                               /* BLE local IR event */
//             BLECONFIG_LOG("ESP_GAP_BLE_LOCAL_IR_EVT");
//             break;
//         case ESP_GAP_BLE_LOCAL_ER_EVT:                               /* BLE local ER event */
//             BLECONFIG_LOG("ESP_GAP_BLE_LOCAL_ER_EVT");
//             break;
//         case ESP_GAP_BLE_NC_REQ_EVT:								/*  NUMERIC CONFIRMATION  */
//             BLECONFIG_LOG("ESP_GAP_BLE_NC_REQ_EVT");
// // #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
// // 			if(BLEDevice::m_securityCallbacks != nullptr){
// // 				esp_ble_confirm_reply(param->ble_security.ble_req.bd_addr, BLEDevice::m_securityCallbacks->onConfirmPIN(param->ble_security.key_notif.passkey));
// // 			}
// // #endif	// CONFIG_BLE_SMP_ENABLE
// 			break;
//         case ESP_GAP_BLE_PASSKEY_REQ_EVT:                           /* passkey request event */
//             BLECONFIG_LOG("ESP_GAP_BLE_PASSKEY_REQ_EVT: ");
//             // esp_log_buffer_hex(m_remote_bda, sizeof(m_remote_bda));
// // #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
// // 			if(BLEDevice::m_securityCallbacks != nullptr){
// // 				esp_ble_passkey_reply(param->ble_security.ble_req.bd_addr, true, BLEDevice::m_securityCallbacks->onPassKeyRequest());
// // 			}
// // #endif	// CONFIG_BLE_SMP_ENABLE
// 			break;
// 			/*
// 			 * TODO should we add white/black list comparison?
// 			 */
// 		case ESP_GAP_BLE_SEC_REQ_EVT:
//             /* send the positive(true) security response to the peer device to accept the security request.
//             If not accept the security request, should sent the security response with negative(false) accept value*/
//             BLECONFIG_LOG("ESP_GAP_BLE_SEC_REQ_EVT");
// // #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
// // 			if(BLEDevice::m_securityCallbacks!=nullptr)
// //             {
// // 				esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, BLEDevice::m_securityCallbacks->onSecurityRequest());
// // 			}
// // 			else
// //             {
// // 				esp_ble_gap_security_rsp(param->ble_security.ble_req.bd_addr, true);
// // 			}
// // #endif	// CONFIG_BLE_SMP_ENABLE
// 			break;
// 			 /*
// 			  *
// 			  */
// 		case ESP_GAP_BLE_PASSKEY_NOTIF_EVT:  //the app will receive this evt when the IO  has Output capability and the peer device IO has Input capability.
//             //display the passkey number to the user to input it in the peer deivce within 30 seconds
//             BLECONFIG_LOG("ESP_GAP_BLE_PASSKEY_NOTIF_EVT");
// // #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
// // 			BLECONFIG_LOG("passKey = %d", param->ble_security.key_notif.passkey);
// // 			if(BLEDevice::m_securityCallbacks!=nullptr)
// //             {
// // 				BLEDevice::m_securityCallbacks->onPassKeyNotify(param->ble_security.key_notif.passkey);
// // 			}
// // #endif	// CONFIG_BLE_SMP_ENABLE
// 			break;
// 		 case ESP_GAP_BLE_KEY_EVT:
//             //shows the ble key type info share with peer device to the user.
//             BLECONFIG_LOG("ESP_GAP_BLE_KEY_EVT");
// #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
//             BLECONFIG_LOG("key type = %s", BLESecurity::esp_key_type_to_str(param->ble_security.ble_key.key_type));
// #endif	// CONFIG_BLE_SMP_ENABLE
//             break;
// 		 case ESP_GAP_BLE_AUTH_CMPL_EVT:
//             BLECONFIG_LOG("ESP_GAP_BLE_AUTH_CMPL_EVT");
// // #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
// //             if(BLEDevice::m_securityCallbacks != nullptr)
// //             {
// //                 BLEDevice::m_securityCallbacks->onAuthenticationComplete(param->ble_security.auth_cmpl);
// //             }
// // #endif	// CONFIG_BLE_SMP_ENABLE
//             break;
// 		default: {
// 			break;
// 		}
// 	} // switch

// 	// if (BLEDevice::m_pClient != nullptr) {
// 	// 	BLEDevice::m_pClient->handleGAPEvent(event, param);
// 	// }

// 	// if (BLEDevice::m_pScan != nullptr) {
// 	// 	BLEDevice::getScan()->handleGAPEvent(event, param);
// 	// }

// 	// if(m_bleAdvertising != nullptr) {
// 	// 	BLEDevice::getAdvertising()->handleGAPEvent(event, param);
// 	// }

// 	// if(m_customGapHandler != nullptr) {
// 	// 	BLEDevice::m_customGapHandler(event, param);
// 	// }

// } // gapEventHandler

// // Initialize the BLE stuff
// void BLEConfig::init()
// {
//     BLECONFIG_LOG("BLE initializing... [%d]", getFreeHeapSize());
//     if (!btStart()) 
//     {
//         BLECONFIG_LOG("ERROR: btStart failed");
//         return;
//     }    

//     esp_bluedroid_status_t bt_state = esp_bluedroid_get_status();
//     esp_err_t errRc = ESP_OK;
//     if (bt_state == ESP_BLUEDROID_STATUS_UNINITIALIZED) 
//     {
//         errRc = esp_bluedroid_init();
//         if (errRc != ESP_OK) 
//         {
//             BLECONFIG_LOG("ERROR: esp_bluedroid_init: rc=%d", errRc);
//             return;
//         }
//     }

//     if (bt_state != ESP_BLUEDROID_STATUS_ENABLED) {
//         errRc = esp_bluedroid_enable();
//         if (errRc != ESP_OK) {
//             BLECONFIG_LOG("ERROR: esp_bluedroid_enable: rc=%d", errRc);
//             return;
//         }
//     }

//     errRc = esp_ble_gap_register_callback(BLEConfig::gapEventHandler);
//     if (errRc != ESP_OK) {
//         BLECONFIG_LOG("ERROR: esp_ble_gap_register_callback: rc=%d", errRc);
//         return;
//     }

//     errRc = esp_ble_gattc_register_callback(BLEConfig::gattClientEventHandler);
//     if (errRc != ESP_OK) {
//         BLECONFIG_LOG("ERROR: esp_ble_gattc_register_callback: rc=%d", errRc);
//         return;
//     }

//     errRc = esp_ble_gatts_register_callback(BLEConfig::gattServerEventHandler);
//     if (errRc != ESP_OK) {
//         BLECONFIG_LOG("ERROR: esp_ble_gatts_register_callback: rc=%d", errRc);
//         return;
//     }

//     errRc = ::esp_ble_gap_set_device_name(m_deviceName.c_str());
//     if (errRc != ESP_OK) {
//         BLECONFIG_LOG("ERROR: esp_ble_gap_set_device_name: rc=%d", errRc);
//         return;
//     };

// #ifdef CONFIG_BLE_SMP_ENABLE   // Check that BLE SMP (security) is configured in make menuconfig
//     esp_ble_io_cap_t iocap = ESP_IO_CAP_NONE;
//     errRc = ::esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(uint8_t));
//     if (errRc != ESP_OK) {
//         BLECONFIG_LOG("ERROR: esp_ble_gap_set_security_param: rc=%d", errRc);
//         return;
//     };
// #endif // CONFIG_BLE_SMP_ENABLE

//     vTaskDelay(200 / portTICK_PERIOD_MS); // Delay for 200 msecs as a workaround to an apparent Arduino environment issue.


//    //BLEDevice::init(m_deviceName.c_str());

//     BLECONFIG_LOG("btStarted: %d", btStarted());

//     BLECONFIG_LOG("BLE initialized [%d]", getFreeHeapSize());
// }
