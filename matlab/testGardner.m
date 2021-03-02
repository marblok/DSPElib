function testGardner

L=8;
dir_name = '../outputs/';
Max=10000;

for ind=1:12,
  tekst=sprintf([dir_name, 'subchannels_%02i.out'], ind-1);
  plik=fopen(tekst, 'rb');
  u{ind}=fread(plik,inf,'float');
  u{ind}=u{ind}(1:2:end)+j*u{ind}(2:2:end);
u{ind}=u{ind}(1:Max);  


% u{ind}=rand(size(u{ind}))/10000*j;
% u{ind}(1:L:end)=2*((rand(size(1:L:length(u{ind})))>0.5)-0.5);
% u{ind}=filter(ones(1,L),1,u{ind});
% u{ind}=filter(ones(1,L)/L,1,u{ind});

% % % delta=[]; delta(6)=1;
% % delta=[]; delta(2)=1;
% delta=[]; delta(4)=1;
% u{ind}=filter(delta,1, u{ind});
  fclose(plik);
%   plot(real(u{ind})), pause
  
%   length((ind*2-1):(12*2):(length(u{ind})*2*12+ind*2-2))
%   length(u{ind})
  x((ind*2-1):(12*2):(length(u{ind})*2*12+ind*2-2))=real(u{ind});
  x((ind*2):(12*2):(length(u{ind})*2*12+ind*2-1))=imag(u{ind});
end

for ind=1:12, output{ind}.re=[]; output{ind}.im=[]; end; hSP=[]; kor=[];
state=0; NoOfInputsProcessed=0; NoOfInputs=2*12; NoOfChannels=12;
delay=0.0; half_SamplingPeriod=4.0; beta=0.1;
for ind=0:length(x)-1,
  pause(0);
  InputNo=rem(ind,2*12); value=x(ind+1);
  if rem(ind,50*2*12)==0,
    [ind, length(x)], pause(0);
  end
  switch (state)
    case {0}
%       disp('0');
      if (rem(InputNo, 2) ==0)
        y0(floor(InputNo/2)+1).re=value;
      else   
        y0(floor(InputNo/2)+1).im=value;
      end
      NoOfInputsProcessed=NoOfInputsProcessed+1;
    
      if (NoOfInputsProcessed<NoOfInputs)
        Stop=0; % break; %return;
      else
        NoOfInputsProcessed=0;
        delay=delay+half_SamplingPeriod; delay=delay-1;
        state=1; 
      end
      Stop=0;
%       return; //wait for y1
    case {1}
%       disp('1');
      if (delay >= 1.0)
        NoOfInputsProcessed=NoOfInputsProcessed+1;
    
        if (NoOfInputsProcessed<NoOfInputs)
          Stop=0; %break; %  return;
        else
          NoOfInputsProcessed=0;
          delay=delay-1.0; %//next sample
          Stop=0; %break;
  %         return; //still waiting for y1
        end
      end
      if Stop ~=0,
        if (rem(InputNo, 2) ==0)
          y1(floor(InputNo/2)+1).re=value;
        else   
          y1(floor(InputNo/2)+1).im=value;
        end
        NoOfInputsProcessed=NoOfInputsProcessed+1;
      
        if (NoOfInputsProcessed<NoOfInputs)
          Stop=0; %break; %return;
        else
          NoOfInputsProcessed=0;
          if (delay > 0)
            delay=delay-1.0; %//next sample
            state=2; 
            Stop=0; %break;
    %         return; //wait for next sample to do linear interpolation
          else 
            delay=delay+half_SamplingPeriod; delay=delay-1;
            state=3; 
            Stop=0; %break;
    %         return; //wait for y2
          end
        end
      end
%       break;
    case {2}
