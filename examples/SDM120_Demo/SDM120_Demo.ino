/**
 *  Modbus master example 2:
 *  The purpose of this example is to query several sets of data
 *  from an external Modbus slave device. 
 *  The link media can be USB or RS232.
 *
 *  Recommended Modbus slave: 
 *  diagslave http://www.modbusdriver.com/diagslave.html
 *
 *  In a Linux box, run 
 *  "./diagslave /dev/ttyUSB0 -b 19200 -d 8 -s 1 -p none -m rtu -a 1"
 * 	This is:
 * 		serial port /dev/ttyUSB0 at 19200 baud 8N1
 *		RTU mode and address @1
 */

#include <ModbusRtu.h>

uint16_t au16data[16]; //!< data array for modbus network sharing
uint8_t u8state; //!< machine state
uint8_t u8query; //!< pointer to message query

/**
 *  Modbus object declaration
 *  u8id : node id = 0 for master, = 1..247 for slave
 *  u8serno : serial port (use 0 for Serial)
 *  u8txenpin : 0 for RS-232 and USB-FTDI
 *               or any pin number > 1 for RS-485
 */
HardwareSerial Serial1(1);
Modbus master(0, MAX485_DIR);
modbus_t telegram[2];

unsigned long u32wait;

float UtoF(uint32_t x);

void setup()
{
    // telegram 0: read registers
    telegram[0].u8id = 1; // slave address
    telegram[0].u8fct = MB_FC_READ_INPUT_REGISTER; // function code (this one is registers read)
    telegram[0].u16RegAdd = 0x0000; // Reg_Volt
    telegram[0].u16CoilsNo = 2; // number of elements (coils or registers) to read
    telegram[0].au16reg = au16data; // pointer to a memory array in the Arduino

    // telegram 1: write a single register
    telegram[1].u8id = 1; // slave address
    telegram[1].u8fct = MB_FC_READ_INPUT_REGISTER; // function code (this one is write a single register)
    telegram[1].u16RegAdd = 0x0046; // Reg_Frequency
    telegram[1].u16CoilsNo = 2; // number of elements (coils or registers) to read
    telegram[1].au16reg = au16data + 2; // pointer to a memory array in the Arduino

    Serial1.begin(9600, SERIAL_8E1, MAX485_RXD, MAX485_TXD);
    master.begin(&Serial1, 9600);
    master.setTimeOut(5000); // if there is no answer in 5000 ms, roll over
    u32wait = millis() + 1000;
    u8state = u8query = 0;

    Serial.begin(9600);
}

void loop()
{
    switch (u8state)
    {
        case 0:
            if (millis() > u32wait) u8state++; // wait state
            break;
        case 1:
            master.query(telegram[u8query]); // send query (only once)
            u8state++;
            u8query++;
            if (u8query > 2) u8query = 0;
            break;
        case 2:
            master.poll(); // check incoming messages
            if (master.getState() == COM_IDLE)
            {
                u8state = 0;
                u32wait = millis() + 1000;
                Serial.print("Voltage: ");
                float fTmp = UtoF((au16data[0] << 16) | au16data[1]);
                Serial.print(fTmp);
                Serial.println(" Volt");

                Serial.print("Freq: ");
                fTmp = UtoF((au16data[2] << 16) | au16data[3]);
                Serial.print(fTmp);
                Serial.println(" Hz");
            }
            break;
    }
}

float UtoF(uint32_t x)
{
    return (*(float*) &x);
}

