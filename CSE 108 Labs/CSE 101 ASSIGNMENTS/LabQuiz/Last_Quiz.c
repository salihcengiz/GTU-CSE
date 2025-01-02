//1. Hafta

void setup() {        
  Serial.begin (9600);
  char* str = "Salih kitap aldi";
  char* keyword = "kitap";
  int len = strlen(str);
  int keylen = strlen(keyword);
  int i = 0;
  while (i < len) {
    int j = 0;
    while (j < keylen && i + j < len && str[i + j] == keyword[j]) {
      j++;
    }
    if (j == keylen) {
      Serial.print(keyword);
      i += j;
    } else {
      Serial.print("-");
      i++;
    }
  }
}

void loop() {
  
}

//2. Hafta

const int buttonPin = 8; //Butonla Zar Atma

void setup() {
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);

}

void loop() {
  if(digitalRead(buttonPin) == HIGH){

    int diceResult = random(1,7);

    Serial.print("Dice Roll Result: ");
    Serial.println(diceResult);

  }

}

int redPin =10; // Trafik ışıkları
int yellowPin = 8;
int greenPin = 5;

void setup() {
  pinMode(redPin, OUTPUT);
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);

}

void loop() {
  digitalWrite(redPin, HIGH);
  delay(3000);
  digitalWrite(redPin, LOW);

  digitalWrite(greenPin, HIGH);
  delay(2000);
  digitalWrite(greenPin, LOW);

  digitalWrite(yellowPin, HIGH);
  delay(1000);
  digitalWrite(yellowPin, LOW);

}

void	setup()	{  //Tüm Harfler Var Mı Kontrol Eden Kod
Serial.begin(9600); 
//	Example	pangram	strings 
char	pangram1[]	=	"The	quick	brown	fox	jumps	over	the	lazy	dog"; 
char	notPangram[]	=	"Raindrops	fell	gently	on	the	green	leaves";	
//	Test	the	functions 
checkPangram(pangram1); 
checkPangram(notPangram); 
} 
void	loop()	{ 
//	Nothing	to	loop	for	in	this	example 
} 
void	checkPangram(char*	inputString)	{ 
//	Initialize	an	array	to	store	whether	each	letter	is	present 
boolean	letterPresent[26]	=	{false}; 
//	Iterate	through	each	character	in	the	input	string 
for	(int	i	=	0;	inputString[i]	!=	'\0';	i++)	{ 
char	currentChar	=	inputString[i]; 
//	Check	if	the	current	character	is	an	uppercase	or	lowercase	letter 
if	((currentChar	>=	'a'	&&	currentChar	<=	'z')	||	(currentChar	>=	'A'	&&	currentChar	<=	
'Z'))	{	
//	Convert	the	letter	to	lowercase	for	simplicity 
char	lowercaseChar	=	toLowerCase(currentChar); 
//	Mark	the	corresponding	letter	as	present 
letterPresent[lowercaseChar	-	'a']	=	true; 
} 
} 
//	Check	if	all	letters	are	present 
boolean	isPangram	=	true; 
for	(int	i	=	0;	i	<	26;	i++)	{	
if	(!letterPresent[i])	{	
isPangram	=	false;	
break; 
} 
} 
//	Display	the	result	on	the	Serial	Monitor 
Serial.print("Input	String:	");	
Serial.println(inputString);	
Serial.print("Result:	");	
if	(isPangram)	{	
Serial.println("Pangram");	
}	else	{ 
Serial.println("Not	a	Pangram");	
} 
Serial.println();	
} 

int	ledPin	=	13;	//Asallık Kontrolü Yapan Kod
void	setup()	{	
Serial.begin(9600);	
pinMode(ledPin,	OUTPUT);	//	Initialize	LED	pin	as	output	
}	
void	loop()	{	
//	Iterate	through	prime	numbers	from	2	to	100	
for	(int	i	=	2;	i	<=	100;	i++)	{	
if	(isPrime(i))	{	
digitalWrite(ledPin,	HIGH);		
Serial.print("Number	");	
Serial.print(i);	
Serial.println("	is	prime");	
delay(3000);	//	Wait	for	10	seconds	
}	
else{	
}	
}	
digitalWrite(ledPin,	LOW);	//	Turn	off	the	LED	
Serial.print("Number	");	
Serial.print(i);	
Serial.println("	is	not	prime");	
delay(3000);	//	Wait	for	10	seconds	
//	Endless	loop	(since	we	don't	need	continuous	execution)	
while	(1)	{	
}	
}	
//	Function	to	check	if	a	number	is	prime	
bool	isPrime(int	num)	{	
if	(num	<	2)	{	
return	false;	
}	
for	(int	i	=	2;	i	<	num;	i++)	{	
if	(num	%	i	==	0)	{	
return	false;	
}	
}	
return	true;	
}