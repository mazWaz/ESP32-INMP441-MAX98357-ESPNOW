#include "sirine.h"
#include "soc/i2s_reg.h"
#include <esp_log.h>

sirine::sirine(i2s_port_t _i2sPort, i2s_pin_config_t &_i2sSpeakerPins,i2s_config_t &_i2s_config){
  i2sPort =  _i2sPort;
  i2sSpeakerPins = _i2sSpeakerPins;
  i2s_config = _i2s_config;
  volume = 100;
  mode = 0;      
  envelopeVolume = 100;  
  phaseAccumulator = 0;
  modeStartTime = 0;
}

void sirine::startSirine(){
  i2s_driver_install(i2sPort, &i2s_config, 0, NULL);
  i2s_set_pin(i2sPort, &i2sSpeakerPins);
  // i2s_zero_dma_buffer(i2sPort);

  i2s_start(i2sPort);
}

void sirine::generateWaveTable(){
    for (int i = 0; i < WAVE_TABLE_SIZE; i++) {
      float t = i / (float)(WAVE_TABLE_SIZE - 1);
      float combineWave;
      float toneWave;
      float airhornWave;

      float wave = (t < 0.5f) ? (2.0f * powf(2.0f * t, N_VALUE) - 1.0f) : (1.0f - 2.0f * powf(2.0f * (t - 0.5f), M_VALUE));
      waveTable[i] = (int16_t)(wave * 32767.0f);

      if (t < 0.3f) {
        combineWave = 1 / 0.15 * (t - 0.15f);
      } else if (t >= 0.3 && t < 0.65) {
        combineWave = 2.0f * powf(2.0f * (t - 0.3f), 1) - 1.0f;
      } else {
        combineWave = 1.01f - 2.0f * powf(2.0f * (t - 1.0f), 1);
      }

      combineTable[i] = (int16_t)(combineWave * 32767.0f);

      if (t < 0.3f) {
        airhornWave = 1 / 0.15 * (t - 0.15f);
      } else if (t >= 0.3f && t < 0.45f) {
        airhornWave = -1 + 10 * (t - 0.3f);
      } else if (t >= 0.45f && t < 0.6f) {
        airhornWave = -0.5f - 6.67f * (t - 0.6f);
      } else if (t >= 0.6f && t < 0.75f) {
        airhornWave = -0.5f + 8.34f * (t - 0.6f);
      } else if (t >= 0.75f && t < 0.9f) {
        airhornWave = 0 - 5 * (t - 0.9f);
      } else if (t >= 0.9f && t < 0.95f) {
        airhornWave = 5 * (t - 0.9f);
      } else {
        airhornWave = -1 - 25 * (t - 1);
      }

      airhornTable[i] = (int16_t)(airhornWave * 32767.0f);

      if (t < 0.1f) {
        toneWave = -0.2f + 12.0f * t;
      } else if (t < 0.2f) {
        toneWave = 1.0f - 10.0f * (t - 0.1f);
      } else if (t < 0.3f) {
        toneWave = 0.0f - 5.0f * (t - 0.2f);
      } else if (t < 0.4f) {
        toneWave = -0.5f + 5.0f * (t - 0.3f);
      } else if (t < 0.5f) {
        toneWave = 0.0f - 10.0f * (t - 0.4f);
      } else if (t < 0.6f) {
        toneWave = -1.0f + 8.0f * (t - 0.5f);
      } else if (t < 0.9f) {
        toneWave = -0.2f - 0.8f * (t - 0.6f) / 0.3f;
      } else {
        toneWave = -1.0f + 10.0f * (t - 0.9f);
      }

      toneTable[i] = (int16_t)(toneWave * 32767.0f);
    }
}

