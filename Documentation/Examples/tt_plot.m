%Adapted from Brad Christensen's PlotTTTES
function tt_plot(timetagger,channel1,channel2,windowsize) 
    tt_start(timetagger);       %Start taking data
    figure
    %if nargin<1
       reverse=0;
    %end
    width=10;
    triggerCounts=[];
    idlerCounts=[];
    coinCounts=[];
    bestEtas=[];
    bestEtas2=[];
    bestEtas3=[];
    display('Use ctrl + c to exit');
    quitLoop=false;
    while(~quitLoop)
       if n==0
           loops=10;
       else
           if n==1
               loops=10;
           else
               loops=1;
           end
       end
       for i=1:loops
           pause(countduration)%Pause so next collection cycle doesn't overlap with previous collection cycle
           [singles,coincidences] = tt_stats(detectornum,countduration*timebins,pulseradius,offsets);
           array(2) = singles(cdeta+1);%singles
           array(4) = singles(cdetb+1);%singles
           array(3) = coincidences(cdeta+1,cdetb+1) - 1/16*max(2*windowsize,8.3)*10^-9*array(2)*array(4)/countduration;%/(120*10^6);%accidental subtracted coincidences
           
           if reverse==0
               triggerCounts=[triggerCounts array(2)];
               idlerCounts=[idlerCounts array(4)];
           else
               triggerCounts=[triggerCounts array(4)];
               idlerCounts=[idlerCounts array(2)];
           end
           coinCounts=[coinCounts array(3)];
           %[bestEta bestLambdaUncorrelated bestLambdaCorrelated]=GetCoupling(triggerCounts(length(triggerCounts)),idlerCounts(length(triggerCounts)),coinCounts(length(triggerCounts)),41.7*10^3*m*10^(n-1));
           %[bestEta2 bestLambdaUncorrelated bestLambdaCorrelated]=GetCoupling(idlerCounts(length(triggerCounts)),triggerCounts(length(triggerCounts)),coinCounts(length(triggerCounts)),41.7*10^3*m*10^(n-1));
           bestEta=coinCounts(length(coinCounts));%/triggerCounts(length(triggerCounts));
           if isnan(bestEta)
               bestEta=0;
           end
           bestEta2=coinCounts(length(coinCounts))/idlerCounts(length(idlerCounts));
           if isnan(bestEta2)
               bestEta2=0;
           end
           bestEta3=coinCounts(length(coinCounts))/triggerCounts(length(triggerCounts));
           if isnan(bestEta3)
               bestEta3=0;
           end
           %bestEtas=[bestEtas bestEta];
           %bestEtas2=[bestEtas2 bestEta2];
           %bestEtas3=[bestEtas3 bestEta3];

            bestEtas=[bestEtas bestEta];
            bestEta2 = bestEta/array(2);
            %bestEta2 = thirdorder(modes,bestEta/array(2),array(2));
            bestEtas2=[bestEtas2 bestEta2];
            bestEta3 = bestEta/array(4);
            %bestEta3 = thirdorder(modes,bestEta/array(4),array(4));
            bestEtas3=[bestEtas3 bestEta3];
           subplot(2,2,1)
           plot(1:length(coinCounts),triggerCounts,'-r','LineWidth',3);
           xlabel('time (s)')
           ylabel('trigger counts')
           %ylim([0 max(triggerCounts)*1.1+1]);
           if length(coinCounts)>width
               xlim([length(coinCounts)-width length(coinCounts)]);
               ylim([0 max(triggerCounts((length(triggerCounts)-width):length(triggerCounts)))+1])
              % if nargin>1
              %     if resize==1
              %         ylim([0 max(coinCounts
           end
           subplot(2,2,2)
           plot(1:length(idlerCounts),idlerCounts,'LineWidth',3);
           xlabel('time (s)')
           ylabel('idler counts')
           %ylim([0 max(idlerCounts)*1.1+1]);
           if length(triggerCounts)>width
               xlim([length(triggerCounts)-width length(triggerCounts)]);
               ylim([0 max(idlerCounts((length(triggerCounts)-width):length(triggerCounts)))+1])
           end
           subplot(2,2,4)
           plot(1:length(coinCounts),bestEtas2,'LineWidth',3);
           hold on
           plot(1:length(coinCounts),bestEtas3,'-r','LineWidth',3);
           hold off
           xlabel('time (s)')
           ylabel('uncorrected heralding')
           %ylim([.67 .76])
           if length(triggerCounts)>width
               xlim([length(triggerCounts)-width length(triggerCounts)]);
               
           end
           subplot(2,2,3)
           plot(1:length(bestEtas),bestEtas,'LineWidth',3);
           xlabel('time (s)')
           ylabel('accidental corrected coin')
           if length(triggerCounts)>width
               xlim([length(triggerCounts)-width length(triggerCounts)]);
           end

          % subplot(2,2,3)
          % plot(1:length(idlerCounts),log(idlerCounts)/log(10),'LineWidth',3);
          % xlabel('time (s)')
          % ylabel('idler counts')
          % if length(triggerCounts)>width
          %     xlim([length(triggerCounts)-width length(triggerCounts)]);
          % end
          % subplot(2,2,4)
          % plot(1:length(idlerCounts),idlerCounts,'LineWidth',3);
          % xlabel('time (s)')
          % ylabel('idler counts')
          % if length(triggerCounts)>width
          %     xlim([length(triggerCounts)-width length(triggerCounts)]);
          % end

           drawnow
       end

       %ch=getkeywait(.1);
       %if ch~=-1
       %    quitLoop=true;
       %end
    end
    tt_stop(timetagger)
