library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

library work;
use work.TypicPack.all;
use work.Packs.all;
use work.TYPES.all;

entity trxAsyncTb is
end trxAsyncTb;

architecture behavioral of trxAsyncTb is
	constant cFcsType_ti	: integer	:= 2;
	constant cAddrWidth_ti	: integer	:= 9;
	constant cMaxAddress_ti	: integer	:= 512;

	component Director
	generic (
		cAddrWidth_ti	: integer	:= 9;	-- 3ff, 1024
		cMaxAddress_ti	: integer	:= 512
	);
	port(
		nResetI				: in	std_logic;
		clock147456kI		: in	std_logic;
		busCsI				: in	std_logic;
		busAleI				: in	std_logic;
		busReadI			: in	std_logic;
		busWriteI			: in	std_logic;
		busAddrData_t16		: inout	std_logic_vector(15 downto 0);
		busInterruptO		: out	std_logic;

		hdlcO				: out	std_logic;
		addrMatch_stoplO	: out	std_logic;
		manchester_oddpO	: out	std_logic;
		nrzi_evenpO			: out	std_logic;
		fcsEnableO			: out	std_logic;
		baudrateDivO		: out	integer range 0 to 255;

		txEmptyI			: in	std_logic;		-- from TxBuffer, write to buffer => 0
		txHaltI				: in	std_logic;		-- from ZeroInsert, empty shift register, latch => 0
		txEnableO			: out	std_logic;
		txWriteO			: out	std_logic;
		txAbortO			: out	std_logic;		-- to FlagInsert
		txFlagSizeRxHoldSize_t4		: out	std_logic_vector(3 downto 0);
		txFrameSizeO_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		txdO_t16			: out	std_logic_vector(15 downto 0);

		rxFullI				: in	std_logic;		-- from RxBuffer, read from buffer => 0
		rxOverflowErrorI	: in	std_logic;		-- from RxBuffer, re-read => 0
		rxOverlapErrorI		: in	std_logic;
		rxOverRunErrorI		: in	std_logic;
		rxFrameErrorI		: in	std_logic;		-- from RxController, strange code ???
		rxFcsErrorI			: in	std_logic;		-- from RxFcs, re-receive => 0
		rxParityErrorI		: in	std_logic;
		rxAbortI			: in	std_logic;		-- from FlagDetector, receive flag => 0
		rxFrameSizeI_tv		: in	std_logic_vector(cAddrWidth_ti downto 0);
		rxAddressI_t16		: in	std_logic_vector(15 downto 0);
		rxdI_t16			: in	std_logic_vector(15 downto 0);
		rxEnableO			: out	std_logic;
		rxReadO				: out	std_logic;
		rxIgnoreO			: out	std_logic
	);
	end component;

	component TxBuffer
	generic (
		cAddrWidth_ti	: integer;
		cMaxAddress_ti	: integer
	);
	port (
		nResetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		txEnableI		: in	std_logic;						-- bus
		readBuffI		: in	std_logic;						-- tx fcs
		writeBuffI		: in	std_logic;						-- bus
		frameSizeI_tv	: in	std_logic_vector(cAddrWidth_ti downto 0);
		txI_t16			: in	std_logic_vector(15 downto 0);	-- bus
		txAvailO		: out	std_logic;						-- tx fcs
		txEmptyO		: out	std_logic;						-- Bus
		txO_t8			: out	std_logic_vector(7 downto 0)	-- tx fcs
	);
	end component;

	component TxAsync
	port (
		nResetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		enableI			: in	std_logic;
		txClock16I		: in	std_logic;
		evenParityI		: in	std_logic;
		oddParityI		: in	std_logic;
		stopLengI		: in	std_logic;
		txAvailI		: in	std_logic;
		txI_t8			: in	std_logic_vector(7 downto 0);
		readBuffO		: out	std_logic;
		frameDefenceO	: out	std_logic;
		txO				: out	std_logic
	);
	end component;

	component TxFcs
	generic (
		cFcsType_ti	: integer
	);
	port (
		nResetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		enableI			: in	std_logic;
		fcsEnableI		: in	std_logic;						-- bus
		readyI			: in	std_logic;						-- tx synch
		txAvailI		: in	std_logic;						-- tx buff
		txI_t8			: in	std_logic_vector(7 downto 0);	-- tx buff
		validFrameO		: out	std_logic;						-- tx synch
		writeByteO		: out	std_logic;						-- tx synch
		readBuffO		: out	std_logic;						-- tx buff
		txO_t8			: out	std_logic_vector(7 downto 0)	-- tx synch
	);
	end component;

	component TxSynchronizer
	port (
		nResetI			: in	std_logic;
		txClockI		: in	std_logic;
		clock147456kI	: in	std_logic;

		readyI			: in	std_logic;		-- tx controller
		haltI			: in	std_logic;		-- tx controller
		validFrameO		: out	std_logic;		-- tx controller
		abortFrameO		: out	std_logic;		-- tx controller
		writeByteO		: out	std_logic;		-- tx controller
		txO_t8			: out	std_logic_vector(7 downto 0);	-- tx controller

		validFrameI		: in	std_logic;		-- tx fcs
		writeByteI		: in	std_logic;		-- tx fcs
		txI_t8			: in	std_logic_vector(7 downto 0);	-- tx fcs
		readyO			: out	std_logic;		-- tx fcs

		abortFrameI		: in	std_logic;		-- bus
		haltO			: out	std_logic		-- bus
	);
	end component;

	component TxChannel
	port (
		nResetI			: in	std_logic;
		txClockI		: in	std_logic;
		txI_t8			: in	std_logic_vector(7 downto 0);
		txEnableI		: in	std_logic;
		writeByteI		: in	std_logic;
		validFrameI		: in	std_logic;
		abortFrameI		: in	std_logic;
		flagSizeI_t3	: in	std_logic_vector(2 downto 0);
		haltO			: out	std_logic;
		readyO			: out	std_logic;
		frameDefenceO	: out	std_logic;
		txO				: out	std_logic
	);
	end component;

	component RxBuffer
	generic (
		cFcsType_ti		: integer;
		cAddrWidth_ti	: integer;
		cMaxAddress_ti	: integer
	);
	port (
		nResetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		ignoreI			: in	std_logic;	-- force reset to rx buffer
		writeBuffI		: in	std_logic;
		readBuffI		: in	std_logic;
		eotI			: in	std_logic;
		rxI_t8			: in	std_logic_vector(7 downto 0);
		fullO			: out	std_logic;
		overflowErrorO	: out	std_logic;
		overlapErrorO	: out	std_logic;
		frameSizeO_tv	: out	std_logic_vector(cAddrWidth_ti downto 0);		-- ?????
		addrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		rxO_t16			: out	std_logic_vector(15 downto 0)
	);
	end component;

	component RxAsync
	port (
		nResetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		enableI			: in	std_logic;
		rxClock16I		: in	std_logic;
		evenParityI		: in	std_logic;
		oddParityI		: in	std_logic;
		stopLengI		: in	std_logic;
		eotHoldI_t4		: in	std_logic_vector(3 downto 0);
		rxI				: in	std_logic;
		rxO_t8			: out	std_logic_vector(7 downto 0);
		writeBuffO		: out	std_logic;
		eotO			: out	std_logic;
		parityErrorO	: out	std_logic;
		eotHoldM_ti		: out	integer;	-- ?????
		eotCountM_ti	: out	integer;	-- ?????
		clockM			: out	std_logic;	-- ?????
		fullM			: out	std_logic;	-- ?????
		frameErrorO		: out	std_logic
	);
	end component;

	component RxFcs
	generic (
		cFcsType_ti	 	: integer
	);
	port (
		nResetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		enableI			: in	std_logic;
		fcsEnableI		: in	std_logic;
		readyI			: in	std_logic;
		validFrameI		: in	std_logic;
		rxI_t8			: in	std_logic_vector(7 downto 0);
		readByteO		: out	std_logic;
		fcsErrorO		: out	std_logic;
		eotO			: out	std_logic;
		writeBuffO		: out	std_logic;
		headO_t16		: out	std_logic_vector(15 downto 0);
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
	end component;

	component RxSynchronizer
	port (
		nResetI			: in	std_logic;
		rxClockI		: in	std_logic;
		clock147456kI	: in	std_logic;

		readyI			: in	std_logic;
		abortDetectI	: in	std_logic;
		validFrameI		: in	std_logic;
		overRunErrorI	: in	std_logic;
		frameErrorI		: in	std_logic;
		rxI_t8			: in	std_logic_vector(7 downto 0);
		readByteO		: out	std_logic;

		readByteI		: in	std_logic;
		validFrameO		: out	std_logic;
		abortDetectO	: out	std_logic;
		overRunErrorO	: out	std_logic;
		frameErrorO		: out	std_logic;
		readyO			: out	std_logic;
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
	end component;

	component RxChannel
	port (
		nResetI			: in	std_logic;
		rxClockI		: in	std_logic;
		rxI				: in	std_logic;
		rxEnableI		: in	std_logic;
		readByteI		: in	std_logic;
		validFrameO		: out	std_logic;
		overRunErrorO	: out	std_logic;
		frameErrorO		: out	std_logic;
		abortDetectO	: out	std_logic;
		--frameStatusM	: out	std_logic;		-- ?????
		--frameMonM		: out	std_logic;		-- ?????
		--enableZeroM		: out	std_logic;		-- ?????
		--flagCounterM_ti	: out	integer;
		--flagDetectM		: out	std_logic;		-- ?????
		readyO			: out	std_logic;
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
	end component;

	-- connection signals
	signal i_hdlcA				: std_logic	:= '0';
	signal i_addrMatch_stoplA	: std_logic	:= '0';
	signal i_manchester_oddpA	: std_logic	:= '0';
	signal i_nrzi_evenpA		: std_logic	:= '0';
	signal i_baudrateDivA_ti	: integer range 0 to 255	:= 0;
	signal i_fcsEnableA			: std_logic	:= '0';
	signal i_asyncA				: std_logic	:= '0';

	signal i_clockx16			: std_logic	:= '0';
	signal i_txClock			: std_logic	:= '0';
	signal i_rxClock			: std_logic	:= '0';

	signal i_nReset				: std_logic	:= '0';
	signal i_clock147456kI		: std_logic	:= '0';
	signal i_busCs				: std_logic	:= '0';
	signal i_busAle				: std_logic	:= '0';
	signal i_busRead			: std_logic	:= '0';
	signal i_busWrite			: std_logic	:= '0';
	signal i_busAddrData_t16	: std_logic_vector(15 downto 0);
	signal i_busInterrupt		: std_logic	:= '0';
	signal i_rts				: std_logic	:= '0';

	signal i_addr_t8			: std_logic_vector(7 downto 0);
	signal i_addr_t10			: std_logic_vector(9 downto 0);
	signal i_rxLength_t10		: std_logic_vector(9 downto 0)	:= (others => '0');
	signal i_data_t16			: std_logic_vector(15 downto 0);

	signal i_txa_t16			: std_logic_vector(15 downto 0);
	signal i_txb_t8				: std_logic_vector(7 downto 0);
	signal i_txc_t8				: std_logic_vector(7 downto 0);
	signal i_txd_t8				: std_logic_vector(7 downto 0);
	signal i_txde_t8			: std_logic_vector(7 downto 0);
	signal i_txef_t8			: std_logic_vector(7 downto 0);
	signal i_txc				: std_logic;
	signal i_txf				: std_logic;
	signal i_txz				: std_logic;

	signal i_txEmptyBA			: std_logic;
	signal i_txHaltEA			: std_logic;
	signal i_txHaltFE			: std_logic;
	signal i_txEnableABF		: std_logic;
	signal i_txWriteBuffAB		: std_logic;
	signal i_txAbortFrameAE		: std_logic;
	signal i_txAbortFrameEF		: std_logic;
	signal i_txFlagSizeRxHoldSize_t4	: std_logic_vector(3 downto 0);
	signal i_txFrameSizeAB_tv	: std_logic_vector(cAddrWidth_ti downto 0);
	signal i_txReadBuffB		: std_logic;
	signal i_txReadBuffC		: std_logic;
	signal i_txReadBuffD		: std_logic;
	signal i_txAvailBCD			: std_logic;
	signal i_txFrameDefenceC	: std_logic;
	signal i_txFrameDefenceF	: std_logic;
	signal i_txReadyED			: std_logic;
	signal i_txReadyFE			: std_logic;
	signal i_txValidFrameDE		: std_logic;
	signal i_txValidFrameEF		: std_logic;
	signal i_txWriteByteDE		: std_logic;
	signal i_txWriteByteEF		: std_logic;

	signal i_rxa_t16			: std_logic_vector(15 downto 0);
	signal i_rxb_t8				: std_logic_vector(7 downto 0);
	signal i_rxc_t8				: std_logic_vector(7 downto 0);
	signal i_rxd_t8				: std_logic_vector(7 downto 0);
	signal i_rxed_t8			: std_logic_vector(7 downto 0);
	signal i_rxfe_t8			: std_logic_vector(7 downto 0);
	signal i_rxc				: std_logic;
	signal i_rxf				: std_logic;
	signal i_rxz				: std_logic;

	signal i_rxFullBA			: std_logic;
	signal i_rxOverflowErrorBA	: std_logic;
	signal i_rxOverlapErrorBA	: std_logic;
	signal i_rxOverRunErrorEA	: std_logic;
	signal i_rxOverRunErrorFE	: std_logic;
	signal i_rxFrameErrorA		: std_logic;
	signal i_rxFrameErrorC		: std_logic;
	signal i_rxFrameErrorE		: std_logic;
	signal i_rxFrameErrorFE		: std_logic;
	signal i_rxFcsErrorDA		: std_logic;
	signal i_rxParityErrorCA	: std_logic;
	signal i_rxAbortDetectEA	: std_logic;
	signal i_rxAbortDetectFE	: std_logic;
	signal i_rxFrameSizeBA_tv	: std_logic_vector(cAddrWidth_ti downto 0);
	signal i_rxHeadDA_t16		: std_logic_vector(15 downto 0);
	signal i_rxEnableAF			: std_logic;
	signal i_rxIgnoreAB			: std_logic;
	signal i_rxReadBuffAB		: std_logic;
	signal i_rxWriteBuffB		: std_logic;
	signal i_rxWriteBuffC		: std_logic;
	signal i_rxWriteBuffD		: std_logic;
	signal i_rxEotB				: std_logic;
	signal i_rxEotC				: std_logic;
	signal i_rxEotD				: std_logic;
	signal i_rxReadyED			: std_logic;
	signal i_rxReadyFE			: std_logic;
	signal i_rxValidFrameED		: std_logic;
	signal i_rxValidFrameFE		: std_logic;
	signal i_rxReadByteDE		: std_logic;
	signal i_rxReadByteEF		: std_logic;
	signal i_rxAddrM_tv			: std_logic_vector(cAddrWidth_ti downto 0);
	signal i_rxEotHoldM_ti		: integer;		-- ?????
	signal i_rxEotCountM_ti		: integer;		-- ?????
	signal i_rxClockM			: std_logic;	-- ?????
	signal i_rxFullM			: std_logic;	-- ?????

begin
	i_nReset	<= '1' after 100 ns;
	i_clock147456kI	<= not i_clock147456kI after 10 ns;
	i_clockx16	<= not i_clockx16 after 15 ns;
	i_txClock	<= not i_txClock after 240 ns;
	i_rxClock	<= i_txClock;
	--i_rts		<= i_txValidFrameDE or i_txFrameDefenceF;
	i_rts		<= i_txAvailBCD or (i_hdlcA and i_txFrameDefenceF) or (not i_hdlcA and i_txFrameDefenceC);
	i_asyncA	<= not i_hdlcA;

	i_rxFrameErrorA	<= (i_hdlcA and i_rxFrameErrorE) or (not i_hdlcA and i_rxFrameErrorC);
	i_rxWriteBuffB	<= (i_hdlcA and i_rxWriteBuffD) or (not i_hdlcA and i_rxWriteBuffC);
	i_rxEotB		<= (i_hdlcA and i_rxEotD) or (not i_hdlcA and i_rxEotC);
	i_rxb_t8		<= i_rxd_t8	when i_hdlcA = '1'	else i_rxc_t8;
	i_rxf			<= i_hdlcA and i_rxz;
	i_rxc			<= not i_hdlcA and i_rxz;

	i_txReadBuffB	<= (i_hdlcA and i_txReadBuffD) or (not i_hdlcA and i_txReadBuffC);
	i_txc_t8		<= i_txb_t8	when i_hdlcA = '0'	else (others => '0');
	i_txd_t8		<= i_txb_t8	when i_hdlcA = '1'	else (others => '0');

	i_txz			<= (i_hdlcA and i_txf) or (not i_hdlcA and i_txc);
	i_rxz			<= i_txz;

	process(i_nReset, i_clock147456kI)
		variable i_repeat_ti	: integer	:= 0;
		variable i_busCycle_ti	: integer	:= 0;
		variable i_seqCycle_ti	: integer	:= 0;
		variable i_sData_t16	: std_logic_vector(15 downto 0)	:= (others => '0');
		variable i_busSeq		: std_logic	:= '0';
		variable i_read			: std_logic	:= '0';

	begin
		if i_nReset = '0' then
			i_repeat_ti		:= 0;
			i_busCycle_ti	:= 0;
			i_seqCycle_ti	:= 0;
			i_busSeq		:= '0';
			i_read			:= '0';
		elsif rising_edge(i_clock147456kI) then
			if i_busSeq = '0' then
				i_busAddrData_t16	<= (others => 'Z');
				case i_seqCycle_ti is
				when 0 =>
					i_addr_t8		<= x"06";
					i_data_t16		<= x"0001";	--"0a81";		-- hdlc, nrzi, fcs enable, rx enable
					i_seqCycle_ti	:= 1;
					i_busSeq		:= '1';
				when 1 =>
					i_addr_t8		<= x"0a";
					i_data_t16		<= x"0001";		-- enable rx full int
					i_seqCycle_ti	:= 2;
					i_busSeq		:= '1';
				when 2 =>
					i_addr_t8		<= x"10";
					i_data_t16		<= x"0200";		-- tx length
					i_seqCycle_ti	:= 3;
					i_busSeq		:= '1';
				when 3 =>
					i_addr_t8		<= x"12";
					i_data_t16		<= x"0003";		-- synch. char. legnth
					i_addr_t10		<= (others => '0');
					i_sData_t16		:= x"f001";
					i_seqCycle_ti	:= 4;
					i_busSeq		:= '1';
				when 4 =>
					if i_addr_t10(8) = '1' then
						i_seqCycle_ti	:= 5;
					else
						i_addr_t10		<= i_addr_t10 + 1;
						i_addr_t8		<= x"00";
						i_data_t16		<= i_sData_t16;
						i_sData_t16		:= i_sData_t16 + 1;
						i_busSeq		:= '1';
					end if;
				when 5 =>
					i_addr_t8		<= x"04";
					i_data_t16		<= x"0008";		-- tx enable
					i_seqCycle_ti	:= 6;
					i_busSeq		:= '1';
				when 6 =>
					if i_rts = '1' then
						i_seqCycle_ti	:= 7;
					end if;
				when 7 =>
					if i_rts = '0' then
						i_seqCycle_ti	:= 8;
					end if;
				when 8 =>
					i_addr_t8		<= x"04";
					i_data_t16		<= x"0009";		-- tx disable
					i_seqCycle_ti	:= 9;
					i_busSeq		:= '1';
				when 9 =>
					if i_busInterrupt = '1' then
						i_seqCycle_ti	:= 10;
					end if;
				when 10 =>
					i_addr_t8		<= x"0c";		-- interrupt status
					i_seqCycle_ti	:= 11;
					i_read			:= '1';
					i_busSeq		:= '1';
				when 11 =>
					i_addr_t8		<= x"0c";		-- interrupt status
					i_busAddrData_t16	<= (others => 'Z');
					i_seqCycle_ti	:= 12;
					i_busSeq		:= '1';
				when 12 =>
					i_addr_t8		<= x"02";		-- rx length
					i_addr_t10		<= (others => '0');
					i_rxLength_t10	<= "1000000010";
					i_seqCycle_ti	:= 13;
					i_busSeq		:= '1';
				when 13 =>
					if i_addr_t10 = i_rxLength_t10 then
						i_seqCycle_ti	:= 14;
						i_sData_t16		:= (others => '0');
					else
						if i_addr_t10 = "0000000000" then
							i_rxLength_t10		<=  "0" & i_busAddrData_t16(9 downto 1);
							if i_busAddrData_t16(0) = '1' then
								i_rxLength_t10	<= i_rxLength_t10 + 1;
							end if;
						end if;
						i_addr_t10		<= i_addr_t10 + 1;
						i_addr_t8		<= x"00";
						i_data_t16		<= (others => 'Z');
						i_busSeq		:= '1';
					end if;
				when 14 =>
					if i_sData_t16(8) = '1' then
						i_sData_t16		:= (others => '0');
						if i_repeat_ti	= 1 then
							i_seqCycle_ti	:= 15;
						else
							i_repeat_ti		:= i_repeat_ti + 1;
							i_read			:= '0';
							i_seqCycle_ti	:= 0;
						end if;
					else
						i_sData_t16		:= i_sData_t16 + 1;
					end if;
				when others =>	null;
				end case;
			else
				case i_busCycle_ti is
				when 0 =>
					i_busAddrData_t16	<= x"00" & i_addr_t8;
					i_busAle			<= '1';
					i_busCs				<= '1';
					i_busCycle_ti		:= 1;
				when 1 =>
					i_busAle			<= '0';
					if i_read = '1' then
						i_busWrite			<= '0';
						i_busRead			<= '1';
						i_busAddrData_t16	<= (others => 'Z');
						--i_data_t16			<= i_busAddrData_t16;
					else
						i_busRead			<= '0';
						i_busWrite			<= '1';
						i_busAddrData_t16	<= i_data_t16;
					end if;
					i_busCycle_ti		:= 2;
				when 2 =>
					i_busRead			<= '0';
					i_busWrite			<= '0';
					i_busCs				<= '0';
					i_data_t16			<= i_busAddrData_t16;
					--i_busCycle_ti		:= 3;
				--when 3 =>
					--i_busAddrData_t16	<= (others => 'Z');
					i_busCycle_ti		:= 0;
					i_busSeq			:= '0';
				when others =>	null;
				end case;
			end if;
		end if;
	end process;

	frame1	: Director		-- A frame
	generic map (
		cAddrWidth_ti	=> cAddrWidth_ti,
		cMaxAddress_ti	=> cMaxAddress_ti
	)
	port map (
		nResetI				=> i_nReset,
		clock147456kI		=> i_clock147456kI,
		busCsI				=> i_busCs,
		busAleI				=> i_busAle,
		busReadI			=> i_busRead,
		busWriteI			=> i_busWrite,
		busAddrData_t16		=> i_busAddrData_t16,
		busInterruptO		=> i_busInterrupt,

		hdlcO				=> i_hdlcA,
		addrMatch_stoplO	=> i_addrMatch_stoplA,
		manchester_oddpO	=> i_manchester_oddpA,
		nrzi_evenpO			=> i_nrzi_evenpA,
		fcsEnableO			=> i_fcsEnableA,
		baudrateDivO		=> i_baudrateDivA_ti,

		txEmptyI			=> i_txEmptyBA,
		txHaltI				=> i_txHaltEA,
		txEnableO			=> i_txEnableABF,
		txWriteO			=> i_txWriteBuffAB,
		txAbortO			=> i_txAbortFrameAE,
		txFlagSizeRxHoldSize_t4	=> i_txFlagSizeRxHoldSize_t4,
		txFrameSizeO_tv		=> i_txFrameSizeAB_tv,
		txdO_t16			=> i_txa_t16,

		rxFullI				=> i_rxFullBA,
		rxOverflowErrorI	=> i_rxOverflowErrorBA,
		rxOverlapErrorI		=> i_rxOverlapErrorBA,
		rxOverRunErrorI		=> i_rxOverRunErrorEA,
		rxFrameErrorI		=> i_rxFrameErrorA,
		rxFcsErrorI			=> i_rxFcsErrorDA,
		rxParityErrorI		=> i_rxParityErrorCA,
		rxAbortI			=> i_rxAbortDetectEA,
		rxFrameSizeI_tv		=> i_rxFrameSizeBA_tv,
		rxAddressI_t16		=> i_rxHeadDA_t16,
		rxdI_t16			=> i_rxa_t16,
		rxEnableO			=> i_rxEnableAF,
		rxReadO				=> i_rxReadBuffAB,
		rxIgnoreO			=> i_rxIgnoreAB
	);

	frame2	: TxBuffer		-- B frame
	generic map (
		cAddrWidth_ti		=> cAddrWidth_ti,
		cMaxAddress_ti		=> cMaxAddress_ti
	)
	port map (
		nResetI			=> i_nReset,
		clock147456kI	=> i_clock147456kI,
		txEnableI		=> i_txEnableABF,
		readBuffI		=> i_txReadBuffB,
		writeBuffI		=> i_txWriteBuffAB,
		frameSizeI_tv	=> i_txFrameSizeAB_tv,
		txI_t16			=> i_txa_t16,
		txAvailO		=> i_txAvailBCD,
		txEmptyO		=> i_txEmptyBA,
		txO_t8			=> i_txb_t8
	);

	frame3	: TxAsync		-- C Frame
	port map (
		nResetI			=> i_nReset,
		clock147456kI	=> i_clock147456kI,
		enableI			=> i_asyncA,
		txClock16I		=> i_clockx16,
		evenParityI		=> i_nrzi_evenpA,
		oddParityI		=> i_manchester_oddpA,
		stopLengI		=> i_addrMatch_stoplA,
		txAvailI		=> i_txAvailBCD,
		txI_t8			=> i_txc_t8,
		readBuffO		=> i_txReadBuffC,
		frameDefenceO	=> i_txFrameDefenceC,
		txO				=> i_txc
	);

	frame4	: TxFcs			-- D frame
	generic map (
		cFcsType_ti		=> cFcsType_ti
	)
	port map (
		nResetI			=> i_nReset,
		clock147456kI	=> i_clock147456kI,
		enableI			=> i_hdlcA,
		fcsEnableI		=> i_fcsEnableA,
		readyI			=> i_txReadyED,
		txAvailI		=> i_txAvailBCD,
		txI_t8			=> i_txd_t8,
		validFrameO		=> i_txValidFrameDE,
		writeByteO		=> i_txWriteByteDE,
		readBuffO		=> i_txReadBuffD,
		txO_t8			=> i_txde_t8
	);

	frame5	: TxSynchronizer	-- E frame
	port map (
		nResetI			=> i_nReset,
		txClockI		=> i_txClock,
		clock147456kI	=> i_clock147456kI,
		abortFrameI		=> i_txAbortFrameAE,
		validFrameI		=> i_txValidFrameDE,
		writeByteI		=> i_txWriteByteDE,
		txI_t8			=> i_txde_t8,
		readyI			=> i_txReadyFE,
		haltI			=> i_txHaltFE,
		validFrameO		=> i_txValidFrameEF,
		abortFrameO		=> i_txAbortFrameEF,
		writeByteO		=> i_txWriteByteEF,
		readyO			=> i_txReadyED,
		haltO			=> i_txHaltEA,
		txO_t8			=> i_txef_t8
	);

	frame6	: TxChannel		-- F frame
	port map (
		nResetI			=> i_nReset,
		txClockI		=> i_txClock,
		txI_t8			=> i_txef_t8,
		txEnableI		=> i_txEnableABF,
		writeByteI		=> i_txWriteByteEF,
		validFrameI		=> i_txValidFrameEF,
		abortFrameI		=> i_txAbortFrameEF,
		flagSizeI_t3	=> i_txFlagSizeRxHoldSize_t4(2 downto 0),
		haltO			=> i_txHaltFE,
		readyO			=> i_txReadyFE,
		frameDefenceO	=> i_txFrameDefenceF,
		txO				=> i_txf
	);

	frame7	: RxBuffer		-- B frame
	generic map (
		cFcsType_ti		=> cFcsType_ti,
		cAddrWidth_ti	=> cAddrWidth_ti,
		cMaxAddress_ti	=> cMaxAddress_ti
	)
	port map (
		nResetI			=> i_nReset,
		clock147456kI	=> i_clock147456kI,
		ignoreI			=> i_rxIgnoreAB,
		writeBuffI		=> i_rxWriteBuffB,
		readBuffI		=> i_rxReadBuffAB,
		eotI			=> i_rxEotB,
		rxI_t8			=> i_rxb_t8,
		fullO			=> i_rxFullBA,
		overflowErrorO	=> i_rxOverflowErrorBA,
		overlapErrorO	=> i_rxOverlapErrorBA,
		frameSizeO_tv	=> i_rxFrameSizeBA_tv,
		addrM_tv		=> i_rxAddrM_tv,
		rxO_t16			=> i_rxa_t16
	);

	frame8	: RxAsync		-- C Frame
	port map (
		nResetI			=> i_nReset,
		clock147456kI	=> i_clock147456kI,
		enableI			=> i_asyncA,
		rxClock16I		=> i_clockx16,
		evenParityI		=> i_nrzi_evenpA,
		oddParityI		=> i_manchester_oddpA,
		stopLengI		=> i_addrMatch_stoplA,
		eotHoldI_t4		=> i_txFlagSizeRxHoldSize_t4,
		rxI				=> i_rxc,
		rxO_t8			=> i_rxc_t8,
		writeBuffO		=> i_rxWriteBuffC,
		eotO			=> i_rxEotC,
		parityErrorO	=> i_rxParityErrorCA,
		eotHoldM_ti		=> i_rxEotHoldM_ti,
		eotCountM_ti	=> i_rxEotCountM_ti,
		clockM			=> i_rxClockM,
		fullM			=> i_rxFullM,
		frameErrorO		=> i_rxFrameErrorC
	);

	frame9	: RxFcs			-- D frame
	generic map (
		cFcsType_ti		=> cFcsType_ti
	)
	port map (
		nResetI			=> i_nReset,
		clock147456kI	=> i_clock147456kI,
		enableI			=> i_hdlcA,
		fcsEnableI		=> i_fcsEnableA,
		readyI			=> i_rxReadyED,
		validFrameI		=> i_rxValidFrameED,
		rxI_t8			=> i_rxed_t8,
		readByteO		=> i_rxReadByteDE,
		fcsErrorO		=> i_rxFcsErrorDA,
		eotO			=> i_rxEotD,
		writeBuffO		=> i_rxWriteBuffD,
		headO_t16		=> i_rxHeadDA_t16,
		rxO_t8			=> i_rxd_t8
	);

	frame10	: RxSynchronizer	-- E frame
	port map (
		nResetI			=> i_nReset,
		rxClockI		=> i_rxClock,
		clock147456kI	=> i_clock147456kI,
		readyI			=> i_rxReadyFE,
		abortDetectI	=> i_rxAbortDetectFE,
		validFrameI		=> i_rxValidFrameFE,
		overRunErrorI	=> i_rxOverRunErrorFE,
		frameErrorI		=> i_rxFrameErrorFE,
		rxI_t8			=> i_rxfe_t8,
		readByteO		=> i_rxReadByteEF,
		readByteI		=> i_rxReadByteDE,
		validFrameO		=> i_rxValidFrameED,
		abortDetectO	=> i_rxAbortDetectEA,
		overRunErrorO	=> i_rxOverRunErrorEA,
		frameErrorO		=> i_rxFrameErrorE,
		readyO			=> i_rxReadyED,
		rxO_t8			=> i_rxed_t8
	);

	frame11	: RxChannel		-- F frame
	port map (
		nResetI			=> i_nReset,
		rxClockI		=> i_rxClock,
		rxI				=> i_rxf,
		rxEnableI		=> i_rxEnableAF,
		readByteI		=> i_rxReadByteEF,
		validFrameO		=> i_rxValidFrameFE,
		overRunErrorO	=> i_rxOverRunErrorFE,
		frameErrorO		=> i_rxFrameErrorFE,
		abortDetectO	=> i_rxAbortDetectFE,
		readyO			=> i_rxReadyFE,
		rxO_t8			=> i_rxfe_t8
	);
end;