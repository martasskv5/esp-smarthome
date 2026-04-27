#include "utils.h"

bool payloadIsOn(const byte *payload, unsigned int length) {
  if (length == 0) {
    return false;
  }

  return payload[0] == '1' || payload[0] == 'o' || payload[0] == 'O' || payload[0] == 't' || payload[0] == 'T';
}
