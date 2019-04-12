import os
from subprocess import check_output
import subprocess

currentDir = os.path.dirname(os.path.realpath(__file__))
datafolder = currentDir + '\\Completion'

files = [f for f in os.listdir(datafolder) if os.path.isfile(os.path.join(datafolder, f))]
for file in files:
    objpath = datafolder + '\\' + file
    cmd2 = 'pssample.exe ' + objpath + '  2048\n'
    p = subprocess.Popen(cmd2, shell=True)
    p.wait()

files = [f for f in os.listdir(datafolder) if os.path.isfile(os.path.join(datafolder, f))]
train_sample_2048_fileList = []
for file in files:
    if file.endswith('.2048.ply'):
        # print('a')
        train_sample_2048_fileList.append(datafolder + '\\' + file)

with open('result_eval_2048_fileList.txt', 'w') as myfile:
    for item in train_sample_2048_fileList:
        myfile.write("%s\n" % item)