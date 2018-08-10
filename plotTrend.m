function plotTrend(dataStruct, handler, titleName)
%
% function plotTrend(dataStruct, handler=1, titleName='')
%
%
    if nargin == 1
        handler = 1;
    end
    
    if nargin <= 2
        titleName = '';
    end
    
    figure(handler); hold off;
    T = dataStruct.Time; R = dataStruct.Rate;
    plot(T, R); hold on;
    xlabel('Time(sec)');ylabel('Rate(Byte/s)');
    title(titleName);
    
    plot(T, dataStruct.MeanRate);
    plot(T, tsmovavg(R,'s',100,1));
    plot(T, tsmovavg(R,'s',500,1));
    
    legend('Rate', 'CMA', 'SMA-100', 'SMA-500');
    hold off;
%     Rate = R; CMA = dataStruct.MeanRate; SMA100 = tsmovavg(R,'s',100,1);
%     SMA500 = tsmovavg(R,'s',500,1);
end