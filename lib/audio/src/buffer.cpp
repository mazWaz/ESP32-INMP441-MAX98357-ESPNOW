#include "buffer.h"

Buffer::Buffer(int numberSample): numberSamplesToBuffer(numberSample){
    semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphore);

    readHead = 0;
    writeHead = 0;
    availableSamples = 0;
    buffering = true;
    bufferSize = 3 * numberSample;
    buffer = (uint8_t *)malloc(bufferSize);

    memset(buffer, 0, bufferSize);
    if (!buffer) {
        Serial.println("Failed to allocate buffer");
    }
}

void Buffer::addBuffer(const uint8_t *samples, int count){
    xSemaphoreTake(semaphore, portMAX_DELAY);
    for (int i = 0; i < count; i++)
    {
        buffer[writeHead] = samples[i];
        writeHead = (writeHead + 1) % bufferSize;
    }
    availableSamples += count;
    xSemaphoreGive(semaphore);
}

void Buffer::removeBuffer(int16_t *samples, int count, int rank){
    xSemaphoreTake(semaphore, portMAX_DELAY);
    for (int i = 0; i < count; i++)
    {
        samples[i] = 0;
        if (availableSamples == 0 && !buffering){
            buffering = true;
            samples[i] = 0;
        }

        if (buffering && availableSamples < numberSamplesToBuffer){
            samples[i] = 0;
        } else{
            buffering = false;
            int16_t sample = buffer[readHead];
            samples[i] = (sample - 128) << rank;
            readHead = (readHead + 1) % bufferSize;
            availableSamples--;
        }
    }
    xSemaphoreGive(semaphore);
}

  void Buffer::flush(){
    xSemaphoreTake(semaphore, portMAX_DELAY);
    readHead = 0;
    writeHead = 0;
    availableSamples = 0;
    xSemaphoreGive(semaphore);
  }

