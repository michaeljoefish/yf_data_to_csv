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

int16_t to_struct_arr(FILE*, CSV*, int);
void calc_all_SMA(CSV*, int);
int16_t get_entry_index(CSV*, uint8_t, uint8_t, uint8_t, int16_t);
uint32_t prwoso(CSV*, uint16_t*, int16_t, int16_t);
void print_all_bars_data(CSV*, int16_t);
void print_bars_offset_end(CSV*, int16_t, int16_t);

int main(int argc, char **argv){
  //char *filepath = "OUTPUT_CSV_FILES/2025-4-16_C_15m.csv";
  char *filepath = "OUTPUT_CSV_FILES/2025-05-12_AMZN_5m.csv";
  if (argc > 1){
    filepath = argv[1];
  }
  FILE* RO_INFILE = fopen(filepath, "r");
  if (!RO_INFILE){
    printf("ERROR: File Not Found");
    return -1;
  }
  int length = 500;
  CSV BUF[length];
  int16_t readable_len = to_struct_arr(RO_INFILE, BUF, length);

  fclose(RO_INFILE);
  RO_INFILE = NULL;

  int16_t entry_index = get_entry_index(BUF, 10, 37, 5, readable_len);

  /*
  int i = 0;
  while( (i < length) && (BUF[i].hr != 0xff) ){
    if(BUF[i].hr == 0x00){ i++; continue;}
    
    printf("%02hhu:%02hhu:00 %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu\n",
         BUF[i].hr, BUF[i].mi, BUF[i].open[DECI], BUF[i].open[FRAC], BUF[i].high[DECI], BUF[i].high[FRAC],
         BUF[i].low[DECI], BUF[i].low[FRAC], BUF[i].close[DECI], BUF[i].close[FRAC], BUF[i].vol);
    i++;
  }
  */

  calc_all_SMA(BUF, length);


  print_all_bars_data(BUF, readable_len);
  printf("\n\n");
  print_bars_offset_end(BUF, 78, readable_len);

  uint16_t entry_stop[] = {207,100,205,7500};
  uint32_t pot_r = prwoso(BUF, entry_stop, entry_index, readable_len);

  printf("\nlast bar = %hhu.%hhu\n", BUF[readable_len - 1].hr, BUF[readable_len - 1].mi);
  printf("PRWOSO = %hu.%hu\n", (uint16_t)(pot_r>>16), (uint16_t)(pot_r & 0xffff));
  

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

int16_t to_struct_arr(FILE* csv_file, CSV* buf, int len){
  // Used to store fractional parts of stats as strings.
  // Will be converted to uint16_t later.
  char r_OHLC[4][21];

  int16_t lines_read = 0;
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

    // Stores  number of lines read(doesn't column-name line, but does include the EOF bar in buf)
    lines_read++;

    // If bad read or EOF, initialize hr to special value and continue.
    if (result != 11){
      buf[ind].hr = 0x00;
      
      if (result == EOF)
        buf[ind].hr = 0xff;

      ind++;
      continue;
    }

    // Set all non-digit first five characters to '0'
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

  return lines_read - 1; 
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


int16_t get_entry_index(CSV *bars, uint8_t hour, uint8_t mins, uint8_t tf_mins, int16_t len){
  uint8_t adj_mins = (mins / tf_mins) * tf_mins;
  
  int16_t i = len - 1;
  while ( i > -1 && (bars[i].hr != hour || bars[i].mi != adj_mins) ){
    i--;
  }

  return i;
}


// Gets the maximum potential R earned before stop is triggered or end of the day is reached.
// WARNING: entry and stop fractional values must be normalized to 4 digits.
//             ex: .92 as a fractional value should be 9200, not 0092
uint32_t prwoso(CSV *bars, uint16_t *ExS, int16_t entry_index, int16_t r_len){
  uint16_t e_DECI = ExS[DECI];
  uint16_t e_FRAC = ExS[FRAC];
  uint16_t s_DECI = ExS[2 + DECI];
  uint16_t s_FRAC = ExS[2 + FRAC];
  uint16_t max_DECI = 0;
  uint16_t max_FRAC = 0;

  //uint8_t adj_mins = (mins / tf_mins) * tf_mins;

  for(int16_t i = entry_index; i < r_len; i++){
    if ( (bars[i].high[DECI] > max_DECI) || (bars[i].high[DECI] == max_DECI && bars[i].high[FRAC] > max_FRAC) ){
      max_DECI = bars[i].high[DECI];
      max_FRAC = bars[i].high[FRAC];
    }
    if ( (bars[i].low[DECI] < s_DECI) || (bars[i].low[DECI] == s_DECI && bars[i].low[FRAC] <= s_FRAC) ){
      break;
    }
  }
  
  // Work backwards (since we can't index the start directly.)
  // If low of bar stops out, then set high as the max.
  // Otherwise, if high of bar > max, then set high as max.
  /*
  int i = r_len - 1;
  while ( i>=0 && ( (bars[i].hr != hour) || (bars[i].mi != adj_mins) ) ){
    if ( (bars[i].low[DECI] < s_DECI) || (bars[i].low[DECI] == s_DECI && bars[i].low[FRAC] <= s_FRAC) ){
      max_DECI = bars[i].high[DECI];
      max_FRAC = bars[i].high[FRAC];
    }
    else if((bars[i].high[DECI] > max_DECI) || (bars[i].high[DECI] == max_DECI && bars[i].high[FRAC] > max_FRAC) ){
      max_DECI = bars[i].high[DECI];
      max_FRAC = bars[i].high[FRAC];
    }

    i--;
  }

  // Index i should now be at the entry bar.
  // If we stopped out on the entry bar (rare, but not impossible), then record the high as the max.
  // Otherwise, check to see if it's greater than the local max.
  if ( (bars[i].low[DECI] < s_DECI) || (bars[i].low[DECI] == s_DECI && bars[i].low[FRAC] <= s_FRAC) ){
    max_DECI = bars[i].high[DECI];
    max_FRAC = bars[i].high[FRAC];
  }
  else if ( (bars[i].high[DECI] > max_DECI) || (bars[i].high[DECI] == max_DECI && bars[i].high[FRAC] > max_FRAC) ){
    max_DECI = bars[i].high[DECI];
    max_FRAC = bars[i].high[FRAC];
  }
  */

  
  printf("Highest price WOSO = %hu.%hu\n", max_DECI, max_FRAC);
  // Difference between entry and stop (as one number, not two uint16 parts)
  uint32_t es_diff = (((uint32_t)(e_DECI))*10000 + e_FRAC) - (((uint32_t)(s_DECI))*10000 + s_FRAC);
  // Difference between max price and entry.
  uint32_t me_diff = (((uint32_t)(max_DECI))*10000 + max_FRAC) - (((uint32_t)(e_DECI)*10000) + e_FRAC);
  // Division of the latter by the former.
  uint32_t mees_div = (uint32_t)(((uint64_t)(me_diff))*10000 / es_diff);

  // Return the result as a uint32_t, where the leftmost 16 bits are DECI, and rightmost 16 bits are FRAC.
  return ((mees_div / 10000)<<16) | (mees_div % 10000);
}

void print_all_bars_data(CSV *BUF, int16_t len){
  for (int16_t i = 0; i<len; i++){
     printf("%02hhu:%02hhu:00 , %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu , %hu.%04hu , %hu.%04hu\n",
           BUF[i].hr, BUF[i].mi, BUF[i].open[DECI], BUF[i].open[FRAC], BUF[i].high[DECI], BUF[i].high[FRAC],
           BUF[i].low[DECI], BUF[i].low[FRAC], BUF[i].close[DECI], BUF[i].close[FRAC], BUF[i].vol,
           BUF[i].ma20[DECI], BUF[i].ma20[FRAC], BUF[i].ma50[DECI], BUF[i].ma50[FRAC]);
  }
}

void print_bars_offset_end(CSV *BUF, int16_t offset, int16_t len){
  for (int16_t i = len - offset; i<len; i++){
     printf("%02hhu:%02hhu:00 , %hu.%04hu , %hu.%04hu , %hu.%04hu , %hu.%04hu , %lu , %hu.%04hu , %hu.%04hu\n",
           BUF[i].hr, BUF[i].mi, BUF[i].open[DECI], BUF[i].open[FRAC], BUF[i].high[DECI], BUF[i].high[FRAC],
           BUF[i].low[DECI], BUF[i].low[FRAC], BUF[i].close[DECI], BUF[i].close[FRAC], BUF[i].vol,
           BUF[i].ma20[DECI], BUF[i].ma20[FRAC], BUF[i].ma50[DECI], BUF[i].ma50[FRAC]);
  }
}
