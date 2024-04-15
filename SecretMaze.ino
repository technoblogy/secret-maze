/* Secret Maze 2 - see http://www.technoblogy.com/show?4NRC

   David Johnson-Davies - www.technoblogy.com - 15th April 2024
   ATtiny85 @ 1 MHz (internal oscillator; BOD disabled)
   
   CC BY 4.0
   Licensed under a Creative Commons Attribution 4.0 International license: 
   http://creativecommons.org/licenses/by/4.0/
*/

#include <avr/sleep.h>
#include <avr/wdt.h>

volatile int Lights, Buttons = 0;
volatile int Bit = 0;                                 // For multiplexing
volatile unsigned long Millis;
const int Timeout = 30;                               // Sleep after this many secs

// Maze packing **********************************************

union Rows {
  struct {
    uint8_t rowf:2, rowe:2, rowd:2, rowc:2, rowb:2, rowa:2, row9:2, row8:2,
            row7:2, row6:2, row5:2, row4:2, row3:2, row2:2, row1:2, row0:2;
  };
  uint32_t Row;
};

// The Mazes **********************************************

uint8_t Maze = 0;
const int TotalMazes = 6;                               // Increase if you add your own mazes

const union Rows Mazes[TotalMazes][16] PROGMEM = {

// Maze in the original Secret Maze
{
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1,
1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1,
1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1,
1, 0, 1, 0, 1, 0, 0, 2, 1, 0, 1, 0, 1, 3, 0, 1,
1, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1,
1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1,
1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1,
1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1,
1, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1,
1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 0, 1, 0, 1,
1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1,
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
},

// PacMan Maze
{
1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 2, 
1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 
1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 
1, 0, 1, 1, 3, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 
1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 
1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 
1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 
1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 
1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 
1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 
1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
},

// Clown Maze
{
0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 
0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 
0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 
0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 
1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 
1, 0, 1, 0, 0, 1, 2, 0, 1, 0, 3, 1, 0, 0, 1, 0, 
1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 
1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0, 
1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 
1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 
1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 0, 
1, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 
1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 0, 
0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 
0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 
0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0
},

// Catherine Maze
{
1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 2, 
1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 
1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 1, 
1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 1, 
1, 0, 0, 1, 0, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 
1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 
1, 0, 0, 0, 0, 0, 1, 3, 1, 1, 1, 1, 1, 1, 0, 1, 
1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 
1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1, 1, 
1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 
1, 0, 1, 1, 0, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 
1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 0, 1, 1, 
1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 
1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
},

// Clover maze
{
0, 0, 0, 1, 0, 0, 2, 1, 3, 0, 0, 1, 0, 0, 0, 1, 
0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 
0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 
1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 
0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 
0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 
0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 
1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 
0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 
0, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 
0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 
1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 
0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 
0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 
0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
},

// Minotaur maze
{
1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 
3, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 
0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 
0, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 
0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 
0, 1, 0, 1, 1, 1, 1, 0, 2, 0, 1, 1, 1, 1, 0, 1, 
0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 
0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 
0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 
0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 
0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 1, 
0, 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1, 
0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 
0, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 1, 1, 1, 0, 
0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0
}
};

// Patterns for choosing a maze

uint8_t Patterns[10]= {0b0101, 0b0011, 0b1010, 0b1100, 0b0110, 0b1001, 0b0111, 0b1110, 0b1101, 0b1011 };
    
// Get one cell in the maze: 0,0 is top right
uint8_t Cell (uint8_t x, uint8_t y) {
  uint32_t row = pgm_read_dword(&Mazes[Maze][y].Row);
  return row>>((15-x)*2) & 3;
}

// Is the cell a wall? Anything outside the 16 x 16 grid is a wall.
bool Wall (int x, int y) {
  if ((x == -1) || (y == -1) || (x == 16) || (y == 16)) return true;
  return (Cell(x, y) == 1);
}

// Returns the walls around x,y
uint8_t Look (int x, int y) {
  return Wall(x, y+1)<<3 | Wall(x+1, y)<<2 | Wall(x-1, y)<<1 | Wall(x, y-1);
}

// Tiny Tone **********************************************

const int Output = 4; 
const int Clock = 0;                                  // 1 MHz
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
  note(0, 5);
  Mydelay(100);
  note(0, 0);
}

void Click () {
  note(0, 6);
  Mydelay(20);
  note(0, 0);
}

// Multiplexing **********************************************

