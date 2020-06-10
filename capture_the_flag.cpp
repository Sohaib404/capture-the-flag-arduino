/*
	Name: Sohaib Khadri
	ID: 1574054
	CMPUT 275, Winter 2019

	Final Project: Capture The Flag
*/

//adding libraries for various devices used
#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <SPI.h>
#include <SD.h>
#include <TouchScreen.h>

// joystick and touch screen variables provided from documentation, use this to help wiring
#define SD_CS 10
#define JOY1_VERT  A9
#define JOY1_HORIZ A8
#define JOY2_VERT  A11
#define JOY2_HORIZ A10
#define JOY1_SEL   53
#define JOY2_SEL   43
#define JOY_CENTER   512
#define JOY_DEADZONE 64

#define YP A3
#define XM A2
#define YM 9
#define XP 8
#define TS_MINX 100
#define TS_MINY 120
#define TS_MAXX 940
#define TS_MAXY 920
#define MINPRESSURE   10
#define MAXPRESSURE 1000
#define DISPLAY_WIDTH  480
#define DISPLAY_HEIGHT 320

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);
MCUFRIEND_kbv tft;

//colours used
#define BLACK   0x0000
#define BLUE    0x001F
#define RED     0xF800
#define GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

//initializing player variable
#define PLAYER_SIZE 10
#define BASE_SIZE 50
int player1X, player1Y;
int player2X, player2Y;
int cursorX, cursorY;
int direction1;
int shoot1Val;
int direction2;
int shoot2Val;
//initializing bullet variable
int bulletSpeed = 15;
int bullet1X, bullet1Y, bullet1Direction;
bool bullet1Instantiated = false;
int bullet2X, bullet2Y, bullet2Direction;
bool bullet2Instantiated = false;
//initializing score variables
int p1Score = 0; int p2Score = 0;
int p1Kills = 0; int p2Kills = 0;
bool p1hasFlag = false;
bool p2hasFlag = false;
//scene variable used to switch between menus and main game scene
int scene = 0;

//forward initialization of functions
void mainMenu();
void help();
void pauseButton();
void trapCheck();
void gameOver();
void redrawPlayer1(uint16_t colour);
void redrawPlayer2(uint16_t colour);
void drawUI();
void drawBases();
void drawMap();


void setup() {
 	init();
  	Serial.begin(9600);

	pinMode(JOY1_SEL, INPUT_PULLUP);
	pinMode(JOY2_SEL, INPUT_PULLUP);

	//    tft.reset();             // hardware reset
 	uint16_t ID = tft.readID();    // read ID from display
 	Serial.print("ID = 0x");
 	Serial.println(ID, HEX);
  	if (ID == 0xD3D3) ID = 0x9481; // write-only shield
  
  	tft.begin(ID);                 // LCD gets ready to work
 	tft.setRotation(1);

	// initial player positions
	player1X = 60;
	player1Y = DISPLAY_HEIGHT/2;
	player2X = DISPLAY_WIDTH - 60;
	player2Y = DISPLAY_HEIGHT/2;

}

//draws mainmenu screen
void mainMenu() {

	tft.fillScreen(TFT_BLACK);
	tft.setCursor(DISPLAY_WIDTH/5,80);
 	tft.setTextColor(WHITE);
	tft.setTextSize(4);
	tft.print("CAPTURE THE");
	tft.setCursor(DISPLAY_WIDTH/3,120);
	tft.print("FLAG");

	tft.drawRect(DISPLAY_WIDTH/2 -101,200, 100, 50, TFT_WHITE);
	tft.setCursor(DISPLAY_WIDTH/2 -101 +5, 215);
	tft.setTextSize(3);
	tft.print("START");

	tft.drawRect(DISPLAY_WIDTH/2+1, 200, 100, 50, TFT_WHITE);
	tft.setCursor(DISPLAY_WIDTH/2+10, 215);
	tft.setTextSize(3);
	tft.print("HELP");

	tft.setCursor(5, DISPLAY_HEIGHT-20);
	tft.setTextSize(2);
	tft.print("Made by Sohaib Khadri");

	tft.setCursor(DISPLAY_WIDTH-135, DISPLAY_HEIGHT-20);
	tft.setTextSize(2);
	tft.print("Version 1.0");

	while(scene == 0) {

	TSPoint touch = ts.getPoint();
	pinMode(YP, OUTPUT); 
	pinMode(XM, OUTPUT); 

		if (touch.z > MINPRESSURE && touch.z < MAXPRESSURE) {
			int16_t screen_x = map(touch.y, TS_MINX, TS_MAXX, DISPLAY_WIDTH-1, 0);
			int16_t screen_y = map(touch.x, TS_MINY, TS_MAXY, DISPLAY_HEIGHT-1, 0);

			if(screen_y > 200 && screen_y < 250) {
			   	if (screen_x < DISPLAY_WIDTH/2) {
			   		scene = 1;
			   	} else {
			   		scene = 2;
			   	}
			}
		}
	}
}

