#include <Servo.h>

//https://github.com/hibit-dev/buzzer/blob/master/lib/pitches.zip
#include "pitches.h"

//https://github.com/SMFSW/Queue
#include "cppQueue.h"
#define	IMPLEMENTATION	FIFO

const int buzz_pin = 10;
const int CW = 2000;
const int CCW = 1000;
const int STOP = 1500;

Servo servos[8];

void setup(){
  for(int i = 0; i < 8; i++)
    servos[i].attach(i+2);
    //start from pin 2 because Arduino sends some signal to pins 0 and 1 during start up
}

// Arduino does not nativly supports c++ libs, and I didn't try to understand 
// how to .h files to do most of the work in normal C++
// this is why I included cppQueue.h and made a Pair struct
struct Pair{
  int first;
  int second;
};

struct Gate{
    // type can be L | I | T
    char type;
    // rot from 0 to 3 (90 deg rotations)
    int rot;
    // position :)
    Pair pos;
};

Gate gates[8];
bool maze[9][9];

void initGates(){
    for(int i = 0; i < 8; i++){
        gates[i].rot = random(0, 4);
    }
    gates[0].type = 'L';
    gates[0].pos.first = 2;
    gates[0].pos.second = 2;
    gates[1].type = 'I';
    gates[1].pos.first = 2;
    gates[1].pos.second = 4;
    gates[2].type = 'T';
    gates[2].pos.first = 2;
    gates[2].pos.second = 6;
    gates[3].type = 'I';
    gates[3].pos.first = 4;
    gates[3].pos.second = 2;
    gates[4].type = 'I';
    gates[4].pos.first = 4;
    gates[4].pos.second = 6;
    gates[5].type = 'T';
    gates[5].pos.first = 6;
    gates[5].pos.second = 2;
    gates[6].type = 'I';
    gates[6].pos.first = 6;
    gates[6].pos.second = 4;
    gates[7].type = 'L';
    gates[7].pos.first = 6;
    gates[7].pos.second = 6;
}

void initMaze(){
    for(int i = 0; i < 9; i++)
        for(int j = 0; j < 9; j++)
            maze[i][j] = 0;
    // if 1 -> can't go to cell
    for(int i = 0; i < 9; i++){
        maze[0][i] = 1;
        maze[i][0] = 1;
        maze[8][i] = 1;
        maze[i][8] = 1;
    }
}

//fill functions and insertGates are self explanatory
void fillI(int x, int y, int rot){
    maze[x][y] = 1;
    switch (rot) {
        case 0:
            maze[x - 1][y] = 1;
            break;
        case 1:
            maze[x][y + 1] = 1;
            break;
        case 2:
            maze[x + 1][y] = 1;
            break;
        case 3:
            maze[x][y - 1] = 1;
            break;
    }
}

void fillT(int x, int y, int rot){
    maze[x][y] = 1;
    switch (rot) {
        case 0:
            maze[x - 1][y] = 1;
            maze[x][y + 1] = 1;
            maze[x][y - 1] = 1;
            break;
        case 1:
            maze[x - 1][y] = 1;
            maze[x + 1][y] = 1;
            maze[x][y + 1] = 1;
            break;
        case 2:
            maze[x + 1][y] = 1;
            maze[x][y + 1] = 1;
            maze[x][y - 1] = 1;
            break;
        case 3:
            maze[x + 1][y] = 1;
            maze[x - 1][y] = 1;
            maze[x][y - 1] = 1;
            break;
    }
}

void fillL(int x, int y, int rot){
    maze[x][y] = 1;
    switch (rot) {
        case 0:
            maze[x - 1][y] = 1;
            maze[x][y + 1] = 1;
            break;
        case 1:
            maze[x + 1][y] = 1;
            maze[x][y + 1] = 1;
            break;
        case 2:
            maze[x + 1][y] = 1;
            maze[x][y - 1] = 1;
            break;
        case 3:
            maze[x - 1][y] = 1;
            maze[x][y - 1] = 1;
            break;
    }
}

