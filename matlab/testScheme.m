function testScheme
kolory = 'brgmcyk';
% dane o schemacie algorytmu
% % I. Bloczki
Bloczki = test_scheme_file;
Bloczki = preprocess_bloczki(Bloczki);

% Bloczki(1).name = 'Source_1';
% Bloczki(1).type = 's'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(1).number_of_outputs = 1;
% Bloczki(1).output_blocks = [2.0]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(1).number_of_inputs = 0; % always 0 for source blocks
% 
% Bloczki(2).name = 'Block A';
% Bloczki(2).type = 'b'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(2).number_of_outputs = 2;
% Bloczki(2).output_blocks = [3.1, 4.1]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(2).number_of_inputs = 1; % always 0 for source blocks
% 
% Bloczki(3).name = 'Block C';
% Bloczki(3).type = 'b'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(3).number_of_outputs = 1;
% Bloczki(3).output_blocks = [5.0]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(3).number_of_inputs = 2; % always 0 for source blocks
% 
% Bloczki(4).name = 'Block H';
% Bloczki(4).type = 'm'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(4).number_of_outputs = 1;
% Bloczki(4).output_blocks = [3.0]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(4).number_of_inputs = 2; % always 0 for source blocks
% 
% Bloczki(5).name = 'Block E';
% Bloczki(5).type = 'b'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(5).number_of_outputs = 2;
% Bloczki(5).output_blocks = [6.0, 8.0]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(5).number_of_inputs = 1; % always 0 for source blocks
% 
% Bloczki(6).name = 'Output';
% Bloczki(6).type = 'b'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(6).number_of_outputs = 0;
% Bloczki(6).output_blocks = []; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(6).number_of_inputs = 1; % always 0 for source blocks
% 
% Bloczki(7).name = 'Source_2';
% Bloczki(7).type = 's'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(7).number_of_outputs = 1;
% Bloczki(7).output_blocks = [8.1]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(7).number_of_inputs = 0; % always 0 for source blocks
% 
% Bloczki(8).name = 'Block D';
% Bloczki(8).type = 'b'; % 's' - source, 'b' - processig block, 'm' - mixed: processing & source block, % 'o' - output (generaly == processing block with no outputs)
% Bloczki(8).number_of_outputs = 1;
% Bloczki(8).output_blocks = [4.0]; % indexes of output blocks (b.i <= b - block index (1, 2, ...), i - output block input index  (0, 1, ...)
% Bloczki(8).number_of_inputs = 2; % always 0 for source blocks


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
    
    Bloczki(ind).horizontal_offset = 1.2*length(Bloczki)*(out_ind-1); % initial offsets
    
    out_ind = out_ind + 1;
  end
  ind = ind + 1;
end

for ind = 1:length(current_Blocks),
  Bloczki(current_Blocks(ind))
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% current_Blocks should now contain all sources and mixed blocks
%
% Now find blocks connected to all blocks stored in current_Blocks
% and create ne lists of unprocessed blocks

while 1,
  clear new_current_Blocks
  
  ind = 1; out_ind = 1; 
	while ind <= length(current_Blocks),
%     Bloczki(current_Blocks(ind))
    for ind_output = 1:Bloczki(current_Blocks(ind)).number_of_outputs,
      ind_ = Bloczki(current_Blocks(ind)).output_blocks(ind_output);
      ind_input = 1 + round((ind_ - floor(ind_)) * 10); % output block input index (1, 2, ...; 0 - not used yet)
      ind_ = floor(ind_);                               % output block index

      if Bloczki(ind_).type == 'b',  % ignore mixed blocks
        % set input order for given output block input to curret block output
        % order plus one
        Bloczki(ind_).inputs_orders(ind_input) = Bloczki(current_Blocks(ind)).output_order + 1;
        Bloczki(ind_).horizontal_offset = Bloczki(current_Blocks(ind)).horizontal_offset + ind_output-1;
%         tmp = Bloczki(ind_).horizontal_offset
%         pause
        if Bloczki(current_Blocks(ind)).number_of_outputs > 0, 
          % if no outputs we do not need to process it later
          new_current_Blocks(out_ind) = ind_; % store indexes to output blocks
          
          % update output order if current input order is higher than current
          % output block output order ;-D
          if Bloczki(ind_).output_order < Bloczki(ind_).inputs_orders(ind_input)
            Bloczki(ind_).output_order = Bloczki(ind_).inputs_orders(ind_input);
            
            % ??? what with input & then output orders of output blocks ??? !!! 
          end
        end
        
        out_ind = out_ind + 1;
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
  
  
	for ind = 1:length(current_Blocks),
      Bloczki(current_Blocks(ind)), %pause
	end
end

disp('FINAL RESULTS');
close all
max_order = 0;
for ind = 1:length(Bloczki),
  Bloczki(ind)
  
  ht = text(Bloczki(ind).output_order, Bloczki(ind).horizontal_offset, Bloczki(ind).name);
  set(ht, 'HorizontalAlignment', 'center', 'VerticalAlignment', 'middle');
  
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
    hl = connect2(Bloczki(ind).output_order, Bloczki(ind).horizontal_offset, [(ind_-1 - (n_o-1)/2), n_o], ...
      Bloczki(ind_out).output_order, Bloczki(ind_out).horizontal_offset, [(ind_input-1 - (n_i-1)/2), n_i]);
    
%     set(hl, 'Color', kolory(ind_+ind_input-1));
    set(hl, 'Color', kolory(floor(rand*length(kolory)+1)));
%     set(hl, 'Color', [randn(1,3)/6]+0.5);
  end
  
  max_order = max([max_order, Bloczki(ind).output_order]);
  
  set(gca, 'Xlim', [-1.5, max_order+0.5]);  
  set(gca, 'Ylim', [-2.5, 40.5]);  
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function connect(x1, y1, x2, y2)

%   line([x1+0.1, x2-0.1], [y1, y2]);

if x2 > x1,
  line([x1+0.1, x1+0.3, x2-0.3, x2-0.1], [y1, y1, y2, y2]);
else
  line([x1+0.1, x1+0.3, x1+0.3,  x2-0.3, x2-0.3, x2-0.1], [y1, y1, y1+(y2-y1)/10, y2+(y1-y2)/10, y2, y2]);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function hl = connect2(x1, y1, out_offset, x2, y2, in_offset)
% out_offset(1)
% in_offset(1)
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
% pause
%   line([x1+0.1, x2-0.1], [y1, y2]);

% if x2 < x1,
%   line([x1+0.1, x1+0.3*(1+abs(out_offset)), x2-0.3*(1+abs(in_offset)), x2-0.1], [[y1, y1]+out_offset, [y2, y2]+in_offset]);
% else
%   % x1 < x2 - wyjœcie bli¿ej, wejœcie dalej
% %   line([x1+0.1, x1+0.3*(1+abs(out_offset)), x1+0.3*(1+abs(out_offset)), ...
% %         x2-0.3*(1+abs(in_offset)), x2-0.3*(1+abs(in_offset)), x2-0.1], ...
% %         [[y1, y1, y1+(y2-y1)/10]+out_offset, [y2+(y1-y2)/10, y2, y2]+in_offset]);
%   line([x1+0.1, x1+0.3*(1+abs(out_offset)), x1+0.3*(1+abs(out_offset)), ...
%         x2-0.3*(1+abs(in_offset)), x2-0.3*(1+abs(in_offset)), x2-0.1], ...
%         [[y1, y1]+out_offset, ((y1+y2)/2)*[1, 1], [y2, y2]+in_offset]);
% end

y1 = y1 + out_offset;
y2 = y2 + in_offset;
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

% 1) remove duplicates
ind = 1;
while ind < length(Bloczki)
  ind2 = ind+1;
  while ind2 <= length(Bloczki),
    if Bloczki(ind).unique_index == Bloczki(ind2).unique_index,
      Bloczki(ind2) = [];
    else
      ind2 = ind2 + 1;
    end
  end
  ind = ind + 1;
end

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