//draws help screen
void help() {
	
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(5,10);
 	tft.setTextColor(WHITE);
	tft.setTextSize(4);
	tft.print("How to play:");

	tft.setTextSize(2);
	tft.setCursor(0,50);
	tft.print("- Use the joystick to move your player  around and the button to shoot.");
	tft.setCursor(0,100);
	tft.print("- You can only shoot one bullet at a    time. Make sure to avoid the traps.");
	tft.setCursor(0,150);
	tft.print("- To score a point, grab the opponents  flag and bring it back to your base in  one piece.");
	tft.setCursor(0,230);
	tft.print("- First player to 3 wins! Have fun!");
	
	tft.setCursor(5, DISPLAY_HEIGHT-20);
	tft.setTextSize(2);
	tft.print("Tap to return to Menu");

	while(scene == 2) {
		TSPoint touch = ts.getPoint();
		pinMode(YP, OUTPUT); 
		pinMode(XM, OUTPUT); 

		if (touch.z > MINPRESSURE && touch.z < MAXPRESSURE) {
		   	scene = 0; 
		}
	}

}


//draws map with traps
void drawMap() {
	//draw green floor
	tft.fillScreen(GREEN);
	tft.fillRect(0, 0, DISPLAY_WIDTH, 60, TFT_BLACK);
	//draw traps
	tft.drawRect(95, 90, 25, 100, TFT_RED);
	tft.fillRect(95+1,90+1,23,98,TFT_BLACK);
	tft.drawRect(190,165, 100, 50, TFT_RED);
	tft.fillRect(190+1,165+1,98, 48,TFT_BLACK);
	tft.drawRect(360, 190, 25, 100, TFT_RED);
	tft.fillRect(360+1,190+1, 23, 98, TFT_BLACK);

}

//draws UI elements
void drawUI() {

	tft.fillRect(0, 0, DISPLAY_WIDTH, 60, TFT_BLACK);
	//draw player 1 info
	tft.setCursor(DISPLAY_WIDTH/5,0);
 	tft.setTextColor(WHITE);
	tft.setTextSize(2);
	tft.print("Player 1");
	tft.setCursor(DISPLAY_WIDTH/5,18);
 	tft.print("Score: " + String(p1Score));
	tft.setCursor(DISPLAY_WIDTH/5,36);
	tft.print("Kills: " + String(p1Kills));
	//draw pause button
	tft.drawRect(DISPLAY_WIDTH/2 -15, 0, 30, 30, TFT_WHITE);
	tft.fillRect(DISPLAY_WIDTH/2 -8, 5, 6, 20, TFT_WHITE);
	tft.fillRect(DISPLAY_WIDTH/2 +2, 5, 6, 20, TFT_WHITE);
	//draw player 2 info
	tft.setCursor(DISPLAY_WIDTH*3/5, 0);
	tft.print("Player 2");
	tft.setCursor(DISPLAY_WIDTH*3/5,18);
 	tft.print("Score: " + String(p2Score));
	tft.setCursor(DISPLAY_WIDTH*3/5,36);
	tft.print("Kills: " + String(p2Kills));

}

