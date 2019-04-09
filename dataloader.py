import os
import random
import numpy as np
from common import show_image
from PIL import Image, ImageDraw
import torch
from torchvision import transforms
from torch.utils.data import Dataset, DataLoader


class DataLoader():
    def __init__(self, root_dir='./chairs-data/', mode='train', view='Top', dimension=224):
        '''
            Function to load the dataset list
            :param root_dir [str] - parent folder
            :param mode [str] - Top, Front, Side
        '''
        self.root_dir = os.path.abspath(root_dir)
        self.mode = mode
        self.view = view
        self.dimension = dimension
        self.imagesTop = []
        self.imagesSide = []
        self.imagesFront = []
        self.load_data(self.root_dir, self.dimension);
        # print(self.imagesTop)

    def __len__(self):
        '''
            Function to get length of the dataset list
        '''
        return len(self.imagesTop)
    

    def load_data(self, root_dir, dimension):
        '''
            Function to load data
        '''
        isPositive = False
        ls = 0

        if self.mode == 'train':
            for folder in [root_dir + "/positive/", root_dir + "/negative/"]:
                isPositive = not isPositive

                length = len(os.listdir(folder)) // 3

                for filename in os.listdir(folder):

                    view = filename.split(".")[0].split("-")[1]
                    img_path = folder + filename

                    if img_path is not None:
                        if view == 'side':
                            self.imagesSide.append(img_path)
                        elif view == 'top':
                            self.imagesTop.append(img_path)
                        else:
                            self.imagesFront.append(img_path)

                    # view = int(filename.split(".")[0])
                    # view = view % 3
                    # img_path = folder + filename

                    # if img_path is not None:
                    #     if view == 1:
                    #         self.imagesSide.append(img_path)
                    #     elif view == 2:
                    #         self.imagesTop.append(img_path)
                    #     else:
                    #         self.imagesFront.append(img_path)

                # if use one-hot: cross entropy
                # if just 1 or 0: sigmoid ~ bceloss
                if isPositive:
                    self.y_vec_top = np.ones((length), dtype=np.int)
                    self.y_vec_side = np.ones((length), dtype=np.int)
                    self.y_vec_front = np.ones((length), dtype=np.int)

                else:
                    self.y_vec_top = np.append(self.y_vec_top, np.zeros((length), dtype=np.int), axis=0 )
                    self.y_vec_side = np.append(self.y_vec_side, np.zeros((length), dtype=np.int), axis=0 )
                    self.y_vec_front = np.append(self.y_vec_front, np.zeros((length), dtype=np.int), axis=0 )

        
        else:
            folder = root_dir+'/'
            length = len(os.listdir(folder)) // 3

            for filename in os.listdir(folder):

                view = int(filename.split(".")[0])
                view = view % 3
                img_path = folder + filename

                if img_path is not None:
                    if view == 1:
                        self.imagesSide.append(img_path)
                    elif view == 2:
                        self.imagesTop.append(img_path)
                    else:
                        self.imagesFront.append(img_path)


    def setMode(self, mode):
        '''
            Function to set mode of the dataset list
        '''
        self.mode = mode


    def augment_data(self, img_original):
        # how_much = 90 * random.randint(0, 3)
        # how_often = random.uniform(0, 1)
        # if how_often > 0.5:
            # img_original = transforms.functional.rotate(img_original, how_much)
        methods = transforms.Compose([
            # transforms.RandomHorizontalFlip(p=0.5),
            # transforms.RandomVerticalFlip(p=0.5),
            transforms.Resize(self.dimension),
            transforms.ToTensor()
        ])
        return methods(img_original)
    

    def pre_process(self, img_original):
        methods = transforms.Compose([
            transforms.Resize(self.dimension),
            transforms.ToTensor()
        ])
        return methods(img_original)


    def __getitem__(self, idx):
        '''
            Function to get one item from the dataset list
            :param idx [int] - index of item to get
        '''
        item_path = ''
        data_truth = 0

        if self.mode == 'train':
            if self.view == 'Top':
                item_path = self.imagesTop[idx]
                data_truth = self.y_vec_top[idx]
            elif self.view == 'Side':
                item_path = self.imagesSide[idx]
                data_truth = self.y_vec_side[idx]
            elif self.view == 'Front':
                item_path = self.imagesFront[idx]
                data_truth = self.y_vec_front[idx]
        else:
            if self.view == 'Top':
                item_path = self.imagesTop[idx]
            elif self.view == 'Side':
                item_path = self.imagesSide[idx]
            elif self.view == 'Front':
                item_path = self.imagesFront[idx]


        data_input = Image.open(item_path).convert('L')

        # data augmentation
        if self.mode == 'train':
            data_input = self.augment_data(data_input)
        else:
            data_input = self.pre_process(data_input)
        
        data_input = 1. - data_input / 255.

        return (data_input, data_truth)


if __name__ == '__main__':
    loader = DataLoader('../../../../Courses_data/LeChairs/chairs-data/', mode='train', view='Top')
    train_data_loader = torch.utils.data.DataLoader(loader,
                                                    batch_size=1,
                                                    shuffle=True,
                                                    num_workers=0)
    idx, (image, label) = next(enumerate(train_data_loader))

    show_image(image, is_tensor=True)

