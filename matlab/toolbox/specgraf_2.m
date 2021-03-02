function varargout = specgraf_2(Akcja, param)
% Spectrogram for long signals
%
% last modification: 2006.09.14
% Author: Marek Blok

%cos(cumsum(n/1000))

% eval_specgram

% audioplayer, audiorecorder !!!
% waitbar

% \fixed 2005.11.20 dealt with "Warning: Log of zero." in ylim evaluation
% \fixed 2005.11.20 Placed all in single file
% \fixed 2005.11.20 Fixed problems with zoom out

if nargin == 0  % LAUNCH GUI

% 	fig = openfig(mfilename,'reuse'); return; %ZapiszFig(1,'specGUI.m')
  if isempty(findobj(0, 'tag', 'Specgraf_DrawFig_2')) 
    
    fig = Specgraf_DrawFig_2;
    set(fig,'DoubleBuffer', 'on');
    set(fig,'tag', 'Specgraf_DrawFig_2', 'Units', 'pixels');
    set(fig,'name', 'Spektrogram d³ugich sygna³ów niestacjonarnych (2006.03.23) dr in¿. Marek Blok',...
      'KeyPressFcn','1;');
    
    specgraf_2('Init');
    specgraf_2('signal');
    set(fig,'Visible','on');
%    specgraf_2('per');
  else
    specgraf_2('Exit');
  end
  return;
end;

if ~isstr(Akcja) % INVOKE NAMED SUBFUNCTION OR CALLBACK
  disp('Something''s wrong');
  return;
end;

% Generate a structure of handles to pass to callbacks, and store it. 
fig=findobj(0, 'tag', 'Specgraf_DrawFig_2');
hEdit_N=findobj(fig,'tag', 'N_edit');
hEdit_filename=findobj(fig,'tag', 'filename_edit');
hEdit_script=findobj(fig,'tag', 'script_edit');
hEdit_L=findobj(fig,'tag', 'L_edit');
hEdit_dF=findobj(fig,'tag', 'dF_edit');
hEdit_k=findobj(fig,'tag', 'k_edit');
hEdit_O=findobj(fig,'tag', 'O_edit');
hEdit_Os=findobj(fig,'tag', 'Os_edit');
hEdit_w=findobj(fig,'tag', 'w_edit');
h_dB=findobj(fig,'tag', 'dB_checkbox');
ha=get(fig, 'UserData');
if length(ha) == 3,
  ha_spec = ha(1); ha_per = ha(2); ha_time = ha(3);
else
  ha_spec = []; ha_per = []; ha_time = [];
end
hEdit_dY=findobj(fig,'tag', 'dY_edit');

if strcmp(Akcja, 'Exit')
  close(fig);
  return;
elseif strcmp(Akcja, 'Init')
  set(hEdit_filename,'UserData','test.wav');
  set(hEdit_filename,'String','test.wav');
  set(hEdit_filename,'Max', 1);

  set(hEdit_L,'UserData','-1');
  set(hEdit_L,'String','-1');
  
  set(hEdit_dF,'UserData','10');
  set(hEdit_dF,'String','10');
  set(hEdit_k,'UserData','2');
  set(hEdit_k,'String','2');

  set(hEdit_N,'UserData','16');
  set(hEdit_N,'String','16');
  set(hEdit_O,'UserData','50');
  set(hEdit_O,'String','50');
  
  set(hEdit_w,'UserData','blackman(M)');
  set(hEdit_w,'String','blackman(M)');

  set(hEdit_Os,'UserData','66');
  set(hEdit_Os,'String','66');

  set(h_dB,'UserData',1);
  set(h_dB,'Value',1);
  
  ha_spec=findobj(fig,'tag', 'spec_axes');
  ha_per=findobj(fig,'tag', 'per_axes');
  ha_time=findobj(fig,'tag', 'time_axes');
%  for ind=[1 2 4], axes(ha(ind)); zoom on; end;
%  axes(ha_spec); zoom off;
  
  set(fig, 'UserData', [ha_spec, ha_per, ha_time]);

%   set(hMenu,'UserData', [NaN, NaN, NaN, NaN, NaN, NaN, NaN, 1, -100, 5, -100, 5]); %handles & maximal values
  draw_params.hp_spec = NaN;
  draw_params.hp_per = NaN;
