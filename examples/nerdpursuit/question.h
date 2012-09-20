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
		 "What does the HTCPCP response 418 stand for?"
		,"Not acceptable"
		,"I'm a teapot"
		,"HTCPCP? Nice try."
		,"Permanently moved."
		,2
	},
	{
		 "What is Haskell?"
		,"An object oriented language"
		,"A functional programming language"
		,"A logical programming language"
		,"An imperative programming language"
		,2
	},
	{
		 "How do you pronounce '>>='?"
		,"bind"
		,"monad"
		,"monoid"
		,"shiftshiftequals"
		,1
	},
	{
		 "What is the type signature of '>>='?"
		,"(>>=) :: m a -> (a -> b) -> b"
		,"(>>=) :: m a -> (a -> m b) -> m"
		,"(>>=) :: m a -> (b -> m b) -> m b"
		,"(>>=) :: m a -> (a -> m b) -> m b"
		,4
	},
	{
		 "What is the name of the javascript engine in google chrome."
		,"V8"
		,"SquirrelFish"
		,"SpiderMonkey"
		,"Rhine"
		,1
	},
//	{
//		 "Which one works as email validator?"
//		,"/[\w.!\$%+-].@[\w-]+(?\.[\w-]+)+/"
//		,"/[\w.!#\$%+-]+@[\w-]+(?:\.[\w-]+)+/"
//		,"/[\w.+!#\$%+]+@[\w-]+(?\.[\w-]+)+/"
//		,"None, you shouldn't validate emails with a regular expression."
//		,4
//	},
	{
		 "Which UTF-8 character is NOT a real, defined character?"
		,"Love Hotel"
		,"Roasted sweet potato"
		,"Chinese symbol for the internet"
		,"A steaming pile of poo"
		,3
	},
	{
		 "Which one is the fastest sorting algorithm?"
		,"Selection Sort"
		,"Quick Sort"
		,"Merge Sort"
		,"All the same"
		,3
	},
	{
		 "The default setting for frames per second in flash is?"
		,"8 fps"
		,"12 fps"
		,"24 fps"
		,"36 fps"
		,2
	},
	{
		 "Which of the following is a pointer to a function that returns an int and may be passed functions like `strlen` as an argument ?"
		,"int (*bla)(size_t(*blub)(const char *s));"
		,"fptr bl = __fptr__(int, strlen);"
		,"int (fptr)(*strlen f);"
		,"int *ftpr = &strlen"
		,1
	},
	{
		 "Which of this expressions has value 11?"
		,"+[++[+[]][+[]]+[]+[++[+[]][+[]]]]"
		,"+[++[+[]][+[]]+[]+[]+[]]"
		,"[[]+[]+[][+[]]][+[]][++[+[]][+[]]]"
		,"+[++[+[]][+[]]+[]+[++[+[]][[]]]]"
		,1
	},
	{
		 "What is the name for half a byte?"
		,"a Nibble"
		,"0x04"
		,"a habyte"
		,"a word"
		,1
	},
	{
		 "Haskell is named after?"
		,"Haskell Carlson"
		,"General Haskell"
		,"Haskell Curry"
		,"H.A. Skell"
		,3
	},
		{
		 "How many packages are there in the PyPI repository?"
		,"About 5K"
		,"About 8K"
		,"About 14K"
		,"About 20K"
		,4
	},
	{
		 "How to remove variable a from memory?"
		,"a = None"
		,"unset a"
		,"del a"
		,"a.unset()"
		,3
	},
	{
		 "Which one is correct JSON?"
		,"\"name\" : 'Answer'"
		,"\"name\" : \"Answer\""
		,"'name' : 'Answer'"
		,"'name' : \"Answer\""
		,2
	},
	{
		 "How to switch back to the previous directory?"
		,"cd ..."
		,"cd -P"
		,"cd -last"
		,"cd -"
		,4
	},
	{
		 "Which one is valid list comprehension?"
		,"[while n in foo: foo[n]]"
		,"[n while n in foo]"
		,"[n for n in foo]"
		,"[n for foo]"
		,3
	},
	{
		 "What does `['3','2','1','0'].map(parseInt)` evaluate to?"
		,"['3', '2', '1', '0']"
		,"[3, NaN, 1, 0]"
		,"[3,2,1,0]"
		,"[3, NaN, 1, NaN]"
		,2
	},
	{
		 "Which regex will match 'blaaa' with the number of 'a' ' s at the end ranging from 2 to 5"
		,"bla{2..5}"
		,"bla{2-5}"
		,"bla{2,5}"
		,"bla[2-5]"
		,3
	},
	{
		 "What is a monad?"
		,"A burrito"
		,"A monoid in the category of endofunctors."
		,"Impossible to understand"
		,"The only functional way to do I/O."
		,2
	},
	{
		 "Which of the following data types are mutable?"
		,"str"
		,"tuple"
		,"list"
		,"bool"
		,3
	},
	{
		 "How to create a couchdb collection called 'nerds' with curl"
		,"curl -X PUT http://localhost:5984/nerds"
		,"curl -X HEAD http://localhost:5984/nerds"
		,"curl -X GET http://localhost:5984/nerds"
		,"curl -X DELETE http://localhost:5984/nerds"
		,1
	},
	{
		 "What's the nodeType value of text nodes?"
		,"1"
		,"2"
		,"3"
		,"4"
		,3
	},	
	{
		 "How can you compute (in R) the correlation coefficient of the vectors A and B?"
		,"corr(A,B);"
		,"cor(A+B)"
		,"cor(A,B)"
		,"cor(A:B)"
		,3
	},
	{
		 "Do you speak R? Given you have an vector Y. What's the result of 'var(Y)'?"
		,"Deletes Y and set it as empty variable"
		,"Calculates sample variance"
		,"Y is in global scope"
		,"There is no var function in R"
		,2
	},
	{
		 "Do you speak R? What's the result of 'x <- c(1,2,3)'?"
		,"nothing... the syntax is not valid"
		,"three variables from x1 to x3"
		,"x = 6 (the sum of the digits)"
		,"creates a vector x"
		,4
	},
	{
		 "What does 'REST' stand for?"
		,"Rails Enterprise Statistics Transformation"
		,"REady STate"
		,"REpresentational State Transfer"
		,"Ruby Enterprise Substitutiona Tool"
		,3
	},
	{
		 "Who developed REST?"
		,"DHH"
		,"Guido Van Rossum"
		,"Peter Norvig"
		,"Roy Fielding"
		,4
	},
	{
		 "What's the average run time of a search in a Binary Search Tree?"
		,"O(n)"
		,"O(n log n)"
		,"O(log n)"
		,"O(1)"
		,3
	},
		{
		 "Why can't you just request files from external servers via XMLHttpRequest in a web-browser?"
		,"XSS"
		,"Cross-Site Scripting"
		,"Cross-Domain Scripting"
		,"Same Origin Policy"
		,4
	},
	{
		 "Where is `size_t` defined?"
		,"stddef.h"
		,"stdarg.h"
		,"stdlib.h"
		,"stdio.h"
		,1
	},
		{
		 "Attribute that is not allowed in Strict DOCTYPE"
		,"bgcolor"
		,"name"
		,"align"
		,"border"
		,1
	},
	{
		 "Which of these methods is an object run through first on type coercion with a string?"
		,"toString"
		,"toObject"
		,"toPrimitive"
		,"valueOf"
		,3
	},
		{
		 "Whats the value of (function() { return typeof arguments; })()?"
		,"array"
		,"object"
		,"undefined"
		,"arguments"
		,2
	},
		{
		 "What's '4' + 2?"
		,"'42'"
		,"402"
		,"6"
		,"0"
		,1
	},
	{
		 "What's typeof null?"
		,"'object'"
		,"'instance'"
		,"'class'"
		,"'william shatner'"
		,1
	},
	{
		 "Which version of ECMAScript does the V8 engine use?"
		,"ECMAScript 3"
		,"ECMAScript 5.1"
		,"ECMAScript 5"
		,"ECMAScript 4"
		,2
	},
		{
		 "Who created Python?"
		,"Vinton Cerf"
		,"Guido van Rossum"
		,"Dennis Ritchie"
		,"Ruben Boersma"
		,2
	},
	{
		 "Who invented node.js?"
		,"Salvatore Sanfilippo"
		,"Ryan Dahl"
		,"David Heinemeier-Hansson"
		,"John Resig"
		,2
	},
};

#endif
