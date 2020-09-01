/*
   Copyright (c) 2019, Sensirion AG
   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.

 * * Neither the name of Sensirion AG nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

#include "sgp40_voc_index.h"

#include <stdio.h>  // printf

/* TO USE CONSOLE OUTPUT (printf) YOU MAY NEED TO ADAPT THE
   INCLUDES ABOVE OR DEFINE THEM ACCORDING TO YOUR PLATFORM.
   #define printf(...)
*/

int32_t voc_index;
int32_t temperature_celsius;
int32_t relative_humidity_percent;
int16_t err;


void setup() {
  /* Initialize I2C bus, SHT, SGP and VOC Engine */
  while ((err = sensirion_init_sensors())) {
    Serial.print("initialization failed: ");
    Serial.println(err);
    sensirion_sleep_usec(1000000); /* wait one second */
  }
  Serial.println("initialization successful");
}


/* Run one measurement per second, persist state every hour */
void loop() {
  err = sensirion_measure_voc_index_with_rh_t(
          &voc_index, &relative_humidity_percent, &temperature_celsius );
  if (err == STATUS_OK) {
    Serial.print("VOC Index: ");
    Serial.println(voc_index);
    Serial.print("Temperature: ");
    Serial.print(temperature_celsius * 0.001f);
    Serial.println("degC");
    Serial.print("Relative Humidity: ");
    Serial.print(relative_humidity_percent * 0.001f);
    Serial.println("RH");
  } else {
    Serial.print("error reading signal: ");
    Serial.println(err);
  }

  sensirion_sleep_usec(1000000); /* wait one second */
}
