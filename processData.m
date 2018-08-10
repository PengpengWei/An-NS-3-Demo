% clear variables
CURPATH = pwd;
cd 'G:\College\Summer\Result of August\Buffer Experiment';

[GenName, GenPath] = uigetfile('*.txt', 'Select data of Generator');
[RecName, RecPath] = uigetfile('*.txt', 'Select data of Receiver');
GenName = [GenPath GenName]; RecName = [RecPath RecName];

cd(CURPATH);
clear CURPATH

GenStruct = speedTable(GenName);
RecStruct = speedTable(RecName);
% GenData = load(GenName); RecData = load(RecName);
% GenTable = [GenData(2:end,3) diff(GenData(:,2)) zeros(length(GenData)-1,1)];
% GenTable = GenTable(find(GenTable(:,2) > 0),:);
% GenTable(:,3) = GenTable(:,1) ./ GenTable(:,2);
% mG = mean(GenTable(:,3)); vG = sqrt(var(GenTable(:,3)));
% GenTable = GenTable(find(GenTable(:,3) < mG + 3 * vG),:);


