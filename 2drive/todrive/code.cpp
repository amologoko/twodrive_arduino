#include <stdio.h>
#include <string.h>
#include <EEPROM.h>
#include "time.h"
#include "lcd.h"
#include "mpr121.h"
#include "clock.h"
#include "eeprom.h"
#include "code.h"

//---------------------------------------------------------------------------------
// crypting functions 
//---------------------------------------------------------------------------------

#define KEYSIZE 16	 /* size of key, in bytes */
#define W            (16)
#define ROT_MASK     (W - 1)
typedef enum { ShiftLeft, ShiftRight } ShiftDir;


typedef enum { KeyWords = KEYSIZE / 4,
	       NumRounds = 15,  /* Number of cryptographic rounds  */
	       TableSize = 32   /* size of table = 2 * (NumRounds + 1) */
} bogus;

WORD Table[ TableSize ];
WORD L[KeyWords]; 

WORD ROT(const WORD x, const WORD y, const ShiftDir dir)
{

  const unsigned int ShiftAmt = (y & (WORD)ROT_MASK);
  const unsigned int ShiftBack = W - ShiftAmt;
  unsigned int result;
  
  if (dir == ShiftLeft)
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




 /* 2 WORD: input plain text, output encrypted text    */
void encrypt(WORD *PlainText, WORD *CryptoText)
{ 

  WORD i, temp;
  WORD A;
  WORD B;

  A = PlainText[0] + Table[0];
  B = PlainText[1] + Table[1];

  for (i = 1; i <= NumRounds; i++) {
    temp = i << 1; 
    A = ROT(A^B, B, ShiftLeft) + Table[temp]; 
    B = ROT(A^B, A, ShiftLeft) + Table[temp+1]; 
  }
  CryptoText[0] = A; 
  CryptoText[1] = B;  
}  /* RC5_Cypto::encrypt */



/* 2 WORD input encrypted text, output plain text    */
void decrypt(WORD *CryptoText, WORD *PlainText) 
{ 
  WORD i, temp;
  WORD B; 
  WORD A;

  B = CryptoText[1]; 
  A = CryptoText[0];

  for (i=NumRounds; i > 0; i--) { 
    temp = i << 1;
    B = ROT(B - Table[temp+1],A, ShiftRight)^A; 
    A = ROT(A - Table[temp],  B, ShiftRight)^B; 
  }
  PlainText[1] = B-Table[1]; 
  PlainText[0] = A-Table[0];  
} 	/*  decrypt */




void crypt_setup() /* secret input key K[0...KEYSIZE-1]   */
{ 
  /* magic constants (courtesty of RSA) */
  static const WORD ROM[ TableSize ] = { 0xb7e1, 0x5618, 0xf450,
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
  WORD i;
  WORD A;
  WORD B;
  WORD j;
  WORD k;
  

  char *keystr;

  // seed the key from the serial #
  keystr = eeprom_sern_read();
  for (int j=0; j < KEYSIZE; j++) 
      SetKey((unsigned char)keystr[j]);

  //printf("Setting up crypto with key %s :: %d\n", keystr, sizeof(WORD));

  /* Copy "ROM" into "RAM" */
  for (i=0; i < TableSize; i++) 
    Table[i] = ROM[i];

  /* 3*t > 3*KeyWords */

  A = 0;
  B = 0;
  i = 0;
  j = 0;

  for (k=0; k < 3*TableSize; k++) {
    Table[i] = ROT(Table[i]+(A+B),3, ShiftLeft);
    A = Table[i];
    L[j] = ROT(L[j]+(A+B),(A+B), ShiftLeft);  
    B = L[j]; 
    i= (i+1) & (TableSize-1);  /* using '&' for % only works for powers of 2  */
    j= (j+1) & (KeyWords-1);
  }
} 	/* setup */
//---------------------------------------------------------------------------------


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

uint32_t twodrive_enc(twodrive_data_t * data)
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
	encrypt((WORD *)&ret,(WORD *)&block);

	return block;
}

uint8_t twodrive_dec(uint32_t block, twodrive_data_t* data)
{
	twodrive_block_t ret;
	//ret.dword = block;
	//decrypt ret 32-bit block using RC5 here

    //printf("Decrypting code %lu\n", block);

	decrypt((WORD *)&block, (WORD *)&ret);

    //printf("%08x %08x\n", ret.words.word0, fletcher16((uint8_t *)&ret, 2));

	if (ret.words.word0 != fletcher16((uint8_t *)&ret, 2))
		return 0;

	data->ndays = ret.bytes.byte3 & 0x1f;
	data->day = (ret.bytes.byte2 & 0x03) << 3 | ret.bytes.byte3 >> 5;
	data->month = (ret.bytes.byte2 >> 2) & 0x0f;
	data->year = (ret.bytes.byte2 >> 6) & 0x03;

	return 1;
}

