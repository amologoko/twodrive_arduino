
#include <stdio.h>
#include <stdint.h>

//---------------------------------------------------------------------------------
#define KEYSIZE 16	 /* size of key, in bytes */
//typedef unsigned int unsigned int; /* Should be 32-bit = 4 bytes        */
#define W            (16)
#define ROT_MASK     (W - 1)

//typedef enum { 0, 1 } ShiftDir;


typedef enum { KeyWords = KEYSIZE / 4,
	       NumRounds = 15,  /* Number of cryptographic rounds  */
	       TableSize = 32   /* size of table = 2 * (NumRounds + 1) */
} bogus;


unsigned int Table[ TableSize ];
unsigned int L[KeyWords]; 



unsigned int ROT(const unsigned int x, const unsigned int y, const int dir)
{

  const unsigned int ShiftAmt = (y & (unsigned int)ROT_MASK);
  const unsigned int ShiftBack = W - ShiftAmt;
  unsigned int result;
  
  if (dir == 0)
    result = (x << ShiftAmt) | (x >> ShiftBack);
  else
    result = (x >> ShiftAmt) | (x << ShiftBack);
  return result;
} /* ROT */



void SetKey( unsigned char KeyChar )
{
    static unsigned int KeyCntr;
    static unsigned int Shift;

    int ix = KeyCntr >> 2;

    /* this is simply a machine independent way of setting L[i] to 
       KeyChar[i], without being affect by "endianess". */
    L[ ix ] = (L[ ix ] & (~(0xff << Shift))) | (KeyChar << Shift);

    Shift = (Shift + 8) & 0x1f;
    KeyCntr = (KeyCntr + 1) & (KEYSIZE - 1);  /* This Depends on KEYSIZE being */
                                              /* a power of two.  The & will   */
                                              /* cause the KeyCntr to wrap     */
                                              /* and only have values in the   */
                                              /* range 0..KEYSIZE-1.           */
}  /* RC5_Crypto */




 /* 2 unsigned int: input plain text, output encrypted text    */
void encrypt(unsigned int *PlainText, unsigned int *CryptoText)
{ 

  unsigned int i, temp;
  unsigned int A;
  unsigned int B;

  A = PlainText[0] + Table[0];
  B = PlainText[1] + Table[1];

  for (i = 1; i <= NumRounds; i++) {
    temp = i << 1; 
    A = ROT(A^B, B, 0) + Table[temp]; 
    B = ROT(A^B, A, 0) + Table[temp+1]; 
  }
  CryptoText[0] = A; 
  CryptoText[1] = B;  
}  /* RC5_Cypto::encrypt */



/* 2 unsigned int input encrypted text, output plain text    */
void decrypt(unsigned int *CryptoText, unsigned int *PlainText) 
{ 
  unsigned int i, temp;
  unsigned int B; 
  unsigned int A;

  B = CryptoText[1]; 
  A = CryptoText[0];

  for (i=NumRounds; i > 0; i--) { 
    temp = i << 1;
    B = ROT(B - Table[temp+1],A, 1)^A; 
    A = ROT(A - Table[temp],  B, 1)^B; 
  }
  PlainText[1] = B-Table[1]; 
  PlainText[0] = A-Table[0];  
} 	/*  decrypt */




void crypt_setup() /* secret input key K[0...KEYSIZE-1]   */
{ 
  /* magic constants (courtesty of RSA) */
  static const unsigned int ROM[ TableSize ] = { 0xb7e1, 0x5618, 0xf450,
                                         0x9287, 0x30bf, 0xcef6,
                                         0x6d2e, 0x0b65, 0xa99d,
                                         0x47d4, 0xe60c, 0x8443,
                                         0x227b, 0xc0b2, 0x5ee9,
                                         0xfd21, 0x9b58, 0x3990,
                                         0xd7c7, 0x75ff, 0x1436,
                                         0xb26e, 0x50a5, 0xeedd,
                                         0x8d14, 0x2b4c, 0xc983,
                                         0x67bb, 0x05f2, 0xa42a,
                                         0x4261, 0xe099 };
  unsigned int i;
  unsigned int A;
  unsigned int B;
  unsigned int j;
  unsigned int k;
  
  /* Copy "ROM" into "RAM" */
  for (i=0; i < TableSize; i++) 
    Table[i] = ROM[i];

  /* 3*t > 3*KeyWords */

  A = 0;
  B = 0;
  i = 0;
  j = 0;

  for (k=0; k < 3*TableSize; k++) {
    Table[i] = ROT(Table[i]+(A+B),3, 0);
    A = Table[i];
    L[j] = ROT(L[j]+(A+B),(A+B), 0);  
    B = L[j]; 
    i= (i+1) & (TableSize-1);  /* using '&' for % only works for powers of 2  */
    j= (j+1) & (KeyWords-1);
  }
} 	/* setup */
//---------------------------------------------------------------------------------

typedef union {
    struct {
        uint8_t    byte3; //NumDays[5], DD[3]
        uint8_t    byte2; //DD[2], MM[4], YY%4[2]
		uint8_t	   byte1;
		uint8_t	   byte0;
    } bytes;
    struct {
         uint16_t   word1;
		 uint16_t   word0;
    } words;
    uint32_t   dword;
} twodrive_block_t;

typedef struct twodrive_ctx_st {
	uint8_t rounds;
	uint32_t *s;
} twodrive_ctx_t; 

