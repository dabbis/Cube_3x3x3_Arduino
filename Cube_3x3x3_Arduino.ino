// Pins
const int layers[9] = {4,3,2,A3,A4,A5,A0,A1,A2};
const int LEDS[9] = {13,12,11,10,9,8,7,6,5};

//The cube is 9 layers (0-8)
unsigned volatile char layer = 0;

// Flag that indicates a new frame. 
unsigned volatile char updateFrame = 0;

// Speed defines how often a new frame is created from a pattern.
// default 1, is 1/10 of a second.
// when speed  10, 10 * 1/10 = 1 second.
unsigned volatile char speed = 1;
unsigned volatile char counter = 0;

// Counter for the PWM 0-15
unsigned volatile char pwm = 0;

// Pointers in buffer for PWM
unsigned volatile char ledZ = 0;
unsigned volatile char ledY = 0;

// Buffer, current frame being shown on the cube.
unsigned volatile char box[3][9] = {{0}};

// The Cube updates @ 50 FPS
// LED has 4bit level of brightness 0-15
// LED fully on has a duty cycle 100/9 = 11,1%
// Because each layer is multiplexed.

// Timer 1 output compare A interrupt service routine.
// Determines when new frame is generated.
ISR(TIMER1_COMPA_vect){
  counter++;
  if(counter >= speed){        
    counter = 0;
    if (updateFrame == 0){
      updateFrame = 1;
    }
  }	
}

// Timer 2 output compare interrupt service routine.
// Controls layer multiplexing and PWM.
ISR(TIMER2_COMPA_vect){
  // PWM current layer. count 0-15 
  // Then change layer. count 0-8
  pwm++;
  if(pwm >= 15){
    pwm = 0;
     
    layer++;
    if(layer >= 9){
      layer = 0;
    }

    ledZ++;
    if(ledZ >= 3){
      ledZ = 0;
      ledY += 3;
      if(ledY >= 9)
      {
        ledY = 0;
      }
    }

    // Control the layers
    if(layer==0){
      digitalWrite(layers[8],HIGH);
      digitalWrite(layers[0],LOW);
    }else{
      digitalWrite(layers[layer-1],HIGH);
      digitalWrite(layers[layer],LOW);
    }
  }

  // Control the PWM
  for(int i = 0;i<3;i++){
    if(box[ledZ][ledY+i] <= pwm){
      digitalWrite(LEDS[ledY+i],LOW);
    }else{
      digitalWrite(LEDS[ledY+i],HIGH);
    }
  }
}

// All LED's with /power/ strength, 0 = off, 15 = on
void all(unsigned char power){

  unsigned char i = 0;
  unsigned char k = 0;

  while(i<3){
    while(k<9){
      box[i][k] = power;
      k++;
    }
    k = 0;
    i++;
  }
}

// Rain randomly from top to bottom /drops/ number of rain-drops.
void rain(unsigned char drops){

  all(0); //turn all LED OFF

  unsigned char k = 0;
  unsigned char i = 1;
  unsigned char t = 0;
  unsigned char drop = 0;


  while(drops >= drop){

    if(updateFrame == 1){

      if(i == 1){
        box[i][k] = 15;
        box[i+1][k] = 0;
        box[i-1][t] = 0;
        i = 0;
      }
      else if(i == 0){
        box[i][k] = 15;
        box[i+1][k] = 0;

        t = k;
        while(t == k){
          k = rand() %9;
        }

        box[i+2][k] = 15;
        i = 1;
        drop++;
      }
      updateFrame = 0;
    }
  }
  box[i+1][k] = 0;
}

// Rain randomly from top to bottom and build up.
void rainBuild(){

  all(0); //turn all LED OFF

  unsigned char k = 0;
  unsigned char i = 2;
  unsigned char j;
  unsigned char sum0;
  unsigned char sum1;
  unsigned char sum2;


  while(sum2 < 135){

    if(updateFrame == 1){

      for(j = 0, sum0 = 0; j<=8; j++){
        sum0 += box[0][j];
      }

      for(j = 0, sum1 = 0; j<=8; j++){
        sum1 += box[1][j];
      }

      for(j = 0, sum2 = 0; j<=8; j++){
        sum2 += box[2][j];
      }

      if(i == 2){

        while(((sum0 < 135) & (box[0][k] == 15)) | 
          ((sum1 < 135) & (box[1][k] == 15)) | 
          (box[2][k] == 15)){
          k = rand() %9;
        }
        box[i][k] = 15;
        i = 1;
      }
      else if(i == 1){
        if(sum1 < 135){
          box[i+1][k] = 0;
          box[i][k] = 15;
        }
        i = 0;
      }
      else if(i == 0){
        if(sum0 < 135){
          box[i+1][k] = 0;
          box[i][k] = 15;
        }
        i = 2;
      }
      updateFrame = 0;
    }
  }
}

