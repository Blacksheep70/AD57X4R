#include <AD57X4R.h>
#include <Streaming.h>


const int DAC_CS = 10;
const long BAUD = 115200;
const unsigned int MILLIVOLT_MAX = 5000;

AD57X4R dac = AD57X4R(DAC_CS);

char input_buffer[128];
uint8_t idx = 0;
boolean input_complete = false;
char *argv[8];
int arg1, arg2, arg3;
unsigned int dac_value_max;

void parse(char *line, char **argv, uint8_t max_args)
{
  uint8_t arg_count = 0;
  while (*line != '\0')
  { /* if not the end of line ....... */
    while (*line == ',' || *line == ' ' || *line == '\t' || *line == '\n')
    {
      *line++ = '\0'; /* replace commas and white spaces with 0 */
    }
    *argv++ = line; /* save the argument position */
    arg_count++;
    if (arg_count == max_args-1)
    {
      break;
    }
    while (*line != '\0' && *line != ',' && *line != ' ' &&
           *line != '\t' && *line != '\n')
    {
      line++; /* skip the argument until ... */
    }
  }
  *argv = '\0'; /* mark the end of argument list */
}

void setup()
{
  // PC communications
  Serial.begin(BAUD);
  Serial.println("* System ready *");

  // Initialize DAC
  dac.init(AD57X4R::AD5754R, AD57X4R::UNIPOLAR_5V);
  dac_value_max = dac.getMaxDacValue();
  Serial << "dac_value_max = " << dac_value_max << endl;
}


void loop()
{
  if (input_complete)
  {
    parse((char*)input_buffer, argv, sizeof(argv));
    if (strcmp(argv[0], "analogWrite") == 0)
    {
      if (0 < strlen(argv[1]))
      {
        unsigned int millivolt_value = atoi(argv[1]);
        unsigned int dac_value = map(millivolt_value,0,MILLIVOLT_MAX,0,dac_value_max);
        dac.analogWrite(AD57X4R::ALL,dac_value);
      }
      else
      {
        Serial << "analogWrite <MILLIVOLT_VALUE>, VALUE = {0.." << MILLIVOLT_MAX << "}" << endl;
      }
    }
    else if (strcmp(argv[0], "readPowerControlRegister") == 0)
    {
      int power_control_register = dac.readPowerControlRegister();
      Serial << "power_control_register = " << _BIN(power_control_register) << endl;
    }
    else
    {
      Serial.println("analogWrite <MILLIVOLT_VALUE>, readPowerControlRegister");
    }

    input_complete = false;
  }
}

void serialEvent()
{
  while (Serial.available())
  {
    uint8_t in_byte;
    in_byte = Serial.read();
    if ((in_byte == '\n') || (in_byte == '\r'))
    {
      Serial.println();
      input_buffer[idx] = 0;
      idx = 0;
      input_complete = true;
    }
    else if (((in_byte == '\b') || (in_byte == 0x7f)) && (idx > 0))
    {
      idx--;
      Serial.write(in_byte);
      Serial.print(" ");
      Serial.write(in_byte);
    }
    else if ((in_byte >= ' ') && (idx < sizeof(input_buffer) - 1))
    {
      input_buffer[idx++] = in_byte;
      Serial.write(in_byte);
    }
  }
}
