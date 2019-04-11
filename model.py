import torch
import torch.nn as nn
import torch.nn.functional as F
from dataloader import DataLoader
import torchvision.models as models

class ResNet(nn.Module):
    def __init__(self):
        super(ResNet, self).__init__()
        ResNet18 = models.resnet18(pretrained=True) #alexnet
        ResNet18.conv1 = nn.Conv2d(1, 64, kernel_size=(7, 7), stride=(2, 2), padding=(3, 3), bias=False)
        modules = list(ResNet18.children())[:-1]

        self.backbone = nn.Sequential(*modules)
        self.fc1 = nn.Linear(512, 256)
        self.relu = nn.ReLU()

        self.dropout1 = nn.Dropout(p=0.4)

        self.fc2 = nn.Linear(256, 2)

        # self.fc2 = nn.Linear(256, 32)
        # self.dropout2 = nn.Dropout(p=0.5)
        # self.fc3 = nn.Linear(32, 2)
        # self.sigmoid = nn.Sigmoid()

    def forward(self, x):
        # print(x.shape)
        out = self.backbone(x)
        # print('backbone', out.shape)
        out = out.view(out.size(0), -1)
        # print('view', out.shape)
        out = self.relu(self.fc1(out))
        # print('fc1',out.shape)
        out = self.dropout1(out)
        out = self.fc2(out)

        ###
        # out = self.relu(out)
        # out = self.dropout2(out)
        # out = self.fc3(out)

        # out = self.sigmoid(out)
        ###

        return out

class AlexNet(nn.Module):
    def __init__(self):
        super(AlexNet, self).__init__()
        alexnet = models.alexnet(pretrained=True)
        # print(alexnet)
        alexnet.features[0] = nn.Conv2d(1, 64, kernel_size=(11, 11), stride=(4, 4), padding=(2, 2))
        modules = list(alexnet.children())[:-1]
        self.alexnet = nn.Sequential(*modules)

        self.classifier = nn.Sequential(
                    nn.Dropout(p=0.5),
                    nn.Linear(256 * 6 * 6, 512),
                    nn.ReLU(inplace=True),
                    nn.Dropout(p=0.5),
                    nn.Linear(512, 256),
                    nn.ReLU(inplace=True),
                    nn.Linear(256, 2)
                )

    def forward(self, x):
        out = self.alexnet(x)
        out = out.view(-1, 256*6*6)
        out = self.classifier(out)
        return out

class OriginNet(nn.Module):
    def __init__(self):
        super(OriginNet, self).__init__()
        self.conv = nn.Sequential(
        nn.Conv2d(1, 32, 5, padding=2),
        nn.ReLU(),
        nn.MaxPool2d(2, stride=2),
        nn.Conv2d(32, 64, 5, padding=2),
        nn.ReLU(),
        nn.MaxPool2d(2, stride=2))

        self.classifier = nn.Sequential(
        nn.Linear(64*14*14, 1024),
        nn.ReLU(),
        nn.Dropout(p=0.4),
        nn.Linear(1024, 2)
        # self.fc2 = nn.Linear(1024, 256)
        # self.dropout2 = nn.Dropout(p=0.5)
        # self.fc3 = nn.Linear(256, 2)
        )

        # self.fc3 = nn.Linear(256, 2)


    def forward(self, x):
        out = self.conv(x)
        out = out.view(-1, 64*14*14)
        out = self.classifier(out)

        return out



if __name__ == '__main__':
    loader = DataLoader('../../../../Courses_data/LeChairs/chairs-data/', 'train')
    train_data_loader = torch.utils.data.DataLoader(loader,
                                                    batch_size=2,
                                                    shuffle=False,
                                                    num_workers=0)
    idx, (image, label) = next(enumerate(train_data_loader))
    # net = ResNet()
    net = OriginNet()
    net.forward(image)