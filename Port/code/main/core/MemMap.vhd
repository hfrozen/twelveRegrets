-------------------------------------------------------------------------------
-- Title	: memory mapping
-- Project	: serial communication
-- File		: MemMap.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

use work.TypicPack.all;

entity MemMap is
	generic (
		cAddrWidth_ti	: integer	:= 9;	-- 3ff, 1024
		cMaxAddress_ti	: integer	:= 512
	);
	port(
		resetI				: in	std_logic;
		resetO				: out	std_logic;
		clock168000kI		: in	std_logic;
		busCsI				: in	std_logic;
		busReadI			: in	std_logic;
		busWriteI			: in	std_logic;
		busAddrI_t8			: in	std_logic_vector(7 downto 0);
		busDataIO_t16		: inout	std_logic_vector(15 downto 0);
		busInterruptO		: out	std_logic;
		--busWaitO			: out	std_logic;

		hdlcO				: out	std_logic;
		--dpllEnableO			: out	std_logic;
		addrMatch_stoplO	: out	std_logic;
		manchester_oddpO	: out	std_logic;
		nrzi_evenpO			: out	std_logic;
		fcsEnableO			: out	std_logic;
		baudrateConstantO_ti	: out	integer range 0 to 255;

		txEmptyI			: in	std_logic;		-- from TxBuffer, write to buffer => 0
		txHaltI				: in	std_logic;		-- from ZeroInsert, empty shift register, latch => 0, ummm...
		txEnableO			: out	std_logic;
		txWriteO			: out	std_logic;
		txAbortO			: out	std_logic;		-- to FlagInsert
		txFlagSizeRxHoldSize_t4	: out	std_logic_vector(3 downto 0);
		txFrameSizeO_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		txdO_t16			: out	std_logic_vector(15 downto 0);

		--flagOpenM			: in	std_logic;		-- ?????
		--txEnableM			: in	std_logic;		-- ?????
		--txAvailM			: in	std_logic;		-- ?????

		rxFullI				: in	std_logic;		-- from RxBuffer, read from buffer => 0
		rxOverflowErrorI	: in	std_logic;		-- from RxBuffer, re-read => 0
		--rxOverlapErrorI		: in	std_logic;
		rxOverRunErrorI		: in	std_logic;
		rxFrameErrorI		: in	std_logic;		-- from RxController, strange code ???
		rxFcsErrorI			: in	std_logic;		-- from RxFcs, re-receive => 0
		rxParityErrorI		: in	std_logic;
		rxAbortI			: in	std_logic;		-- from FlagDetector, receive flag => 0
		rxFrameSizeI_tv		: in	std_logic_vector(cAddrWidth_ti downto 0);
		rxAddressI_t16		: in	std_logic_vector(15 downto 0);
		rxdI_t16			: in	std_logic_vector(15 downto 0);
		rxEnableO			: out	std_logic;
		rxReadO				: out	std_logic
		--rxIgnoreO			: out	std_logic
	);
end MemMap;