// Interrupt multiplexes display and buttons
ISR(TIMER0_COMPA_vect) {
  // Check button
  int button = ((Bit<<1) + 1) % 5;
  if (PINB & 1<<button) {
    Buttons = Buttons & ~(1<<Bit);
  } else {
    Buttons = Buttons | 1<<Bit;
  }
  Bit = (Bit + 1) & 0x03;                             // Next row
  // Light LED
  button = ((Bit<<1) + 1) % 5;                        // Button
  int light = Bit ^ 0x03;                             // LED
  if (Lights & 1<<Bit) {
    DDRB = (DDRB & (1<<Output)) | 1<<Bit | 1<<light;
    PORTB = 1<<light | 1<<button;
  } else {
    DDRB = (DDRB & (1<<Output)) | 1<<Bit;
    PORTB = 1<<button;
  }
  Millis = Millis + 2;
}

// INT0 interrupt wakes from sleep
ISR(INT0_vect) {
}

// Keys **********************************************

uint8_t ReadKeys () {
  unsigned long Start = Mymillis();
  do {                                                // Go to sleep after Timeout
    if (Mymillis() - Start > Timeout*1000) {
      Sleep();
      Start = Mymillis();
    }
  } while (Buttons == 0);                             // Wait for keypress
  // Wait until keys are stable
  int keys, count = 0, lastkeys = 0;
  do {
    keys = Buttons;
    count++;
    if (lastkeys != keys) count = 0;
    lastkeys = keys;
  } while (count < 7);
  return keys;
}

// Sleep **********************************************

void Sleep () {
  TIMSK = 0;                                          // Disable interrupt
  DDRB = 0b1000;                     
  PORTB = 0b0100;                                     // Set up for interrupt
  GIMSK = 1<<INT0;                                    // Enable interrupt on INT0 (PB2) 
  sleep_enable();
  sleep_cpu();
  GIMSK = 0;                                          // Turn off INT0 interrupt
  TIMSK = 1<<OCIE0A;                                  // Enable Timer/Counter0 interrupt
  Buttons = 0b1000;                                   // We pressed button on PB3
  Mydelay(500);
  while (Buttons != 0);                               // Ignore this keypress
}

// Setup **********************************************

void setup() {
  MCUSR = 0;
  wdt_disable();                                      // Disable watchdog timer
  // Set up Timer/Counter0 to generate 500Hz interrupt
  TCCR0A = 2<<WGM00;                                  // CTC mode
  TCCR0B = 2<<CS00;                                   // /8 prescaler
  OCR0A = 249;                                        // 500 Hz interrupt
  TIMSK = 1<<OCIE0A;                                  // Enable interrupt
  // Disable what we don't need to save power
  ADCSRA &= ~(1<<ADEN);                               // Disable ADC
  PRR = 1<<PRUSI | 1<<PRADC;                          // Turn off clocks to unused peripherals
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  GIFR = 1<<INTF0;                                    // Clear the INT0 flag, in case it was set
}

// Play game **********************************************

void loop () {
  Mydelay(1000);
  Buttons = 0;
  // Select maze
  int maze = -1;
  unsigned long Start = Mymillis();
  do {
    if (Mymillis() - Start > Timeout*1000) {
      Sleep();
      Start = Mymillis();
    }
    maze = (maze + 1) % TotalMazes;
    Lights = Patterns[maze];                          // Show next pattern
    unsigned long Delay = Mymillis();                 // Delay 1 s
    while ((Mymillis() - Delay < 1000) && (Buttons == 0));
  } while (Buttons == 0);
  // Wait for release key
  Click();
  while (Buttons != 0);
  Maze = maze;
  Lights = 0b1111;
  Mydelay(1000);
  
  // New maze
  int x, y;                                           // Player's position
  for (int xx=0; xx<16; xx++) {
    for (int yy=0; yy<16; yy++) {
      if (Cell(xx, yy) == 2) {
        x = xx; y = yy;                               // Set player to start
      }
    }
  }
  Lights = Look(x, y);                                // Display starting position
  
  do {
    int keys = ReadKeys();
    int dx = 0, dy = 0;                               // Movement
    if (keys & 0b1000) dy = 1; else if (keys & 0b100) dx = 1;
    else if (keys & 0b10) dx = -1; else if (keys & 0b1) dy = -1;
    // Can we go this way?
    if (Wall(x+dx, y+dy)) Beep();                     // Hit a wall
    else {
      x = x + dx; y = y + dy;                         // Move
      Click();
    }
    Lights = Look(x, y);                              // Display new position
    do {
      if (Buttons == 0b1111) { 
        while (Buttons != 0); 
        wdt_enable(4);                                // Will cause a reset
        for(;;); 
      }
    } while (Buttons != 0);                           // Wait for key release
  } while (Cell(x, y) != 3);                          // Have we reached goal?
  
  Lights = 0;
  for (int n=0; n<=7; n++) {                          // Success tune!
    Lights = Lights ^ 0b1111;                         // Flash lights
    note(n, 4);
    if (n!=4 && n!=11) n++;
    Mydelay(400);
  }
  note(0, 0);                                         // Stop notes
  Mydelay(2000);                                      // Pause
}
