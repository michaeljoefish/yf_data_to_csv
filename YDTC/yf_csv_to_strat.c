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

  uint16_t ma20[2];
  uint16_t ma50[2];
  
}CSV;

void to_struct_arr(FILE*, CSV*, int);
void calc_all_SMA(CSV*, int);

int main(void){
  char *filepath = "OUTPUT_CSV_FILES/2025-4-16_C_15m.csv";
  FILE* RO_INFILE = fopen(filepath, "r");
  if (!RO_INFILE){
    printf("ERROR: File Not Found");
    return -1;
  }
  int length = 500;
  CSV BUF[length];
  to_struct_arr(RO_INFILE, BUF, length);

  fclose(RO_INFILE);
  RO_INFILE = NULL;

  int i = 0;
  while( (i < length) && (BUF[i].hr != 0xff) ){
    if(BUF[i].hr == 0x00){ i++; continue;}
    
    printf("%02hhu:%02hhu:00 %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu\n",
         BUF[i].hr, BUF[i].mi, BUF[i].open[DECI], BUF[i].open[FRAC], BUF[i].high[DECI], BUF[i].high[FRAC],
         BUF[i].low[DECI], BUF[i].low[FRAC], BUF[i].close[DECI], BUF[i].close[FRAC], BUF[i].vol);
    i++;
  }

  calc_all_SMA(BUF, length);

  i = 0;

  while( (i < length) && (BUF[i].hr != 0xff) ){
    if(BUF[i].hr == 0x00){ i++; continue;}
    
    printf("%02hhu:%02hhu:00 %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu , %hu.%04hu , %hu.%04hu\n",
         BUF[i].hr, BUF[i].mi, BUF[i].open[DECI], BUF[i].open[FRAC], BUF[i].high[DECI], BUF[i].high[FRAC],
           BUF[i].low[DECI], BUF[i].low[FRAC], BUF[i].close[DECI], BUF[i].close[FRAC], BUF[i].vol,
           BUF[i].ma20[DECI], BUF[i].ma20[FRAC], BUF[i].ma50[DECI], BUF[i].ma50[FRAC]);
    i++;
  }
  
  

  /*

  printf("%hhu:%hhu:00 %hu.%s , %hu.%s , %hu.%s , %hu.%s , %lu\n\n",
         C.hr, C.mi, C.open[DECI], r_OHLC[0], C.high[DECI], r_OHLC[1],
         C.low[DECI], r_OHLC[2], C.close[DECI], r_OHLC[3], C.vol);


  
  printf("%hhu:%hhu:00 %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu\n",
         C.hr, C.mi, C.open[DECI], C.open[FRAC], C.high[DECI], C.high[FRAC],
         C.low[DECI], C.low[FRAC], C.close[DECI], C.close[FRAC], C.vol);
  */
  
  
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
    }

    // Now scan the strings in r_OHLC to struct as uint16_t
    // Also do banker's rounding if applicable
    //Banker's rounding also accounts for if frac rounding also rounds deci
    sscanf(r_OHLC[0], "%4hu", &buf[ind].open[FRAC]);
    if (r_OHLC[0][4] >= 0x35){
        buf[ind].open[FRAC]++;
        if (buf[ind].open[FRAC] == 10000){
          buf[ind].open[DECI]++;
          buf[ind].open[FRAC] = 0;
        }
    }
    
    sscanf(r_OHLC[1], "%4hu", &buf[ind].high[FRAC]);
    if (r_OHLC[1][4] >= 0x35){
        buf[ind].high[FRAC]++;
        if (buf[ind].high[FRAC] == 10000){
          buf[ind].high[DECI]++;
          buf[ind].high[FRAC] = 0;
        }
    }
    
    sscanf(r_OHLC[2], "%4hu", &buf[ind].low[FRAC]);
    if (r_OHLC[2][4] >= 0x35){
        buf[ind].low[FRAC]++;
        if (buf[ind].low[FRAC] == 10000){
          buf[ind].low[DECI]++;
          buf[ind].low[FRAC] = 0;
        }
    }
    
    sscanf(r_OHLC[3], "%4hu", &buf[ind].close[FRAC]);
    if (r_OHLC[3][4] >= 0x35){
        buf[ind].close[FRAC]++;
        if (buf[ind].close[FRAC] == 10000){
          buf[ind].close[DECI]++;
          buf[ind].close[FRAC] = 0;
        }
    }

      
    ind++;
  }
   
}

