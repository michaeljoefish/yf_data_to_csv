#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DECI 0
#define FRAC 1

typedef struct CSV_DATA{

  uint8_t  hr;
  uint8_t  mi;

  uint16_t open[2];
  uint16_t high[2];
  uint16_t low[2];
  uint16_t close[2];

  uint64_t vol;
}CSV;

void to_struct_arr(FILE*, CSV*, int);

int main(void){
  char *filepath = "OUTPUT_CSV_FILES/2025-4-16_C_5m.csv";
  FILE* RO_INFILE = fopen(filepath, "r");
  if (!RO_INFILE){
    printf("ERROR: File Not Found");
    return -1;
  }
  int length = 500;
  CSV BUF[length];
  to_struct_arr(RO_INFILE, BUF, length);

  /*

  printf("%hhu:%hhu:00 %hu.%s , %hu.%s , %hu.%s , %hu.%s , %lu\n\n",
         C.hr, C.mi, C.open[DECI], r_OHLC[0], C.high[DECI], r_OHLC[1],
         C.low[DECI], r_OHLC[2], C.close[DECI], r_OHLC[3], C.vol);


  
  printf("%hhu:%hhu:00 %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu\n",
         C.hr, C.mi, C.open[DECI], C.open[FRAC], C.high[DECI], C.high[FRAC],
         C.low[DECI], C.low[FRAC], C.close[DECI], C.close[FRAC], C.vol);
  */

  int i = 0;
  printf("%hhu\n", BUF[i].hr);

  while( (i < length) && (BUF[i].hr != 0xff) ){
    if(BUF[i].hr == 0x00){ i++; continue;}
    
    printf("%hhu:%hhu:00 %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu\n",
         BUF[i].hr, BUF[i].mi, BUF[i].open[DECI], BUF[i].open[FRAC], BUF[i].high[DECI], BUF[i].high[FRAC],
         BUF[i].low[DECI], BUF[i].low[FRAC], BUF[i].close[DECI], BUF[i].close[FRAC], BUF[i].vol);
    i++;
  }
  
  fclose(RO_INFILE);
  RO_INFILE = NULL;
  return 0;
}

void to_struct_arr(FILE* csv_file, CSV* buf, int len){
  // Used to store fractional parts of stats as strings.
  // Will be converted to uint16_t later.
  char r_OHLC[4][21];

  int result = 0;
  int ind = 0;

  char str[150];

  // Read first line
  if (fgets(str, 149, csv_file) != NULL){
    printf("%s\n", str);
  }

  while( (result != EOF) && (ind < len) ){
    // Scan line contents into buf and r_OHLC
    result = fscanf(csv_file,
      "%*hu-%*hhd-%*hhd %hhu:%hhu:%*hhu-%*hhu:%*hhu,%hu.%20[^,],%hu.%20[^,],%hu.%20[^,],%hu.%20[^,],%lu%*s",
      &buf[ind].hr, &buf[ind].mi, &buf[ind].open[DECI], r_OHLC[0], &buf[ind].high[DECI], r_OHLC[1],
      &buf[ind].low[DECI], r_OHLC[2], &buf[ind].close[DECI], r_OHLC[3], &buf[ind].vol);
    /*printf("%d", result);
    printf("%hhu:%hhu:00 %hu.%s , %hu.%s , %hu.%s , %hu.%s , %lu\n\n",
         buf[ind].hr, buf[ind].mi, buf[ind].open[DECI], r_OHLC[0], buf[ind].high[DECI], r_OHLC[1],
         buf[ind].low[DECI], r_OHLC[2], buf[ind].close[DECI], r_OHLC[3], buf[ind].vol);*/

    // If bad read or EOF, initialize hr to special value and continue.
    if (result != 11){
      buf[ind].hr = 0x00;
      
      if (result == EOF)
        buf[ind].hr = 0xff;

      ind++;
      continue;
    }

    for(int i = 0; i<4; i++){
      for(int k = 0; k<5; k++){
        if (r_OHLC[i][k] < 0x30 || r_OHLC[i][k] > 0x39)
          r_OHLC[i][k] = 0x30;
      }

      // Banker's Rounding
      if (r_OHLC[i][4] >= 0x35)
        r_OHLC[i][3] += 1;

      sscanf(r_OHLC[0], "%4hu", &buf[ind].open[FRAC]);
      sscanf(r_OHLC[1], "%4hu", &buf[ind].high[FRAC]);
      sscanf(r_OHLC[2], "%4hu", &buf[ind].low[FRAC]);
      sscanf(r_OHLC[3], "%4hu", &buf[ind].close[FRAC]);
    }

    ind++;
  }
  
  
}