%       disp('2');
      if (rem(InputNo, 2) ==0)
        y1(floor(InputNo/2)+1).re=y1(floor(InputNo/2)+1).re*(-delay);
        y1(floor(InputNo/2)+1).re=y1(floor(InputNo/2)+1).re+(1+delay)*value;
      else   
        y1(floor(InputNo/2)+1).im=y1(floor(InputNo/2)+1).im*(-delay);
        y1(floor(InputNo/2)+1).im=y1(floor(InputNo/2)+1).im+(1+delay)*value;
      end
      NoOfInputsProcessed=NoOfInputsProcessed+1;

      if (NoOfInputsProcessed<NoOfInputs)
        Stop=0; % break;
      else
        NoOfInputsProcessed=0;
        delay=delay+half_SamplingPeriod; delay=delay-1;
        state=3; 
        Stop=0; %break;
      end
%       return; //wait for y2
    case {3}
%       disp('3');
      if (delay >= 1.0)
        NoOfInputsProcessed=NoOfInputsProcessed+1;
    
        if (NoOfInputsProcessed<NoOfInputs)
          Stop=0; %break; % return;
        else
          NoOfInputsProcessed=0;
          delay=delay-1.0; % //next sample
          Stop=0; %break;
  %         return; //still waiting for y1
        end
      end

      if Stop ~=0,
        if (rem(InputNo, 2) ==0)
          y2(floor(InputNo/2)+1).re=value;
        else   
          y2(floor(InputNo/2)+1).im=value;
        end
        NoOfInputsProcessed=NoOfInputsProcessed+1;
      
        if (NoOfInputsProcessed<NoOfInputs)
          Stop=0; %break;
        else
          NoOfInputsProcessed=0;
          if (delay > 0)
            delay=delay-1.0; %//next sample
            state=4; 
            Stop=0; %break;
    %         return; //wait for next sample to do linear interpolation
          end
        end
%         break;
      end
    case {4}
%       disp('4')
      if (rem(InputNo,2) ==0)
        y2(floor(InputNo/2)+1).re=y2(floor(InputNo/2)+1).re*(-delay);
        y2(floor(InputNo/2)+1).re=y2(floor(InputNo/2)+1).re+(1+delay)*value;
      else   
        y2(floor(InputNo/2)+1).im=y2(floor(InputNo/2)+1).im*(-delay);
        y2(floor(InputNo/2)+1).im=y2(floor(InputNo/2)+1).im+(1+delay)*value;
      end
      NoOfInputsProcessed=NoOfInputsProcessed+1;

      if (NoOfInputsProcessed<NoOfInputs)
        Stop=0; %break; %return;
      else
        NoOfInputsProcessed=0;
      end
%      break;
  end

  if Stop~=0,
    korekta=0;
    for ind=1:NoOfChannels,
      output{ind}.re(end+1)=y1(ind).re;
      output{ind}.im(end+1)=y1(ind).im;
      
%       [y0(ind).re y1(ind).re y2(ind).re (y0(ind).re-y2(ind).re)]
% %       [y0(ind).im y1(ind).im y2(ind).im (y0(ind).im-y2(ind).im)]
      korekta=korekta+beta*y1(ind).re*(y0(ind).re-y2(ind).re);
      korekta=korekta+beta*y1(ind).im*(y0(ind).im-y2(ind).im);
%       y1(ind).re*(y0(ind).re-y2(ind).re)
%       pause
    end
%     half_SamplingPeriod=half_SamplingPeriod-korekta;
    delay=delay-korekta;
hSP(end+1)=half_SamplingPeriod;
kor(end+1)=korekta;
  
%     //y0 <= y2
%     DSP_complex_ptr temp_y;
    temp_y=y0;
    y0=y2; y2=temp_y;
%     // and wait for y1
%   //  delay+=(half_SamplingPeriod-1);
    delay=delay+half_SamplingPeriod; delay=delay-1;
    state=1; 
  end
  Stop=1;
end

whos
output{1}
output{2}
figure(1)
for ind=1:NoOfChannels
  plot(output{ind}.re+j*output{ind}.im, 'ro');
  axis equal;
  pause
end
figure(2)
subplot(2,1,1)
plot(kor)
subplot(2,1,2)
plot(hSP)
