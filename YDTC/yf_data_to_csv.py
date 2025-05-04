import yfinance as yf
import datetime as dt
import pandas as pd
import os


### Working on intraday stuff first
WORKDIR = "YDTC"
FOLDER_OUTPUT = "OUTPUT_CSV_FILES"



DICT_INTRADAY_INTERVALS_OFFSETS = {
        "1m": 1,
        "2m": 1,
        "5m": 1,
        "15m": 2,
        "30m": 5,
        "60m": 19,
        "1h": 19,

    }

DICT_STOCK_EXCHANGE_HOLIDAYS = {
    "1/1": "New Year's Day",
    "1/15": "Presidents' Day - subject to change",
    "3/29": "Good Friday - subject to change",
    "5/27": "Memorial Day - subject to change",
    "6/19": "Juneteenth",
    "7/4": "Independence Day",
    "9/2": "Labor Day - subject to change",
    "11/28": "Thanksgiving Day - subject to change",
    "12/25": "Christmas Day",
    }

## TODO Implement this function
def ticker_data_to_csv_basic(tckr: yf.ticker.Ticker, date_start: dt.datetime, date_end: dt.datetime, tf: str) -> None:
    tckr_symbol = tckr.info["symbol"]
    PATH_FILE_OUTPUT_CSV = f"{FOLDER_OUTPUT}/{date_end.year}-{date_end.month}-{date_end.day}_{tckr_symbol}_{tf}.csv"

    tckr_hist = tckr.history(start = date_start, end = date_end + dt.timedelta(days=1),
                             interval = tf, prepost = False)
    
    tckr_hist.to_csv(PATH_FILE_OUTPUT_CSV)

def get_datetime_start(offset: int, date: dt.datetime) -> dt.datetime:
    """
    Error Checking?: None

    Preconditions:  Date is a valid datetime object.
                    Date is not a weekend or a stock holiday

    Input:  offset: [int] (How many previous days you want to include in data)
                        (e.g. You want 5 min chart on Wed, but you also want Mon and Tues charts)
            date: [dt.datetime] (Original date requested by client)

    Output: date_start: [int], representing the adjusted start date, as you may have holidays or weekdays going backwards,
                        and you don't want to/can't pull data from those days, so you need your input date
                        minus an adjusted offset from the one passed to the function.
    """

    adjusted_offset = offset

    ### This whole thing is fucky.
    days_back = 1
    date_start = date


    while (days_back <= adjusted_offset):
        date_start = date - dt.timedelta(days = days_back)
        if  (
                (date_start.weekday() == 5) or (date_start.weekday() == 6) or
                (DICT_STOCK_EXCHANGE_HOLIDAYS.get(f"{date_start.month}/{date_start.day}", False))
            ):
            adjusted_offset += 1

        days_back += 1
        
    
    return date_start

## TODO 1. Remove error/input managing.
#       2. 
def intraday_timeframe_manager(ticker: yf.ticker.Ticker, date: dt.datetime, timeframes: list) -> None:

    for tf in timeframes:
        offset_in_days = DICT_INTRADAY_INTERVALS_OFFSETS.get(tf, -1)

        if offset_in_days != -1:
            date_start = get_datetime_start(offset_in_days, date)
            ticker_data_to_csv_basic(ticker, date_start, date, tf)
        else:
            print(f"Timeframe \"{tf}\" is not a valid timeframe. Please reference the valid timeframes in the README")
            print("Other valid timeframes will continue to be retrieved.")

    
## TODO Put all code, or at least what is necessary, into a while loop, so process can be repeated
#           for multiple stocks. May change later so that multiple stock tickers can be entered at once.
#           I would probably still use a while loop for that as well.
def intraday_input_manager() -> None:
    print("NOTICE: Please review the yfinance limits on intraday data.\n")
    TICKER = None
    DATE = None
    market_is_closed = False

    while(True):
        try:
            TICKER = yf.Ticker(input("Enter the stock ticker(e.g. MSFT): "))
            TICKER.info["symbol"]
        except Exception:
            print("The symbol you entered does not exist. Please try again.")
        else:
            break

    while(True):
        try:
            temp_str = input("Enter the date in yyyy/mm/dd format (e.g. 2024/09/05): ")
            DATE = dt.datetime.strptime(f"{temp_str}", "%Y/%m/%d")
            market_is_closed = DICT_STOCK_EXCHANGE_HOLIDAYS.get(f"{DATE.month}/{DATE.day}", False)

            if(DATE.weekday() == 5) or (DATE.weekday() == 6) or (market_is_closed):
                raise Exception("The Stock Market is closed on the day you entered.")
            
        except ValueError:
            print("There was an error with the date you inputted, please try again.")
        except Exception as e:
            print(e)
        else:
            break
    


    while(True):
        timeframes = input("Enter your intraday timeframes of interest, separated by spaces"
                           + "\n(e.g. 1m 2m 5m 1h etc.): ")
        
        timeframes_as_list = timeframes.split(" ")

        intraday_timeframe_manager(TICKER, DATE, timeframes_as_list)

        answer_continue = input("Would you like to enter more timeframes?(y/n): ")
        if(answer_continue != "y"):
            break





"""PATH_FILE_OUTPUT_CSV = f"YDTC/10-4-2024.{ticker_name}.5min.csv"


msft_hist = msft.history(start=dt.datetime(2024,10,3, hour=9, minute=25, tzinfo=None),
                            end=dt.datetime(2024,10,3, hour=16, minute=0, tzinfo=None),
                            interval='5m', prepost=False)

msft_hist.to_csv(PATH_FILE_OUTPUT_CSV)"""


def main() -> None:
    
    if not os.path.exists(f"{FOLDER_OUTPUT}"):
        os.mkdir(f"{FOLDER_OUTPUT}")
        
    intraday_input_manager()

    
if __name__ == "__main__":
    main()