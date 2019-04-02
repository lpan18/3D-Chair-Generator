# this is a temp score file == 
import random
file1 = open("score.txt","w")  
score = random.uniform(0, 1)
file1.write(str(score)) 
file1.close() 