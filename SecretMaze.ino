/* Secret Maze PCB v2

   David Johnson-Davies - www.technoblogy.com - 24th March 2018
   ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <avr/sleep.h>

volatile int Lights, Buttons = 0;
volatile int Row = 0;
volatile unsigned long Millis;
const int Timeout = 30;                   // Sleep after this many secs

// The Maze **********************************************

// Maze definition
const int Startx = 8, Starty = 7;
const int Goalx = 2, Goaly = 7;
unsigned int Maze[16] = { 0xFFFF, 0x8811, 0xAEF5, 0x8011, 0xF7C7, 0x9435, 0xA5A5, 0xA8A9,
                          0x8B85, 0xA83D, 0xAFE1, 0xE06F, 0x8B01, 0xAAF5, 0x8811, 0xFFFF };
    
// Get one bit in the maze: 0,0 is top right
int Bit (int x, int y) {
  return Maze[y]>>x & 1;
}

// Returns the walls around x,y
int Look (int x, int y) {
  return Bit(x, y+1)<<3 | Bit(x+1, y)<<2 | Bit(x-1, y)<<1 | Bit(x, y-1);
}

// Tiny Tone **********************************************

const int Output = 4; 
const int Clock = 0;                               // 1 MHz
const uint8_t scale[] PROGMEM = {239,226,213,201,190,179,169,160,151,142,134,127};

void note (int n, int octave) {
  int prescaler = 8 + Clock - (octave + n/12);
  if (prescaler<1 || prescaler>15 || octave==0) prescaler = 0;
  DDRB = (DDRB & ~(1<<Output)) | (prescaler != 0)<<Output;
  OCR1C = pgm_read_byte(&scale[n % 12]) - 1;
  GTCCR = (Output == 4)<<COM1B0;
  TCCR1 = 1<<CTC1 | (Output == 1)<<COM1A0 | prescaler<<CS10;
}

unsigned long Mymillis () {
  cli();
  unsigned long temp = Millis;
  sei();
  return temp;
}
  
void Mydelay (unsigned long ms) {
  unsigned long Start = Mymillis();
  while (Mymillis() - Start < ms);
}

void Beep () {
  note(0, 4);
  Mydelay(100);
  note(0, 0);
}

// Multiplexing **********************************************

// Interrupt multiplexes display and buttons
ISR(TIMER0_COMPA_vect) {
  // Check button
  int button = ((Row<<1) + 1) % 5;
  if (PINB & 1<<button) {
    Buttons = Buttons & ~(1<<Row);
  } else {
    Buttons = Buttons | 1<<Row;
  }
  Row = (Row + 1) & 0x03;                  // Next row
  // Light LED
  button = ((Row<<1) + 1) % 5;             // Button
  int light = Row ^ 0x03;                  // LED
  if (Lights & 1<<Row) {
    DDRB = (DDRB & (1<<Output)) | 1<<Row | 1<<light;
    PORTB = 1<<light | 1<<button;
  } else {
    DDRB = (DDRB & (1<<Output)) | 1<<Row;
    PORTB = 1<<button;
  }
  Millis = Millis + 2;
}

// INT0 interrupt wakes from sleep
ISR(INT0_vect) {
}

// Setup **********************************************
  
// Position of player in the maze
int X = Startx, Y = Starty;

void setup() {
  // Set up Timer/Counter0 to generate 625Hz interrupt
  TCCR0A = 2<<WGM00;                     // CTC mode
  TCCR0B = 2<<CS00;                      // /8 prescaler
  OCR0A = 249;                           // 500 Hz interrupt
  TIMSK = 1<<OCIE0A;                     // Enable interrupt
  // Disable what we don't need to save power
  ADCSRA &= ~(1<<ADEN);                  // Disable ADC
  PRR = 1<<PRUSI | 1<<PRADC;             // Turn off clocks to unused peripherals
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  Lights = Look(X, Y);                   // Display initial position
}

void loop () {
  unsigned long Start = Mymillis();
  do {                                   // Go to sleep after Timeout
    if (Mymillis() - Start > Timeout*1000) {
      TIMSK = 0;                         // Disable interrupt
      DDRB = 0b1000;                     
      PORTB = 0b0100;                    // Set up Down pushbutton
      GIMSK = 1<<INT0;                   // Enable interrupt on INT0 (PB2) 
      sleep_enable();
      sleep_cpu();
      GIMSK = 0;                         // Turn off INT0 interrupt
      TIMSK = 1<<OCIE0A;                 // Enable Timer/Counter0 interrupt
      Buttons = 0b100;                   // We pressed button on PB2
      while (Buttons != 0);              // Ignore this keypress
      Start = Mymillis();
    }
  } while (Buttons == 0);                // Wait for keypress
  // Wait until keys are stable
  int keys, count = 0, lastkeys = 0;
  do {
    keys = Buttons;
    count++;
    if (lastkeys != keys) count = 0;
    lastkeys = keys;
  } while (count < 7);
  // Respond to keypress
  int dx = 0, dy = 0;                    // Movement
  if (keys & 0b1000) dy = 1; else if (keys & 0b100) dx = 1;
  else if (keys & 0b10) dx = -1; else if (keys & 0b1) dy = -1;
  // Can we go this way?
  if (Bit(X+dx, Y+dy)) Beep();           // Hit a wall
  else {
    X = X + dx; Y = Y + dy;              // Move
  }
  Lights = Look(X, Y);                   // Display new position
  while (Buttons != 0);                  // Wait for key release
  // Have we reached goal?
  if ((X == Goalx) && (Y == Goaly)) {
    for (int n=0; n<=7; n++) {           // Success tune!
      note(n, 4);
      if (n!=4 && n!=11) n++;
      Mydelay(200);
    }
    note(0, 0);
    Mydelay(1000);                       // Pause
    X = Startx; Y = Starty;              // Return to start
    Lights = Look(X, Y);
  }
}
