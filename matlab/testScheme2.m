% \todo dodaæ w DSP_lib 
%  Bloczki(ind).params="wo=0.1223;M=12;";
%  Bloczki(ind).clock.M=10;
%  Bloczki(ind).clock.L=3;
function testScheme2
kolory = 'brgmcyk';
% dane o schemacie algorytmu
% % I. Bloczki
Bloczki = test_scheme_file;
Bloczki = preprocess_bloczki(Bloczki);

% do testów losowe wymieszanie wprowadzonych bloczków

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% przetworzenie informacji o bloczkach na strukturê 
% schematu blokowego

% % 1) dla ka¿dego bloczka dodatkowo utwórz informacje
% - poziom poszczególnych wejœæ
% - poziom wyjœæ (wspólny dla wszytkich)
% - offset przesuniêcia horyzontalnego


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% initialization
for ind = 1:length(Bloczki),
  Bloczki(ind).output_order = -1; % not set yet
  Bloczki(ind).horizontal_offset = 0; 
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% OdnajdŸ Ÿród³a i bloczki mieszane
clear current_Blocks

ind = 1; out_ind = 1;
while ind <= length(Bloczki),
  if (Bloczki(ind).type == 's') | (Bloczki(ind).type == 'm'),
    current_Blocks(out_ind) = ind; % store index to source block
    Bloczki(ind).output_order = 0; % set base (0) level for outputs
    
%     Bloczki(ind).horizontal_offset = 1.2*length(Bloczki)*(out_ind-1); % initial offsets
    Bloczki(ind).horizontal_offset = out_ind-1; % initial offsets
    
    out_ind = out_ind + 1;
  end
  ind = ind + 1;
end

% for ind = 1:length(current_Blocks),
%   Bloczki(current_Blocks(ind))
% end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% current_Blocks should now contain all sources and mixed blocks
%
% Now find blocks connected to all blocks stored in current_Blocks
% and create ne lists of unprocessed blocks

Processed_blocks = zeros(size(Bloczki));
while 1,
  clear new_current_Blocks
  [length(current_Blocks), length(Bloczki)]
  
  ind = 1; out_ind = 1; 
	while ind <= length(current_Blocks),
%     Bloczki(current_Blocks(ind))
    Processed_blocks(current_Blocks(ind)) = 1;
    no_of_outputs = Bloczki(current_Blocks(ind)).number_of_outputs;
    for ind_output = 1:no_of_outputs,
      ind_ = Bloczki(current_Blocks(ind)).output_blocks(ind_output);
      ind_input = 1 + round((ind_ - floor(ind_)) * 10); % output block input index (1, 2, ...; 0 - not used yet)
      ind_ = floor(ind_);                               % output block index

      if Bloczki(ind_).type == 'b',  % ignore mixed blocks
        % set input order for given output block input to curret block output
        % order plus one
        Bloczki(ind_).inputs_orders(ind_input) = Bloczki(current_Blocks(ind)).output_order + 1;
        
        % adjust horizontal offest of the output blocks
        korekta = (ind_output-1) - (no_of_outputs - 1)/2;
        if korekta > 0,
          for ind_kor = 1:length(Bloczki),
            if Bloczki(ind_kor).horizontal_offset > Bloczki(ind_).horizontal_offset,
              Bloczki(ind_kor).horizontal_offset = Bloczki(ind_kor).horizontal_offset + 1;
            end
          end
        end
        if korekta < 0,
          for ind_kor = 1:length(Bloczki),
            if Bloczki(ind_kor).horizontal_offset < Bloczki(ind_).horizontal_offset,
              Bloczki(ind_kor).horizontal_offset = Bloczki(ind_kor).horizontal_offset - 1;
            end
          end
        end
        Bloczki(ind_).horizontal_offset = Bloczki(current_Blocks(ind)).horizontal_offset + korekta;
        

        if Bloczki(current_Blocks(ind)).number_of_outputs > 0, 
          % if no outputs we do not need to process it later
          if Processed_blocks(ind_) == 0, % only if not processed before
            new_current_Blocks(out_ind) = ind_; % store indexes to output blocks
            out_ind = out_ind + 1;
            Processed_blocks(ind_) = -1;  % mark that it will be processed next
          end
          
          % update output order if current input order is higher than current
          % output block output order ;-D
          if Bloczki(ind_).output_order < Bloczki(ind_).inputs_orders(ind_input)
            Bloczki(ind_).output_order = Bloczki(ind_).inputs_orders(ind_input);
            
            % ??? what with input & then output orders of output blocks ??? !!! 
          end
        end
        
      end
    end
    ind = ind + 1;
	end
	
  if exist('new_current_Blocks', 'var') 
    current_Blocks = new_current_Blocks;
  else
    clear current_Blocks
    break;
  end
  
  ile_new = length(current_Blocks)
  
