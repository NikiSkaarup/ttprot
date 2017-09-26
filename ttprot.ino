// ttprot - ttProtocol
// communication
#define PIN_IN 8
#define PIN_DATA 10
#define PIN_OUT 12

// STATE
#define BITS_PER_TRANSFER 8
#define SENDING 1
#define RECEIVING 2
uint8_t state;

// IN
uint8_t inState = 0x0;
uint8_t lastInState = 0x0;
uint8_t inBitFirst = 0x0;
uint8_t inBitCount = 0x0;
uint8_t inBit = 0x0;
uint8_t inData = 0x0;
String inString = "";

// OUT
uint8_t lastOutState = 0x0;
uint8_t outStringCharIndex = 0x0;
uint8_t outBitCount = 0x0;
uint8_t outChar = 0x0;
String outString = "";

void setup()
{
  Serial.begin(57600);
  while (!Serial)
    ; // Wait for serial

  // communication
  pinMode(PIN_IN, INPUT);
  pinMode(PIN_OUT, OUTPUT);
  pinMode(PIN_DATA, OUTPUT);
  initialize(); // initialize serial data transfer setup
}

void initialize()
{
  delay(10000);
  inState = digitalRead(PIN_IN);
  lastInState = inState;
  if (inState)
  {
    changeState(RECEIVING);
    flipOut();
    Serial.println(RECEIVING);
  }
  else
  {
    changeState(SENDING);
    flipOut();
    Serial.println(SENDING);
  }
}

void loop()
{
  //delay(10); // 75 40 20 10
  inState = digitalRead(PIN_IN);
  if (inState != lastInState)
  {
    lastInState = inState;

    if (state == SENDING)
      handleSending();
    else
      handleReceiving();
  }
  input();
}

void input()
{
  if (outString.length() == 0)
  {
    while (Serial.available() > 0)
    {
      char inChar = Serial.read();
      outString += inChar;
    }
  }
}

void handleReceiving()
{
  inBit = digitalRead(PIN_DATA);
  if (inBit == 0x0 && inBitFirst == 0x0)
  {
    if (inString.length() > 0x0)
      Serial.println(inString);

    inString = "";
    changeState(SENDING);
    flipOut();
    return;
  }
  if (inBitFirst == 0x0)
  {
    /* 
      ignore first incomming bit 
      and let transmitter know we can receive data
    */
    inBitFirst++;
    inBitCount = 0x0;
    inData = 0x0;
    flipOut();
    return;
  }

  inData += (inBit << inBitCount);
  inBitCount++;
  if (inBitCount >= BITS_PER_TRANSFER)
  {
    Serial.print((char)inData);
    Serial.print(" || ");
    Serial.println(inData);
    inString += (char)inData;
    inBitFirst = 0x0;
  }
  flipOut();
}

void handleSending()
{
  if (outString.length() > 0x0)
  {
    if (outChar == 0x0 && outStringCharIndex < outString.length())
    {
      // set the new char to transmi
      outChar = outString[outStringCharIndex];
      outStringCharIndex++;
      outBitCount = 0x0;
      // send 1 to ensure receiver will listen
      digitalWrite(PIN_DATA, HIGH);
      flipOut();
      /*
        required as we can't transmit
        before we inform receiver
      */
      return;
    }

    digitalWrite(PIN_DATA, (outChar >> outBitCount) & 0x01);
    outBitCount++;
    if (outBitCount >= BITS_PER_TRANSFER)
    {
      outBitCount = 0x0;
      outChar = 0x0;
    }
  }
  else
  {
    changeState(RECEIVING);
  }

  if (outChar == 0x0 && outStringCharIndex >= outString.length())
  {
    outBitCount = 0x0;
    outStringCharIndex = 0x0;
    outString = "";
  }

  flipOut();
}

void changeState(unsigned int newState)
{
  state = newState;
  // configure for new state and reset previous

  if (SENDING == newState)
  {
    pinMode(PIN_DATA, OUTPUT);
  }
  else
  {
    pinMode(PIN_DATA, INPUT);
  }
}

void flipOut()
{
  // Flip the state of the out pin.
  lastOutState = !lastOutState;
  digitalWrite(PIN_OUT, lastOutState);
}
