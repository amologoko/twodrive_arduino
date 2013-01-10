#ifndef __CODE_H__
#define __CODE_H__

typedef union twodrive_block_st {
    struct {
      uint8_t    byte3; //NumDays[5], DD[3]
      uint8_t    byte2; //DD[2], MM[4], YY%4[2]
      uint8_t    byte1;
      uint8_t    byte0;
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
	uint8_t  month;
	uint8_t  day;
	uint8_t  ndays;
} twodrive_data_t; 

#define SECS_DAY ((unsigned long)3600*24)

void crypt_setup();
uint32_t twodrive_enc(void *pdata);
uint8_t twodrive_dec(uint32_t block, twodrive_data_t *data);

#endif
