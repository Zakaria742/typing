#ifndef UTILS_H
#define UTILS_H


#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "info.h"
#include <time.h>
char error_buf[512];

struct termios origin;
termInfo terminal = {0};
info Tinfo = {0};
configuration conf = {0};
missed_positions mis_pos = {0};
void die(const char *str, error_print_mode mode){
  switch(mode){
    case NORMAL:
      fputs(str, stderr);
      break;
    default:
      perror(str);
      break;
  }
  exit(-1);
}

void disable_raw_mode(void){
  if(tcsetattr(STDOUT_FILENO, TCSAFLUSH, &origin) == -1) die("tcsetattr", PERROR);
}

void raw_mode(){
  tcgetattr(STDIN_FILENO, &origin);
  atexit(disable_raw_mode);
  struct termios raw = origin;
  raw.c_oflag &= ~( OPOST | ONLCR );
  raw.c_iflag &= ~(IGNBRK | IGNCR | ICRNL | IXOFF);
  raw.c_lflag &= ~(ECHO | IEXTEN |  ICANON | ISIG);
  raw.c_cc[VMIN] = 0;
  raw.c_cc[VTIME] = 1;

  tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw);
}

void init_info(){
  Tinfo.right = 1;
  Tinfo.down = 1;
  Tinfo.current_letter = 0;
  Tinfo.time = 0;
  Tinfo.success = 0;
  Tinfo.mistakes = 0;
  Tinfo.running = 1;
  Tinfo.sentence_len = 0;
  Tinfo.first_time = 0;
  Tinfo.input_letter = '\0';
  Tinfo.max_lines = conf.lines;
  Tinfo.max_characters = Tinfo.max_lines * terminal.cols;
  Tinfo.current_chunk = 0;
}

void init_config(){
  conf.preview = 1;
  conf.live_mistakes = 1;
  conf.print_mistakes = 1;
  strcpy(conf.transparency, "");
  for(int i = 0; i < 3; i++){
    conf.color[i] = 0;
    conf.cursor_color[i] = 0;
    conf.letter_color[i] = 0;
    conf.error_color[i] = 0;
  }
  conf.lines = 4;
  conf.mode = SIMPLE;
  conf.time = 0.0f;
  memset(conf.path_sentences, ' ', 100);
}

void init_missed_buffer(){
  mis_pos.character = (char*)calloc(Tinfo.sentence_len, sizeof(char));
  mis_pos.x = (int*)calloc(Tinfo.sentence_len, sizeof(int));
  mis_pos.y = (int*)calloc(Tinfo.sentence_len, sizeof(int));
}

void free_missed_buffer(){
  free(mis_pos.character);
  free(mis_pos.x);
  free(mis_pos.y);
}

void getTinfo(){
  struct winsize w;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
  terminal.cols = w.ws_col;
  terminal.rows = w.ws_row;
}

void prompt(bool *running){
  disable_raw_mode();
  char c;
  int length = Tinfo.max_lines + 1;
  printf("\x1b[33m\x1b[%d;0f\nAre you sure you want to quit?\x1b[0m\r\n", length);
  read(STDIN_FILENO, &c, 1);
  if(c == 'y') *running = 0;
  else system("clear");
  raw_mode();
}

void print_info(char *sentence){
  int length = Tinfo.max_lines + 4;
  printf("\x1b[%d;0fTime : %0.1f\r\n" , length, Tinfo.time);
  printf("Current letter : %c\r\n", sentence[Tinfo.current_letter]);
  printf("Letter given : %c\r\n", Tinfo.input_letter);
}

int check_answer(char letter, char current_letter){
  return (letter == current_letter) ? 1 : 0;
}
void init_cursor(){
  Tinfo.right = 1;
  Tinfo.down = 1;
}

