#ifndef _INFO_H
#define _INFO_H

typedef enum {
  NORMAL = 10,
  PERROR
}error_print_mode;

typedef enum{
  SIMPLE = 20,
  TIMED
}mode;

typedef struct Programinfo{
  bool first_time;
  bool running;
  char input_letter;
  int right;
  int down;
  int current_letter;
  int success;
  int mistakes;
  int max_lines;
  int max_characters;
  int current_chunk;
  long int sentence_len;
  float time;
}info;

typedef struct terminalInfo{
  int cols;
  int rows;
}termInfo;

typedef struct configuration_file{
  _Bool preview;//1
  _Bool live_mistakes;//1
  _Bool print_mistakes;//1
  char transparency[4];//4
  int lines;//4
  mode mode;// 4
  float time; //4
  unsigned short color[3];//6
  unsigned short cursor_color[3];//6
  unsigned short letter_color[3];//6
  unsigned short error_color[3];//6
  char path_sentences[100]; //100

}configuration;

typedef struct mistakes_position{
  char *character;
  int *x;
  int *y;
}missed_positions;

#endif
