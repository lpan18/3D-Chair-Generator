import sys
import os
from optparse import OptionParser

import torch
import torch.nn as nn
import torch.backends.cudnn as cudnn
from torch.autograd import Variable
import csv

from model import ResNet, OriginNet, AlexNet
from dataloader import DataLoader
import numpy as np


def test_net(net, model='epoch30.pth', data_root='./Rendered/', gpu=True):

    view = ['Top', 'Front', 'Side']

    test_evaluations = []

    softmax = nn.Softmax(dim=0)

    for v in view:

        net_state = torch.load('output_' + v + '/' + model)
        net.load_state_dict(net_state)
        net.eval()

        test_loader = DataLoader(data_root, 'test', v, 224)
        test_data_loader = torch.utils.data.DataLoader(test_loader,
                                                    batch_size=1,
                                                    shuffle=False,
                                                    num_workers=0)

        temp = []

        with torch.no_grad():
            for i, (data_input, _) in enumerate(test_data_loader):

                # torch to float
                input_torch = data_input.float()

                input_torch = Variable(input_torch.cuda())
               
                # run net
                pred_torch = net(input_torch)
                # pre = pred_torch.detach().cpu().numpy()[0]
                # print(pred_torch.squeeze())
                # print(softmax(pred_torch.squeeze()))
                
                probabilities = softmax(pred_torch.squeeze())[1]
                # print(probabilities)

                temp.append(probabilities)

        test_evaluations.append(temp)

    evaluation_chairs = np.amin(test_evaluations, axis=0)
    return evaluation_chairs
    # print(evaluation_chairs)

def get_args():
    parser = OptionParser()
    parser.add_option('-m', '--model', dest='model', default='epoch20.pth', help='output directory')
    parser.add_option('-g', '--gpu', action='store_true', dest='gpu', default=True, help='use cuda')

    (options, _) = parser.parse_args()
    return options

def writeFile(evaluation_chairs):
    f = open('score.txt', 'w')
    for item in evaluation_chairs:
        f.write(str(item) + '\n')
    f.close()

if __name__ == '__main__':
    args = get_args()

    net = ResNet()
    # net = OriginNet()
    # net = AlexNet()

    if args.gpu:
        net.cuda()
        cudnn.benchmark = True

    evaluation_chairs = test_net(net=net, model=args.model, gpu=args.gpu)
    writeFile(evaluation_chairs)