typedef struct twodrive_data_st {
	uint16_t year;
	uint8_t month;
	uint8_t day;
	uint8_t ndays;
} twodrive_data_t; 


uint16_t fletcher16( uint8_t const *data, size_t bytes )
{
        uint16_t sum1 = 0xff, sum2 = 0xff;
 
        while (bytes) {
                size_t tlen = bytes > 20 ? 20 : bytes;
                bytes -= tlen;
                do {
                        sum2 += sum1 += *data++;
                } while (--tlen);
                sum1 = (sum1 & 0xff) + (sum1 >> 8);
                sum2 = (sum2 & 0xff) + (sum2 >> 8);
        }
        /* Second reduction step to reduce sums to 8 bits */
        sum1 = (sum1 & 0xff) + (sum1 >> 8);
        sum2 = (sum2 & 0xff) + (sum2 >> 8);
        return sum2 << 8 | sum1;
}

uint32_t twodrive_enc(struct twodrive_data_st const *data)
{
	uint32_t block;
	if(data->month > 12 || data->day > 31 || data->ndays > 31)
		return 0;
	twodrive_block_t ret;
	ret.dword = 0x0;
	ret.bytes.byte3 = data->day << 5 | data->ndays;
	ret.bytes.byte2 = (data->year % 4) << 6 | data->month << 2 | data->day >> 3;
	ret.words.word0 = fletcher16((uint8_t *)&ret, 2);
	//TODO: Encrypt ret 32-bit block using RC5 here
	//return ret.dword;
	encrypt((unsigned int *)&ret,(unsigned int *)&block);
	return block;
}

uint8_t twodrive_dec(uint32_t block, struct twodrive_data_st* data)
{
	twodrive_block_t ret;
	//ret.dword = block;
	//decrypt ret 32-bit block using RC5 here
	decrypt((unsigned int *)&block, (unsigned int *)&ret);
	if (ret.words.word0 != fletcher16((uint8_t *)&ret, 2))
		return 0;
	data->ndays = ret.bytes.byte3 & 0x1f;
	data->day = (ret.bytes.byte2 & 0x03) << 3 | ret.bytes.byte3 >> 5;
	data->month = (ret.bytes.byte2 >> 2) & 0x0f;
	data->year = (ret.bytes.byte2 >> 6) & 0x03;
	return 1;
}

extern "C" int rc5_main();


///////////////////////////////////////////////////////////////////////

static FILE uartout = {0} ;

// create a output function
// This works because Serial.write, although of
// type virtual, already exists.
static int uart_putchar (char c, FILE *stream)
{
    Serial.write(c) ;
    return 0 ;
}

void setup(void)
{
   // Start the UART
   Serial.begin(9600) ;
   Serial.setTimeout(1000000);

   // fill in the UART file descriptor with pointer to writer.
   fdev_setup_stream (&uartout, uart_putchar, NULL, _FDEV_SETUP_WRITE);

   // The uart is the standard output device STDOUT.
   stdout = &uartout;
   
   printf("todrive crypt test\n");
}

/*
void loop()
{
	char *keystr = "ABCDEFGHIJKLMNOP";
    for (int j=0; j < KEYSIZE; j++) 
		SetKey((unsigned char)keystr[j]);
        
	struct twodrive_data_st data;
	struct twodrive_data_st data2;

	data.year = 2012;
	data.month = 8;
	data.day = 13;
	data.ndays = 10;

	uint32_t code = twodrive_enc(&data);
	printf("Access code: %u\n", code);
	if(!twodrive_dec(code, &data2))
		printf("Checksum %d test failed!\n");
	else
		printf("Code valid for %d days starting %d/%d\n", data2.ndays, data2.month, data2.day);

	getchar();

    while (1) {
    }
}
*/

void readline(char *s, int len) {
    int p = 0;
    int c;

    while (1) {
        if (Serial.available() > 0) {
            c = Serial.read();
            s[p++] = c;
            putchar(c);

            if (p == len) {
                s[p] = 0;
                break;
            }
            if (c == '\n' || c == '\r') {
                s[p] = 0;
                break;
            }
        }
    }

    return;
}

void loop()
{
	twodrive_data_t data;
	twodrive_data_t data2;
    char s[32];
    int  j;

    while (1) {
        printf("Enter the serial #:  ");
        readline(s, 16);
        if (strlen(s) >= KEYSIZE) {
            break;
        }
        printf("%s : %d\n", s, strlen(s));
    } 

    for (j=0; j < KEYSIZE; j++) 
		SetKey((unsigned char)s[j]);

    // Setup, encrypt, and decrypt 
    crypt_setup();  
    printf("Crypt setup\n");

    printf("Enter the start day in MM/DD/YYYY format:  ");
    readline(s, 10);

	data.year  = atoi(s+6);
    s[5] = 0;
	data.day   = atoi(s+3);
    s[2] = 0;
	data.month = atoi(s);

    printf("Entered %d/%d/%d\n", data.month, data.day, data.year); 

    printf("Enter # of days:  ");
    readline(s, 2);
 	data.ndays = atoi(s);

	uint32_t code = twodrive_enc(&data);
	printf("Access code: %lu\n", code);
	if (!twodrive_dec(code, &data2))
		printf("Checksum %d test failed!\n");
	else
		printf("Code valid for %d days starting %d/%d\n", data2.ndays, data2.month, data2.day); 

    while (1)
        ;
}
