///////////////////////////////////////////////////////////////////////////////
// Members: Paul Solleza
//          Peter Cvijovic
// Section: LBL EA1
//
// goals:
// > generate a valid sudoku grid
// > generate random numbers to interchange rows and columns within a same group
// > interchange 2 rows and columns
// > interchange two groups of row and two groups of columns
// > strike out cells
//
// sudoku three requirements:
// > for each row: every number from 1 to 9 should occur exactly once
// > for each column: every number from 1 to 9 should occur exactly once
// > for each 3x3 square with a thicker border (there are nine of them):
//   every number from 1 to 9 should occur exactly once.
//
///////////////////////////////////////////////////////////////////////////////

#include<Arduino.h> // arduino library

#include<Adafruit_GFX.h>    // Core graphics library
#include<Adafruit_ST7735.h> // Hardware-specific library
#include<SPI.h>

#define TFT_RST 8 // Reset line for TFT (or connect to +5V)
#define TFT_DC  7 // Data/command line for TFT
#define SD_CS   5 // Chip select line for SD card
#define TFT_CS  6 // Chip select line for TFT display
//dimensions of LCD screen
#define TFT_WIDTH 128
#define TFT_HEIGHT 160

Adafruit_ST7735 tft = Adafruit_ST7735( TFT_CS, TFT_DC, TFT_RST );

#define JOY_VERT_ANALOG 0
#define JOY_HORZ_ANALOG 1
#define JOY_SEL 9
// joystick control
#define JOY_DEADZONE 64
#define MILLIS_PER_FRAME 50 // 20fps

int JOY_HORZ_CENTRE = 512;
int JOY_VERT_CENTRE = 512;

int g_joyX = 0;    // initial X-position of cursor for Grid
int g_joyY = 0;    // initial Y-position of cursor for Grid
int g_cursorX = 0;
int g_cursorY = 0;

const uint16_t RED = tft.Color565( 0xff, 0x00, 0x00 );

void scanJoystick();
void updateNumber();
void updateScreen();
void drawScreen();

void mode_menu();
void draw_menu();
void scanJoystick_menu();
void updateCursor_menu();
int selected = 0;
int old_selection = 0;

void mode_board();
void draw_board();
void scanJoystick_board();
void updateCursor_board();
void update_grid();

void mode_result();
void draw_result_completed();
void draw_result_error();
void scanJoystick_result();
void updateCursor_result();

struct sudoku_grid {
  uint8_t value;
  bool fixed; // sizeof(bool) == 1 byte
};
sudoku_grid soln_grid[9][9]; // 0 to 8 by 0 to 8, occupies 162 bytes
sudoku_grid grid[9][9];

int difficulty = 0;
const int easy   = 81 - 35;
const int medium = 81 - 30;
const int hard   = 81 - 25;

void setup();
void clear_grid();
void empty_grid();
void generate_grid();

void random_gen_swap(int check);
void row_swap(int k1, int k2);
void col_swap(int k1,int k2);

void random_gen_change(int check);
void row_change(int k1,int k2);
void col_change(int k1,int k2);

bool solve_grid(int row, int col);
bool test_row(int row, int col);
bool test_col(int row, int col);
bool test_box(int row, int col);

int RNGesus();
void reduce_grid();
bool test_unique(int row, int col);
int num_row;
int num_col;
uint8_t num_val;

void setup_grid();
void print_grid();
bool test_soln();

int main() {
  setup();

  while(true) {
    mode_menu();

    setup_grid();

    mode_board();
  }

  Serial.end();  // terminate serial 1 communication
  return 0;      // no error
}

void setup() {
  init();
  Serial.begin(9600);        // initialize serial communication
  tft.initR(INITR_BLACKTAB);

  pinMode( JOY_SEL, INPUT );     // Init joystick
  digitalWrite( JOY_SEL, HIGH ); // enables pull-up resistor

  // calibrate joystick
  Serial.print("Initializing Joystick. DO NOT TOUCH...");
  JOY_HORZ_CENTRE = analogRead(JOY_HORZ_ANALOG);
  JOY_VERT_CENTRE = analogRead(JOY_VERT_ANALOG);
  Serial.println("OK!");

  tft.fillScreen(0x0000);

  // fills the initialized 9x9 grid with "0"'"s and "false"
  clear_grid();
}

