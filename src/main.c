#include "../libs/utils.h"

int main(int argc, char **argv){

  getTinfo();
  init_config();

  system("mkdir -p ~/.config/typing");
  check_configs("/home/aira/.config/typing/config.conf");
  init_info();

  if(argc > 2){
    if(strcmp(argv[1], "timed") == 0)
    {
      conf.mode = TIMED;
      conf.time = atof(argv[2]);
    }
    else conf.mode = SIMPLE;
  }

  char c = '\0'; 
  char *sentence = read_file(conf.path_sentences);
  format_sentence(sentence);
  Tinfo.sentence_len = strlen(sentence);

  init_missed_buffer();

  if(conf.preview){
    printf("The paragraph : %s\n\n", sentence);
  }
  printf("Press 'Enter' on the keyboard to start\n");
  if(read(STDIN_FILENO, &c, 1) != -1 && c == '\n'){

    char **chunks = create_chunks(sentence);

    if(conf.mode == TIMED) Tinfo.time = conf.time;
    else Tinfo.time = 0;

    main_loop(chunks, sentence, conf.mode);

    int rows = Tinfo.sentence_len / Tinfo.max_characters;
    for(int i = 0; i <= rows; i++){
      free(chunks[i]);
    }
    free(chunks);
  }else{
    puts("Exiting...\n");
    return 0;
  }
  printf("\r\nSuccess : %d\r\nMistakes : %d\r\n", Tinfo.success, Tinfo.mistakes);
  if(conf.mode == TIMED)printf("Your WPM is : %d\r\n", (int)((Tinfo.success)/((conf.time-Tinfo.time)/60)));
  else printf("Your WPM is : %d\r\n", (int)((Tinfo.success)/(Tinfo.time/60)));
  if(conf.print_mistakes) print_mistakes(sentence, 1, 30);
  free(sentence);
  free_missed_buffer();
  return 0;
}