architecture behavioral of MemMap is
	signal i_reset					: std_logic	:= '0';
	signal i_txHaltInt				: std_logic	:= '0';
	signal i_enIntTxHalt			: std_logic	:= '0';

	signal i_txEmptyInt				: std_logic	:= '0';
	signal i_enIntTxEmpty			: std_logic	:= '0';

	signal i_rxFullInt				: std_logic	:= '0';
	signal i_rxFullEdge				: std_logic	:= '0';
	signal i_enIntRxFull			: std_logic	:= '0';

	signal i_rxOverflowErrorInt		: std_logic	:= '0';
	signal i_enIntRxOverflowError	: std_logic	:= '0';

	--signal i_rxOverlapErrorInt		: std_logic	:= '0';
	--signal i_enIntRxOverlapError	: std_logic	:= '0';

	signal i_rxOverRunErrorInt		: std_logic	:= '0';
	signal i_enIntRxOverRunError	: std_logic	:= '0';

	signal i_rxFrameErrorInt		: std_logic	:= '0';
	signal i_enIntRxFrameError		: std_logic	:= '0';

	signal i_rxFcsErrorInt			: std_logic	:= '0';
	signal i_enIntRxFcsError		: std_logic	:= '0';

	--signal i_rxParityErrorInt		: std_logic	:= '0';
	--signal i_enIntRxParityError		: std_logic	:= '0';
	--signal i_rxParityErrorHold		: std_logic	:= '0';

	signal i_rxAbortInt				: std_logic	:= '0';
	signal i_enIntRxAbort			: std_logic	:= '0';
	signal i_rxAbortHold			: std_logic := '0';

	signal i_hdlc					: std_logic	:= '1';
	--signal i_dpllEnable				: std_logic	:= '0';
	signal i_addrMatch_stopl		: std_logic := '0';
	signal i_manchester_oddp		: std_logic := '0';
	signal i_nrzi_evenp				: std_logic := '0';
	signal i_fcsEnable				: std_logic := '1';
	--	hdlc	addrM	mach	nrz		fcs
	--	1		X		1		0		X	-> manchester
	--	1		X		0		1		X	-> nrz
	--	1		X		0		0		X	-> nrzi
	--	0		0		X		X		X	-> async. 1 stop
	--	0		1		X		X		X	-> async. 2 stop
	--	0		X		0		X		X	-> async. non parity
	--	0		X		1		0		X	-> async. odd parity
	--	0		X		1		1		X	-> async. even parity

	signal i_baudrateConstant_ti			: integer range 0 to 255 := 1;

	signal i_hdlcAddr_t16			: std_logic_vector(15 downto 0);
	signal i_txEnable				: std_logic := '0';
	signal i_txAbort				: std_logic := '0';
	signal i_txFrameSize_tv			: std_logic_vector(cAddrWidth_ti downto 0);
	signal i_txFlagSizeRxHoldSize_t4	: std_logic_vector(3 downto 0) := "0000";

	signal i_rxEnable				: std_logic := '0';

	signal i_clearTxIntrs			: std_logic	:= '0';
	signal i_clearRxIntrs			: std_logic	:= '0';

	component Dff
	port (
		dI		: in	std_logic;
		clockI	: in	std_logic;
		clearI	: in	std_logic;
		resetI	: in	std_logic;
		qO		: out	std_logic
	);
	end component;