// opening screen. choosed difficulty
void mode_menu() {
  // reset selected and old_selection
  selected = 0; // 0, 1, 2
  old_selection = selected;

  // display menu interface
  draw_menu();

  // user can now choose difficulty
  while(true) {
    // check for input
    scanJoystick_menu();

    // button press --> exit function
    if( digitalRead(JOY_SEL) == LOW ) {
      // feedback
      tft.setTextSize(1);
      tft.setTextColor(0xFFFF,0x0000);
      tft.setCursor(28, 140);
      tft.print("Loading...");

      // set difficulty
      if(selected == 0) { difficulty = easy; }
      if(selected == 1) { difficulty = medium; }
      if(selected == 2) { difficulty = hard; }
      break; // end loop, function
    }

    delay(100);
  }
}

void draw_menu() {
    // fill screen with black
    tft.fillScreen(0x0000);

    // title
    tft.setTextSize(3);
    tft.setTextColor(0xFFFF,0x0000);
    tft.setCursor(12, 21);
    tft.print("SUDOKU");

    // list difficulty levels
    tft.setTextSize(1);
    tft.setTextColor(0xFFFF,0x0000);
    tft.setCursor(28, 60); // 60 * i*14, i == [0,2]
    tft.print("BEGINNER");
    tft.setCursor(28, 74);
    tft.print("INTERMEDIATE");
    tft.setCursor(28, 88);
    tft.print("HARD");

    // initial cursor
    tft.drawRect( 28 - 3, (selected * 14) + 60 - 3, 80, 14, RED );
}

void scanJoystick_menu() {
  int vert = analogRead(JOY_VERT_ANALOG);
  // check joystick
  if( abs(vert - JOY_VERT_CENTRE) > JOY_DEADZONE ) {
    // if joystick points down
    if(vert - JOY_VERT_CENTRE > 0) {
      selected = constrain( selected + 1, 0, 2 );
    }
    // if joystick points up
    else if(vert - JOY_VERT_CENTRE < 0) {
      selected = constrain( selected - 1, 0, 2 );
    }
  }

  // update cursor and old_selection
  if(old_selection != selected) { updateCursor_menu(); }
}

void updateCursor_menu() {
  // draw over old_selection
  tft.drawRect( 28 - 3, (old_selection * 14) + 60 - 3, 80, 14, 0x0000 );
  // draw new selected
  tft.drawRect( 28 - 3, (selected * 14) + 60 - 3, 80, 14, RED );

  // update old_selection
  old_selection = selected;
}

void mode_board() {
  // reset cursor to top left square
  g_joyX = 0;
  g_cursorX = g_joyX;
  g_joyY = 0;
  g_cursorY = g_joyY;

  // display generated sudoku
  draw_board();

  int prevTime = millis();
  int t = 0;
  // user can now try to solve the puzzle
  while(true) {
    scanJoystick_board();

    if( digitalRead(JOY_SEL) == LOW ) {
      if(g_joyY == 9) { break; } // either QUIT or VERIFY
      else { update_grid(); } // update number in grid
    }

    t = millis();
    if( 100 > (t - prevTime) ) { delay( 100 - (t - prevTime) ); }
    prevTime = millis();
  }

  if(g_joyX == 1) { mode_result(); } // VERIFY
  // if QUIT, do nothing
}