//logic used to pause mid game
void pauseButton() {
	
	TSPoint touch = ts.getPoint();
	pinMode(YP, OUTPUT);
	pinMode(XM, OUTPUT);
		if (touch.z > MINPRESSURE && touch.z < MAXPRESSURE) {
		   int16_t screen_x = map(touch.y, TS_MINX, TS_MAXX, DISPLAY_WIDTH-1, 0);
		   int16_t screen_y = map(touch.x, TS_MINY, TS_MAXY, DISPLAY_HEIGHT-1, 0);

		   if(screen_y < 40) {
		   	if (screen_x < DISPLAY_WIDTH/2 +10 && screen_x > DISPLAY_WIDTH/2 -10) {

		   		tft.fillRect(DISPLAY_WIDTH/2 -8, 5, 22, 20, TFT_BLACK);
		   		tft.fillTriangle(DISPLAY_WIDTH/2 -8, 5, DISPLAY_WIDTH/2 -8, 25, DISPLAY_WIDTH/2 +8, 15, TFT_WHITE);
		   		
		   		bool paused = true;
		   		delay(100);	
		   		//loop runs until game is unpaused so that everything is frozen in time
		   		while(paused) {

		   		TSPoint touch = ts.getPoint();
				pinMode(YP, OUTPUT);
				pinMode(XM, OUTPUT);
		   			if (touch.z > MINPRESSURE && touch.z < MAXPRESSURE) {
		   				int16_t screen_x = map(touch.y, TS_MINX, TS_MAXX, DISPLAY_WIDTH-1, 0);
		   				int16_t screen_y = map(touch.x, TS_MINY, TS_MAXY, DISPLAY_HEIGHT-1, 0);

		   				if(screen_y < 40) {
		   					if (screen_x < DISPLAY_WIDTH/2 +10 && screen_x > DISPLAY_WIDTH/2 -10) {
					   		paused = false;
						   		tft.fillRect(DISPLAY_WIDTH/2 -8, 5, 22, 20, TFT_BLACK);
								tft.fillRect(DISPLAY_WIDTH/2 -8, 5, 6, 20, TFT_WHITE);
								tft.fillRect(DISPLAY_WIDTH/2 +2, 5, 6, 20, TFT_WHITE);
								delay(100);
							}
						}
					}	
		   		}
		   		
		   	}
		   }
		}

}

				
		   		
//draws bases and flags on the map or in inventory
void drawBases() {
	//draw bases
	tft.drawRect(0, DISPLAY_HEIGHT-50, BASE_SIZE, BASE_SIZE, TFT_RED);
    tft.drawRect(DISPLAY_WIDTH - 50, 60, BASE_SIZE, BASE_SIZE, BLUE);


    if(p2hasFlag) {
    	//draw red flag in inventory, player 2 has red flag
    	tft.fillRect(25-1,DISPLAY_HEIGHT-90, 20, 20, GREEN);
    	tft.fillTriangle(DISPLAY_WIDTH - 25, 20, DISPLAY_WIDTH - 25, 30, DISPLAY_WIDTH - 15, 25, TFT_RED);
    	tft.drawLine(DISPLAY_WIDTH - 26, 20, DISPLAY_WIDTH - 26, 35, WHITE);
    } else {
    	//draw red flag on map
    	tft.fillRect(DISPLAY_WIDTH-26, 20, 12, 17, TFT_BLACK);
    	tft.fillTriangle(25,DISPLAY_HEIGHT-80, 25, DISPLAY_HEIGHT-90, 35, DISPLAY_HEIGHT-85, TFT_RED);
    	tft.drawLine(25-1,DISPLAY_HEIGHT-75, 25-1, DISPLAY_HEIGHT-90, BLACK);
    }

    if(p1hasFlag) {
    	//draw blue flag in inventory, player 1 has blue flag
    	tft.fillRect(DISPLAY_WIDTH -25, 140, 20, 20, GREEN);
    	tft.fillTriangle(25, 20, 25, 30, 35, 25, BLUE);
    	tft.drawLine(25-1, 20, 25-1, 35, TFT_WHITE);
    } else {
    	//draw blue flag on map
    	tft.fillRect(26, 20, 12, 17, TFT_BLACK);
    	tft.fillTriangle(DISPLAY_WIDTH -25, 140, DISPLAY_WIDTH - 25, 150, DISPLAY_WIDTH - 15, 145, BLUE);
    	tft.drawLine(DISPLAY_WIDTH-25-1, 140, DISPLAY_WIDTH-25-1, 155, BLACK);
    }
    
}

