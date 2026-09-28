#include "crc16.h"

uint16_t crc16_ccitt(const uint8_t *data, size_t len)
{
  uint16_t crc = 0xFFFF;

  while (len--)
  {
    crc ^= (uint16_t)(*data++) << 8;
    for (int i = 0; i < 8; i++)
    {
      crc = (crc & 0x8000) ? (uint16_t)((crc << 1) ^ 0x1021) : (uint16_t)(crc << 1);
    }
  }

  return crc;
}
