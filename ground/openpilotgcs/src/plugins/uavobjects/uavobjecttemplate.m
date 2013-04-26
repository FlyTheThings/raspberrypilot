function [] = OPLogConvert(varargin)
%% Define indices and arrays of structures to hold data
% THIS FILE IS AUTOMATICALLY GENERATED.

outputType='mat'; %Default output is a .mat file
checkCRC = false;
wrongSyncByte=0;
wrongMessageByte=0;
lastWrongSyncByte=0;
lastWrongMessageByte=0;
str1=[];
str2=[];
str3=[];
str4=[];
str5=[];

fprintf('\n\n***OpenPilot log parser***\n\n');
global crc_table;
crc_table = [ ...
    hex2dec('00'),hex2dec('07'),hex2dec('0e'),hex2dec('09'),hex2dec('1c'),hex2dec('1b'),hex2dec('12'),hex2dec('15'),hex2dec('38'),hex2dec('3f'),hex2dec('36'),hex2dec('31'),hex2dec('24'),hex2dec('23'),hex2dec('2a'),hex2dec('2d'), ...
    hex2dec('70'),hex2dec('77'),hex2dec('7e'),hex2dec('79'),hex2dec('6c'),hex2dec('6b'),hex2dec('62'),hex2dec('65'),hex2dec('48'),hex2dec('4f'),hex2dec('46'),hex2dec('41'),hex2dec('54'),hex2dec('53'),hex2dec('5a'),hex2dec('5d'), ...
    hex2dec('e0'),hex2dec('e7'),hex2dec('ee'),hex2dec('e9'),hex2dec('fc'),hex2dec('fb'),hex2dec('f2'),hex2dec('f5'),hex2dec('d8'),hex2dec('df'),hex2dec('d6'),hex2dec('d1'),hex2dec('c4'),hex2dec('c3'),hex2dec('ca'),hex2dec('cd'), ...
    hex2dec('90'),hex2dec('97'),hex2dec('9e'),hex2dec('99'),hex2dec('8c'),hex2dec('8b'),hex2dec('82'),hex2dec('85'),hex2dec('a8'),hex2dec('af'),hex2dec('a6'),hex2dec('a1'),hex2dec('b4'),hex2dec('b3'),hex2dec('ba'),hex2dec('bd'), ...
    hex2dec('c7'),hex2dec('c0'),hex2dec('c9'),hex2dec('ce'),hex2dec('db'),hex2dec('dc'),hex2dec('d5'),hex2dec('d2'),hex2dec('ff'),hex2dec('f8'),hex2dec('f1'),hex2dec('f6'),hex2dec('e3'),hex2dec('e4'),hex2dec('ed'),hex2dec('ea'), ...
    hex2dec('b7'),hex2dec('b0'),hex2dec('b9'),hex2dec('be'),hex2dec('ab'),hex2dec('ac'),hex2dec('a5'),hex2dec('a2'),hex2dec('8f'),hex2dec('88'),hex2dec('81'),hex2dec('86'),hex2dec('93'),hex2dec('94'),hex2dec('9d'),hex2dec('9a'), ...
    hex2dec('27'),hex2dec('20'),hex2dec('29'),hex2dec('2e'),hex2dec('3b'),hex2dec('3c'),hex2dec('35'),hex2dec('32'),hex2dec('1f'),hex2dec('18'),hex2dec('11'),hex2dec('16'),hex2dec('03'),hex2dec('04'),hex2dec('0d'),hex2dec('0a'), ...
    hex2dec('57'),hex2dec('50'),hex2dec('59'),hex2dec('5e'),hex2dec('4b'),hex2dec('4c'),hex2dec('45'),hex2dec('42'),hex2dec('6f'),hex2dec('68'),hex2dec('61'),hex2dec('66'),hex2dec('73'),hex2dec('74'),hex2dec('7d'),hex2dec('7a'), ...
    hex2dec('89'),hex2dec('8e'),hex2dec('87'),hex2dec('80'),hex2dec('95'),hex2dec('92'),hex2dec('9b'),hex2dec('9c'),hex2dec('b1'),hex2dec('b6'),hex2dec('bf'),hex2dec('b8'),hex2dec('ad'),hex2dec('aa'),hex2dec('a3'),hex2dec('a4'), ...
    hex2dec('f9'),hex2dec('fe'),hex2dec('f7'),hex2dec('f0'),hex2dec('e5'),hex2dec('e2'),hex2dec('eb'),hex2dec('ec'),hex2dec('c1'),hex2dec('c6'),hex2dec('cf'),hex2dec('c8'),hex2dec('dd'),hex2dec('da'),hex2dec('d3'),hex2dec('d4'), ...
    hex2dec('69'),hex2dec('6e'),hex2dec('67'),hex2dec('60'),hex2dec('75'),hex2dec('72'),hex2dec('7b'),hex2dec('7c'),hex2dec('51'),hex2dec('56'),hex2dec('5f'),hex2dec('58'),hex2dec('4d'),hex2dec('4a'),hex2dec('43'),hex2dec('44'), ...
    hex2dec('19'),hex2dec('1e'),hex2dec('17'),hex2dec('10'),hex2dec('05'),hex2dec('02'),hex2dec('0b'),hex2dec('0c'),hex2dec('21'),hex2dec('26'),hex2dec('2f'),hex2dec('28'),hex2dec('3d'),hex2dec('3a'),hex2dec('33'),hex2dec('34'), ...
    hex2dec('4e'),hex2dec('49'),hex2dec('40'),hex2dec('47'),hex2dec('52'),hex2dec('55'),hex2dec('5c'),hex2dec('5b'),hex2dec('76'),hex2dec('71'),hex2dec('78'),hex2dec('7f'),hex2dec('6a'),hex2dec('6d'),hex2dec('64'),hex2dec('63'), ...
    hex2dec('3e'),hex2dec('39'),hex2dec('30'),hex2dec('37'),hex2dec('22'),hex2dec('25'),hex2dec('2c'),hex2dec('2b'),hex2dec('06'),hex2dec('01'),hex2dec('08'),hex2dec('0f'),hex2dec('1a'),hex2dec('1d'),hex2dec('14'),hex2dec('13'), ...
    hex2dec('ae'),hex2dec('a9'),hex2dec('a0'),hex2dec('a7'),hex2dec('b2'),hex2dec('b5'),hex2dec('bc'),hex2dec('bb'),hex2dec('96'),hex2dec('91'),hex2dec('98'),hex2dec('9f'),hex2dec('8a'),hex2dec('8d'),hex2dec('84'),hex2dec('83'), ...
    hex2dec('de'),hex2dec('d9'),hex2dec('d0'),hex2dec('d7'),hex2dec('c2'),hex2dec('c5'),hex2dec('cc'),hex2dec('cb'),hex2dec('e6'),hex2dec('e1'),hex2dec('e8'),hex2dec('ef'),hex2dec('fa'),hex2dec('fd'),hex2dec('f4'),hex2dec('f3')  ...
    ];