char *strtolow(char *str){
  for(int i = 0; str[i] != '\0'; i++){
    str[i] = tolower(str[i]);
  }
  return str;
}
char **create_chunks(char *sentence){
  int cols = Tinfo.max_characters;
  int rows = Tinfo.sentence_len / cols;
  char **chunks = (char**)calloc(rows + 1, sizeof(char*));
  for(int i = 0; i < rows; i++){
    chunks[i] = (char*)calloc(cols+1, sizeof(char));
    memcpy(chunks[i], &sentence[cols*i], cols);
    chunks[i][cols] = '\0';
  }
  int left_over = Tinfo.sentence_len - rows*cols;
  chunks[rows] = (char*)calloc(left_over+1, sizeof(char));
  memcpy(chunks[rows], &sentence[cols*rows], left_over);
  chunks[rows][left_over] = '\0';
  return chunks;
}
void print_mistakes(char *sentence, int mode, int h){
  int height = h;
  int length = 0;
  int start = 0;
  switch(mode){
    case 0:
      length = Tinfo.current_letter;
      if(Tinfo.current_chunk == 0) start = 0;
      else start = ((Tinfo.current_chunk)* Tinfo.max_characters + 1);
      for(int i = start; i < length; i++){
        printf("\x1b[%d;%df\x1b[48;2;%hu;%hu;%hum%s%c\x1b[0m", mis_pos.y[i]+height-(Tinfo.current_chunk), mis_pos.x[i], conf.error_color[0], conf.error_color[1], conf.error_color[2], conf.transparency,mis_pos.character[i]);
      }
      break;
    case 1:
      length = Tinfo.current_letter;
      start = 0;
      for(int i = start; i < length; i++){
        if(mis_pos.character[i] == '\0')  printf("\x1b[%d;%df%s%c\x1b[0m", mis_pos.y[i]+height, mis_pos.x[i], conf.transparency,sentence[i]);
        else printf("\x1b[%d;%df\x1b[48;2;%hu;%hu;%hum%s%c\x1b[0m", mis_pos.y[i]+height, mis_pos.x[i], conf.error_color[0], conf.error_color[1], conf.error_color[2], conf.transparency,mis_pos.character[i]);
      }
      puts("\r\n");
      break;
  }

}
void update(char c, char *sentence){
  //Updating the columns values inside of the termInfo struct variable "terminal"
  getTinfo();

  switch(c){
    case '\x7f': //0x7f is 'BACKSPACE' in hex
      if(Tinfo.right*Tinfo.down > 1)
      {Tinfo.right--;
        Tinfo.current_letter--;}
      printf("\x1b[%d;%df%s%c\x1b[0m", Tinfo.down, Tinfo.right, conf.transparency, sentence[Tinfo.current_letter]);
      break;
    case '*':
      prompt(&(Tinfo.running));
      break;
    default:
      break;
  }

  for(int i = 0; i < 3; i++){
    conf.color[i] = conf.cursor_color[i];
  }
  //Checking if the input letter is correct
  if(check_answer(c, sentence[Tinfo.current_letter])){
    if(c == ' ' && Tinfo.first_time) {
      Tinfo.success++;
    }
    Tinfo.right++;
    Tinfo.current_letter++;
    Tinfo.first_time = 1;
  }
  else if( c != '\0' && c != '*' && Tinfo.first_time){
    Tinfo.mistakes++;
    Tinfo.first_time = 0;
    mis_pos.character[Tinfo.current_letter] = sentence[Tinfo.current_letter];
    for(int i = 0; i < 3; i++) conf.color[i] = conf.error_color[i];

  }
  mis_pos.x[Tinfo.current_letter] = Tinfo.right;
  mis_pos.y[Tinfo.current_letter] = Tinfo.down + Tinfo.current_chunk;
  if(Tinfo.right*Tinfo.down > Tinfo.max_characters){
    Tinfo.current_chunk++;
    init_cursor();
    system("clear");
  }

  //Terminal Bounds checking
  if(Tinfo.right < 1 && Tinfo.down >= 1) {
    Tinfo.down--;
    Tinfo.right= terminal.cols;
  }
  else if(Tinfo.right > terminal.cols){
    Tinfo.down += 1;
    Tinfo.right = 1;
  }



  //Checking for the end of the sentence
  if(Tinfo.current_letter >= Tinfo.sentence_len || Tinfo.time < 0){
    Tinfo.running = 0;
    Tinfo.success++;
  }

}

//Reading the file
char *read_file(const char *path){
  FILE *file = fopen(path, "r");
  if(file == NULL){
    printf("\x1b[38;2;200;0;0mError opening the file '%s'\x1b[0m\r\n", path);
    die("fopen", PERROR);
  }
  fseek(file, 0, SEEK_END);
  int size = ftell(file);
  rewind(file);
  char *sentence = (char*)calloc(size+1, sizeof(char));
  fread(sentence, sizeof(sentence), size, file);
  sentence[size] = '\0';
  fclose(file);
  return sentence;
}

//Converting hex to integer
int htoi(char character){
  if(ispunct(character)){
    snprintf(error_buf, 71, "\x1b[38;2;210;40;0m[config_error]\x1b[0m '%c' is an invalid hex character\n", character);
    die(error_buf, NORMAL);
  }
  if((int)character - 87 < 0){
    return (int)(character)-48;
  }
  return (int)character - 87;
}

