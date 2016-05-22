#include "serial_comm.h";
comm_state this_state;
LOG_LEVEL log_level = STATUS;  // default to reporting status

// TODO remove these - just here for compile check
void setup() {
  Serial.begin(9600);
}

void loop() {
  Serial.print("HELLO HELEN\n");
  delay(1000);
}

/*
 * Main serial parser
 */
void serial_communicator() {
  byte byte_read;
  bool command_complete = false;
  bool data_complete = false;
  int start_timeout;
  int num_chars_read = 0;
  byte command[2];
  byte data[MAX_DATA_LEN];
  
  logger("STARTING COMM PARSE", DEBUG);
  
  // check to see if anything has been sent
  if (Serial.available()) {
    byte_read = Serial.read();
    
    // if we don't have the correct start, something got garbled
    if (START_CHAR != byte_read) {
      logger("Incorrect start character :", ERROR);
      logger(hex_format(byte_read), ERROR);
      logger(".  Expected : ", ERROR);
      logger(hex_format(START_CHAR), ERROR);
      logger(". Ignoring message", ERROR);
      return;
    }
    logger("Start Character Read", DEBUG);
    
    // prep the timeout
    start_timeout = millis();
    
    // start parsing the message
    do {
      if (Serial.available()) {
        // something was sent - reset the timeout
        start_timeout = millis();
        logger("Reading next", DEBUG);
        
        // parse the command
        while(!command_complete) {
          command[num_chars_read++] = Serial.read();
          
          // once we have two command characters, move on
          if (2 == num_chars_read) {
            command_complete = true;
            num_chars_read = 0;
          }
        }
        
        // parse and validate the data
        while(!data_complete) {
          if (num_chars_read++ > MAX_DATA_LEN) {
            Serial.println("Input data exceeded max message size");
            return;
          }
          
          byte_read = Serial.read();
          
          if (END_OF_DATA != byte_read) {
            data[num_chars_read] = byte_read;
          } else { // we've reached the end of the data, time to validate
            // reset to reflect data length
            --num_chars_read;
            
            if (CRC8(data, num_chars_read) != byte_read) {
              Serial.print("Mismatched checksum.  Sent = ");
              Serial.print(byte_read, HEX);
              Serial.print(". Calculated = ");
              Serial.println(CRC8(data, num_chars_read), HEX);
              return;
            }
            data_complete = true;
          }
        }
        
        // check the data is terminated correctly
        if (Serial.read() != END_OF_MSG) {
          Serial.print("Incorrect end character :");
          Serial.print(byte_read, HEX);
          Serial.print(".  Expected : ");
          Serial.print(END_OF_MSG, HEX);
          Serial.println(". Ignoring message");
          return;
        }
        else break;
 
      }
    } while(!check_timeout(start_timeout)); // while we haven't timed out, keep parsing
  }  // end START_CHAR parse
  
  /*
  // call the appropriate command handler
  switch(command[0]) {
    case 0x00 :
      switch(command[1]) {
        case 0x01 : hello_world_handler(data, num_chars_read);
                    break;
        case 0x02 : set_log_level_handler(data, num_chars_read);
                    break;
        case 0x10 : get_time_handler(data, num_chars_read);
                    break;
        case 0x11 : set_time_handler(data, num_chars_read);
                    break;
        case 0x15 : get_status_handler(data, num_chars_read);
                    break;
        case 0x16 : set_status_handler(data, num_chars_read);
                    break;
        case 0x17 : release_status_handler(data, num_chars_read);
                    break;
        default :
          Serial.print("Invalid command code : ");
          Serial.print(command[0], HEX);
          Serial.println(command[1], HEX);
          return;
      }
      break;
    case 0x01 : // fall through - no commands here yet
    default :
      Serial.print("Invalid command code : ");
      Serial.print(command[0], HEX);
      Serial.println(command[1], HEX);
      return;
  }
  */
}

/*
 * handler functions for each command
 * TODO give bodies
 */
bool hello_world_handler(const byte* data, const int data_len) {
  //
}

bool set_log_level_handler(const byte* data, const int data_len) {
  switch(data[0]) {
    case 0x00 : log_level = SILENT;
                Serial.println("LOGGER going SILENT");
                break;
    case 0x01 : log_level = ERROR;
                Serial.println("LOGGER set to ERROR");
                break;
    case 0x02 : log_level = STATUS;
                Serial.println("LOGGER set to STATUS");
                break;
    case 0x03 : log_level = DEBUG;
                Serial.println("LOGGER set to DEBUG");
                break;
    default :
        log_level = STATUS;
        logger("Unknown Log Level : ",ERROR);
        logger(hex_format(data[0]), ERROR);
  }
}

bool get_time_handler(const byte* data, const int data_len) {}
bool set_time_handler(const byte* data, const int data_len) {}
bool get_status_handler(const byte* data, const int data_len) {}
bool set_status_handler(const byte* data, const int data_len) {}
bool release_status_handler(const byte* data, const int data_len) {}

bool check_timeout(int start_time) {
  // if the start time plus the timeout value is still past the current time, no time out yet
  if ((start_time + TIMEOUT_VALUE) > millis()) return false;
  return true;
}

//CRC-8 - based on the CRC8 formulas by Dallas/Maxim
//code released under the therms of the GNU GPL 3.0 license
byte CRC8(const byte *data, byte len) {
  byte crc = 0x00;
  while (len--) {
    byte extract = *data++;
    for (byte tempI = 8; tempI; tempI--) {
      byte sum = (crc ^ extract) & 0x01;
      crc >>= 1;
      if (sum) {
        crc ^= 0x8C;
      }
      extract >>= 1;
    }
  }
  return crc;
}

/*
 * initialize the comm state
 */
void comm_state_reset() {
  this_state.connection = false;
  
  this_state.a0 = this_state.a1 = this_state.a2 = this_state.a3 = this_state.a4 = this_state.a5 = 0;
  this_state.d0 = this_state.d1 = this_state.d2 = this_state.d3 = this_state.d4 = this_state.d5 = false;
  this_state.d6 = this_state.d7 = this_state.d8 = this_state.d9 = this_state.d10 = this_state.d11 = false;
  this_state.d12 = this_state.d13 = false;
  
  this_state.override[0] = this_state.override[1] = this_state.override[2] = 0x00;
}

/*
 * check log level and send if appropriate
 */
void logger(const char* msg, const LOG_LEVEL msg_level) {
  if (log_level >= msg_level) Serial.print(msg);
}

/*
 * format bytes read into character string hex
 */
char* hex_format(const byte num) {
  char hex_string[4];
  
  sprintf(hex_string, "0x%%, %dx", num);
  
  return hex_string;
}