if nargin==0
	%%
	if (exist('uigetfile')) %#ok<EXIST>
		[FileName, PathName]=uigetfile('*.opl');
		logfile=fullfile(PathName, FileName);
		
	else
		error('Your technical computing program does not support file choosers. Please input the file name in the argument. ')
	end	
elseif nargin>0
	logfile=varargin{1};
	if nargin>1
		outputType=varargin{2};
	end
end

if ~strcmpi(outputType,'mat') && ~strcmpi(outputType,'csv')
	error('Incorrect file format specified. Second argument must be ''mat'' or ''csv''.');
end

$(INSTANTIATIONCODE)


fid = fopen(logfile);
buffer=fread(fid,Inf,'uchar=>uchar');
fseek(fid, 0, 'bof');

bufferIdx=1;

correctMsgByte=hex2dec('20');
correctTimestampedByte=hex2dec('A0');
correctSyncByte=hex2dec('3C');
unknownObjIDList=zeros(1,2);

% Parse log file, entry by entry
% prebuf = buffer(1:12);

last_print = -1e10;

startTime=clock;

while (1)
	if (feof(fid)); break; end

	try
	%% Read message header
	% get sync field (0x3C, 1 byte)
	sync = fread(fid, 1, 'uint8');
	if sync ~= correctSyncByte
		prebuf = [prebuf(2:end); sync];
		wrongSyncByte = wrongSyncByte + 1;
		continue
	end
    
	% get msg type (quint8 1 byte ) should be 0x20, ignore the rest?
	msgType = fread(fid, 1, 'uint8');
	if msgType ~= correctMsgByte && msgType ~= hex2dec('A0')
		wrongMessageByte = wrongMessageByte + 1;	
		continue
	end

	% get msg size (quint16 2 bytes) excludes crc, include msg header and data payload
	msgSize = fread(fid, 1, 'uint16');
	% get obj id (quint32 4 bytes)
	objID = fread(fid, 1, 'uint32');

	if msgType == correctMsgByte
		%% Process header if we are aligned
		timestamp = typecast(uint8(prebuf(1:4)), 'uint32');
		datasize = typecast(uint8(prebuf(5:12)), 'uint64');
	elseif msgType == correctTimestampedByte
		timestamp = fread(fid,1,'uint16');
	end

	if (isempty(objID)) 	%End of file
		break;
	end
	
	%% Read object

	switch objID