// Dims/Illuminate all the LED's /times/ times in the /order/ order.
// 0 = down
// 1 = up
// 2 = up and down
// 3 = down and up.
void glow(unsigned char times, unsigned char order){

  unsigned char time = 0;
  unsigned char dir;
  unsigned char glower;
  unsigned char count;

  if((order == 1) | (order == 2)){
    dir = 1;
    glower = 0;
  }
  else if((order == 0) | (order == 3)){
    dir = 0;
    glower = 15;
  }

  if(order<=1){
    count = times*15;
  }
  else{
    count = times*30;
  }

  while(count>=time){

    if (updateFrame == 1){
      all(glower);
      updateFrame = 0;
      time++;

      if(glower <= 0){

        if(order == 0){
          glower = 15;
        }
        else{
          dir = 1;
        }
      }

      if(glower >= 15){

        if(order == 1){
          glower = 0;
        }
        else{
          dir = 0;
        }
      }

      if(dir == 1){
        glower++;
      }

      if(dir == 0){
        glower--;
      }
    }
  }
}


// Firework
void firework(unsigned char times, unsigned char s){
  
  const static unsigned char pattern[8][3][9] = 
  {{{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}},
  {{0,0,0,0,15,0,0,0,0},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}},
  {{0,0,0,0,1,0,0,0,0},{0,0,0,0,15,0,0,0,0},{0,0,0,0,0,0,0,0,0}},
  {{0,0,0,0,0,0,0,0,0},{0,0,0,0,1,0,0,0,0},{0,0,0,0,15,0,0,0,0}},
  {{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{15,15,15,15,1,15,15,15,15}},
  {{0,0,0,0,0,0,0,0,0},{15,15,15,15,0,15,15,15,15},{1,1,1,1,0,1,1,1,1}},
  {{15,15,15,15,0,15,15,15,15},{1,1,1,1,0,1,1,1,1},{0,0,0,0,0,0,0,0,0}},
  {{1,1,1,1,0,1,1,1,1},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}}};  

  unsigned char i = 0;
  unsigned char k = 0;
  unsigned char j = 0;
  unsigned char count = 0;
  speed = s;

  all(0); //turn all LED OFF

  while(times*8 > count){
    if (updateFrame == 1){
      i = 0;
      while(i<3){
        while(k<9){
          box[i][k] = pattern[j][i][k];
          k++;
        }
        k = 0;
        i++;	
      }
      updateFrame = 0;
      count++;
      j++;
      if(j >7){
        j = 0;
      }
    }
  }
  speed = 1;
}


// Fade down from top to bottom.
void fadeDown(unsigned char times){

  unsigned char i = 2;
  unsigned char k = 0;
  unsigned char q = 15;
  unsigned char count = 0;

  all(15);

  while(times*48 > count){

    if (updateFrame == 1){

      while(k<9){
        box[i][k] = q;
        k++;
      }

      if(q == 0){
        i--;
        q = 15;
      }
      q--;
      k = 0;
      count++;
      updateFrame = 0;
    }
  }
}


// Iterates over all the leds start from [0][0] to [3][8] with a /style/ style.
// 0 = Around the world: All off in the beginning, turn on from bottom.
// 1 = Build up: All off in the beginning, turn on from bottom and stay on.
// 2 = Death from below: All on in the beginning, turn off from bottom.
void iterator(unsigned char times, unsigned char style){

  unsigned char i = 0;
  unsigned char k = 0;
  unsigned char q = 0;
  unsigned char count = 0;

  while(times*27 > count){

    if (updateFrame == 1){

      i = 0;
      while(i<3){
        while(k<9){
          if(style == 0){
            if (k+(i*9) == q){
              box[i][k] = 15;
            }
            else{
              box[i][k] = 0;			
            }
          }
          else if(style == 1){
            if (k+(i*9) >= q ){
              box[i][k] = 0;
            }
            else{
              box[i][k] = 15;			
            }
          }
          else if(style == 2){
            if (k+(i*9) >= q){
              box[i][k] = 15;
            }
            else{
              box[i][k] = 0;			
            }
          }
          k++;
        }
        k = 0;
        i++;
      }
      updateFrame = 0;
      count++;

      q++;
      if(q == 27){
        q = 0;
      }
    }
  }
}