void draw_board() {
  // fill screen with black
  tft.fillScreen(0x0000);

  for (int irow = 0; irow < 9; irow++) {
    for (int icol = 0; icol < 9; icol++) {
      // draw square
      tft.drawRect(icol*14, irow*14, 14, 14, 0xFFFF);

      char ch = grid[irow][icol].value + '0'; //take numbers from generate_grid
                                              //and display them on grid
      if(ch == '0') { tft.drawChar(icol*14 + 5, irow*14 + 4, ' ', 0xFFFF, 0x0000, 1); }
      else if(grid[irow][icol].fixed == false) { tft.drawChar(icol*14 + 5, irow*14 + 4, ch, 0xFFFF, 0x0000, 1); }
      else{ tft.drawChar(icol*14 + 5, irow*14 + 4, ch, RED, 0x0000, 1); }
    }
  }

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      tft.drawRect(i*42.6, j*42.6, 44, 44, 0xFFFF);
      tft.drawRect(i*42.6, j*42.6, 41, 41, 0xFFFF);
    }
  }

  tft.fillRect(0, 126, 128, 34, 0x0000);
  tft.fillRect(126, 0, 2, 126, 0x0000);
  tft.fillRect(0, 126, 56, 33, 0x0000);
  tft.fillRect(56, 126 , 56, 33, 0x0000);

  // two buttons
  tft.drawRect(0*42, 126, 42, 33, 0xFFFF);
  tft.drawRect(1*42, 126, 42, 33, 0xFFFF);
  tft.drawRect(2*42, 126, 42, 33, 0xFFFF);
  tft.setTextColor(0xFFFF,0x0000);
  tft.setTextSize(1);
  tft.setCursor(10, 138);
  tft.print("QUIT");
  tft.setCursor(45, 138);
  tft.print("VERIFY");

  // initial cursor
  tft.drawRect( g_joyX*14, g_joyY*14, 14, 14, RED );
}

void scanJoystick_board() {
  int vert = analogRead(JOY_VERT_ANALOG);
  // check joystick
  if( abs(vert - JOY_VERT_CENTRE) > JOY_DEADZONE ) {
    // if joystick points down
    if(vert - JOY_VERT_CENTRE > 0) {
      g_joyY = constrain( g_joyY + 1, 0, 9 );
    }
    // if joystick points up
    else if(vert - JOY_VERT_CENTRE < 0) {
      g_joyY = constrain( g_joyY - 1, 0, 9 );
    }
  }

  if(g_joyY == 9 && g_joyX > 1) { g_joyX = 1; }

  int horz = analogRead(JOY_HORZ_ANALOG);
  // check joystick
  if( abs(horz - JOY_HORZ_CENTRE) > JOY_DEADZONE ) {
    // if joystick points right
    if( (g_cursorY == 9) && (horz - JOY_HORZ_CENTRE > 0) ) {
      g_joyX = 1;
    }
    else if(horz - JOY_HORZ_CENTRE > 0) {
      g_joyX = constrain( g_joyX + 1, 0, 8 );
    }
    // if joystick points left
    else if( (g_cursorY == 9) && (horz - JOY_HORZ_CENTRE < 0) ) {
      g_joyX = 0;
    }
    else if(horz - JOY_HORZ_CENTRE < 0) {
      g_joyX = constrain( g_joyX - 1, 0, 8 );
    }
  }

  if( (g_joyX != g_cursorX) || (g_joyY != g_cursorY) ) { updateCursor_board(); }
}

void updateCursor_board() {
  // draw over old cursor
  if(g_cursorY == 9) { tft.drawRect( g_cursorX*42, g_cursorY*14, 42, 33, 0xFFFF ); }
  else { tft.drawRect( g_cursorX*14, g_cursorY*14, 14, 14, 0xFFFF ); }

  // draw new cursor
  if(g_joyY == 9) { tft.drawRect( g_joyX*42, g_joyY*14, 42, 33, RED ); }
  else { tft.drawRect( g_joyX*14, g_joyY*14, 14, 14, RED ); }

  // update cursor values
  g_cursorX = g_joyX;
  g_cursorY = g_joyY;
}

// y-coordinate == row
// x-coordinate == col
void update_grid() {
  // if num cannot be changed
  if(grid[g_cursorY][g_cursorX].fixed == true) { return; }

  // "increment" num
  int num = grid[g_cursorY][g_cursorX].value;
  num++;
  if(num > 9) { num = 0; }
  grid[g_cursorY][g_cursorX].value = num;

  // draw new num
  char ch = num + '0';
  if(ch == '0') { tft.drawChar(g_cursorX*14 + 5, g_cursorY*14 + 4, ' ' , 0xFFFF, 0x0000, 1); }
  else{ tft.drawChar(g_cursorX*14 + 5, g_cursorY*14 + 4, ch , 0xFFFF, 0x0000, 1); }
}

void mode_result() {
  // if soln is correct
  if( test_soln() ) {
    draw_result_completed();

    delay(3*1000);
    return;
  }

  // if soln is wrong
  selected = 0;
  old_selection = selected;

  draw_result_error();

  while(true) {
    scanJoystick_result();

    if( digitalRead(JOY_SEL) == LOW ) { break; }

    delay(MILLIS_PER_FRAME);
  }

  if(selected == 0) { mode_board(); } // if user wants to retry
}

