#include <Bluepad32.h>
#define BOARD_LED 2           // GPIO pin that controls that blue LED on the ESP32 for testing if it actually recives inputs from controller.

//using board "DOIT ESP32 DEVKIT V1 - esp32-bluepad32"



ControllerPtr myControllers[BP32_MAX_GAMEPADS];

//-----------------------------This is to set up the Connection Status of the Controller and Program Details-----------------------------------------------------
void onConnectedController(ControllerPtr ctl) {
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            Serial.printf("CALLBACK: Controller is connected, index=%d\n", i);
            ControllerProperties properties = ctl->getProperties();
            Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id, properties.product_id);
            myControllers[i] = ctl;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot) {
        Serial.println("CALLBACK: Controller connected, but could not found empty slot");
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    bool foundController = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            Serial.printf("CALLBACK: Controller disconnected from index=%d\n", i);
            myControllers[i] = nullptr;
            foundController = true;
            break;
        }
    }
    if (!foundController) {
        Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
    }
}
//---------------------------------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------Display for Controller Button Bitmask Status---------------------------------------------------
void dumpGamepad(ControllerPtr ctl) {
    Serial.printf(
        "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, %4d, brake: %4d, throttle: %4d, misc: 0x%02x, gyro x:%6d y:%6d z:%6d, accel x:%6d y:%6d z:%6d\n",
        ctl->index(),        // Controller Index
        ctl->dpad(),         // D-pad
        ctl->buttons(),      // bitmask of pressed buttons
        ctl->axisX(),        // (-511 - 512) left X Axis
        ctl->axisY(),        // (-511 - 512) left Y axis
        ctl->axisRX(),       // (-511 - 512) right X axis
        ctl->axisRY(),       // (-511 - 512) right Y axis
        ctl->brake(),        // (0 - 1023): brake button
        ctl->throttle(),     // (0 - 1023): throttle (AKA gas) button
        ctl->miscButtons(),  // bitmask of pressed "misc" buttons
        ctl->gyroX(),        // Gyro X
        ctl->gyroY(),        // Gyro Y
        ctl->gyroZ(),        // Gyro Z
        ctl->accelX(),       // Accelerometer X
        ctl->accelY(),       // Accelerometer Y
        ctl->accelZ()        // Accelerometer Z
    );
}
//------------------------------------------------------------------------------------------------------------------------

//----------------------------------------Button Mapping------------------------------------------------------------------
void processGamepad(ControllerPtr ctl) {
    if ( (ctl->axisY() <= 50 && ctl->axisY() >= -50) || (ctl->axisRY() <= 50 && ctl->axisRY() >= -50) || (ctl->throttle() == 0) ) {     //Controls will go back to low instead of being stuck on high when not pressed
 
    }

    //left motor
    if (ctl->axisY() >= -512 && ctl->axisY() <= -400) {         //bitmask number for Right Joystick Y-coord position on xbox controller
        digitalWrite(BOARD_LED, HIGH);                            //accpets inputs from y positions -512 to -400
        digitalWrite(19, LOW);
        digitalWrite(18, HIGH);
    }
    if (ctl->axisY() <= 511 && ctl->axisY() >= 400) {          //bitmask number for Right Joystick Y-coord position on xbox controller
        digitalWrite(BOARD_LED, HIGH);                           //accpets inputs from y positions 512 to 400
        digitalWrite(19, HIGH);
        digitalWrite(18, LOW);
    }

    //right motor
    if (ctl->axisRY() >= -512 && ctl->axisRY() <= -400) {         //bitmask number for Left Joystick Y-coord position on xbox controller
        digitalWrite(BOARD_LED, HIGH);                            //accpets inputs from y positions -512 to -400
        digitalWrite(5, HIGH);
        digitalWrite(17, LOW);
    }
    if (ctl->axisRY() <= 511 && ctl->axisRY() >= 400) {          //bitmask number for Left Joystick Y-coord position on xbox controller
        digitalWrite(BOARD_LED, HIGH);                           //accpets inputs from y positions 512 to 400
        digitalWrite(5, LOW);
        digitalWrite(17, HIGH);
    }

    dumpGamepad(ctl);
}

// Function to call the funtions where the buttons are mapped
void processControllers() {
    for (auto myController : myControllers) {
        if (myController && myController->isConnected() && myController->hasData()) {
            if (myController->isGamepad()) {
                processGamepad(myController);
            }
        }
    }
}

// Arduino setup function. Runs in CPU 1
void setup() {
    Serial.begin(115200);
    
    // Display Firmware
    Serial.printf("Firmware: %s\n", BP32.firmwareVersion());
    const uint8_t* addr = BP32.localBdAddress();
    Serial.printf("BD Addr: %2X:%2X:%2X:%2X:%2X:%2X\n", addr[0], addr[1], addr[2], addr[3], addr[4], addr[5]);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedController, &onDisconnectedController);

    // for board LED
    pinMode(BOARD_LED, OUTPUT);

    // right motor
    pinMode(18, OUTPUT);
    pinMode(19, OUTPUT);

    // left motor
    pinMode(5, OUTPUT);
    pinMode(17, OUTPUT);

}

// Arduino loop function. Runs in CPU 1.
void loop() {

//-------------------------------------------For Controller--------------------------------------------
    // This call fetches all the controllers' data.
    bool dataUpdated = BP32.update();
    if (dataUpdated) {
        processControllers();
    }
    delay(100);

}

