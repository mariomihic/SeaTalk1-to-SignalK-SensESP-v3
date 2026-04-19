/*
 * SeaTalk 1 to Signal K Bridge
 * Created by: [mariomihic]
 * Date: April 2024
 * Hardware: ESP32 + PC817 Optocoupler
 * Framework: SensESP v3.3.0
 */


#include <Arduino.h>
#include "sensesp_app_builder.h"
#include "sensesp/signalk/signalk_output.h"

using namespace sensesp;

#define SEATALK_RX_PIN 4

// --- SIGNAL K IZLAZI ---
SKOutputFloat* sk_heading;
SKOutputFloat* sk_rudder;
SKOutputString* sk_ap_mode;
SKOutputFloat* sk_depth;
SKOutputFloat* sk_stw;
SKOutputFloat* sk_temp;
SKOutputFloat* sk_awa;
SKOutputFloat* sk_aws;
SKOutputFloat* sk_log_trip;
SKOutputFloat* sk_log_total;
SKOutputFloat* sk_uptime;

void parse_seatalk_logic(uint8_t* m, uint8_t len) {
    switch (m[0]) {
        case 0x00: // DUBINA
            if (len >= 5) sk_depth->set(((m[3] << 8) | m[4]) / 10.0 / 3.28084);
            break;

        case 0x10: // WIND ANGLE
            if (len >= 4) {
                float angle = ((m[2] << 8) | m[3]) / 2.0;
                sk_awa->set(angle * (M_PI / 180.0));
            }
            break;

        case 0x11: // WIND SPEED
            if (len >= 4) {
                float s = (m[2] & 0x7F) + (m[3] & 0x0F) / 10.0;
                sk_aws->set((m[2] & 0x80) ? s : s * 0.514444);
            }
            break;

        case 0x20: // SPEED THROUGH WATER (Standard)
            if (len >= 4) sk_stw->set(((m[2] << 8) | m[3]) / 10.0 * 0.514444);
            break;

        case 0x26: // SPEED THROUGH WATER (Alternative Tridata)
            if (len >= 7) sk_stw->set(((m[2] << 8) | m[1]) / 100.0 * 0.514444);
            break;

        case 0x23: // WATER TEMP
        case 0x27:
            if (len >= 4) {
                float t = (m[0] == 0x23) ? (float)m[2] : (((m[3] << 8) | m[2]) - 100) / 10.0;
                sk_temp->set(t + 273.15);
            }
            break;

        case 0x21: // TRIP LOG
        case 0x22: // TOTAL LOG
            if (len >= 5) {
                float miles = ((m[2] << 8) | m[3]) / 10.0;
                if (m[0] == 0x21) sk_log_trip->set(miles * 1852.0);
                else sk_log_total->set(miles * 1852.0);
            }
            break;

        case 0x84: // HEADING, RUDDER, MODE
            if (len >= 9) {
                float h = ((m[1] >> 4) & 0x03) * 90.0 + (m[2] & 0x3F) * 2.0;
                if ((m[1] >> 4) & 0x0C) h += ((m[1] >> 4) & 0x0C == 0x0C) ? 2.0 : 1.0;
                if (h >= 360) h -= 360;
                sk_heading->set(h * (M_PI / 180.0));
                sk_rudder->set((int8_t)m[6] * (M_PI / 180.0));
                uint8_t Z = m[4] & 0x0F;
                sk_ap_mode->set((Z & 0x02) ? "auto" : "standby");
            }
            break;
    }
}

void setup() {
    SetupLogging();
    Serial2.begin(4800, SERIAL_8N1, SEATALK_RX_PIN, -1, false);

    SensESPAppBuilder builder;
    auto sensesp_app = builder.set_hostname("seatalk-bridge")
                              ->set_sk_server("192.168.1.50", 3000)
                              ->get_app();

    sk_heading = new SKOutputFloat("navigation.headingMagnetic", "rad");
    sk_rudder  = new SKOutputFloat("steering.rudderAngle", "rad");
    sk_ap_mode = new SKOutputString("steering.autopilot.state");
    sk_depth   = new SKOutputFloat("navigation.depth.belowTransducer", "m");
    sk_stw     = new SKOutputFloat("navigation.speedThroughWater", "m/s");
    sk_temp    = new SKOutputFloat("environment.water.temperature", "K");
    sk_awa     = new SKOutputFloat("navigation.wind.angleApparent", "rad");
    sk_aws     = new SKOutputFloat("navigation.wind.speedApparent", "m/s");
    sk_log_trip = new SKOutputFloat("navigation.logTrip", "m");
    sk_log_total = new SKOutputFloat("navigation.log", "m");
    sk_uptime  = new SKOutputFloat("sensors.bridge.uptime", "s");

    event_loop()->onRepeat(10, []() {
        static uint8_t buf[20];
        static uint8_t idx = 0;
        static unsigned long last_byte_time = 0;

        while (Serial2.available()) {
            uint8_t b = Serial2.read();
            unsigned long now = millis();
            if (now - last_byte_time > 15) idx = 0;
            last_byte_time = now;

            if (idx < 20) buf[idx++] = b;

            if (idx >= 3) {
                uint8_t expected = (buf[1] & 0x0F) + 3;
                if (idx == expected) {
                    parse_seatalk_logic(buf, idx);
                    idx = 0;
                }
            }
        }
    });

    event_loop()->onRepeat(1000, []() {
        sk_uptime->set((float)millis() / 1000.0);
    });

    sensesp_app->start();
}

void loop() { event_loop()->tick(); }
