# Development environment setup
    This project is being worked on in Linux,
    specifically Ubuntu 22.04 (Though I'm sure
    other flavors and versions of Linux may work.)

    
    For the Python part of this project, ensure you have
    python3, pandas, and yfinance installed.
    You can install them with the following commands:
    
    ```bash
    # Installing python
    sudo apt update
    sudo apt install python3
    
    # Installing yfinance
    pip install yfinance
    
    # Installing pandas
    pip install pandas
    ```

    
    The C part of this project was compiled with
    gcc version 11.4.0 (Ubuntu 11.4.0-1ubuntu1~22.04),
    which is part of the build-essential package.

    If not already installed, you can
    install gcc 11.4.0 with the follwing commands:

    ```bash
    sudo apt update
    sudo apt install build-essential
    ```
    
    


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
    cd YDTC
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
    cd YDTC
    make clean
    make
    ```
