#include "pressanykey/pressanykey.h"
#include "typing.h"

#define TIMEOUT 50; // half a second 10ms * 50 = 500

typedef enum {
	 start
	,press_any_key
	,question
	,question_print
	,press_1_0
	,press_1_1
	,press_2_0
	,press_2_1
	,press_3_0
	,press_3_1
	,press_4_0
	,press_4_1
  ,timeout
  ,check
} nerd_state;

typedef struct nerd_sm {
   nerd_state state
  ;uint8_t    question   
  ;uint8_t    answer 
	;uint32_t   last_time  
  ;
} nerd_sm;


void random_question (nerd_sm *sm) {
	// sm.question = x
}

char * get_question(uint8_t q_id) { return NULL;}
uint8_t get_answer(uint8_t q_id) { return 0;}

// 50 * 10 ms half a second for click ...
#define CLICK_TIME 50 

# define CHK_TIMEOUT(x) if ((x) < (count-sm.last_time)) {sm.state = timeout; break;}
# define CHK_ANSWR(x)   if ((CLICK_TIME) < (count-sm.last_time)) {sm.answer=(x); sm.state = check; break;}

void do_nerd_sm (bool pressed, uint32_t count) {
	static nerd_sm sm;
	
	switch (sm.state) {
		case start:
			// print_welcome_banner;
			//fallthrough!
		case press_any_key:
			type("press any key to continue!");
			sm.state = question;
			break;
		case question:
			if (pressed) {
				random_question(&sm);
				sm.state=question_print;
			}	
			break;
		case question_print:
			if (!pressed) {
				type(get_question(sm.question));
				sm.state=press_1_0;
			};
			break;
		case press_1_0:
      CHK_TIMEOUT(500)
      if (pressed) { sm.state = press_1_1; }
      break;
		case press_1_1:
      CHK_TIMEOUT(CLICK_TIME)
      if (!pressed) sm.state = press_2_0;
      break;
		case press_2_0:
      CHK_ANSWR(1);
      if (pressed) sm.state = press_2_1;
      break;
		case press_2_1:
      CHK_TIMEOUT(CLICK_TIME)
      if (!pressed) sm.state = press_3_0;
      break;
		case press_3_0:
      CHK_ANSWR(2);
      if (pressed) sm.state = press_3_1;
      break;
		case press_3_1:
      CHK_TIMEOUT(CLICK_TIME)
      if (!pressed) sm.state = press_4_0;
      break;
		case press_4_0:
      CHK_ANSWR(3);
      if (pressed) sm.state = press_4_1;
      break;
		case press_4_1:
      CHK_TIMEOUT(CLICK_TIME)
      if (!pressed) {
        sm.answer = 4;
        sm.state  = check;
      }
      break;
    case check:
			if (sm.answer == get_question(sm.question)) {
				type("huzzah!");
			} else {
				type("oh noes!");
			}
			sm.state = press_any_key;
			break;      
    case timeout:
      type("oh please!");
      sm.state = press_any_key;
      break;
		default:
			sm.state = start;
	}
	sm.last_time = count;
}



