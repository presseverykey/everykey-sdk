#ifndef _QUESTION_
#define _QUESTION_


typedef struct question {
   char * question
  ;char * answer1
  ;char * answer2
  ;char * answer3
  ;char * answer4
  ;uint8_t  answer
  ;
} question;
//question questions[]; 
const question questions[] = {
  {
     "What is the right answer?\n"
    ,"One\n"
    ,"One\n"
    ,"One\n"
    ,"42\n"
    ,4
  }
};

#endif
