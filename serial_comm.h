#include <Arduino.h>

#define START_CHAR 0x01
#define END_OF_DATA 0x03
#define END_OF_MSG 0x04

// largest data size allowed
#define MAX_DATA_LEN 512

// length of time between expected serial reads before error
#define TIMEOUT_VALUE 3000

enum LOG_LEVEL {
    SILENT,
    ERROR,
    STATUS,
    DEBUG
};

struct comm_state {
  bool connection;
  
  // analog inputs
  double a0;
  double a1;
  double a2;
  double a3;
  double a4;
  double a5;
  
  // digital outputs
  bool d0;
  bool d1;
  bool d2;
  bool d3;
  bool d4;
  bool d5;
  bool d6;
  bool d7;
  bool d8;
  bool d9;
  bool d10;
  bool d11;
  bool d12;
  bool d13;

  byte override[3];
};