void draw_result_completed() {
  tft.fillScreen(0x0000);

  tft.setTextSize(2);
  tft.setTextColor(0xFFFF,0x0000);
  tft.setCursor(6, 21);
  tft.print("COMPLETED!");
}

void draw_result_error() {
  tft.fillScreen(0x0000);

  tft.setTextSize(3);
  tft.setTextColor(0xFFFF,0x0000);
  tft.setCursor(12, 21);
  tft.print("ERROR!");

  tft.setTextSize(1);
  tft.setTextColor(0xFFFF,0x0000);
  tft.setCursor(28, 60);
  tft.print("TRY AGAIN?");

  tft.setCursor(28, 74);
  tft.print("YES /");
  tft.setCursor(70, 74);
  tft.print("NO");

  // initial cursor
  tft.drawRect( (selected * 42) + 28 - 3, 74 - 4, 28, 14, RED );
}

void scanJoystick_result() {
  // check joystick
  int horz = analogRead(JOY_HORZ_ANALOG);
  if( abs(horz - JOY_HORZ_CENTRE) > JOY_DEADZONE ) {
    // if joystick points right
    if(horz - JOY_HORZ_CENTRE > 0) {
      selected = constrain( selected + 1, 0, 1 );
    }
    // if joystick points left
    else if(horz - JOY_HORZ_CENTRE < 0) {
      selected = constrain( selected - 1, 0, 1 );
    }
  }

  // update cursor and old_selection
  if(old_selection != selected) { updateCursor_result(); }
}

void updateCursor_result() {
  // draw over old_selection
  tft.drawRect( (old_selection * 42) + 28 - 3, 74 - 4, 28, 14, 0x0000 );
  // draw new selected
  tft.drawRect( (selected * 42) + 28 - 3, 74 - 4, 28, 14, RED );

  // update old_selection
  old_selection = selected;
}

// cleans out grid. sets to 0 and false
void clear_grid() {
  for(int i=0; i<9; ++i ) {   // 0 to 8
    for(int j=0; j<9; ++j ) { // 0 to 8
      grid[i][j].value = 0;
      grid[i][j].fixed == false;
    }
  }
}

// clears any non fixed value on grid
void empty_grid() {
  for(int i=0; i<9; ++i ) {   // 0 to 8
    for(int j=0; j<9; ++j ) { // 0 to 8
      if( grid[i][j].fixed == false ) { grid[i][j].value = 0; }
    }
  }
}

// generates vanilla sudoku:
//   1,2,3,4,5,6,7,8,9
//   4,5,6,7,8,9,1,2,3
//   7,8,9,1,2,3,4,5,6
//   2,3,4,5,6,7,8,9,1
//   5,6,7,8,9,1,2,3,4
//   8,9,1,2,3,4,5,6,7
//   3,4,5,6,7,8,9,1,2
//   6,7,8,9,1,2,3,4,5
//   9,1,2,3,4,5,6,7,8
// then transforms it to make a new puzzle
// also prints soln on serial-monitor
void generate_grid() {
  int n=1; // starting value of every row
  int k=1; // current value
  for( int i=0; i<9; ++i ) { // row index: 0 to 8
    k = n; // set current value to starting value
    for( int j=0; j<9; ++j ) { // col index: 0 to 8
      // assign value and fix
      grid[i][j].value = k;
      grid[i][j].fixed = true;

      //next value
      k++;
      if(k > 9) { k = 1; } // values: 1 to 9. wraparound
    }

    // notice that each row starts with last num from prev row + 4
    // note: last num from prev row is already incremented by 1
    n = k + 3;
    if(n > 9) { n = (n%9) + 1; } // 6 --> 2, 7 --> 3
  }

  // certain transformations on a complete sudoku yields
  // a still complete sudoku
  random_gen_swap(0);
  random_gen_swap(1);
  random_gen_change(0);
  random_gen_change(1);

  // copy soln onto soln_grid[][]
  for(int i=0; i<9; ++i ) {   // 0 to 8
    for(int j=0; j<9; ++j ) { // 0 to 8
      soln_grid[i][j].value = grid[i][j].value;
      soln_grid[i][j].fixed = true;
    }
  }

  // print soln_grid on serial-monitor
  print_grid();
}

