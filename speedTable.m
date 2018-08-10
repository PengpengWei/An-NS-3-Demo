function [trimmedStruct, trimmedData, rawData] = speedTable(fileName)
% [trimmedData, rawData] = speedTable(fileName)
%
% In the trimmedData: [Time; file size; time interval; Rate]
% In the rawData: [No.; Time; file size]
%
    rawData = load(fileName);
    trimmedData = [rawData(2:end,2) rawData(2:end,3) diff(rawData(:,2)) zeros(length(rawData)-1,1)];
    trimmedData = trimmedData(find(trimmedData(:,3) > 0),:);
    trimmedData(:,4) = trimmedData(:,2) ./ trimmedData(:,3);
    mD = mean(trimmedData(:,4)); vD = sqrt(var(trimmedData(:,4)));
    trimmedData = trimmedData(find(trimmedData(:,4) < mD + 3 * vD),:);
    
    trimmedStruct.Time = trimmedData(:,1);
    trimmedStruct.FileSize = trimmedData(:,2);
    trimmedStruct.Interval = trimmedData(:,3);
    trimmedStruct.Rate = trimmedData(:,4);
    trimmedStruct.MeanRate = zeros(size(trimmedStruct.Rate));
    for i = 1 : length(trimmedStruct.Rate)
        trimmedStruct.MeanRate(i) = mean(trimmedStruct.Rate(1:i));
    end
    
end