void fillGate(int x, int y, char type, int rot){
    switch (type) {
        case 'I':
            fillI(x, y, rot);
            break;
        case 'L':
            fillL(x, y, rot);
            break;
        case 'T':
            fillT(x, y, rot);
            break;
        default:
            break;
    }
}

void insertGates(){
    for(auto& gate : gates){
        fillGate(gate.pos.first, gate.pos.second, gate.type, gate.rot);
    }
}

// basically BFS, retruns bool value of 7,7 cell, if true -> maze can be completed
bool walkTheMaze(){
    Pair start, curr, add;
    start.first = 1;
    start.second = 1;
    cppQueue q(sizeof(Pair), 10, IMPLEMENTATION);
    int x, y;
    bool visited[9][9] = {};
    q.push(&start);
    while (!q.isEmpty()){
        q.pop(&curr);
        x = curr.first;
        y = curr.second;
        visited[x][y] = 1;
        if(!maze[x + 1][y] && !visited[x + 1][y]){
            add.first = x + 1;
            add.second = y;
            q.push(&add);
        }
        if(!maze[x][y + 1] && !visited[x][y + 1]){
            add.first = x;
            add.second = y + 1;
            q.push(&add);
        }
        if(!maze[x - 1][y] && !visited[x - 1][y]){
            add.first = x - 1;
            add.second = y;
            q.push(&add);
        }
        if(!maze[x][y - 1] && !visited[x][y - 1]){
            add.first = x;
            add.second = y - 1;
            q.push(&add);
        }
    }
    return visited[7][7];
}

// applyes rotation 
// all numbers were found with try-error-repeat method, because .write() works in mysterious ways for 360* servos
// could not find propper delay for 270* so it is implemented as negative 90* rotation
void rotate(int degrees, bool clockwise, int i){
    int ms = clockwise ? CW : CCW;
    switch(degrees){
        case 90:
            servos[i].write(ms);
            delay(300);
            servos[i].write(STOP);
            break;
        case 180:
            servos[i].write(ms);
            delay(500);
            servos[i].write(STOP);
            break;
        case 270:
            ms = clockwise ? CCW : CW;
            servos[i].write(ms);
            delay(300);
            servos[i].write(STOP);
            break;
    }
}

//https://www.ninsheetmusic.org/download/pdf/1464
int melody[] = {
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5,
  NOTE_E4, NOTE_G4, NOTE_A4, NOTE_B4, NOTE_C5,
  NOTE_D5, NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_AS4, NOTE_C5, NOTE_D5,
  NOTE_DS5, NOTE_FS4, NOTE_G4, NOTE_A4, NOTE_AS4, NOTE_C5, NOTE_D5,
  NOTE_DS5, NOTE_F4, NOTE_F4, NOTE_F4
};

int noteDurations[] = {
  200, 200, 200, 200, 200,
  200, 200, 200, 200, 200,
  200, 200, 200, 200, 200, 200, 200,
  200, 200, 200, 200, 200, 200, 200,
  200, 400, 200, 800
};

// it does music, it should sound like The Legend of Zelda chest opening sound, but I do not have a musical degree, so it will do.
void mazeMusic(){ 
    int size = sizeof(noteDurations) / sizeof(int);
    for(int note = 0; note < size; note++){
        tone(buzz_pin, melody[note], noteDurations[note]);
        delay(150);
        noTone(buzz_pin);
    }
}

void loop(){
    //mazeMusic();
    do{ //function names speak for them selfes, nothing to explain
        initMaze();
        initGates();
        insertGates();
    } while (!walkTheMaze());
    for(int i = 0; i < 8; i++) // apply rotation to gates
        rotate(gates[i].rot * 90, true, i);
    // regenerate the maze, so it won't "print" same maze again
    initMaze();
    initGates();
    insertGates();
    delay(1000*10);
    for(int i = 0; i < 8; i++) // reverse rotation
        rotate(gates[i].rot * 90, false, i);
    delay(1000*10);
}