// swaps row or col in a three group
void random_gen_swap(int check) {
  int k1, k2;
   //There are three groups.So we are using for loop three times.
   for( int i=0; i<3; i++ ) {
     // pick random row or col
      k1 = i*3 + RNGesus() % 3;
      k2 = i*3 + RNGesus() % 3;
      //This while is just to ensure k1 is not equal to k2.
      while(k1 == k2) { k2 = i*3 + RNGesus() % 3; }

      //check is global variable.
      //We are calling random_gen two time from the main func.
      //Once it will be called for columns and once for rows.
      if     (check == 0) { row_swap(k1,k2); } //calling a function to interchange the selected rows.
      else if(check == 1) { col_swap(k1,k2); }
   }
}

// for row
void row_swap(int k1, int k2) {
    uint8_t temp;
    for( int j=0; j<9; j++ ) {
       temp = grid[k1][j].value;
       grid[k1][j].value = grid[k2][j].value;
       grid[k2][j].value = temp;
    }
 }

// for col
void col_swap(int k1,int k2) {
  uint8_t temp;
   for(int i=0;i<9;i++) {
      temp=grid[i][k1].value;
      grid[i][k1].value=grid[i][k2].value;
      grid[i][k2].value=temp;
   }
}

// swaps row or col groups of three
void random_gen_change(int check) {
  int k1, k2;
    k1 = RNGesus() % 3;
    k2 = RNGesus() % 3;
    while(k1 == k2) { k2 = RNGesus() % 3; }

    // indices: 0,1,2 --> 0,3,6
    k1 = k1 * 3;
    k2 = k2 * 3;

    //check is global variable.
    //We are calling random_gen two time from the main func.
    //Once it will be called for columns and once for rows.
    if     (check == 0) { row_change(k1,k2); } //calling a function to interchange the selected rows.
    else if(check == 1) { col_change(k1,k2); }
}

// for row group
void row_change(int k1,int k2) {
   uint8_t temp;
   for(int n=1;n<=3;n++) {
      for(int j=0;j<9;j++) {
         temp=grid[k1][j].value;
         grid[k1][j].value=grid[k2][j].value;
         grid[k2][j].value=temp;
      }

      k1++;
      k2++;
   }
}

// for col group
void col_change(int k1,int k2) {
   uint8_t temp;
   for(int n=1;n<=3;n++) {
      for(int i=0;i<9;i++) {
         temp=grid[i][k1].value;
         grid[i][k1].value=grid[i][k2].value;
         grid[i][k2].value=temp;
      }

      k1++;
      k2++;
   }
}

// solves sudoku using bruteforce, backtracking and recursion
// false --> sudoku has no soln
// true ---> sudoku has a soln
bool solve_grid( int row, int col ) {
  // look for next non fixed value
	while(grid[row][col].fixed == true) {
	  col++;
	  if(col > 8) {
	  	col = 0;
		  row++;
	  }
    // sudoku is solved if all squares have assigned valid n
	  if(row > 8) { return true; }
	}

	// insert 1 to 9 into non fixed square and see if it forms a soln
  for ( int n=1; n<10; ++n ) {
    // plug in n
		grid[row][col].value = n;
    // test if n is valid
		if( test_col( row, col ) && test_row( row, col ) && test_box( row, col ) ) {
			// go to next square
      int next_row = row;
			int next_col = col + 1;
			if(next_col > 8) {
			  next_col = 0;
			  next_row++;
			}

      // sudoku is solved if all squares have assigned valid n
			if(next_row > 8) { return true;	}
      // recurse
			if( solve_grid( next_row, next_col) ) { return true; }
		}
  }
  // sudoku has no soln if no n is valid
	grid[row][col].value = 0;
	return false;
}

// check if value is already on row
bool test_row( int row, int col ) {
	for( int j=0; j<9; ++j ) {
		if(j != col) {
			if( grid[row][j].value == grid[row][col].value ) { return false; }
		}
	}
	return true;
}

// check if value is already on col
bool test_col( int row, int col ) {
  for( int i=0; i<9; ++i ) {
		if(i != row) {
			if( grid[i][col].value == grid[row][col].value ) { return false; }
		}
	}
	return true;
}

