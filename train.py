import sys
import os
import csv
from optparse import OptionParser

import torch
import torch.nn as nn
import torch.backends.cudnn as cudnn
from torch.autograd import Variable
from torchvision import transforms

# from model import ResNet
from dataloader import DataLoader
from model import ResNet, OriginNet, AlexNet
import numpy as np


def train_net(net, v, epochs=5, lr=0.001, data_root='../../../../Courses_data/LeChairs/chairs-data/', save_cp=True, gpu=True):

    criterion = nn.CrossEntropyLoss()
    # criterion = nn.BCELoss()
    # optimizer = torch.optim.Adam(net.parameters(), lr=0.001)
    optimizer = torch.optim.SGD(net.parameters(), lr=0.001, momentum=0.9)


    train_loader = DataLoader(data_root, 'train', v, 224)
    train_data_loader = torch.utils.data.DataLoader(train_loader,
                                                    batch_size=64,
                                                    shuffle=True,
                                                    num_workers=0)

    for epoch in range(epochs):

        print('Epoch %d/%d' % (epoch + 1, epochs))
        print('Training...')
        net.train()

        epoch_loss = 0

        for i, (data_input, label) in enumerate(train_data_loader):
            
            # torch to float
            input_torch = data_input.float()
            label_torch = label.long()
            # print(label_torch.shape)

            # load image tensor to gpu
            input_torch = Variable(input_torch.cuda())
            label_torch = Variable(label_torch.cuda())

            # get predictions 
            optimizer.zero_grad()
            pred_torch = net(input_torch)
            # print(pred_torch.shape)

            loss = criterion(pred_torch, label_torch)

            # optimize weights
            loss.backward()
            optimizer.step()

            # record loss
            epoch_loss += loss.item()
            # print('Epoch %d | Iteration %d - Loss: %.6f' % (epoch+1, i+1, loss.item()))

            if (i+1)%50==0:
                print('Epoch %d | Iteration %d - Loss: %.6f' % (epoch+1, i+1, loss.item()))
                gt_label = label_torch.detach().cpu()
                pred = pred_torch.detach().cpu()
                classes = np.argmax(pred.numpy(), axis=1)
                accuracy = sum(classes == gt_label.numpy()) / classes.shape
                print('acurracy', accuracy)

        # save model when necessary
        if (epoch+1)%5==0:
            torch.save(net.state_dict(), 'output_%s/epoch%d.pth' % (v, epoch + 1))
            print('Checkpoint %d saved!' % (epoch + 1))
        
        # show loss per epoch
        print('Epoch %d finished! - Loss: %.6f' % (epoch+1, epoch_loss / (i + 1)))
            
    
def get_args():
    parser = OptionParser()
    parser.add_option('-e', '--epochs', dest='epochs', default=20, type='int', help='number of epochs')
    parser.add_option('-g', '--gpu', action='store_true', dest='gpu', default=True, help='use cuda')

    (options, _) = parser.parse_args()
    return options

if __name__ == '__main__':
    args = get_args()

    view = ['Top', 'Front', 'Side']
    os.makedirs('output_Top', exist_ok=True)
    os.makedirs('output_Front', exist_ok=True)
    os.makedirs('output_Side', exist_ok=True)

    for v in view:
        net = ResNet()
        # net = OriginNet()
        # net = AlexNet()

        if args.gpu:
            net.cuda()
            cudnn.benchmark = True

        train_net(net=net, v=v, epochs=args.epochs, gpu=args.gpu)
