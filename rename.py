import os
import math
from PIL import Image

filenames = []
source_path = "chairs-data/positive/"
target_path = "chairs-data/positive-rename/"

# for filename in os.listdir(source_path):
for count, filename in enumerate(os.listdir(source_path)):
    filenames.append(filename)

for i in range(0, len(filenames)):
    local_name = filenames[i].split(".")[0]
    view = int(local_name)
    view = view % 3
    # 0 - front - c
    # 1 - side - a
    # 2 - top - b
    if view == 0:
        new_name = local_name + '-front.bmp'
    elif view == 1:
        new_name = local_name + '-side.bmp'
    elif view == 2:
        new_name = local_name + '-top.bmp'
        
    os.rename(source_path + filenames[i], target_path + new_name)
    print(filenames[i], view, new_name)