// check if value is already on box
bool test_box( int row, int col ){
	int irow = (row / 3) * 3;
	int icol = (col / 3) * 3;

  for( int i=irow; i<irow + 3; ++i ) {
		for( int j=icol; j<icol + 3; ++j ) {
			if( (i != row) && (j != col) ) {
				if( grid[i][j].value == grid[row][col].value ) { return false; }
			}
		}
	}
	return true;
}

// random number generator / God
int RNGesus() {
  int analogPin = 7; // analog pin 7 should not be connected to anything
  int val = 0;
  int result = 0;

  for( int i=0; i<4; i++ ) { // get bit 4 times to get 4 bits
    val = analogRead(analogPin);  //analog pin 7 voltage fluctuates
    val = val & 1;                // remainder is least significant bit
    result = (result << 1) + val; // shift key_val left before adding new bit
    delay(10); // wait for 10ms to allow for voltage fluctuation
  }

  return result; // returns 4-bit num [0-15]
}

// strikes out squares while maintaining uniqueness
void reduce_grid() {
  for(int i=0; i<difficulty; ++i ) { // number of values to strike out
    // pick random square
    num_row = RNGesus() % 9; // 0 to 8
    num_col = RNGesus() % 9; // 0 to 8

    // store value before striking out
    num_val = grid[num_row][num_col].value;
    // strike out selected square
    grid[num_row][num_col].value = 0;
    grid[num_row][num_col].fixed = false;

    // do again if square is already stroke out
    //          or soln is no longer unique
    while( num_val==0 || test_unique(0,0) ) {
      // restore square only if its not already stroke out
      if(num_val != 0) {
        grid[num_row][num_col].value = num_val;
        grid[num_row][num_col].fixed = true;
      }

      // find next square to try to strike out
      num_col++;
      if(num_col > 8) {
        num_col = 0;
        num_row++;
      }
      if(num_row > 8) { num_row = 0; }

      // store value then strike out again
      num_val = grid[num_row][num_col].value;
      grid[num_row][num_col].value = 0;
      grid[num_row][num_col].fixed = false;
    }
  }
  // clean up to prevent bugs
  empty_grid();
}

// based on solver
// true ---> another soln exists
// false --> no other soln exists i.e. solution is unique
bool test_unique( int row, int col ) {
	while(grid[row][col].fixed == true) {
	  col++;
	  if(col > 8) {
	  	col = 0;
		  row++;
	  }
	  if(row > 8) { return true; }
	}

	for ( int n=1; n<10; ++n ) {
    // excludes value removed to check for other possible soln
    if( row==num_row && col==num_col && n==num_val ) { continue; }

		grid[row][col].value = n;
		if( test_col( row, col ) && test_row( row, col ) && test_box( row, col ) ) {
			int next_row = row;
			int next_col = col + 1;

			if(next_col > 8) {
			  next_col = 0;
			  next_row++;
			}
			if(next_row > 8) { return true;	}
      // recurse
			if( test_unique( next_row, next_col ) ) { return true; }
    }
  }
	grid[row][col].value = 0;
	return false;
}

void setup_grid() {
  // generate random sudoku
  generate_grid();
  reduce_grid();
  // make sure sudoku is complete and unique
  while( !solve_grid(0,0) ) { reduce_grid(); }
  empty_grid();
}

// prints solution grid on serial monitor for verification
void print_grid() {
  for(int i=0; i<9; ++i ) {   // 0 to 8
    for(int j=0; j<9; ++j ) { // 0 to 8
      char ch = soln_grid[i][j].value + '0'; // index = ith row, jth column

      if( ch == '0' ) { Serial.print(" "); } // 0 is not a valid input in sudoku
      else { Serial.print(ch); }

      if(j == 8) { Serial.println(); } // newline
      else { Serial.print(","); }      // comma separate
    }
  }
  Serial.println();
}

// check if player input is correct
// false --> soln is wrong
// true ---> soln is correct
bool test_soln() {
  // go through 9x9 grid
  for(int i=0; i<9; ++i ) {   // 0 to 8
    for(int j=0; j<9; ++j ) { // 0 to 8
      // if any value doesn't match, solution is wrong
      if( soln_grid[i][j].value != grid[i][j].value ) { return false; }
    }
  }
  // else, solution is correct
  return true;
}