void sirine::generateI2sTone(uint8_t _mode){
  if (_mode != mode) {
      modeStartTime = millis();
      phaseAccumulator = 0;
  }
  unsigned long elapsedTime = millis() - modeStartTime;

  float frequency;
  mode = _mode;

  switch (mode) {
    case 1:  //Wail
      {
        float t = (elapsedTime % 5000) / 5000.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
        frequency = 790 + 766 * pola;
      }
      break;

    case 2:  // Yelp
      {
        float t = (elapsedTime % 400) / 400.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
        frequency = 790 + 766 * pola;
      }
      break;

    case 3:  // Piercer
      {
        float t = (elapsedTime % 60) / 60.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
        frequency = 790 + 766 * pola;
      }
      break;

    case 4:  //HiLo
      {
        float t = (elapsedTime % 1900) / 1900.0f;
        frequency = 614.5f + 53.5f * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 5:  // B-HiLo
      {
        float t = (elapsedTime % 900) / 900.0f;
        frequency = 973 + 163 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 6:  //DIN-HiLo
      {
        float t = (elapsedTime % 1500) / 1500.0f;
        frequency = 518.5f + 73.5f * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 7:  // AirHorn
      frequency = 430;
      break;

    case 8:  // Airhorn clasic
      frequency = 215;
      break;

    case 9:  //HiLo 446p
      {
        float t = (elapsedTime % 1500) / 1500.0f;
        frequency = 555 + 75 * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 10:  //HiLo 500 - 660
      {
        float t = (elapsedTime % 2000) / 2000.0f;
        frequency = 589 + 82 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 11:  //HiLo 510 - 1150
      {
        float t = (elapsedTime % 700) / 700.0f;
        frequency = 830 + 320 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 12:  //HiLo 610 - 760
      {
        float t = (elapsedTime % 1100) / 1100.0f;
        frequency = 685 + 75 * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 13:  //HiLo GEN
      {
        float t = (elapsedTime % 1000) / 1000.0f;
        frequency = 600 + 155 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 14:  //HiLo POL
      {
        float t = (elapsedTime % 1000) / 1000.0f;
        frequency = 518.5f + 73.5f * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 15:  //HiLo SPO
      {
        float t = (elapsedTime % 2000) / 2000.0f;
        frequency = 471 + 26 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 16:  //HiLo UMH
      {
        float t = (elapsedTime % 1000) / 1000.0f;
        frequency = 558 + 113 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 17:  // Warning chirp
      {
        unsigned long cycleTime = elapsedTime % 800;
        float t = elapsedTime % 200 / 200.0f;
        float pola = 2.0f * powf(2.0f * t, 2.0f);
        if (cycleTime <= 66) {
          frequency = 939 + 1079 * pola;
        } else {
          frequency = 0;
        }
      }
      break;

    case 18:  // Warning chirp
      {
        unsigned long cycleTime = elapsedTime % 800;
        float t = cycleTime / 800.0f;
        if (cycleTime <= 66) {
          frequency = 1079 + 139 * sin(t * 24 * PI);
        } else {
          frequency = 0;
        }
      }
      break;

    case 19:  // K9 Alarm
      {
        unsigned long cycleTime = elapsedTime % 700;
        float t = cycleTime / 700.0f;
        if (cycleTime <= 175) {
          frequency = 1079 + 139 * sin(t * 24 * PI);
        } else {
          frequency = 0;
        }
      }
      break;

    case 20:  // Wail - C
      {
        unsigned long cycleTime = elapsedTime % 5000;
        if (cycleTime < 700) {
          float t = cycleTime / 700.0f;
          frequency = 907 + 687 * sin(M_PI * t / 2);
        } else if (cycleTime >= 700 && cycleTime < 4400) {
          frequency = 1594;
        } else {
          float t = (cycleTime - 4400) % 600 / 600.0f;
          frequency = 907 + 687 * (1 - sin(M_PI * t / 2));
        }
      }
      break;

    case 21:  // Yelp - C
      {
        unsigned long cycleTime = elapsedTime % 300;
        float t = cycleTime / 300.0f;
        if (t < 0.88) {
          float x = 2.0f * powf(2.0f * t, 0.2f);
          frequency = 907 + 687 * x;
        } else {
          float x = 2.0f - 2.0f * powf(2.0f * (t - 0.88f), 1.0f);
          frequency = 907 + 687 * x;
        }
      }
      break;

    case 22:  // Comp Wail
      {
        float t = (elapsedTime % 5000) / 5000.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
        frequency = 540 + 448 * pola;
      }
      break;

    case 23:  // Comp Yelp
      {
        float t = (elapsedTime % 400) / 400.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
        frequency = 540 + 448 * pola;
      }
      break;

    case 24:  // Comp Piercer
      {
        float t = (elapsedTime % 60) / 60.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
        frequency = 540 + 448 * pola;
      }
      break;

    case 25:  // Alternate Wail
      {
        float t = (elapsedTime % 6000) / 6000.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
        frequency = 524 + 1142 * pola;
      }
      break;

    case 26:  // Alternate Yelp
      {
        float t = (elapsedTime % 300) / 300.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.5f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
        frequency = 524 + 1142 * pola;
      }
      break;

    case 27:  // B-Wail
      {
        float t = (elapsedTime % 5000) / 5000.0f;
        frequency = (t == 0) ? 790 : 790 + 766 * sin(t * PI);
      }
      break;

    case 28:  //B-Yelp
      {
        float t = (elapsedTime % 600) / 600.0f;
        frequency = 1100 + 500 * sin(t * 4 * PI);
      }
      break;

    case 29:  // Dozer
      {
        float t = (elapsedTime % 78) / 78.0f;
        frequency = 983.0f + 383.0f * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 30:  //Ultimate HiLo
      {
        float t = (elapsedTime % 1000) / 1000.0f;
        if (t <= 0.5) {
          frequency = 600.0f * (sin(22 * M_PI * t) > 0 ? 1 : 0);
        } else {
          frequency = 1356.0f * (sin(22 * M_PI * t) < 0 ? 1 : 0);
        }
      }
      break;

    case 31:  //Warble
      {
        float t = (elapsedTime % 500) / 500.0f;
        frequency = 1075.0f * powf(2.0f * t, 0.4f);
      }
      break;

    case 32:  //Woop
      {
        float t = (elapsedTime % 250) / 250.0f;
        frequency = 1075.0f * powf(2.0f * t, 0.4f);
      }
      break;

    case 33:  //Wail 378p3
      {
        float t = (elapsedTime % 3500) / 3500.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
        frequency = 810 + 776 * pola;
      }
      break;

    case 34:  //Wail 850 - 1700 4s
      {
        float t = (elapsedTime % 4000) / 4000.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
        frequency = 850 + 850 * pola;
      }
      break;

    case 35:  //Wail 850 - 1700 5.25s
      {
        float t = (elapsedTime % 5250) / 5250.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
        frequency = 850 + 850 * pola;
      }
      break;

    case 36:  // Yelp 225
      {
        float t = (elapsedTime % 225) / 225.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
        frequency = 915 + 645 * pola;
      }
      break;

    case 37:  // Yelp 246
      {
        float t = (elapsedTime % 246) / 246.0f;
        float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
        frequency = 915 + 645 * pola;
      }
      break;

    case 38:  // Wail Yelp
      {
        unsigned long cycleTime = elapsedTime % 6600;
        if (cycleTime < 5000) {
          float t = cycleTime / 5000.0f;
          float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
          frequency = 790 + 766 * pola;
        } else {
          float t = (cycleTime - 5000) % 400 / 400.0f;
          float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
          frequency = 790 + 766 * pola;
        }
      }
      break;

    case 39:  // Wail Yelp Piercer
      {
        unsigned long cycleTime = elapsedTime % 7500;
        if (cycleTime < 5000) {
          float t = cycleTime / 5000.0f;
          float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 1));
          frequency = 790 + 766 * pola;
        } else if (cycleTime >= 5000 && cycleTime < 6600) {
          float t = (cycleTime - 5000) % 400 / 400.0f;
          float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
          frequency = 790 + 766 * pola;
        } else {
          float t = ((cycleTime - 6600) % 60) / 60.0f;
          float pola = (t < 0.5f) ? (2.0f * powf(2.0f * t, 0.4f)) : (2.0f - 2.0f * powf(2.0f * (t - 0.5f), 0.4f));
          frequency = 790 + 766 * pola;
        }
      }
      break;

    case 40:  // Mechanical Wail
      {
        float t = (elapsedTime % 14000) / 14000.0f;
        frequency = 450 + 750 * sin(t * PI);
      }
      break;

    case 41:  // Italy Police
      {
        float t = (elapsedTime % 800) / 800.0f;
        frequency = 830 + 320 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 42:  // AUstria Ambulance Tone
      {
        float t = (elapsedTime % 1400) / 1400.0f;
        frequency = 522.5f + 77.5f * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 43:  // Swiss Tone
      {
        float t = (elapsedTime % 1300) / 1300.0f;
        frequency = 522.5f + 77.5f * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 44:  // Germany Land Tone
      {
        float t = (elapsedTime % 1400) / 1400.0f;
        frequency = 442.5f + 57.5f * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 45:  //Martin Tone
      {
        unsigned long cycleTime = elapsedTime % 2000;
        if (cycleTime < 1000) {
          float t = (cycleTime % 83) / 83.0f;
          frequency =   1115 + 555 * (sin(2 * M_PI * t) > 0.4 ? 1 : -1);
        } else {
          float t = (cycleTime % 83) / 83.0f;
          frequency =  800 + 400 * (sin(2 * M_PI * t) > 0.4 ? 1 : -1);
        }
      }
      break;

    case 46:  //Martin HiLo
      {
        unsigned long cycleTime = elapsedTime % 1000;
        if (cycleTime < 500) {
          float t = (cycleTime % 83) / 83.0f;
          frequency = 950 + 400 * (sin(2 * M_PI * t) > 0.4 ? 1 : -1);
        } else {
          float t = (cycleTime % 83) / 83.0f;
          frequency = 800 + 400 * (sin(2 * M_PI * t) > 0.4 ? 1 : -1);
        }
      }
      break;

    case 47:  // Airhorn HiLo
      {
        float t = (elapsedTime % 2000) / 2000.0f;
        frequency = 442.5f + 57.5f * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 48:  // Comp HiLo 2800 2000 List
      {
        float t = (elapsedTime % 1900) / 1900.0f;
        frequency = 488.5f + 85.5f * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 49:  // Austria Police Tone
      {
        unsigned long cycleTime = elapsedTime % 2300;
        if (cycleTime < 300) {
          frequency = 444;
        } else if (cycleTime >= 300 && cycleTime < 600) {
          frequency = 572;
        } else if (cycleTime >= 600 && cycleTime < 1200) {
          float t = (cycleTime - 600) / 600.0f;
          frequency = 508 + 64 * (sin(5 * M_PI * t) > 0 ? -1 : 1);
        } else if (cycleTime >= 1200 && cycleTime < 1800) {
          frequency = 572;
        } else {
          frequency = 0;
        }
      }
      break;

    case 50:  // Austria Fire Tone
      {
        unsigned long cycleTime = elapsedTime % 3500;
        if (cycleTime < 500) {
          frequency = 397;
        } else if (cycleTime >= 500 && cycleTime < 1500) {
          frequency = 532;
        } else if (cycleTime >= 1500 && cycleTime < 2000) {
          frequency = 397;
        } else if (cycleTime >= 2000 && cycleTime < 3100) {
          frequency = 532;
        } else {
          frequency = 0;
        }
      }
      break;

    case 51:  // Italy Ambulance
      {
        unsigned long cycleTime = elapsedTime % 1500;
        if (cycleTime < 500) {
          float t = cycleTime % 300 / 300.0f;
          frequency = 533.5f + 136.5f * (sin(2 * M_PI * t) > 0 ? 1 : -1);
        } else {
          frequency = 397;
        }
      }
      break;

    case 52:  // 397 AVF
      {
        unsigned long cycleTime = elapsedTime % 1500;
        if (cycleTime < 500) {
          float t = cycleTime % 300 / 300.0f;
          frequency = 533.5f + 136.5f * (sin(2 * M_PI * t) > 0 ? 1 : -1);
        } else {
          frequency = 397;
        }
      }
      break;

    case 53:  // FRA AF
      {
        unsigned long cycleTime = elapsedTime % 1200;
        if (cycleTime < 190) {
          frequency = 431;
        } else if (cycleTime >= 190 && cycleTime < 360) {
          frequency = 532;
        } else if (cycleTime >= 360 && cycleTime < 500) {
          frequency = 424;
        } else {
          frequency = 0;
        }
      }
      break;

    case 54:  //Triton
      {
        unsigned long cycleTime = elapsedTime % 3500;
        if (cycleTime < 900) {
          frequency = 641;
        } else if (cycleTime >= 900 && cycleTime < 1800) {
          frequency = 812;
        } else if (cycleTime >= 1800 && cycleTime < 2700) {
          frequency = 484;
        } else {
          frequency = 813;
        }
      }
      break;

    case 55:  // Holand Slow
      {
        float t = (elapsedTime % 1000) / 1000.0f;
        frequency = 442.5f + 57.5f * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 56:  // Holand fast
      {
        float t = (elapsedTime % 400) / 400.0f;
        frequency = 442.5f + 57.5f * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 57:  // comp HiLo 1800 1400 List
      {
        float t = (elapsedTime % 1300) / 1300.0f;
        frequency = 350 + 50 * (sin(2 * M_PI * t) > 0 ? 1 : -1);
      }
      break;

    case 58:  // COmposite Triton
      {
        unsigned long cycleTime = elapsedTime % 3500;
        if (cycleTime < 900) {
          frequency = 434;
        } else if (cycleTime >= 900 && cycleTime < 1800) {
          frequency = 538;
        } else if (cycleTime >= 1800 && cycleTime < 2700) {
          frequency = 323;
        } else {
          frequency = 538;
        }
      }
      break;

    case 59:  // France Fire Tone
      {
        float t = (elapsedTime % 2100) / 2100.0f;
        frequency = 470.5 + 26.5 * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 60:  // France SAMU Tone
      {
        float t = (elapsedTime % 1100) / 1100.0f;
        frequency = 557 + 113 * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 61:  // France Police Tone
      {
        float t = (elapsedTime % 1100) / 1100.0f;
        frequency = 518 + 74 * (sin(2 * M_PI * t) > 0 ? -1 : 1);
      }
      break;

    case 62:  // France Ambulance Tone
      {
        unsigned long cycleTime = elapsedTime % 1200;
        if (cycleTime < 190) {
          frequency = 431;
        } else if (cycleTime >= 190 && cycleTime < 360) {
          frequency = 532;
        } else if (cycleTime >= 360 && cycleTime < 500) {
          frequency = 424;
        } else {
          frequency = 0;
        }
      }
      break;

    default:
      frequency = 0;
      break;
  }

  phaseStep = (uint32_t)(frequency * (1UL << 24) * WAVE_TABLE_SIZE / SAMPLE_RATE_WAVE);
}

const uint8_t sirine::modeTableMap(int mode){
    static const uint8_t map[63] = {
      0, 0, 0, 0, 0, 0,
      0, 3, 3, 0, 0,
      0, 0, 0, 0, 0,
      0, 0, 0, 0, 0,
      0, 1, 1, 1, 0,
      0, 0, 0, 0, 0,
      0, 0, 0, 0, 0,
      0, 0, 0, 0, 1,
      2, 2, 2, 2, 1,
      1, 3, 1, 2, 2,
      2, 0, 0, 0, 3,
      3, 1, 1, 2, 2,
      2, 2
    };

    if (mode >= 0 && mode < 63) {
        return map[mode];
    }
    return 0; // Default case
}

void sirine::generateSineWave(int vol) {
  uint32_t index = (phaseAccumulator >> 24) & (WAVE_TABLE_SIZE - 1);

  int16_t rawSample;
  uint8_t tableType = (mode < 63) ? modeTableMap(mode) : 0;

  switch (tableType) {
    case 1:
      rawSample = combineTable[index];
      break;
    case 2:
      rawSample = toneTable[index];
      break;
    case 3:
      rawSample = airhornTable[index];
      break;
    default:
      rawSample = waveTable[index];
      break;
  }

  volume = vol;
  int16_t scaledSample = (int32_t(rawSample) * volume * envelopeVolume) / (100 * 150);

  phaseAccumulator += phaseStep;

  int16_t buffer[2] = { scaledSample, scaledSample };

  size_t bytesWritten;
  i2s_write(i2sPort, buffer, sizeof(buffer), &bytesWritten, portMAX_DELAY);
}

void sirine::stopSirine(){
  i2s_stop(i2sPort);
  i2s_driver_uninstall(i2sPort);
}

void sirine::cleanBuffer(){
  i2s_zero_dma_buffer(i2sPort);
}
