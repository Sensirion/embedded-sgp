/*
   Copyright (c) 2020, Sensirion AG
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

void setup() {
  int16_t err;
  Serial.begin(115200);  // start serial for output

  // wait for serial connection from PC
  // comment the following line if you'd like the output
  // without waiting for the interface being ready
  while(!Serial);

  /* Initialize I2C bus, SHT, SGP and VOC Engine */
  while ((err = sensirion_init_sensors())) {
    Serial.print("initialization failed: ");
    Serial.println(err);
    sensirion_sleep_usec(1000000); /* wait one second */
  }
  Serial.println("initialization successful");
}


/* Run one measurement per second */
void loop() {
  int16_t err;
  int32_t voc_index;
  int32_t temperature_celsius;
  int32_t relative_humidity_percent;
  err = sensirion_measure_voc_index_with_rh_t(
          &voc_index, &relative_humidity_percent, &temperature_celsius );
  if (err == STATUS_OK) {
    Serial.print("VOCindex:");
    Serial.print(voc_index);
    Serial.print("\t");
    Serial.print("Humidity[%RH]:");
    Serial.print(relative_humidity_percent * 0.001f);
    Serial.print("\t");
    Serial.print("Temperature[degC]:");
    Serial.println(temperature_celsius * 0.001f);
  } else {
    Serial.print("error reading signal: ");
    Serial.println(err);
  }

  sensirion_sleep_usec(1000000); /* wait one second */
}