$(SWITCHCODE)
		otherwise
			unknownObjIDListIdx=find(unknownObjIDList(:,1)==objID, 1, 'first');
			if isempty(unknownObjIDListIdx)
				unknownObjIDList=[unknownObjIDList; uint32(objID) 1]; %#ok<AGROW>
			else
				unknownObjIDList(unknownObjIDListIdx,2)=unknownObjIDList(unknownObjIDListIdx,2)+1; 
			end
			
			datasize = typecast(buffer(datasizeBufferIdx + 4:datasizeBufferIdx + 12-1), 'uint64');

			msgBytesLeft = datasize - 1 - 1 - 2 - 4;
			if msgBytesLeft > 255
				msgBytesLeft = 0;
			end
			bufferIdx=bufferIdx+double(msgBytesLeft);
	end
	catch
		% One of the reads failed - indicates EOF
		break;
	end

	if (wrongSyncByte ~= lastWrongSyncByte || wrongMessageByte~=lastWrongMessageByte ) ||...
			bufferIdx - last_print > 5e4 %Every 50,000 bytes show the status update

		lastWrongSyncByte=wrongSyncByte;
		lastWrongMessageByte=wrongMessageByte;

		str1=[];
		for i=1:length([str2 str3 str4 str5]);
			str1=[str1 sprintf('\b')]; %#ok<AGROW>
		end
		str2=sprintf('wrongSyncByte instances:    % 10d\n', wrongSyncByte );
		str3=sprintf('wrongMessageByte instances: % 10d\n\n', wrongMessageByte );
		
		str4=sprintf('Completed bytes: % 9d of % 9d\n', bufferIdx, length(buffer));
		
	        % Arbitrary times two so that it is at least as long	
		estTimeRemaining=(length(buffer)-bufferIdx)/(bufferIdx/etime(clock,startTime)) * 2;
		h=floor(estTimeRemaining/3600);
		m=floor((estTimeRemaining-h*3600)/60);
		s=ceil(estTimeRemaining-h*3600-m*60);
		
		str5=sprintf('Est. time remaining, %02dh:%02dm:%02ds \n', h,m,s);

		last_print = bufferIdx;
		
		fprintf([str1 str2 str3 str4 str5]);
	end

	%Check if at end of file. If not, load next prebuffer
	if bufferIdx+12-1 > length(buffer)
		break;
	end
% 	bufferIdx=bufferIdx+12;

end


for i=2:size(unknownObjIDList,1) %Don't show the first one, as it was simply a dummy placeholder
   disp(['Unknown object ID: 0x' dec2hex(unknownObjIDList(i,1),8) ' appeared ' int2str(unknownObjIDList(i,2)) ' times.']);
end

%% Clean Up and Save mat file
fclose(fid);

%% Prune vectors
$(CLEANUPCODE)


%% Perform typecasting on vectors
$(ALLOCATIONCODE)

%% Save data to file
if strcmpi(outputType,'mat')
	[path, name]=fileparts(logfile);
	matfile = fullfile(path,[name '.mat']);
	save(matfile $(SAVEOBJECTSCODE));
else
$(EXPORTCSVCODE)
end

fprintf('%d records in %0.2f seconds.\n', length(buffer), etime(clock,startTime));



	
function	OPLog2csv(structIn, structName, logfile)
	%Get each field name from the structure
	fieldNames = fieldnames(structIn);
	
	%Create a text string with the field names
	headerOut=sprintf('%s,',fieldNames{:});
	headerOut=headerOut(1:end-1); %Trim off last `,` and `\t`
	
	%Assign the structure arrays to a matrix.
	matOut=zeros(max(size(structIn.(fieldNames{1}))), length(fieldNames));
	
	if	isempty(structIn.(fieldNames{1}));
		matOut=[];
	else
		for i=1:length(fieldNames)
			matOut(:,i)=structIn.(fieldNames{i});
		end
	end	
	% Create filename by replacing opl by csv
	[path, name] = fileparts(logfile);
	csvDirName=[name '_csv'];
	[dummyA, dummyB]=mkdir(fullfile(path, csvDirName)); %Dummy outputs so the program doens't throw warnings about "Directory already exists"
	csvfile=fullfile(path, csvDirName , [name '.csv']);
	
	%Write to csv.
	dlmwrite(csvfile, headerOut, '');
	dlmwrite(csvfile, matOut, '-append');

function crc = compute_crc(data)
    global crc_table;
    crc = 0;
    for i = 1:length(data)
        crc = crc_table(1+bitxor(data(i),crc));
    end

function out=mcolon(inStart, inFinish)
%% This function was inspired by Bruno Luong's 'mcolon'. The name is kept the same as his 'mcolon'
% function, found on Matlab's file exchange. The two functions return identical
% results, although his is much faster. Unfortunately, C-compiled mex
% code would make this function non-cross-platform, so a Matlab scripted
% version is provided here.
	if size(inStart,1) > 1 || size(inFinish,1) > 1
		if size(inStart,2) > 1 || size(inFinish,2) > 1
			error('Inputs must be vectors, i.e just one column wide.')		
		else
			inStart=inStart';
			inFinish=inFinish';
		end
	end
	
	diffIn=diff([inStart; inFinish]);
	numElements=sum(diffIn)+length(inStart);
	
	out=zeros(1,numElements);
	
	idx=1;
	for i=1:length(inStart)
		out(idx:idx+diffIn(i))=inStart(i):inFinish(i);
		idx=idx+diffIn(i)+1;
	end