void calc_all_SMA(CSV *buf, int length){
  if (length < 50){
    printf("Length = %d, buffer is too small to calc SMA", length);
    exit(1);
  }
  uint64_t window_sum_20_DECI = 0;
  uint64_t window_sum_20_FRAC = 0;
  uint64_t window_sum_50_DECI = 0;
  uint64_t window_sum_50_FRAC = 0;

  int i = 0;

  // init first 20 (0-19) to 0.0 for both MAs, store sums
  // The 20th 20MA will eventually be calculated
  for (i = 0; i < 20; i++){
    window_sum_20_DECI += buf[i].close[DECI];
    window_sum_20_FRAC += buf[i].close[FRAC];
    buf[i].ma20[DECI] = 0;   buf[i].ma20[FRAC] = 0;
    buf[i].ma50[DECI] = 0;   buf[i].ma50[FRAC] = 0;
  }
  // Don't forget to do win_50 = win_20 before next part
  window_sum_50_DECI = window_sum_20_DECI;
  window_sum_50_FRAC = window_sum_20_FRAC;

  // init remaining 30 (20-49) to 0.0 for 50MA, store sums
  // The 50th 50MA will eventually be calculated.
  for (i = 20; i<50; i++){
    window_sum_50_DECI += buf[i].close[DECI];
    window_sum_50_FRAC += buf[i].close[FRAC];
    buf[i].ma50[DECI] = 0;   buf[i].ma50[FRAC] = 0;
  }

  // calculate 20th 20MA and 50th 50ma
  uint64_t temp20 = ((window_sum_20_DECI * 10000) + window_sum_20_FRAC) / 20;
  buf[19].ma20[FRAC] = (uint16_t)(temp20 % 10000);
  buf[19].ma20[DECI] = (uint16_t)(temp20 / 10000);
  
  uint64_t temp50 = ((window_sum_50_DECI * 10000) + window_sum_50_FRAC) / 50;
  printf("%lu\n\n", (window_sum_50_DECI * 10000) + window_sum_50_FRAC);
  buf[49].ma50[FRAC] = (uint16_t)(temp50 % 10000);
  buf[49].ma50[DECI] = (uint16_t)(temp50 / 10000);

  // Calculate 20MA for remaining bars
  i = 20;
  while ((i < length) && (buf[i].hr != 0xFF)){
    window_sum_20_DECI = window_sum_20_DECI - (buf[i-20].close[DECI]) + (buf[i].close[DECI]);
    window_sum_20_FRAC = window_sum_20_FRAC - (buf[i-20].close[FRAC]) + (buf[i].close[FRAC]);
    temp20 = ((window_sum_20_DECI * 10000) + window_sum_20_FRAC) / 20;
    buf[i].ma20[FRAC] = (uint16_t)(temp20 % 10000);
    buf[i].ma20[DECI] = (uint16_t)(temp20 / 10000);
    
    i++;
  }

  // Calculate 50MA for remaining bars
  i = 50;
  while((i < length) && (buf[i].hr != 0xFF)){
    window_sum_50_DECI = window_sum_50_DECI - (buf[i-50].close[DECI]) + (buf[i].close[DECI]);
    window_sum_50_FRAC = window_sum_50_FRAC - (buf[i-50].close[FRAC]) + (buf[i].close[FRAC]);
    temp50 = ((window_sum_50_DECI * 10000) + window_sum_50_FRAC) / 50;
    buf[i].ma50[FRAC] = (uint16_t)(temp50 % 10000);
    buf[i].ma50[DECI] = (uint16_t)(temp50 / 10000);
    
    i++;
  }
}