% 	for ind = 1:length(current_Blocks),
%       Bloczki(current_Blocks(ind)), %pause
% 	end
end

disp('FINAL RESULTS');
close all; figure(1); set(1, 'Color', 'w');
max_order = 0; max_offset = 0; min_offset = 0;
for ind = 1:length(Bloczki),
  Bloczki(ind)
  
  DrawBlock(Bloczki(ind).output_order, Bloczki(ind).horizontal_offset, ...
            Bloczki(ind).name, ...
            Bloczki(ind).number_of_inputs, Bloczki(ind).number_of_outputs, []);
  
  for ind_=1:Bloczki(ind).number_of_outputs,
    ind_out = Bloczki(ind).output_blocks(ind_);
    ind_input = 1 + round((ind_out - floor(ind_out)) * 10); % output block input index (1, 2, ...; 0 - not used yet)
    ind_out = floor(ind_out);                               % output block index
    
    n_o = Bloczki(ind).number_of_outputs;
    n_i = Bloczki(ind_out).number_of_inputs;
% %     connect(Bloczki(ind).output_order, Bloczki(ind).horizontal_offset + (ind_-1 - (n_o-1)/2)/n_o, ...
% %       Bloczki(ind_out).output_order, Bloczki(ind_out).horizontal_offset + (ind_input-1 - (n_i-1)/2)/n_i);
%     connect2(Bloczki(ind).output_order, Bloczki(ind).horizontal_offset, (ind_-1 - (n_o-1)/2)/n_o, ...
%       Bloczki(ind_out).output_order, Bloczki(ind_out).horizontal_offset, (ind_input-1 - (n_i-1)/2)/n_i);
    hl = connect3(Bloczki(ind).output_order, Bloczki(ind).horizontal_offset, [(ind_-1 - (n_o-1)/2), n_o], ...
      Bloczki(ind_out).output_order, Bloczki(ind_out).horizontal_offset, [(ind_input-1 - (n_i-1)/2), n_i]);
    
%     set(hl, 'Color', kolory(ind_+ind_input-1));
    set(hl, 'Color', kolory(floor(rand*length(kolory)+1)));
%     set(hl, 'Color', [randn(1,3)/6]+0.5);
  end
  
  max_order = max([max_order, Bloczki(ind).output_order]);
  max_offset = max([max_offset, Bloczki(ind).horizontal_offset]);
  min_offset = min([min_offset, Bloczki(ind).horizontal_offset]);
  
  set(gca, 'Xlim', [-0.5, max_order+0.5]);  
  set(gca, 'Ylim', [min_offset-0.5, max_offset+0.5]);  
end
set(gca, 'Visible', 'off');

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function hl = connect3(x1, y1, out_offset, x2, y2, in_offset)

yb1 = 1/5 * out_offset(1) / out_offset(2);
yb2 = 1/5 * in_offset(1) / in_offset(2);

if out_offset(1) == 0,
  out_offset = out_offset(1);
else
  out_offset = (out_offset(1) + (out_offset(1) > 0)/3) / out_offset(2);
end
if in_offset(1) == 0,
  in_offset = in_offset(1);
else
  in_offset = (in_offset(1) + (in_offset(1) > 0)/3) / in_offset(2);
end

y1 = y1 + yb1; 
y2 = y2 + yb2;


if x1 < x2,
  % \BUG but only if there are no other blocks between the two ones just 
  %      being connected
  x_mid = (x1 + x2)/2;
  hl = line([x1+0.15, x_mid, x_mid, x2-0.15], ...
        [y1, y1, y2, y2]);