//draws player 1 to screen
void redrawPlayer1(uint16_t colour) {

	tft.fillRect(player1X - PLAYER_SIZE/2, player1Y - PLAYER_SIZE/2,
               PLAYER_SIZE, PLAYER_SIZE, colour);
	
}

//draws player 1 to screen
void redrawPlayer2(uint16_t colour) {

	tft.fillRect(player2X - PLAYER_SIZE/2, player2Y - PLAYER_SIZE/2,
               PLAYER_SIZE, PLAYER_SIZE, colour);
	
}

//all movement logic and collider logic for player 1
void player1() {

	int xVal = analogRead(JOY1_HORIZ);
	int yVal = analogRead(JOY1_VERT);
  	//horizontal and vertical speed multipliers
	int xSpeed = abs(xVal - JOY_CENTER) / 150;
	int ySpeed = abs(yVal - JOY_CENTER) / 150;

  	//check for input, and move player 1 accordingly
	if ((yVal < JOY_CENTER - JOY_DEADZONE)) {
  		//move up
		tft.fillRect(player1X - PLAYER_SIZE/2, player1Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
		player1Y -= 1*ySpeed;
		//upper map constrain
		if (player1Y < 60 + PLAYER_SIZE/2) {
			player1Y = 60 + PLAYER_SIZE/2;
		}
		//player 1 is now facing up
    	direction1 = 0;

	} else if ((yVal > JOY_CENTER + JOY_DEADZONE)) {
  		//move down
		tft.fillRect(player1X - PLAYER_SIZE/2, player1Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
		player1Y += 1*ySpeed;
		//lower map constrain
    	if (player1Y > DISPLAY_HEIGHT - PLAYER_SIZE/2) {
    		player1Y = DISPLAY_HEIGHT - PLAYER_SIZE/2;
    	}
    	//player is now facing down
    	direction1 = 2;

	}

  	
	if ((xVal > JOY_CENTER + JOY_DEADZONE)) {
  		//move left
		tft.fillRect(player1X - PLAYER_SIZE/2, player1Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
		player1X -= 1*xSpeed;
		//left map constrain
    	if (player1X < PLAYER_SIZE/2) {
    		player1X = PLAYER_SIZE/2;
    	}
    	//player is now facing left
		direction1 = 3;

	} else if ((xVal < JOY_CENTER - JOY_DEADZONE)) {
  		//move right
  		tft.fillRect(player1X - PLAYER_SIZE/2, player1Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
    	player1X += 1*xSpeed;
		//right map constrain
    	if (player1X > DISPLAY_WIDTH - PLAYER_SIZE/2) {
    		player1X = DISPLAY_WIDTH - PLAYER_SIZE/2;
    	}
    	//player is now facing right
    	direction1 = 1;

	}

	//checking if player is near enemy flag, if so then grab flag
	if(player1X > DISPLAY_WIDTH-25 && player1X < DISPLAY_WIDTH-15) {
		if(player1Y > 140 && player1Y < 150) {
			p1hasFlag = true;
			drawBases();
  		}
  	}

  	//checking if player is in his base with or without enemy flag. Also checking if max score reached
	if(player1X < 50 && player1Y > DISPLAY_HEIGHT-50) {
		if(p1hasFlag) {
			p1Score += 1;
	  		drawUI();
	  		p1hasFlag = false;
	  		drawBases();
	  		if(p1Score == 3) {
	  			scene = 3;
	  		}
  		}
	}

	//checking if player has fallen into any traps
	if (player1X > 95 - PLAYER_SIZE/2 && player1X < 120 + PLAYER_SIZE/2) {
		if (player1Y > 90 - PLAYER_SIZE/2 && player1Y < 190 + PLAYER_SIZE/2) {
			player1X = 60;
			player1Y = DISPLAY_HEIGHT/2;
			p1hasFlag = false;
			drawBases();
		} 
	} else if (player1X > 190 - PLAYER_SIZE/2 && player1X < 290 + PLAYER_SIZE/2) {
		if (player1Y > 165 - PLAYER_SIZE/2 && player1Y < 215 + PLAYER_SIZE/2) {
			player1X = 60;
			player1Y = DISPLAY_HEIGHT/2;
			p1hasFlag = false;
			drawBases();
		} 
	} else if (player1X > 360 - PLAYER_SIZE/2 && player1X < 385 + PLAYER_SIZE/2) {
		if (player1Y > 190 - PLAYER_SIZE/2 && player1Y < 290 + PLAYER_SIZE/2) {
			player1X = 60;
			player1Y = DISPLAY_HEIGHT/2;
			p1hasFlag = false;
			drawBases();
		} 
	}

	//draw player after calculating movement and checking collisions
	redrawPlayer1(TFT_RED);
	delay(20);
}

//all movement logic and collider logic for player 2
void player2() {

	int xVal = analogRead(JOY2_HORIZ);
	int yVal = analogRead(JOY2_VERT);
  	//horizontal and vertical speed multipliers
	int xSpeed = abs(xVal - JOY_CENTER) / 150;
	int ySpeed = abs(yVal - JOY_CENTER) / 150;

	//check for input, and move player 2 accordingly
	if ((yVal < JOY_CENTER - JOY_DEADZONE)) {
	  	//move up
		tft.fillRect(player2X - PLAYER_SIZE/2, player2Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
	    player2Y -= 1*ySpeed;
	    //upper map constrain
	    if (player2Y < 60 + PLAYER_SIZE/2) {
	    	player2Y = 60 + PLAYER_SIZE/2;
	    }
	    //player 2 is now facing up
	    direction2 = 0;

	} else if ((yVal > JOY_CENTER + JOY_DEADZONE)) {
	  	//move down
	  	tft.fillRect(player2X - PLAYER_SIZE/2, player2Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
	    player2Y += 1*ySpeed;
	    //lower map constrain
	    if (player2Y > DISPLAY_HEIGHT - PLAYER_SIZE/2) {
	    	player2Y = DISPLAY_HEIGHT - PLAYER_SIZE/2;
	    }
	    //player 2 is now facing down
	    direction2 = 2;
	}


	if ((xVal > JOY_CENTER + JOY_DEADZONE)) {
  		//move left
  	  	tft.fillRect(player2X - PLAYER_SIZE/2, player2Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
	    player2X -= 1*xSpeed;

    	//left map constrain
    	if (player2X < PLAYER_SIZE/2) {
    		player2X = PLAYER_SIZE/2;
    	}
    	//player 2 is now facing left
    	direction2 = 3;

	} else if ((xVal < JOY_CENTER - JOY_DEADZONE)) {
	  	//move right
		tft.fillRect(player2X - PLAYER_SIZE/2, player2Y - PLAYER_SIZE/2, PLAYER_SIZE, PLAYER_SIZE, GREEN);
	    player2X += 1*xSpeed;
	    //right map constrain
	    if (player2X > DISPLAY_WIDTH - PLAYER_SIZE/2) {
	    	player2X = DISPLAY_WIDTH - PLAYER_SIZE/2;
	    }
	    //player 2 is now facing right
	    direction2 = 1;

	}

	//checking if player is near enemy flag, if so then grab flag
	if(player2X > 25 && player2X < 35) {
		if(player2Y < DISPLAY_HEIGHT-75 && player2Y > DISPLAY_HEIGHT-90) {
  			p2hasFlag = true;
  			drawBases();
  		}
  	}

  	//checking if player is in his base with or without enemy flag. Also checking if max score reached
	if(player2X > DISPLAY_WIDTH-50 && player2Y < 110) {
	  	if(p2hasFlag) {
	  		p2Score += 1;
	  		drawUI();
	  		p2hasFlag = false;
	  		drawBases();
	  		if(p2Score == 3) {
	  			scene = 3;
	  		}
	  	}
  	}

  	//checking if player has fallen into any traps
	if(player2X > 95 - PLAYER_SIZE/2 && player2X < 120 + PLAYER_SIZE/2) {
		if (player2Y > 90 - PLAYER_SIZE/2 && player2Y < 190 + PLAYER_SIZE/2) {
			player2X = DISPLAY_WIDTH - 60;
	  		player2Y = DISPLAY_HEIGHT/2;
	  		p2hasFlag = false;
	  		drawBases();
		} 
	} else if (player2X > 190 - PLAYER_SIZE/2 && player2X < 290 + PLAYER_SIZE/2) {
		if (player2Y > 165 - PLAYER_SIZE/2 && player2Y < 215 + PLAYER_SIZE/2) {
			player2X = DISPLAY_WIDTH - 60;
  			player2Y = DISPLAY_HEIGHT/2;
  			p2hasFlag = false;
  			drawBases();
		} 
	} else if (player2X > 360 - PLAYER_SIZE/2 && player2X < 385 + PLAYER_SIZE/2) {
		if (player2Y > 190 - PLAYER_SIZE/2 && player2Y < 290 + PLAYER_SIZE/2) {
			player2X = DISPLAY_WIDTH - 60;
  			player2Y = DISPLAY_HEIGHT/2;
  			p2hasFlag = false;
  			drawBases();
		} 
	}

	//draw player after calculating movement and checking collisions
	redrawPlayer2(BLUE);
	delay(20);

}

//logic and drawing of player 1's bullet
void Bullet1() {
	int shoot1Val = digitalRead(JOY1_SEL);
	// if button is pressed, instantiate bullet in player direction
	if(shoot1Val == 0){
		if(bullet1Instantiated == false) {
		bullet1X = player1X;
		bullet1Y = player1Y;
		bullet1Direction = direction1;
		bullet1Instantiated = true;
		tft.fillRect(bullet1X-2, bullet1Y-2,2,2, TFT_BLACK);
		}
	}
	//move bullet in player direction if still on screen
	if(bullet1Instantiated && bullet1X > 0 && bullet1X < DISPLAY_WIDTH && bullet1Y > 60 && bullet1Y < DISPLAY_HEIGHT) {
		tft.fillRect(bullet1X-2, bullet1Y-2,2,2, GREEN);
	switch (bullet1Direction) {
		case 0:
			bullet1Y -= bulletSpeed;
			break;
		case 1:
			bullet1X += bulletSpeed;
			break;
		case 2:
			bullet1Y += bulletSpeed;
			break;
		case 3:
			bullet1X -= bulletSpeed;
			break;
	}
	tft.fillRect(bullet1X-2, bullet1Y-2,2,2, TFT_BLACK);
	} else {
		bullet1Instantiated = false;
	}

	//check if bullet hit enemy. Add kill and respawn enemy
	if(bullet1X < (player2X + PLAYER_SIZE/2)+1 && bullet1X > (player2X -PLAYER_SIZE/2)-1 && bullet1Y < (player2Y + PLAYER_SIZE)+1 && bullet1Y > (player2Y -PLAYER_SIZE/2)-1) {
		p1Kills += 1;
		drawUI();
		p2hasFlag = false;
		drawBases();
		redrawPlayer2(GREEN);
		player2X = DISPLAY_WIDTH - 100;
 		player2Y = DISPLAY_HEIGHT/2;
 		redrawPlayer2(BLUE);
	}
}

//logic and drawing of player 2's bullet
void Bullet2() {
	int shoot2Val = digitalRead(JOY2_SEL);
	// if button is pressed, instantiate bullet in player direction
	if(shoot2Val == 0){
		
		if(bullet2Instantiated == false) {
		bullet2X = player2X;
		bullet2Y = player2Y;
		bullet2Direction = direction2;
		bullet2Instantiated = true;
		tft.fillRect(bullet2X-2, bullet2Y-2,2,2, TFT_BLACK);
		}
	}
	//move bullet in player direction if still on screen
	if(bullet2Instantiated && bullet2X > 0 && bullet2X < DISPLAY_WIDTH && bullet2Y > 60 && bullet2Y < DISPLAY_HEIGHT) {
		tft.fillRect(bullet2X-2, bullet2Y-2,2,2, GREEN);
	switch (bullet2Direction) {
		case 0:
			bullet2Y -= bulletSpeed;
			break;
		case 1:
			bullet2X += bulletSpeed;
			break;
		case 2:
			bullet2Y += bulletSpeed;
			break;
		case 3:
			bullet2X -= bulletSpeed;
			break;
	}
	tft.fillRect(bullet2X-2, bullet2Y-2,2,2, TFT_BLACK);
	} else {
		bullet2Instantiated = false;
	}
	//check if bullet hit enemy. Add kill and respawn enemy
	if(bullet2X < (player1X + PLAYER_SIZE/2)+1 && bullet2X > (player1X -PLAYER_SIZE/2)-1 && bullet2Y < (player1Y + PLAYER_SIZE)+1 && bullet2Y > (player1Y -PLAYER_SIZE/2)-1) {
		
		p2Kills += 1;
		drawUI();
		p1hasFlag = false;
		drawBases();
		redrawPlayer1(GREEN);
		player1X = 60;
		player1Y = DISPLAY_HEIGHT/2;
 		redrawPlayer1(TFT_RED);
	}
}

//draws game over scene
void gameOver() {
	
	//display winner
	tft.fillScreen(TFT_BLACK);
	tft.setCursor(DISPLAY_WIDTH/6,80);
	tft.setTextColor(WHITE);
	tft.setTextSize(4);
	tft.print("CONGRATULATIONS!");
	tft.setCursor(DISPLAY_WIDTH/6,120);
	if(p1Score > p2Score) {
		tft.print("PLAYER 1 WINS!");
	} else {
		tft.print("PLAYER 2 WINS!");
	}

	tft.setTextSize(2);
	tft.setCursor(DISPLAY_WIDTH/5,160);
	if(p1Kills > 5) {
		tft.print("Player 1 was on a RAMPAGE!");
	}
	tft.setCursor(DISPLAY_WIDTH/5,180);
	if(p2Kills > 5) {
		tft.print("Player 2 was on a RAMPAGE!");
	}

	//reset scores
	p1Score = 0;
	p2Score = 0;
	p1Kills = 0;
	p2Kills = 0;


	tft.setCursor(DISPLAY_WIDTH/2 -100, DISPLAY_HEIGHT-50);
	tft.setTextSize(2);
	tft.print("Tap to return to Menu");

	delay(100);
	while(scene == 3) {
		TSPoint touch = ts.getPoint();
		pinMode(YP, OUTPUT); 
		pinMode(XM, OUTPUT); 

		if (touch.z > MINPRESSURE && touch.z < MAXPRESSURE) {
		   	//move to main menu
		   	scene = 0;
		   
		}
	}
	
}

int main() {
	setup();
	while(true) {
		//jump to main menu screen
		if(scene == 0) {
			mainMenu();
		}
		//jump to main game, first draw everything once, then start game loop
		if (scene == 1){
			drawMap();
			drawUI();
			drawBases();
			redrawPlayer1(TFT_RED);
	  		redrawPlayer2(BLUE);
		}
	  	while (scene == 1) {
	    	player1();
	    	player2();
	    	Bullet1();
	    	Bullet2();
	    	pauseButton();
	    	
	  	}
	  	//jump to help screen
	  	if(scene == 2) {
	  		help();
	  	}
	  	//jump to game over screen
	  	if(scene == 3) {
	  		gameOver();
	  	}

  	}

	Serial.end();
	return 0;
}