// Patterns.
// 0 = floor - middle1 - top - middle
// 1 = left - middle2 - right - middle2
// 2 = front - middle3 - back - middle3
// 3 = middle3 - corner1 - middle2 - corner2 (clockwise)
// 4 = middle3 - corner2 - middle2 - corner1 (anticlockwise)
// 5 = even - odd - even - odd
// 6 = front - left - back - right
void pattern(unsigned char times, unsigned char pattern){

  const static unsigned char patterns[13][3][9] = 
  {{{15,15,15,15,15,15,15,15,15},{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0}}, //0 floor
  {{0,0,0,0,0,0,0,0,0},{15,15,15,15,15,15,15,15,15},{0,0,0,0,0,0,0,0,0}}, //1 middle1
  {{0,0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0,0},{15,15,15,15,15,15,15,15,15}}, //2 top
  {{15,15,15,0,0,0,0,0,0},{15,15,15,0,0,0,0,0,0},{15,15,15,0,0,0,0,0,0}}, //3 left
  {{0,0,0,15,15,15,0,0,0},{0,0,0,15,15,15,0,0,0},{0,0,0,15,15,15,0,0,0}}, //4 middle2	
  {{0,0,0,0,0,0,15,15,15},{0,0,0,0,0,0,15,15,15},{0,0,0,0,0,0,15,15,15}}, //5 right
  {{0,0,15,0,0,15,0,0,15},{0,0,15,0,0,15,0,0,15},{0,0,15,0,0,15,0,0,15}}, //6 back
  {{0,15,0,0,15,0,0,15,0},{0,15,0,0,15,0,0,15,0},{0,15,0,0,15,0,0,15,0}}, //7 middle3
  {{15,0,0,15,0,0,15,0,0},{15,0,0,15,0,0,15,0,0},{15,0,0,15,0,0,15,0,0}}, //8 front
  {{0,0,15,0,15,0,15,0,0},{0,0,15,0,15,0,15,0,0},{0,0,15,0,15,0,15,0,0}}, //9 corner1
  {{15,0,0,0,15,0,0,0,15},{15,0,0,0,15,0,0,0,15},{15,0,0,0,15,0,0,0,15}}, //10 corner2
  {{0,15,0,15,0,15,0,15,0},{15,0,15,0,15,0,15,0,15},{0,15,0,15,0,15,0,15,0}}, //11 even
  {{15,0,15,0,15,0,15,0,15},{0,15,0,15,0,15,0,15,0},{15,0,15,0,15,0,15,0,15}}}; //12 odd  

  unsigned char buffer[4];
  unsigned char i = 0;
  unsigned char k = 0;
  unsigned char j = 0;
  unsigned char count = 0;

  if(pattern == 0){
    buffer[0] = 0;
    buffer[1] = 1;
    buffer[2] = 2;
    buffer[3] = 1;
  }
  else if(pattern == 1){
    buffer[0] = 3;
    buffer[1] = 4;
    buffer[2] = 5;
    buffer[3] = 4;
  }
  else if(pattern == 2){
    buffer[0] = 8;
    buffer[1] = 7;
    buffer[2] = 6;
    buffer[3] = 7;
  }
  else if(pattern == 3){
    buffer[0] = 7;
    buffer[1] = 9;
    buffer[2] = 4;
    buffer[3] = 10;
  }
  else if(pattern == 4){
    buffer[0] = 7;
    buffer[1] = 10;
    buffer[2] = 4;
    buffer[3] = 9;
  }
  else if(pattern == 5){
    buffer[0] = 11;
    buffer[1] = 12;
    buffer[2] = 11;
    buffer[3] = 12;
  }
  else if(pattern == 6){
    buffer[0] = 8;
    buffer[1] = 3;
    buffer[2] = 6;
    buffer[3] = 5;
  }

  while(times*4 > count){

    if (updateFrame == 1){
      all(0); //turn all LED OFF
      i = 0;
      while(i<3){
        while(k<9){
          box[i][k] = patterns[buffer[j]][i][k];
          k++;
        }
        k = 0;
        i++;	
      }
      updateFrame = 0;
      count++;
      j++;
      if(j >3){
        j = 0;
      }
    }
  }
}


