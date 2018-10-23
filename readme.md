# A simple NS-3 demo
## Purpose
This project aims to explore impacts on network traffic throughput brought by buffer size limitation under UDP cases.

## How to run a simulation and get start to analyze data?
### Prerequisit
To run the simulation code, [NS-3 environment](https://www.nsnam.org/) is needed. The simulation may takes about 1 hr, largely depending on the performance of your computer.
### Get raw data from *.cap
To export the data from the simulation results with a "csv" format, you may use network packet analyzer solfware, such as wireshark, winpcap, etc. 
### Extract data from *.csv
readCsvResult.py facilitates the process of obtain raw data. It can be run with Python 3, with several packages, namely csv, easygui, numpy and pandas. This step is not a necessity.
### Data processing
Once your get the raw data with the right form either by Python or by hand, speedTable.m is ready to run to calculate the bit rates of each packet.
### Figure Plotting
Congrates! Now you can execute plotTrend.m to plot figures of transmission rates. The overall trend of throughput rate is reflected on the figures.

## My results
See ./report/report.pdf