%   draw_params.hp_spec_slice = NaN;
  draw_params.hp_spec_t2 = NaN;
  draw_params.hp_per2 = NaN;
  draw_params.min_spec = 0;
  draw_params.max_spec = 1;
  draw_params.min_per = 0;
  draw_params.max_per = 1;
  setappdata(fig, 'draw_params', draw_params);

  set(hEdit_dY,'String','90');
  return;
  
elseif strcmp(Akcja, 'signal')
  if nargin==2,
    Ktory=param;
  else
    %save strings
    tekst=[get(hEdit_L, 'String') 0];
    set(hEdit_L,'UserData', setstr(tekst));
    
    tekst=[get(hEdit_filename, 'String') 0];
    set(hEdit_filename,'UserData', setstr(tekst));
  end;

  %generate signal
  tekstL=get(hEdit_L,'UserData');
  ind=find(tekstL==0);
  if ~isempty(ind)
    tekstL=tekstL(1:ind(1)-1);
  end;
  eval(['L=' tekstL ';'], 'L=1;')

  n=0:L-1;
  tekst_filename=get(hEdit_filename,'UserData');
  ind=find(tekst_filename==0);
  if ~isempty(ind)
    tekst_filename=tekst_filename(1:ind(1)-1);
  end;
  L_tmp = L; 
  if L_tmp <=0, L_tmp =1000; end;
  if (L >=100*48000) | (L <= 0), L = 100*48000; end;
%   L_tmp =1000000; 