//Reading the config file
void check_configs(char *path){
  char *config = read_file(path);
  char *temp = strtok(config, " \n");
  char config_array[22][100] = {0};
  int i = 0;
  while(temp != NULL){
    strncpy(config_array[i++], temp, strlen(temp)%100);
    temp = strtok(NULL, " \n");
  }
  for(int i = 0; i < 22; i+=2){
    if(strcmp(config_array[i], "cursor_color") == 0){
      if(strlen(config_array[i+1]) != 7){
        printf("\x1b[38;2;255;0;0m\"%s\"\n", config_array[i+1]);
        die("Invalid rgb value\x1b[0m\n", NORMAL);
      }
      char *temp = &config_array[i+1][1];
      for(int i = 0; i < 3; i++){
        conf.cursor_color[i] = htoi(temp[i*2])*16 + htoi(temp[(i*2)+1]);
      }
    }
    else if(strcmp(config_array[i], "lines") == 0){
      conf.lines = atoi(config_array[i+1]);
    }
    else if(strcmp(config_array[i], "file_name") == 0){
      if(config_array[i+1][0] != '/'){
        ;
        snprintf(error_buf, 71 + strlen(config_array[i+1]), "\x1b[38;2;210;40;0m[config_error]\x1b[0m '%s' is an invalid words file path\nUse the following format /home/user/path/to/file\n", config_array[i+1]);
        die(error_buf, NORMAL);

      }
      strncpy(conf.path_sentences, config_array[i+1], 100);
    }
    else if(strcmp(config_array[i], "preview") == 0){
      if(strcmp(strtolow(config_array[i+1]), "false") == 0) conf.preview = 0;
    }
    else if(strcmp(config_array[i], "default_mode") == 0){
      if(strcmp(strtolow(config_array[i+1]), "timed") == 0) conf.mode = TIMED;
      else if(strcmp(strtolow(config_array[i+1]), "simple") == 0) conf.mode = SIMPLE;
      else{
        snprintf(error_buf, 71 + strlen(config_array[i+1]), "\x1b[38;2;210;40;0m[config_error]\x1b[0m '%s' mode is inknown\n", config_array[i+1]);
        die(error_buf, NORMAL);
      }
    }
    else if(strcmp(config_array[i], "time") == 0){
      conf.time = atof(config_array[i+1]);
    }
    else if(strcmp(config_array[i], "letter_color") == 0){
      if(strlen(config_array[i+1]) != 7){
        printf("\x1b[38;2;255;0;0m\"%s\"\n", config_array[i+1]);
        die("Invalid rgb value\x1b[0m\n", NORMAL);
      }
      char *temp = &config_array[i+1][1];
      for(int i = 0; i < 3; i++){
        conf.letter_color[i] = htoi(temp[i*2])*16 + htoi(temp[(i*2)+1]);
      }

    }
    else if(strcmp(config_array[i], "error_color") == 0){
      if(strlen(config_array[i+1]) != 7){
        printf("\x1b[38;2;255;0;0m\"%s\"\n", config_array[i+1]);
        die("Invalid rgb value\x1b[0m\n", NORMAL);
      }
      char *temp = &config_array[i+1][1];
      for(int i = 0; i < 3; i++){
        conf.error_color[i] = htoi(temp[i*2])*16 + htoi(temp[(i*2)+1]);
      }

    }
    else if(strcmp(config_array[i], "live_mistakes") == 0){
      if(strcmp(strtolow(config_array[i+1]), "false") == 0) conf.live_mistakes = 0;
    }else if(strcmp(config_array[i], "print_mistakes") == 0){
      if(strcmp(strtolow(config_array[i+1]), "false") == 0) conf.print_mistakes = 0;
    }
    else if(strcmp(config_array[i], "transparency") == 0){
      if(strcmp(config_array[i+1], "true") == 0){
        strcpy(conf.transparency, "\x1b[2m");
      }
    }
    else{
      snprintf(error_buf, 71 + strlen(config_array[i+1]), "\x1b[38;2;210;40;0m[config_error]\x1b[0m '%s' config keyword is unknown\n", config_array[i]);
      die(error_buf, NORMAL);

    }

  }
  free(config);
}


//Removing /n for better readablity
void format_sentence(char *sentence){
  for(int i = 0; sentence[i] != '\0'; i++){
    if(sentence[i] == '\n' || sentence[i] == '\r') sentence[i] = ' ';
  }
}


//The main loop
void main_loop(char **chunk, char *main_sentence, mode m){
  system("clear");
  char c = '\0';

  long int old_time = time(0);
  int given_timer = 0;
  if(m == TIMED){
    given_timer = Tinfo.time;
  }

  setvbuf(stdout,  NULL, _IONBF, 0);
  raw_mode();
  while(read(STDIN_FILENO, &c, 1) != -1 && Tinfo.running){
    update(c, main_sentence);
    Tinfo.input_letter = c;
    c = '\0';
    printf("\x1b[H%s%s\x1b[0m\r\n", conf.transparency, chunk[Tinfo.current_chunk]);
    if(conf.live_mistakes){
    print_mistakes(chunk[Tinfo.current_chunk], 0, 0);
    }
    printf("\x1b[%d;%df", Tinfo.down, Tinfo.right);

    printf("\x1b[48;2;%hu;%hu;%hum\x1b[1m%c\x1b[0m", conf.color[0], conf.color[1], conf.color[2],  main_sentence[Tinfo.current_letter]);

    print_info(main_sentence);
    if(m == TIMED){
      Tinfo.time = (old_time + given_timer - time(0));
    }else{
      Tinfo.time = time(0) - old_time;
    }
  }

}


#endif