// A worm that goes randomly through the cube without eating himself 
// makes /steps/ number of steps.
// goes at the /s/ speed 10/s dot s
void worm(unsigned char steps, unsigned char s){

  unsigned char worm[5][3] = {{0}}; // Worm is 5 "dots" long
  unsigned char test[3];
  unsigned char i;
  unsigned char k;
  unsigned char pm;
  unsigned char dir;
  unsigned char collision;
  unsigned char tail;
  unsigned char count = 0;

  // Set the startup position.
  worm[0][0] = rand()%3; //X
  worm[0][1] = rand()%3; //Y
  worm[0][2] = rand()%3; //Z

  // Set the speed of the worm.
  speed = s;

  all(0); //turn all LED OFF

  while(steps >= count){

    if (updateFrame == 1){

      collision = 1;
      while(collision){
        collision = 0;

        // Load the head coordinates into test.
        test[0] = worm[0][0]; //X
        test[1] = worm[0][1]; //Y
        test[2] = worm[0][2]; //Z


        // Choose Random direction.
        // 0 = X
        // 1 = Y
        // 2 = Z
        dir = rand()%3;

        // Choose +/- on the axis
        pm = rand()%2;

        if(pm){
          test[dir]++;
        }
        else{
          test[dir]--;
        }

        // Check if the point is out of the cube.
        if(test[dir] > 2){
          collision = 1;
        }

        // Check if the  point intersects with the tail of the worm.
        if(((test[0] == worm[1][0]) && (test[1] == worm[1][1]) && (test[2] == worm[1][2])) ||
          ((test[0] == worm[2][0]) && (test[1] == worm[2][1]) && (test[2] == worm[2][2])) ||
          ((test[0] == worm[3][0]) && (test[1] == worm[3][1]) && (test[2] == worm[3][2])) ||
          ((test[0] == worm[4][0]) && (test[1] == worm[4][1]) && (test[2] == worm[4][2]))){
          collision = 1;
        }
      }

      // Update the tail.
      for(tail = 4; tail>0; tail--){
        worm[tail][0] = worm[tail-1][0];
        worm[tail][1] = worm[tail-1][1];
        worm[tail][2] = worm[tail-1][2];
      }

      // Update the head.
      worm[0][0] = test[0];
      worm[0][1] = test[1];
      worm[0][2] = test[2];

      // Display the worm.
      for(int n = 0; n<5; n++){
        i = worm[n][2];
        k = worm[n][1] + worm[n][0]*3;

        if(n == 0){
          box[i][k] = 15;
        }
        if(n == 1){
          box[i][k] = 10;
        }
        if(n == 2){
          box[i][k] = 7;
        }
        if(n == 3){
          box[i][k] = 5;
        }
        if(n >= 4){
          box[i][k] = 0;
        }
      }
      count++;
      updateFrame = 0;
    }
  }
  speed = 1;
}


void setup(){
  
  // Initialize pins
  for (int i = 0; i<9; i++)  {
    pinMode(layers[i], OUTPUT);
    pinMode(layers[i],HIGH);
    pinMode(LEDS[i], OUTPUT);
    pinMode(LEDS[i],HIGH);
  }
  
  // Timer/Counter 1 initialization (~10 Hz)
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 1hz increments
  OCR1A = 1562;// = (16*10^6) / (10*1024) - 1 (must be <65536)
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS10 and CS12 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  // Timer/Counter 2 initialization (~7 kHz)
  TCCR2A = 0;// set entire TCCR2A register to 0
  TCCR2B = 0;// same for TCCR2B
  TCNT2  = 0;//initialize counter value to 0
  // set compare match register for 8khz increments
  OCR2A = 8;// = (16*10^6) / (7000*256) - 1 (must be <256)
  // turn on CTC mode
  TCCR2A |= (1 << WGM21);
  // Set CS21 bit for 256 prescaler
  TCCR2B |= (1<<CS22) | (1 << CS21);   
  // enable timer compare interrupt
  TIMSK2 |= (1 << OCIE2A);

  interrupts();
}

void loop(){

  //all(16);
  iterator(1,1);  //build up
  glow(1,0);	  //down

  rain(20);
  rainBuild();
  iterator(1,2); //Death from below

  pattern(2,2);
  pattern(2,1);
  pattern(2,0);
  pattern(2,6);
  pattern(3,5);
  fadeDown(1);

  firework(2,2);
  worm(70,2);

}



