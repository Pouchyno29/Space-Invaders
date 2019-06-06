
      #include <SPI.h>
      #include <Adafruit_GFX.h>
      #include <Adafruit_PCD8544.h>

      //Set up the pin for the LCD screen
      Adafruit_PCD8544 display = Adafruit_PCD8544(3, 4, 5, 7, 6);

      //Chip Select        == > from the LCD connect to pin 7 on the Arduino
      //RESET          == > from the LCD connect to pin 6 on the Arduino
      //D/C            == > from the LCD connect to pin 5 on the Arduino
      //MASTER OUT SLAVE IN  == > from the LCD connect to pin 4 on the Arduino
      //CLOCK          == > from the LCD connect to pin 3 on the Arduino

      //This is a SPI LCD Display

      //Set up the pin for the shift register
      #define enable 2  // Connect pin 15 == > from the chip to pin 2 on the Arduino
      #define load 8    // Connect pin 1  == > from the chip to pin 3 on the Arduino
      #define clock 9   // Connect pin 2  == > from the chip to pin 4 on the Arduino
      #define data 10   // Connect pin 9  == > from the chip to pin 5 on the Arduino

      //This is a SN74HC165N Shift register, which is a parallel to serial converter

      //This is the first frame for alien #1
       static const unsigned char PROGMEM alien_1_1[] =
      {
        B00011000,
        B00111100,
        B01111110,
        B11011011,
        B11111111,
        B00100100,
        B01011010,
        B10100101,
      };

      //SPACE SHIP
      static const unsigned char PROGMEM small_ship[] =
      {
        B00000000,
        B00000000,
        B00000000,
        B00011000,
        B00011000,
        B01111110,
        B11111111,
        B11111111,
      };

      //Block to protect the ship
      static const unsigned char PROGMEM block[] =
      {
        B00111100,
        B01111110,
        B11111111,
        B11100111,
        B11000011,
        B11000011,
        B00000000,
        B00000000,
      };

      byte previous; // Create a variable to hold the previous set of bytes

      int gameSpeed = 190; //This is the speed of the game
      int score = 0;//========
      //To keep the game speed constant, we keep track of the last time the snake was moved
      unsigned long prevUpdateTime;


      void setup() {
        Serial.begin(9600);

          pinMode(enable,OUTPUT);
          pinMode(load,OUTPUT);
          pinMode(clock,OUTPUT);
          pinMode(data,INPUT);
          digitalWrite(load,HIGH);
          digitalWrite(enable,HIGH);

        display.begin(); //I don't know why I need to put this here, since there is nothing to display yet but if I don't it will not work

        display.setContrast(60); // Set the screen brightness.

        display.clearDisplay(); // Clear the screen.

        initGame();//=============
      }

      void loop() {
          //============================
         moveAliens();
         moveMissiles();
         movePlayer();

         display.clearDisplay();
         drawAliens();
         drawMissiles();
         drawPlayer();
         display.display();

         checkAlienMissileImpact();
         checkPlayerMissileImpact();
         checkLevelClear();
        //======================================
          unsigned long curTime;
          do{
            //readButtons();
            //processButtons();
            curTime = millis();
          }
          while ((curTime - prevUpdateTime) < gameSpeed);//Once enough time  has passed, proceed. The lower this number, the faster the game is
          prevUpdateTime = curTime;

      
      }
      //==============================================================
      struct Missile {
        int speed;
        int x;
        int y;
        boolean enabled;
      };
      Missile alienMissile;
      Missile playerMissile;

      struct Player{
        int x;
        int y;
        int lives;
      };
      Player player;

      #define NB_ALIENS  6
      #define SPACE_X  10
      struct Alien {
        int x;
        int y;
        int lives;
      };
      //int aliensX = 0;
      //int aliensY = 0;
      int aliensSpeedX = 2;
      int aliensSpeedY = 2;
      Alien aliens[NB_ALIENS];

      void checkAlienMissileImpact(){
        if (alienMissile.y > player.y){//Check height first
          if ((alienMissile.x > player.x-2) && (alienMissile.x < (player.x+7))){//Check horizontal alignment
            die();
          }
        }
      }

      void checkPlayerMissileImpact(){
        for (int i=0; i<NB_ALIENS; i++){
          if (aliens[i].lives>0){
            if ((playerMissile.y < (aliens[i].y+8)) && (playerMissile.y > (aliens[i].y))){
              if (playerMissile.x < (aliens[i].x+8) && playerMissile.x > (aliens[i].x)){
                aliens[i].lives = aliens[i].lives -1;
                score = score+1;
              }
            }
          }
        }
      }

      void checkLevelClear(){
        boolean alienLivesLeft = false;
        for (int i=0; i<NB_ALIENS; i++){
          if (aliens[i].lives>0){
            alienLivesLeft = true;
            break;
          }
        }
        if (!alienLivesLeft){
          levelCleared();
        }
      }

      void levelCleared(){
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(0,20);
        display.println("Level clear!");
        display.println("Score:");
        display.println(score);
        display.display();
        delay(1500);

        initGame();
        if (gameSpeed >= 30){
          gameSpeed = gameSpeed - 20;
        }
        prevUpdateTime = millis();
      }

      void initGame(){
        //Init alien positions
        for (int i=0; i<NB_ALIENS; i++){
          aliens[i].lives = 1;
          aliens[i].x = aliensSpeedX + SPACE_X*i;
          aliens[i].y = 0;
        }
        //Init missiles
        alienMissile.speed = 4;
        alienMissile.x = 0;
        alienMissile.y = 0;
        alienMissile.enabled = false;

        playerMissile.speed = 6;
        playerMissile.x = 0;
        playerMissile.y = 0;
        playerMissile.enabled = false;

        //Init player position and score
        player.x = 10;
        player.y = 40;

        //randomSeed(analogRead(0));//Use unconnected analog pin to get random value

        prevUpdateTime = millis();
      }

      void drawAliens(){
        for (int i=0; i<NB_ALIENS; i++){
          if (aliens[i].lives>0){
            display.drawBitmap(aliens[i].x, aliens[i].y, alien_1_1, 8, 8, 1);
          }
        }
      }

      void moveAliens(){
        boolean moveDown = false;
        //Check if position needs switching
        for (int i=0; i<NB_ALIENS; i++){
          if (aliens[i].lives > 0){
            if ((aliens[i].x > 74) || aliens[i].x <2){
              aliensSpeedX = -aliensSpeedX;//Switch X direction
              moveDown = true;
              break;
            }
          }
        }

        //Move aliens one row down if needed
        if (moveDown == true){
          for (int i=0; i<NB_ALIENS; i++){
            aliens[i].y = aliens[i].y + aliensSpeedY;
            if (aliens[i].y > 34){
              die();
            }
          }
        }
        moveDown = false;

        //Move position of row of aliens left/right
        for (int i=0; i<NB_ALIENS; i++){
          aliens[i].x = aliens[i].x + aliensSpeedX;
        }
      }

      void moveMissiles(){
        if (alienMissile.enabled == false){
          //Generate new missile start position by selecting a random alien (actually, should check if it is dead first...)
          int alienNb = random(NB_ALIENS-1);
          alienMissile.x = aliens[alienNb].x + 3;//Missile starts in the "middle" of an alien
          alienMissile.y = aliens[alienNb].y + 8;//Missile starts just below alien
          alienMissile.enabled = true;
        }
        else {
          //Move missile down and remove if it reaches the ground
          alienMissile.y = alienMissile.y + alienMissile.speed;
          if (alienMissile.y>48){
            alienMissile.enabled = false;
          }
        }

        if (playerMissile.enabled == true){
          playerMissile.y = playerMissile.y - playerMissile.speed;
          if (playerMissile.y<0){
            playerMissile.enabled = false;
          }
        }
      }

      void drawMissiles(){
        if (alienMissile.enabled){
          display.drawRect(alienMissile.x, alienMissile.y, 2, 3, BLACK);
        }
        if (playerMissile.enabled){
          display.drawRect(playerMissile.x, playerMissile.y, 2, 3, BLACK);
        }
      }

      void drawPlayer(){
        display.drawBitmap(player.x, player.y, small_ship, 8, 8, 1);
      }

      #define PLAYER_SPEED_X  10
     // int dir = 0;
      int shoot = 0;
      void movePlayer(){

      //This block of codes is for the shift register.
     digitalWrite(load,LOW); //Set the loading pin to low to read all the buttons.
     delayMicroseconds(5); // wait 5 seconds.
     digitalWrite(load,HIGH);// Set the loading pin to hight to stop reading the buttons.
     delayMicroseconds(5); //wait 5 seconds.

     digitalWrite(clock,HIGH); // Set CLOCK PULSE to hight to activate the CLOCK PULSE signal.
     digitalWrite(enable,LOW); //set ENABLE to low to ENABLE the shift register.
     byte incoming=shiftIn(data,clock,MSBFIRST); //Saving all the bites to a byte variable call incoming stating from the MOST SIGNIFICANT BITES FIRST.
     digitalWrite(enable,HIGH); //Set ENABLE to high to disable the shift register.
        //===========================================================================================================

        if( previous != incoming ){ //Check if the previous bites read is not the same as the current bites.

            if( bitRead(incoming, 6) == 0){ //Check if the sixth bite from the 8 bites that was read is a ZERO.
                 player.x = player.x - PLAYER_SPEED_X;
                 //drawPlayer();
                 display.display(); //Display the ship to the screen with its current position.
              }

              if( bitRead(incoming, 7) == 0){//Check if the seventh bite from the 8 bites that was read is a ZERO.
                 player.x = player.x + PLAYER_SPEED_X;
                // drawPlayer();
                 display.display(); //Display the ship to the screen with its current position.
              }
              if( bitRead(incoming,0) == 0){//Check if the first bite from the 8 bites that was read is a ZERO.

                if (playerMissile.enabled == false){//Only one missile at a time can be active...
                    playerMissile.x = player.x + 3;
                    playerMissile.y = player.y;
                    playerMissile.enabled = true;
                  }

               display.display(); // We display everything on the screen as their current position keep changing.
              }
        }
      }

      void die(){
        display.setTextSize(1);
        display.setTextColor(BLACK);
        display.setCursor(12,20);
        display.println("Game over!");
        display.println("Score:");
        display.println(score);
        display.display();

        while (true){
          //Do nothing
        }
      }