begin
	resetO				<= resetI or i_reset;

	--busInterruptO		<= i_txHaltInt or i_txEmptyInt or
	--						i_rxParityErrorInt or i_rxOverflowErrorInt or i_rxOverlapErrorInt or i_rxOverRunErrorInt or
	--						i_rxFrameErrorInt or i_rxFcsErrorInt or i_rxAbortInt or i_rxFullInt;
	busInterruptO		<= i_txHaltInt or i_txEmptyInt or
							--i_rxOverflowErrorInt or i_rxOverlapErrorInt or i_rxOverRunErrorInt or
							i_rxOverflowErrorInt or i_rxOverRunErrorInt or
							i_rxFrameErrorInt or i_rxFcsErrorInt or i_rxAbortInt or i_rxFullInt;

	fcsEnableO			<= i_fcsEnable;
	txEnableO			<= i_txEnable;
	txAbortO			<= i_txAbort;
	txFlagSizeRxHoldSize_t4 	<= i_txFlagSizeRxHoldSize_t4;
	txFrameSizeO_tv		<= i_txFrameSize_tv;
	rxEnableO			<= i_rxEnable;

	hdlcO				<= i_hdlc;
	--dpllEnableO			<= i_dpllEnable;
	addrMatch_stoplO	<= i_addrMatch_stopl;
	manchester_oddpO	<= i_manchester_oddp;
	nrzi_evenpO			<= i_nrzi_evenp;
	fcsEnableO			<= i_fcsEnable;
	baudrateConstantO_ti	<= i_baudrateConstant_ti;

	txWriteO			<= busCsI and busWriteI when busAddrI_t8 = x"00"	else '0';
	rxReadO				<= busCsI and busReadI when busAddrI_t8 = x"00"		else '0';

	--					: Dff	port map (dI, clockI, clearI, resetI, qO);
	TxHaltIntr			: Dff	port map (i_enIntTxHalt, txHaltI, i_clearTxIntrs, resetI, i_txHaltInt);
	TxEmptyIntr			: Dff	port map (i_enIntTxEmpty, txEmptyI, i_clearTxIntrs, resetI, i_txEmptyInt);
	RxOverflowErrorIntr	: Dff	port map (i_enIntRxOverflowError, rxOverflowErrorI, i_clearRxIntrs, resetI, i_rxOverflowErrorInt);
	--RxOverlapErrorIntr	: Dff	port map (i_enIntRxOverlapError, rxOverlapErrorI, i_clearRxIntrs, resetI, i_rxOverlapErrorInt);
	RxOverRunErrorIntr	: Dff	port map (i_enIntRxOVerRunError, rxOverRunErrorI, i_clearRxIntrs, resetI, i_rxOverRunErrorInt);
	RxFrameErrorIntr	: Dff	port map (i_enIntRxFrameError, rxFrameErrorI, i_clearRxIntrs, resetI, i_rxFrameErrorInt);
	RxFcsErrorIntr		: Dff	port map (i_enIntRxFcsError, rxFcsErrorI, i_clearRxIntrs, resetI, i_rxFcsErrorInt);
	--RxParityErrorIntr	: Dff	port map (i_enIntRxParityError, rxParityErrorI, i_clearRxIntrs, resetI, i_rxParityErrorInt);
	--RxParityErrorHold	: Dff	port map ('1', rxParityErrorI, i_clearRxIntrs, resetI, i_rxParityErrorHold);
	RxAbortIntr			: Dff	port map (i_enIntRxAbort, rxAbortI, i_clearRxIntrs, resetI, i_rxAbortInt);
	RxAbortHold			: Dff	port map ('1', rxAbortI, i_clearRxIntrs, resetI, i_rxAbortHold);

	RxFullProc	: process(resetI, clock168000kI)
	begin
		if resetI = '1' then
			i_rxFullInt			<= '0';
			i_rxFullEdge		<= '0';
			--rxIgnoreO			<= '0';
		elsif rising_edge(clock168000kI) then
			if i_clearRxIntrs = '1' then
				--rxIgnoreO			<= '0';

				if i_rxFullInt = '1' then
					i_rxFullInt			<= '0';
				end if;
			end if;

			if rxFullI = '1' and i_rxFullEdge = '0' then
				i_rxFullEdge		<= '1';
				if i_rxFullInt = '0' then
					if i_hdlc = '1' and i_addrMatch_stopl = '1' then
						if rxAddressI_t16 = x"ffff" or rxAddressI_t16 = i_hdlcAddr_t16 then
							i_rxFullInt		<= i_enIntRxFull;
						--else
						--	rxIgnoreO		<= '1';
						end if;
					else
						i_rxFullInt		<= i_enIntRxFull;
					end if;
				end if;
			elsif rxFullI = '0' then
				if i_rxFullEdge = '1' then
					i_rxFullEdge	<= '0';
				end if;

				--rxIgnoreO		<= '0';
			end if;
		end if;
	end process RxFullProc;

	MapProc	: process(resetI, clock168000kI, busCsI, busReadI, busWriteI, busDataIO_t16, rxdI_t16, rxFrameSizeI_tv,
						txHaltI, txEmptyI, rxOverflowErrorI, rxOverRunErrorI, rxFrameErrorI, rxFcsErrorI, rxFullI,	-- , rxOverlapErrorI
						busAddrI_t8, i_hdlcAddr_t16, i_txFrameSize_tv, i_txFlagSizeRxHoldSize_t4, i_baudrateConstant_ti,
						i_hdlc, i_addrMatch_stopl, i_manchester_oddp, i_nrzi_evenp, i_fcsEnable,
						i_txAbort, i_txEnable, i_rxEnable, i_rxAbortHold,
						i_rxFrameErrorInt, i_rxFcsErrorInt, i_rxFullInt,
						i_enIntTxHalt, i_enIntTxEmpty, i_enIntRxAbort, i_enIntRxOverRunError, -- i_enIntRxOverlapError,
						i_enIntRxOverflowError, i_enIntRxFrameError, i_enIntRxFcsError, i_enIntRxFull,
						i_txHaltInt, i_txEmptyInt, i_rxAbortInt, i_rxOverRunErrorInt, i_rxOverflowErrorInt)	-- , i_rxOverlapErrorInt
	begin
		if resetI = '1' then
			i_hdlc					<= '1';
			i_addrMatch_stopl		<= '0';
			i_manchester_oddp		<= '0';
			i_nrzi_evenp			<= '0';
			i_txAbort				<= '0';
			i_txEnable				<= '0';
			i_fcsEnable				<= '0';
			i_rxEnable				<= '0';

			i_enIntTxHalt			<= '0';
			i_enIntTxEmpty			<= '0';
			i_enIntRxOverflowError	<= '0';
			--i_enIntRxOverlapError	<= '0';
			i_enIntRxOverRunError	<= '0';
			i_enIntRxFrameError		<= '0';
			i_enIntRxAbort			<= '0';
			i_enIntRxFcsError		<= '0';
			i_enIntRxFull			<= '0';

			i_clearTxIntrs			<= '0';
			i_clearRxIntrs			<= '0';

			i_hdlcAddr_t16			<= (others => '1');
			i_txFrameSize_tv		<= (others => '0');
			i_txFlagSizeRxHoldSize_t4	<= (others => '0');
			i_baudrateConstant_ti	<= 0;
			busDataIO_t16			<= (others => 'Z');
		elsif rising_edge(clock168000kI) then
			if busCsI = '1' then
				if busAddrI_t8 /= x"06" and busAddrI_t8 /= x"0c" then
					i_clearTxIntrs	<= '0';
					i_clearRxIntrs	<= '0';
				end if;

				case busAddrI_t8 is
				when x"00" =>	-- buffer
					if busReadI = '1' then		-- read, rx
						busDataIO_t16	<= rxdI_t16;
						i_clearRxIntrs	<= '1';
					elsif busWriteI = '1' then	-- write, tx
						txdO_t16		<= busDataIO_t16;
						i_clearTxIntrs	<= '1';
						i_txEnable		<= '0';
					end if;

				when x"02" =>	-- rx length
					if busReadI = '1' then
						busDataIO_t16	<= "000000" & rxFrameSizeI_tv;
					end if;

				when x"04" =>	-- fast control
					if busWriteI = '1' then
						case busDataIO_t16 is
						when x"0008" =>	i_txEnable	<= '1';
						when x"0009" =>	i_txEnable	<= '0';
						when x"0080" => i_txAbort	<= '1';
						when x"0090" => i_txAbort	<= '0';
						when x"0800" =>	i_rxEnable	<= '1';
						when x"0900" =>	i_rxEnable	<= '0';
						when others =>	null;
						end case;
					end if;

				when x"06" =>	-- status & control
					if busReadI = '1' then
						busDataIO_t16	<= "000" & i_hdlc &
											"0" & i_addrMatch_stopl & i_manchester_oddp & i_nrzi_evenp &
											i_fcsEnable & "0" & i_txAbort & i_txEnable &
											"000" & i_rxEnable;
											--i_fcsEnable & txEmptyI & i_txAbort & i_txEnable &
											--txEnableM & flagOpenM & txAvailM & i_rxEnable;
						i_clearTxIntrs	<= '1';
						i_clearRxIntrs	<= '1';
					elsif busWriteI = '1' then
						i_hdlc				<= busDataIO_t16(12);
						--i_hdlc				<= '1';
						--i_dpllEnable		<= busDataIO_t16(11);
						i_addrMatch_stopl	<= busDataIO_t16(10);
						i_manchester_oddp	<= busDataIO_t16(9);
						i_nrzi_evenp		<= busDataIO_t16(8);
						i_fcsEnable			<= busDataIO_t16(7);
						i_txAbort			<= busDataIO_t16(5);
						i_txEnable			<= busDataIO_t16(4);
						i_rxEnable			<= busDataIO_t16(0);
					end if;

				when x"08" =>	-- status
					if busReadI = '1' then
						busDataIO_t16	<= "000000" & txHaltI & txEmptyI &
										i_rxAbortHold & "0" & rxOverflowErrorI & "0" & -- rxOverlapErrorI &
										rxOverRunErrorI & rxFrameErrorI & rxFcsErrorI & rxFullI;
						i_clearTxIntrs	<= '1';
						i_clearRxIntrs	<= '1';
					end if;

				when x"0a" =>	-- interrupt enable
					if busReadI = '1' then
						busDataIO_t16	<= "000000" & i_enIntTxHalt & i_enIntTxEmpty &
										i_enIntRxAbort & i_enIntRxOverRunError & "0" & i_enIntRxOverflowError &	-- i_enIntRxOverlapError
										i_enIntRxFrameError & i_enIntRxFcsError & "0" & i_enIntRxFull;
					elsif busWriteI = '1' then
						i_enIntTxHalt			<= busDataIO_t16(9);
						i_enIntTxEmpty			<= busDataIO_t16(8);
						i_enIntRxAbort			<= busDataIO_t16(7);
						i_enIntRxOverRunError	<= busDataIO_t16(6);
						--i_enIntRxOverlapError	<= busDataIO_t16(5);
						i_enIntRxOverflowError	<= busDataIO_t16(4);
						i_enIntRxFrameError		<= busDataIO_t16(3);
						i_enIntRxFcsError		<= busDataIO_t16(2);
						i_enIntRxFull			<= busDataIO_t16(0);
					end if;

				when x"0c" =>	-- interrupt status
					if busReadI = '1' then
						busDataIO_t16	<= "000000" & i_txHaltInt & i_txEmptyInt &
										i_rxAbortInt & i_rxOverRunErrorInt & "0" & i_rxOverflowErrorInt &	-- i_rxOverlapErrorInt
										i_rxFrameErrorInt & i_rxFcsErrorInt & "0" & i_rxFullInt;
						--i_clearTxIntrs	<= '1';
						--i_clearRxIntrs	<= '1';
					end if;

				when x"0e" =>	-- address
					if busReadI = '1' then
						busDataIO_t16	<= i_hdlcAddr_t16;
					elsif busWriteI = '1' then
						i_hdlcAddr_t16	<= busDataIO_t16;
					end if;

				when x"10" =>	-- tx length
					if busReadI = '1' then
						busDataIO_t16	<= "000000" & i_txFrameSize_tv;
					elsif busWriteI = '1' then
					if (conv_integer(busDataIO_t16) > cMaxAddress_ti) then
							i_txFrameSize_tv	<= conv_std_logic_vector(cMaxAddress_ti, cAddrWidth_ti + 1);
						else
							i_txFrameSize_tv	<= busDataIO_t16(cAddrWidth_ti downto 0);
						end if;
					end if;

				when x"12" =>	-- sync. length
					if busReadI = '1' then
						busDataIO_t16	<= "000000000000" & i_txFlagSizeRxHoldSize_t4;
					elsif busWriteI = '1' then
						i_txFlagSizeRxHoldSize_t4	<= busDataIO_t16(3 downto 0);
					end if;

				when x"14" =>	-- baudrate
					if busReadI = '1' then
						busDataIO_t16	<= conv_std_logic_vector(i_baudrateConstant_ti, 16);
					elsif busWriteI = '1' then
						i_baudrateConstant_ti	<= conv_integer(busDataIO_t16(7 downto 0));
					end if;

				when x"16" =>
					if busWriteI = '1' then
						i_reset		<= busDataIO_t16(0);
					end if;

				when x"18" =>
					if busReadI = '1' then
						busDataIO_t16	<= x"375a";
					end if;

				--when x"18" =>
				--	if busReadI = '1' then
				--	elsif busWriteI = '1' then
				--	end if;

				when others =>	null;

				end case;
			else
				busDataIO_t16	<= (others => 'Z');
			end if;		-- busCsI = '0'
		end if;		-- clock168000kI
	end process MapProc;
end behavioral;
