
int logic ;
String cameraDetectionResults;
String boundingBoxDetails = ""; // To store bounding box details
    bool bb_found = false; // Moved declaration outside the #if directive for broader scope
void cameradetection() {

    
    cameraDetectionResults= "";
    delay(500);

    if (ei_sleep(5) != EI_IMPULSE_OK) {
        return;
    }

    snapshot_buf = (uint8_t*)malloc(EI_CAMERA_RAW_FRAME_BUFFER_COLS * EI_CAMERA_RAW_FRAME_BUFFER_ROWS * EI_CAMERA_FRAME_BYTE_SIZE);
    if(snapshot_buf == nullptr) {
        ei_printf("ERR: Failed to allocate snapshot buffer!\n");
        return;
    }

    ei::signal_t signal;
    signal.total_length = EI_CLASSIFIER_INPUT_WIDTH * EI_CLASSIFIER_INPUT_HEIGHT;
    signal.get_data = &ei_camera_get_data;

    if (!ei_camera_capture((size_t)EI_CLASSIFIER_INPUT_WIDTH, (size_t)EI_CLASSIFIER_INPUT_HEIGHT, snapshot_buf)) {
        ei_printf("Failed to capture image\r\n");
        free(snapshot_buf);
        return;
    }

    ei_impulse_result_t result = { 0 };
    EI_IMPULSE_ERROR err = run_classifier(&signal, &result, debug_nn);
    if (err != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", err);
        free(snapshot_buf);
        return;
    }

    ei_printf("Predictions (DSP: %d ms., Classification: %d ms., Anomaly: %d ms.): \n",
              result.timing.dsp, result.timing.classification, result.timing.anomaly);


#if EI_CLASSIFIER_OBJECT_DETECTION == 1
// cameraDetectionResults = "<ul>"; // Start with an unordered list
    bb_found = result.bounding_boxes[0].value > 0;
   for (size_t ix = 0; ix < result.bounding_boxes_count; ix++) {
    auto bb = result.bounding_boxes[ix];
    if (bb.value == 0) continue;

    // Convert C-style strings to String objects before concatenation
    String label(bb.label); // Assuming bb.label is a 'const char*' type

    // Now concatenate using the String class's '+' operator
    cameraDetectionResults += "<li>";
    cameraDetectionResults += label + " (" + String(bb.value, 2) + ") "; // Use two decimal places for value
    cameraDetectionResults += "[x: " + String(bb.x) + ", y: " + String(bb.y);
    cameraDetectionResults += ", width: " + String(bb.width) + ", height: " + String(bb.height) + "]";
    cameraDetectionResults += "</li>";
        if (bb.value >= 0.8 && bb.x >= 40 && bb.x <= 60 && bb.y >= 40 && bb.y <= 60) {
            digitalWrite(LED, HIGH);
            logic=1;

        } else if (bb.value < 0.8 && (bb.x > 60 || bb.x < 40) && (bb.y > 60 || bb.y < 40)) {
            Serial.println("not in range");
            digitalWrite(LED, LOW);
           
        }
        else {
          logic =0 ;
        }
    }
    // cameraDetectionResults += "</ul>"; // Close the unordered list
    if (!bb_found) {
        ei_printf("    No objects found\n");
    }
#else
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
        ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);
    }
#endif

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

//     cameraDetectionResults += "Predictions: DSP: " + String(result.timing.dsp) + " ms, Classification: " + String(result.timing.classification) + " ms, Anomaly: " + String(result.timing.anomaly) + " ms. ";
// #if EI_CLASSIFIER_OBJECT_DETECTION == 1
//     if (bb_found) {
//         cameraDetectionResults += "Bounding Boxes Found:\n" + boundingBoxDetails;
//     } else {
//         cameraDetectionResults += "No objects found.";
//     }
// #else
//     cameraDetectionResults += "Classification results here.";
// #endif

    free(snapshot_buf);
}
