# yf_data_to_csv

    yf_data_to_csv is a simple CLI program, written in Python, that allows a user to extract
    historical stock market data from Yahoo Finance by utilizing the yfinance API.
    This data is then stored in a CSV file.
    The source code for this program is located in yf_data_to_csv.py.

    
# Running yf_data_to_csv.py
    
    yf_data_to_csv.py currently runs on Python version 3.10.12,
    though other versions may work.
    
    Example run usage:
    ```bash
    python3 yf_data_to_csv.py
    ```
    

    
# Notes on data analysis
    
    This project does not yet have the full capability to analyze CSV files returned by
    yf_data_to_csv.py, but this functionality is a work in progress.
    If you would like to make contributions to the data analysis source code,
    you can make changes in yf_csv_to_strat.c. The build process for
    yf_csv_to_strat.c is listed below.

    
# How To Build (only for .c files)
    
    ```bash
    make clean
    make
    ```