% SIZ=WAVREAD(FILE,'size')
  eval(['size_tmp = fileread(''' tekst_filename ''', ''size'');'], 'size_tmp = -1;');
  if (size_tmp ~= -1) & (max(size_tmp) < L),
    L = max(size_tmp);
  end
  eval(['[x, Fs]=fileread(''' tekst_filename ''', L);'], 'x=randn(1,L_tmp); L = L_tmp; Fs=48000;')

%   size(x,1)
  if size(x,2) == 2,
    x = x(:,1) + j*x(:,2);
  end
    
  x=x(:);
  if L <= 0, L = length(x); end;
  if length(x)<L;
    x(L)=0;
  else
    x=x(1:L);
  end;

  setappdata(fig, 'signal', x);
  setappdata(fig, 'Fs', Fs);
%   max_x(Ktory)=max(abs([real(x); imag(x)]));
   
  %draw signal
  draw_params = getappdata(fig, 'draw_params');
  if isfinite(draw_params.hp_spec),
    delete(draw_params.hp_spec);
    draw_params.hp_spec = NaN;
  end
  if isfinite(draw_params.hp_spec_t2),
    delete(draw_params.hp_spec_t2);
    draw_params.hp_spec_t2 = NaN;
  end
  if isfinite(draw_params.hp_per),
    delete(draw_params.hp_per);
    draw_params.hp_per = NaN;
  end
%   if isfinite(draw_params.hp_spec_slice),
%     delete(draw_params.hp_spec_slice);
%     draw_params.hp_spec_slice = NaN;
%   end
  if isfinite(draw_params.hp_per2),
    delete(draw_params.hp_per2);
    draw_params.hp_per2 = NaN;
  end
  draw_params.min_spec = 0;
  draw_params.max_spec = 1;
  draw_params.min_per = 0;
  draw_params.max_per = 1;
  setappdata(fig, 'draw_params', draw_params);

  %compute and draw periodogram
  specgraf_2('spec')
%  specgraf_2 zoom_on;
  return;
  
elseif strcmp(Akcja, 'spec')
  if nargin==2,
    Ktory=param;
  else
    %save strings
    tekst=[get(hEdit_dF, 'String') 0];
    set(hEdit_dF,'UserData', setstr(tekst));
    tekst=[get(hEdit_k, 'String') 0];
    set(hEdit_k,'UserData', setstr(tekst));
    
    tekst=[get(hEdit_N, 'String') 0];
    set(hEdit_N,'UserData', setstr(tekst));
    tekst=[get(hEdit_O, 'String') 0];
    set(hEdit_O,'UserData', setstr(tekst));
    tekst=[get(hEdit_w, 'String') 0];
    set(hEdit_w,'UserData', setstr(tekst));
    
    tekst=[get(hEdit_Os, 'String') 0];
    set(hEdit_Os,'UserData', setstr(tekst));
  end;

  %get signal
  x=getappdata(fig, 'signal');
  Fs=getappdata(fig, 'Fs');
  draw_params=getappdata(fig, 'draw_params');
  
  
  tekst_k=get(hEdit_k,'UserData');  
  ind=find(tekst_k==0);
  if ~isempty(ind)
    tekst_k=tekst_k(1:ind(1)-1);
  end;
  eval(['k=' tekst_k ';'], 'k=1;')
  
  tekst_dF=get(hEdit_dF,'UserData');  
  ind=find(tekst_dF==0);
  if ~isempty(ind)
    tekst_dF=tekst_dF(1:ind(1)-1);
  end;
  eval(['dF=' tekst_dF ';'], 'dF=1;')
  
  % wielkoœæ segmentu do obliczania periodografu 
  % oraz wielkoœæ transformaty DFT
  M = 2.^(ceil(log2(ceil(Fs/dF))));
  K = M*k;

  tekst_N=get(hEdit_N,'UserData');  
  ind=find(tekst_N==0);
  if ~isempty(ind)
    tekst_N=tekst_N(1:ind(1)-1);
  end;
  eval(['N=' tekst_N ';'], 'N=1;')
%   if M>K
%     M=K;
%     set(hEditM,'String', num2str(M));
%   end;

  tekstO=get(hEdit_O,'UserData');
  ind=find(tekstO==0);
  if ~isempty(ind)
    tekstO=tekstO(1:ind(1)-1);
  end;
%   eval(['O=' tekstO ';'], 'O=0;')
  O=eval(tekstO, '0'); % \Fixed 2005.11.03 
  O=round(O/100*M); % \Fixed 2005.11.03 nak³adkowanie podawane w procentach !!! 

  tekstOs=get(hEdit_Os,'UserData');
  ind=find(tekstOs==0);
  if ~isempty(ind)
    tekstOs=tekstOs(1:ind(1)-1);
  end;
  Os=eval(tekstOs, '0');
  M_segment = N*M-(N-1)*O;
  Os=round(Os/100*M_segment); % \Fixed 2005.11.03 nak³adkowanie podawane w procentach !!! 

  tekstW=get(hEdit_w,'UserData');
  ind=find(tekstW==0);
  if ~isempty(ind)
    tekstW=tekstW(1:ind(1)-1);
  end;
  eval(['w=' tekstW ';'], 'w=ones(1,M);')

  w=w(:);
  if length(w)<M;
    w(M)=0;
  else
    w=w(1:M);
  end;
  
%   x=get(hp_re(Ktory), 'Ydata')+j*get(hp_im(Ktory), 'Ydata');

%   O=floor(O/100*M); % \Fixed 2005.11.03
  if O>=M
    O=M-1;
    set(hEdit_O,'String', num2str(O/M*100));
  end;
%   N = fix((length(x)-O)/(M-O));
%   set(hEditN, 'String', sprintf('%i', N));

%   [Spec, f, t]=specgram(x, K, 1, w, O);
%   Spec=(abs(Spec).^2)/norm(w)^2;
  [Spec, f, t, Per]=eval_specgram(x, K, Fs, w, O, N, Os);
  f2 = f;
%   [Per, f2]=psd(x, K, 1, w, O);
%   Per=abs(Per);


  setappdata(fig, 'f',f); %   set(get(ha(2),'Ylabel'),'UserData', f);
  setappdata(fig, 'Spec',Spec); %   set(get(ha_spec,'Ylabel'),'UserData', Spec);
  setappdata(fig, 'Per',Per); %   set(get(ha_per,'Ylabel'),'UserData', Per);

%   draw_params.min_spec=min([min(Spec(finite(Spec))); 0]);
%   draw_params.max_spec=max([max(Spec(finite(Spec))); 0.001]);
%   draw_params.min_per=min([min(Per(finite(Per))); 0]);
%   draw_params.max_per=max([max(Per(finite(Per))); 0.001]);
  draw_params.min_spec=min([min(Spec(finite(Spec))); 0]);
  draw_params.max_spec=max([max(Spec(finite(Spec))); draw_params.min_spec+0.001]);
  draw_params.min_per=min([min(Per(finite(Per))); 0]);
  draw_params.max_per=max([max(Per(finite(Per))); draw_params.min_per+0.001]);
  if get(h_dB, 'UserData') %dB
    Spec=10*log10(Spec);
    Per=10*log10(Per);
  end
  
  %spektrogram
  if length(t)>1
    t2=t+(t(2)-t(1))/2;
  else
    t2=M/2*(1/Fs);
    t=t2;
  end;

  axes(ha_spec);
  if isnan(draw_params.hp_spec)
    hold on;
    draw_params.hp_spec=image(t2, f, Spec);
    set(draw_params.hp_spec, 'CDataMapping', 'scaled');
    colormap(hot);
    hold off;
  else
    set(draw_params.hp_spec,'Xdata', t2, 'Ydata', f, 'Cdata', Spec);
  end
  if length(t)==1,
    tlim_=[t(1)-0.5 t(1)+0.5];
  else
    tlim_=[t(1) t(length(t))+t(2)-t(1)];
  end;
  if isreal(x)
    set(ha_spec, 'Ylim', [0 Fs/2], 'Xlim', tlim_);
  else
    set(ha_spec, 'Ylim', [-Fs/2 Fs/2], 'Xlim', tlim_);
  end
  set(ha_time, 'Xlim', tlim_);
%  set(get(ha_spec,'ZLabel'),'UserData',[]);    
%  reset(get(ha_spec,'ZLabel'));    
  eval('zoom reset', 'set(get(draw_params.ha_spec,''ZLabel''),''UserData'',[]);');    
  
  axes(ha_per);
  if isnan(draw_params.hp_per)
    hold on;
    draw_params.hp_per=plot(f2, Per, 'Color', 'k');
    hold off;
  else
    set(draw_params.hp_per,'Xdata', f2, 'Ydata', Per, 'Color', 'k');
  end
%   set(ha_per, 'Xdir', 'reverse', 'YTick', []); %, 'Xlim', [t(1) t(length(t))], 'Ylim', [-1.1 1.1]*max(max_x));
  if isreal(x),
    set(ha_per, 'Xlim', [0 Fs/2]);
  else
    set(ha_per, 'Xlim', [-Fs/2 Fs/2]);
  end
  eval('rmappdata(get(draw_params.ha_per,''Zlabel''),''ZOOMAxesData'')','set(get(ha_per,''ZLabel''),''UserData'',[])');

%   set(hMenu,'UserData', [hp_re, hp_im, hp_spec_t, hp_spec_t2, hp_spec, hp_per, hp_per2, max_x, min_spec, max_spec, min_per, max_per]);
  setappdata(fig, 'draw_params', draw_params);
  
%   %  delete
%   if 1
%     if isfinite(draw_params.hp_spec_slice),
%       delete(draw_params.hp_spec_slice)
%       draw_params.hp_spec_slice=NaN;
%       setappdata(fig, 'draw_params', draw_params);
%     end;
%   end
  
  specgraf_2('spec_ylim');
  specgraf_2 zoom_on;
  specgraf_2 zoom_off;
  return;
  
elseif strcmp(Akcja, 'dB')
  pom=get(h_dB, 'UserData');
  if pom~=get(h_dB, 'Value')
%     sygn=getappdata(fig, 'signal');
    Fs=getappdata(fig, 'Fs');
    f=getappdata(fig, 'f');
    per=getappdata(fig, 'Per');
    Spec=getappdata(fig, 'Spec');
    draw_params = getappdata(fig, 'draw_params');
    
    if get(h_dB, 'Value') %dB
%       sygn=10*log10(sygn);
      per=10*log10(per);
      Spec=10*log10(Spec);
%     else %lin
% %       sygn=10.^(sygn/10);
%       per=10.^(per/10);
    end;
%     setappdata(fig, 'Per', per);
    set(h_dB, 'UserData', get(h_dB, 'Value'));

    if isfinite(draw_params.hp_per2),
      delete(draw_params.hp_per2);
      draw_params.hp_per2 = NaN;
    end
    setappdata(fig, 'draw_params', draw_params);
    
    set(draw_params.hp_spec,'Cdata', Spec);
    set(draw_params.hp_per,'Xdata', f, 'Ydata', per);
    
    specgraf_2('spec_ylim');
  end
  return
  
elseif strcmp(Akcja, 'spec_ylim')
  draw_params=getappdata(fig, 'draw_params');
%   pom=get(hMenu,'UserData');
%   hp_re=pom(:,1);
%   hp_im=pom(:,2);
%   hp_spec_t=pom(:,3);
%   hp_spec=pom(:,5);
%   hp_per=pom(:,6);
%   min_spec=pom(:,9);
%   max_spec=pom(:,10);
%   min_per=pom(:,11);
%   max_per=pom(:,12);
  if get(h_dB, 'UserData') %dB
    tekst=get(hEdit_dY,'String');
    eval(['dY=' tekst ';'], 'dY=90;');
    if dY<=0, dY=10; end;
    params_=[min(draw_params.min_spec) max(draw_params.max_spec)];
    ind_params = find(abs(params_) <= eps);
    if length(ind_params) > 0,
      params_(ind_params) = NaN*ones(size(ind_params));
    end
    ylim_=10*log10(params_);
    if ~finite(ylim_(2))
      ylim_(2)=0;
    end
    ylim_(1)=ylim_(2)-dY;
    params_=[min(draw_params.min_per) max(draw_params.max_per)];
    ind_params = find(abs(params_) <= eps);
    if length(ind_params) > 0,
      params_(ind_params) = NaN*ones(size(ind_params));
    end
    ylim_per=10*log10(params_);
    if ~finite(ylim_per(2))
      ylim_per(2)=0;
    end
    ylim_per(1)=ylim_per(2)-dY;
  else
    ylim_=[0 max(draw_params.max_spec)];
    ylim_per=[0 max(draw_params.max_per)];
  end
  ylim_per(2)=max([ylim_per(2) ylim_(2)]);
%   f=getappdata(fig, 'f'); %get(get(ha(2),'Ylabel'),'UserData');
%   Spec=getappdata(fig, 'Spec'); %get(get(ha_spec,'Ylabel'),'UserData');
%   Per=getappdata(fig, 'Per'); %get(get(ha_per,'Ylabel'),'UserData');
%   Spec=64*(Spec-ylim_(1))/(ylim_(2)-ylim_(1));
% %   colormap(hot)

%   set(draw_params.hp_spec,'Cdata', Spec);
  set(ha_spec,'Clim', ylim_per);
%   set(draw_params.hp_per,'Xdata', f, 'Ydata', Per);
  set(ha_per,'Ylim', ylim_per);
  set(ha_time,'Ylim', ylim_per);

%   if get(h_dB, 'UserData') %dB
%     set(hp_spec_t(Ktory),'Ydata', 20*log10(abs(get(hp_re(Ktory),'Ydata')+j*get(hp_im(Ktory),'Ydata'))));
%   else
%     set(hp_spec_t(Ktory),'Ydata', abs(get(hp_re(Ktory),'Ydata')+j*get(hp_im(Ktory),'Ydata')));
%   end
  
  SPECgraf_2 zoom_on;
  eval('zoom reset', 'set(get(ha_spec,''ZLabel''),''UserData'',[]);');    
  SPECgraf_2 zoom_off;
  return
elseif strcmp(Akcja, 'zoom_on')
  zoom on;
  pom=get(fig, 'WindowButtonDownFcn');
  set(fig, 'WindowButtonDownFcn', 'specgraf_2 zoom');
  set(get(ha_spec,'Xlabel'), 'UserData', pom);
  return;  
elseif strcmp(Akcja, 'zoom_off')
  if get(findobj(fig,'tag','checkbox_zoom'),'Value') == 0,
    pom = get(get(ha_spec,'Xlabel'), 'UserData');
    set(fig, 'WindowButtonDownFcn', pom);
    zoom off;
    set(fig, 'WindowButtonDownFcn', 'specgraf_2 zoom');
    set(get(ha_spec,'Xlabel'), 'UserData', '1;');
  end
  return;  
elseif strcmp(Akcja, 'zoom_spec')
  if get(findobj(fig,'tag','checkbox_zoom'),'Value') ~= 0,
    Specgraf_2 zoom_on;
  else
    Specgraf_2 zoom_off;
  end
elseif strcmp(Akcja, 'zoom')
%  if strcmp(get(fig,'SelectionType'),'normal') | (gca~=ha_spec)
  pause(0);
  if (get(gco,'Parent')~=ha_spec) | get(findobj(fig,'tag','checkbox_zoom'),'Value')
    eval(get(get(ha_spec,'Xlabel'), 'UserData'));
  elseif get(gco,'Parent')==ha_spec
    pom=get(ha_spec, 'CurrentPoint');
    f_=pom(1,2); t_=pom(1,1);

    draw_params = getappdata(fig, 'draw_params');
%     f=get(draw_params.hp_spec, 'Ydata');
    f=getappdata(fig, 'f');
    ind_f=find(abs(f-f_)==min(abs(f-f_))); ind_f=ind_f(1);
    t=get(draw_params.hp_spec, 'Xdata');
%    if length(t)>1,
%      t=t+(t(2)-t(1))/2;
%    end;
    ind_t=find(abs(t-t_)==min(abs(t-t_)));
    ind_t=ind_t(1);
    set(findobj(fig,'tag', 'text_t_f'),...
      'ForegroundColor', 'r', 'String', sprintf('n=%i, f=%6.3f', t(ind_t),f(ind_f)));

    Spec=getappdata(fig, 'Spec');
    if length(Spec(:,ind_t))>length(f)
      ind=find(f==0);
      Spec(ind(1),:)=[];
    end;
    if get(h_dB, 'Value') %dB
      axes(ha_per);
      if isnan(draw_params.hp_per2)
        hold on;
%         draw_params.hp_per2=plot(f, 10*log10(Spec(:,ind_t)), 'Color', 'r', 'LineWidth', 2);
        draw_params.hp_per2=plot(f, 10*log10(Spec(:,ind_t)), 'Color', 'r', 'LineWidth', 1);
        hold off;
      else
        set(draw_params.hp_per2,'Xdata', f, 'Ydata', 10*log10(Spec(:,ind_t)));
      end
      axes(ha_time);
      if isnan(draw_params.hp_spec_t2)
        hold on;
        draw_params.hp_spec_t2=plot(t, 10*log10(Spec(ind_f,:)), 'Color', 'b', 'LineWidth', 1);
        hold off;
      else
        set(draw_params.hp_spec_t2,'Xdata', t, 'Ydata', 10*log10(Spec(ind_f,:)));
      end
    else
      axes(ha_per);
      if isnan(draw_params.hp_per2)
        hold on;
        draw_params.hp_per2=plot(f, Spec(:,ind_t), 'Color', 'r', 'LineWidth', 2);
        hold off;
      else
        set(draw_params.hp_per2,'Xdata', f, 'Ydata', Spec(:,ind_t));
      end
      axes(ha_time);
      if isnan(draw_params.hp_spec_t2)
        hold on;
        draw_params.hp_spec_t2=plot(t, Spec(ind_f,:), 'Color', 'b', 'LineWidth', 1);
        hold off;
      else
        set(draw_params.hp_spec_t2,'Xdata', t, 'Ydata', Spec(ind_f,:));
      end
    end;

    setappdata(fig, 'draw_params', draw_params);
  end;
 return;  
end;


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [Spec, f, t, Per]=eval_specgram_1(x, K, Fs, w, O, N, Os)
global spegraf_progress_stop; 
spegraf_progress_stop=0;
hb=waitbar(0,'Spectrogram evaluation in progress','CreateCancelBtn','global spegraf_progress_stop; spegraf_progress_stop=1;');

try
	L=length(x);
	M=length(w);
	% M próbek -> okienkowanie -> fft o d³ugoœci K
	% przesuñ o (M - O) i tak N razy
	%  pocz¹tki segmentów: 1, 1+(M-O) = M-O+1, 1+2*(M-O) = 2*M-2*O+1, ... , 1+(N-1)*(M-O) = (N-1)*M-(N-1)*O+1
	%  koñce segmentów: M, M+(M-O) = 2*M-O, M+2*(M-O) = 3*M-2*O, ... , M+(N-1)*(M-O) = N*M-(N-1)*O
	M_segment = N*M-(N-1)*O;
	
	if L < M_segment,
      x(M_segment) = 0;
      L = M_segment;
	end
	
	Spec=[]; t = []; Per = 0;
	start = 1; koniec = M_segment;
  ile = 0; w_skala = norm(w)^2;
	while koniec <= L,
	%   x_ = x(start:koniec).*w;
      t=[t, ((start+koniec)/2-1)/Fs];
      
      X = 0;
      for ind=0:N-1,
        x_ = x(start+(ind)*(M-O)+[0:M-1]);
        x_ = x_(:).*w(:);
        X_ = fft(x_, K);
        
        if isreal(x),
          X_ = abs(X_(1:end/2+1));
        else
          X_ = fftshift(abs(X_));
        end
        X_ = (X_.^2)/M/w_skala;
%   Spec=(abs(Spec).^2)/norm(w)^2;
        
        X=X+X_;
      end
      X = X/N;
      Spec(:,end+1)=X(:);
      Per=Per+X;
      ile = ile + 1;
      
      start = start + (M_segment - Os);
      koniec = koniec + (M_segment - Os);
      waitbar(koniec/L, hb);
      if spegraf_progress_stop == 1,
        break;
      end
	end
  Per=Per/ile;
	
  if isreal(x),
  	f = linspace(0, Fs/2, K/2+1);
  else
  	f = linspace(-Fs/2, Fs/2, K+1);
    f(end) = [];
  end
catch
  test = 0;
end
delete(hb);
clear global spegraf_progress_stop;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [Spec, f, t, Per]=eval_specgram(x, K, Fs, w, O, N, Os)
global spegraf_progress_stop; 
spegraf_progress_stop=0;
hb=waitbar(0,'Spectrogram evaluation in progress','CreateCancelBtn','global spegraf_progress_stop; spegraf_progress_stop=1;');

try
	L=length(x);
	M=length(w);
  
  offset_ = round(Os/M); 
  if offset_ <= 0, offset = 1; end;
  if offset_ > N, offset = N; end;
  
	% M próbek -> okienkowanie -> fft o d³ugoœci K
	% przesuñ o (M - O) i tak N razy
	%  pocz¹tki segmentów: 1, 1+(M-O) = M-O+1, 1+2*(M-O) = 2*M-2*O+1, ... , 1+(N-1)*(M-O) = (N-1)*M-(N-1)*O+1
	%  koñce segmentów: M, M+(M-O) = 2*M-O, M+2*(M-O) = 3*M-2*O, ... , M+(N-1)*(M-O) = N*M-(N-1)*O
	M_segment = N*M-(N-1)*O;
	
	if L < M_segment,
      x(M_segment) = 0;
      L = M_segment;
	end
	
	Per = 0;
	start = 1; koniec = M;
  ile = 0; 
%   w_skala = norm(w)^2
  w_skala = mean(w.^2);
  if isreal(x),
    temp_X = zeros(K/2+1, N); ind = N;
    ile_tmp=ceil((L-M_segment+(M-O))/((M-O)*offset_));
    Spec = zeros(K/2+1, ile_tmp); t = zeros(1, ile_tmp); 
  else
    temp_X = zeros(K, N); ind = N;
    ile_tmp=ceil((L-M_segment+(M-O))/((M-O)*offset_));
    Spec = zeros(K, ile_tmp); t = zeros(1, ile_tmp); 
  end
  segment_ind_tmp = 1;
	while koniec <= L,
	%   x_ = x(start:koniec).*w;
%     t=[t, ((start+koniec)/2-1)/Fs];
    

    x_ = x(start+[0:M-1]);
    x_ = x_(:).*w(:);
    X_ = fft(x_, K);
    if isreal(x),
      X_ = abs(X_(1:end/2+1));
    else
      X_ = fftshift(abs(X_));
    end
    
%     temp_X(:,ind) = (X_.^2)/M/w_skala;
    temp_X(:,segment_ind_tmp) = (X_.^2)/M/w_skala;
    segment_ind_tmp = rem(segment_ind_tmp, N) + 1;
    
    ind = ind -1;
    if ind == 0,
      X = 0;
      for ind_=1:N,
        X=X+temp_X(:,ind_);
      end
      X = X/N;

%       for ind_=offset_:N-1,
%         temp_X(:,ind_+1) = temp_X(:,ind_);
%       end
      
%       Spec(:,end+1)=X(:);
      t(ile+1)=((start+koniec)/2-1)/Fs;
      Spec(:,ile+1)=X(:);
      Per=Per+X;
      ile = ile + 1;
        
      ind = offset_;
    end
      
    start = start + (M-O);
    koniec = koniec + (M-O);
    waitbar(koniec/L, hb);
    if spegraf_progress_stop == 1,
      break;
    end
	end
  Per=Per/ile;
  if ile < ile_tmp,
    Spec = Spec(:, 1:ile);
    t = t(1:ile); 
  end  
	
  if isreal(x),
  	f = linspace(0, Fs/2, K/2+1);
  else
  	f = linspace(-Fs/2, Fs/2, K+1);
    f(end) = [];
  end
catch
  test = 0;
end
delete(hb);
clear global spegraf_progress_stop;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function fig_h=Specgraf_DrawFig_2
fig_h=figure;
set(fig_h, ...
  'Units', 'Normalized',...
  'Position', [0.1047 0.1146 0.7813 0.7448],...
  'Color', [1.0000 1.0000 1.0000],...
  'Name', 'Untitled',...
  'NumberTitle', 'off',...
  'Menu', 'none',...
  'Tag', 'figure1'...
  );
h=axes;
set(h, ...
  'Units', 'Normalized',...
  'Position', [0.0330 0.5203 0.9540 0.1413],...
  'Color', [1.0000 1.0000 1.0000],...
  'XColor', [0.0000 0.0000 0.0000],...
  'YColor', [0.0000 0.0000 0.0000],...
  'FontSize', 10,...
  'Box', 'on',...
  'Tag', 'time_axes'...
  );
h=axes;
set(h, ...
  'Units', 'Normalized',...
  'Position', [0.5490 0.7175 0.4400 0.2769],...
  'Color', [1.0000 1.0000 1.0000],...
  'XColor', [0.0000 0.0000 0.0000],...
  'YColor', [0.0000 0.0000 0.0000],...
  'FontSize', 10,...
  'Box', 'on',...
  'Tag', 'per_axes'...
  );
h=axes;
set(h, ...
  'Units', 'Normalized',...
  'Position', [0.0330 0.0825 0.9550 0.3846],...
  'Color', [1.0000 1.0000 1.0000],...
  'XColor', [0.0000 0.0000 0.0000],...
  'YColor', [0.0000 0.0000 0.0000],...
  'FontSize', 10,...
  'Box', 'on',...
  'Tag', 'spec_axes'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'frame',...
  'Units', 'Normalized',...
  'Position', [0.0050 0.7483 0.2450 0.2420],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', '',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'frame2'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'frame',...
  'Units', 'Normalized',...
  'Position', [0.2550 0.7497 0.2450 0.2406],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', '',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'frame4'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.4480 0.9217 0.0370 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''spec'')',...
  'Tag', 'k_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.4140 0.9273 0.0310 0.0252],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'k =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text25'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.0180 0.8140 0.2210 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''signal'')',...
  'Tag', 'spript_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.0120 0.8503 0.1380 0.0280],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'preprocessing script =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text24'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'pushbutton',...
  'Units', 'Normalized',...
  'Position', [0.0110 0.7035 0.1180 0.0378],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'Refresh',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'SPECgraf_2 Refresh',...
  'Tag', 'pushbutton6'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'checkbox',...
  'Units', 'Normalized',...
  'Position', [0.8720 0.0126 0.0660 0.0196],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'zoom',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'Specgraf_2 zoom_spec',...
  'Tag', 'checkbox_zoom'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'pushbutton',...
  'Units', 'Normalized',...
  'Position', [0.0140 0.0098 0.0660 0.0378],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'Exit',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'SPECgraf_2 Exit',...
  'Tag', 'pushbutton5'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.4640 0.0028 0.3540 0.0350],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', '',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text_t_f'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.1750 0.0126 0.0750 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''spec_ylim'')',...
  'Tag', 'dY_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.1110 0.0154 0.0620 0.0308],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'dY [dB] =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text21'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'checkbox',...
  'Units', 'Normalized',...
  'Position', [0.4480 0.7091 0.0500 0.0322],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'dB',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''dB'')',...
  'Tag', 'dB_checkbox'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.2930 0.8643 0.0570 0.0406],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''spec'')',...
  'Tag', 'N_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.2630 0.8741 0.0300 0.0266],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'N =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text18'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.0470 0.7608 0.1110 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''signal'')',...
  'Tag', 'L_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.0170 0.7636 0.0290 0.0266],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'L =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text17'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.4130 0.8685 0.0750 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''spec'')',...
  'Tag', 'O_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.3600 0.8699 0.0500 0.0280],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'O [%] =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text16'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.3010 0.8196 0.1870 0.0336],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''spec'')',...
  'Tag', 'w_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.2600 0.8210 0.0400 0.0294],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'w[n] =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text15'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.3240 0.7692 0.0750 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''spec'')',...
  'Tag', 'Os_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.2620 0.7706 0.0530 0.0308],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'Os [%] =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text13'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.2600 0.9622 0.1470 0.0266],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'Spectrograf parameters',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text10'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.3250 0.9231 0.0750 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''spec'')',...
  'Tag', 'dF_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.2640 0.9273 0.0550 0.0266],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'dF [Hz] =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text9'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'edit',...
  'Units', 'Normalized',...
  'Position', [0.0190 0.8923 0.2210 0.0364],...
  'BackGroundColor', [1.0000 1.0000 1.0000],...
  'String', 'Edit Text',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', 'specgraf_2(''signal'')',...
  'Tag', 'filename_edit'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.0130 0.9287 0.0990 0.0266],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'wave filename =',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text4'...
  );
h=uicontrol;
x=version;
if (x(1)>='5'),
  set(h, ...
    'FontSize', 10);
end;
set(h, ...
  'Style', 'text',...
  'Units', 'Normalized',...
  'Position', [0.0120 0.9636 0.1010 0.0238],...
  'BackGroundColor', [0.9255 0.9137 0.8471],...
  'String', 'Testing signal',...
  'Value', 0,...
  'Visible', 'on',...
  'Callback', '',...
  'Tag', 'text2'...
  );