else
  % \BUG but only if there are no other blocks between the two ones just 
  %      being connected
  if rem(abs(y2-y1),2) == 1,
    y_mid = (y1 + y2)/2;
  else
    y_mid = (y1 + y2)/2 + 1/2;
  end
  hl = line([x1+0.15, x1+0.3, x1+0.3, x2-0.3, x2-0.3, x2-0.15], ...
        [y1, y1, y_mid, y_mid, y2, y2]);
end
return

if out_offset > 0,
  % ci¹gnij w górê
  if y1 > y2,
    if in_offset > 0,
      y_mid = y1+0.5;
    else
      y_mid = y2+0.5;
    end
  else
    if in_offset > 0,
      y_mid = y2+0.5;
    else
      y_mid = (y1+y2)/2;
    end
  end
  hl = line([x1+0.1, x1+0.3*(1+abs(out_offset)), x1+0.3*(1+abs(out_offset)), ...
        x2-0.3*(1+abs(in_offset)), x2-0.3*(1+abs(in_offset)), x2-0.1], ...
        [y1, y1, ...
         y_mid*[1, 1], ...
         y2, y2]);
else
  % ci¹gnij w dó³
  if y1 > y2,
    if in_offset < 0,
      y_mid = y2-0.5;
    else
      y_mid = (y1+y2)/2;
    end
  else
    if in_offset < 0,
      y_mid = y1-0.5;
    else
      y_mid = y2-0.5;
    end
  end
  hl = line([x1+0.1, x1+0.3*(1+abs(out_offset)), x1+0.3*(1+abs(out_offset)), ...
        x2-0.3*(1+abs(in_offset)), x2-0.3*(1+abs(in_offset)), x2-0.1], ...
        [y1, y1, ...
         y_mid*[1, 1], ...
         y2, y2]);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function Bloczki = preprocess_bloczki(Bloczki)

% % 1) remove duplicates
% ind = 1;
% while ind < length(Bloczki)
%   ind2 = ind+1;
%   while ind2 <= length(Bloczki),
%     if Bloczki(ind).unique_index == Bloczki(ind2).unique_index,
%       Bloczki(ind2) = [];
%     else
%       ind2 = ind2 + 1;
%     end
%   end
%   ind = ind + 1;
% end

% 2) replace unique_index'es with index in Bloczki
for ind = 1:length(Bloczki),
  for ind_2 = 1:length(Bloczki(ind).output_blocks),
    tmp = floor(Bloczki(ind).output_blocks(ind_2));
    for ind_3 = 1:length(Bloczki),
      if Bloczki(ind_3).unique_index == tmp,
        % replace unique_index with actual index
        Bloczki(ind).output_blocks(ind_2) = Bloczki(ind).output_blocks(ind_2) - tmp + ind_3;
        break;
      end
    end
  end
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function DrawBlock(x, y, name, no_of_inputs, no_of_outputs, params)

block_name = name;
block_label = [];
if block_name(end) == '>',
  ind = find(block_name == '<');
  if length(ind) > 0,
    block_label = block_name((ind(1)+1):end-1);
    block_name = block_name(1:(ind(1)-1));
  end
end

switch block_name,
  otherwise
%     ht = text(x, y, block_name);
%     if length(block_label) > 0,
%       ht(2) = text(x, y-0.15, block_label);
%     end
    if length(block_label) > 0,
      ht = text(x, y, [block_name, 10, block_label]);
    else
      ht = text(x, y, block_name);
    end
    set(ht, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'middle');
    
    % frame
    line(x+[-1, -1, 1, 1, -1]/10, y+[-1, 1, 1, -1, -1]/10);
    
    % inputs
    for ind = 1:no_of_inputs,
      dy = (ind - 1 - (no_of_inputs-1)/2)/no_of_inputs * 1/5;
      line(x+[-1, -1.5]/10, y+dy*[1,1]);
      hold on
      plot(x+[-1.5]/10, y+dy, 'o');
      hold off
    end
    % outputs
    for ind = 1:no_of_outputs,
      dy = (ind - 1 - (no_of_outputs-1)/2)/no_of_outputs * 1/5;
      line(x+[1, 1.5]/10, y+dy*[1,1]);
      hold on
      set(plot(x+[1.5]/10, y+dy, '.'), 'MarkerSize', 20);
      hold off
    end
end
