#include "pressanykey/pressanykey.h"
#include "typing.h"
#include "question.h"

#define TIMEOUT 50; // half a second 10ms * 50 = 500

typedef enum {
	 start
	,press_any_key
	,question_
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

nerd_sm sm;

void random_question (nerd_sm *sm) {
  sm->question = 0;
}
void init_game(){
  sm.state = start;
}

void type_question(uint8_t q_id) {
	question q;
	q = questions[q_id];
	type(q.question);
	command("# 1.)");
	command(q.answer1);
	command("# 2.)");
	command(q.answer2);
	command("# 3.)");
	command(q.answer3);
	command("# 4.)");
	command(q.answer4);
}
uint8_t get_answer(uint8_t q_id) {
	return questions[q_id].answer;
}

// 50 * 10 ms half a second for click ...
#define CLICK_TIME 30 

# define CHK_TIMEOUT(x) if ((x) < (count-sm.last_time)) {sm.state = timeout; break;}
# define CHK_ANSWR(x)   if ((CLICK_TIME) < (count-sm.last_time)) {sm.answer=(x); sm.state = check; break;}

void do_nerd_sm (bool pressed, uint32_t count) {
	char result[2];
	result[1]=0;


	switch (sm.state) {
		case start:
		case press_any_key:
			type("press anykey to continue!");
			sm.state = question_;
			break;
		case question_:
			if (pressed) {
				command("\nclear\n");
				random_question(&sm);
				sm.state=question_print;
			}	
			break;
		case question_print:
			if (!pressed) {
				type ("question\n");
				type_question(sm.question);
				sm.state=press_1_0;
				sm.last_time = count;
			};
			break;
		case press_1_0:
			CHK_TIMEOUT(500)
			if (pressed) { 
				sm.state = press_1_1; 
				sm.last_time = count;	
			}
			break;
		case press_1_1:
			CHK_TIMEOUT(CLICK_TIME)
			if (!pressed) {
				sm.state = press_2_0;
				sm.last_time = count;
			}
			break;
		case press_2_0:
			CHK_ANSWR(1);
			if (pressed) {
				sm.state = press_2_1;
				sm.last_time = count;
			}
			break;
		case press_2_1:
			CHK_TIMEOUT(CLICK_TIME)
			if (!pressed) {
				sm.state = press_3_0;
				sm.last_time = count;
			}
			break;
		case press_3_0:
			CHK_ANSWR(2);
			if (pressed) {
				sm.state = press_3_1;
				sm.last_time = count;
			}
			break;
		case press_3_1:
			CHK_TIMEOUT(CLICK_TIME)
			if (!pressed) {
				sm.state = press_4_0;
				sm.last_time = count;
			}
			break;
		case press_4_0:
			CHK_ANSWR(3);
			if (pressed) {
				sm.state = press_4_1;
				sm.last_time = count;
			}
			break;
		case press_4_1:
			CHK_TIMEOUT(CLICK_TIME)
				if (!pressed) {
					sm.answer = 4;
					sm.state  = check;
				}
			break;
		case check:
			//	result[0]= sm.answer+0x30;
			//	type(result);
			if (sm.answer == get_answer(sm.question)) {
				command("cowsay hurrah!\n");
			} else {
				type("oh noes!\n");
			}
			sm.state = press_any_key;
			break;      
		case timeout:
			type("oh please!\n");
			sm.state = press_any_key;
			break;
		default:
			sm.state = start;
	}
}



