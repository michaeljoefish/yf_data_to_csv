import yfinance as yf
import datetime as dt
import pandas as pd

msft = yf.Ticker("MSFT")

msft_hist = msft.history(start=dt.datetime(2024,10,3, hour=9, minute=29, tzinfo=None),
                            end=dt.datetime(2024,10,3, hour=9, minute=36, tzinfo=None), interval='1m', prepost=False)

print(msft_hist)
