import csv
import easygui
import numpy as np
import pandas as pd

fileName = easygui.fileopenbox()
df = pd.read_csv(fileName)
df1 = pd.DataFrame([df['No.'],df['Time'],df['Length']]).T
dataList = df1.values.tolist()

fileName = fileName[:-3] + 'txt'
with open(fileName, 'w+') as f:
    for entry in dataList:
        for item in entry:
            f.write(str(item))
            f.write(' ')
        f.write('\n')
