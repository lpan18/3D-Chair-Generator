import os
import math
from PIL import Image


def compress(filepath):
    print(filepath)
    file_path = filepath
    image = Image.open(file_path)#.convert('L')
    imw, imh = image.size
    tgt_size = 224
    min_size = min(imw, imh)
    max_size = max(imw, imh)
    start_x = math.floor((max_size - min_size) / 2) 
    end_x = start_x + min_size   
    area = (start_x, 0, end_x, min_size)
    cropped_img = image.crop(area)
    cropped_img = cropped_img.resize((tgt_size, tgt_size))
    cropped_img.save(filepath)

filenames = []
# source_path = "/Users/trailingend/Documents/SFU/CMPT764/chair-modeling/ChairImagesLegOffset"

source_path = "/media/lei/XIAO/Project/chair-modeling/Rendered"

for root, dirs, files in os.walk(source_path):
    for file in files:
        if '.bmp' in file:
            filenames.append(os.path.join(root, file))

for i in range(0, len(filenames)):
    compress(filenames[i])