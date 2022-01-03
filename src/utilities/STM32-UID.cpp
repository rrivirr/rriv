#include "STM32-UID.h"
#include "Arduino.h"

/*
void foobar()
{
  uint32_t idPart1 = STM32_UUID[0];
  uint32_t idPart2 = STM32_UUID[1];
  uint32_t idPart3 = STM32_UUID[2];
  //do something with the overall 96 bits
}
*/

void getSTM32UUID(unsigned char * uuid)
{
  uint32_t * stmUUID = (uint32_t *) malloc(sizeof(uint32_t) * 3);

  stmUUID[0] = STM32_UUID_F103[0];
  stmUUID[1] = STM32_UUID_F103[1];
  stmUUID[2] = STM32_UUID_F103[2];

  memcpy((void *)uuid, &stmUUID[0], 12);

  free(stmUUID);
}

void decodeUniqueId(unsigned char * uuid, char * uuidString, int uuidLength)
{
  uuidString[2 * uuidLength] = '\0';
  for (short i = 0; i < uuidLength; i++)
  {
    sprintf(&uuidString[2 * i], "%02X", (byte)uuid[i]);
  }
  Serial2.println(uuidString);
}