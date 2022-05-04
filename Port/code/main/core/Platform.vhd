-------------------------------------------------------------------------------
-- Title	: main
-- Project	: serial communication
-- File		: Platform.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

use work.TypicPack.all;
use work.Packs.all;

entity Platform is
	generic (
		cFcsType_ti		: integer	:= 2;
		cAddrWidth_ti	: integer	:= 9;
		cMaxAddress_ti	: integer	:= 512
	);
	port (
		clock168000kI	: in	std_logic;
		clock147456kI	: in	std_logic;
		rxI				: in	std_logic;
		txO				: out	std_logic;
		rtsO			: out	std_logic;
		--clock16M		: out	std_logic;	-- ?????
		--clockM			: out	std_logic;	-- ?????

		--taddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);	-- ?????
		--tbM_t8			: out	std_logic_vector(7 downto 0);				-- ?????
		--enableClockRM	: out	std_logic;
		--treadBuffM		: out	std_logic;
		--tavailM			: out	std_logic;
		rAddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);	-- ?????
		rbM_t8			: out	std_logic_vector(7 downto 0);				-- ?????
		rWriteBuffM		: out	std_logic;
		rReadBuffM		: out	std_logic;

		--txEnableM		: out	std_logic;	-- ?????
		--txAvailM		: out	std_logic;	-- ?????
		--rxFullM			: out	std_logic;	-- ?????
		--rxEotM			: out	std_logic;	-- ?????

		resetI			: in	std_logic;
		--clock147456kI	: in	std_logic;
		busCsI			: in	std_logic;
		busReadI		: in	std_logic;
		busWriteI		: in	std_logic;
		busAddrI_t8		: in	std_logic_vector(7 downto 0);
		busDataIO_t16	: inout	std_logic_vector(15 downto 0);
		busInterruptO	: out	std_logic
	);
end Platform;

architecture behavioral of Platform is
	signal i_reset				: std_logic;
	signal i_clockx16			: std_logic;	-- 147456000 / div / 2
	signal i_txClock			: std_logic;
	signal i_rxClock			: std_logic;

	signal i_hdlcA				: std_logic;
	signal i_asyncA				: std_logic;
	--signal i_dpllEnableA		: std_logic;
	signal i_addrMatch_stoplA	: std_logic;
	signal i_manchester_oddpA	: std_logic;
	signal i_nrzi_evenpA		: std_logic;
	signal i_baudrateConstantA_ti	: integer range 0 to 4000	:= 0;
	signal i_fcsEnableA			: std_logic;

	signal i_txa_t16			: std_logic_vector(15 downto 0);
	signal i_txb_t8				: std_logic_vector(7 downto 0);
	signal i_txc_t8				: std_logic_vector(7 downto 0);
	signal i_txd_t8				: std_logic_vector(7 downto 0);
	signal i_txde_t8			: std_logic_vector(7 downto 0);
	signal i_txef_t8			: std_logic_vector(7 downto 0);
	signal i_txc				: std_logic;
	signal i_txf				: std_logic;
	signal i_txg				: std_logic;

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
	signal i_rxg				: std_logic;

	signal i_rxFullBA			: std_logic;
	signal i_rxOverflowErrorBA	: std_logic;
	--signal i_rxOverlapErrorBA	: std_logic;
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
	--signal i_rxIgnoreAB			: std_logic;
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

	--signal i_flagOpenM			: std_logic;
	--signal i_txEnableM			: std_logic;

begin
	--clock16M		<= i_clockx16;
	--clock16M		<= i_txClock;

	i_asyncA		<= not i_hdlcA;

	rtsO			<= i_txAvailBCD or (i_hdlcA and i_txFrameDefenceF) or (i_asyncA and i_txFrameDefenceC);
	i_rxFrameErrorA	<= (i_hdlcA and i_rxFrameErrorE) or (i_asyncA and i_rxFrameErrorC);
	i_rxWriteBuffB	<= (i_hdlcA and i_rxWriteBuffD) or (i_asyncA and i_rxWriteBuffC);
	i_rxEotB		<= (i_hdlcA and i_rxEotD) or (i_asyncA and i_rxEotC);
	i_rxb_t8		<= i_rxd_t8 when i_hdlcA = '1'		else
						i_rxc_t8 when i_asyncA = '1'	else
						(others => '0');
	i_rxg			<= i_hdlcA and rxI;
	i_rxc			<= i_asyncA and rxI;

	i_txReadBuffB	<= (i_hdlcA and i_txReadBuffD) or (i_asyncA and i_txReadBuffC);
	i_txc_t8		<= i_txb_t8	when i_asyncA = '1'	else (others => '0');	-- async
	i_txd_t8		<= i_txb_t8 when i_hdlcA = '1'	else (others => '0');	-- sync
	txO				<= (i_hdlcA and i_txg) or (i_asyncA and i_txc);
	--txO				<= i_txg;

	--treadBuffM		<= i_txReadBuffB;
	--tavailM			<= i_txAvailBCD;

	rbM_t8			<= i_rxb_t8;
	rReadBuffM		<= i_rxReadBuffAB;
	rWriteBuffM		<= i_rxWriteBuffB;

	-- wrong!!!!!
	-- bps = 147456000 / (i_baudrateConstantA_ti + 1) * 32
	-- baudrateConstant = 147456000 / (bps * 32) - 1;
	-- bps = 73728000 / (i_baudrateConstantA_ti + 1) * 32
	-- baudrateConstant = 73728000 / (bps * 32) - 1;
	BaudrateGenerator	: process(resetI, clock147456kI)
		variable baudrateCounter_ti	: integer;
	begin
		if resetI = '1' then
			baudrateCounter_ti	:= 0;
			i_clockx16			<= '0';
		elsif rising_edge(clock147456kI) then
			if baudrateCounter_ti >= i_baudrateConstantA_ti then
				baudrateCounter_ti	:= 0;
				i_clockx16			<= not i_clockx16;	-- divide by 2 and divide terminal by 16
			else
				baudrateCounter_ti	:= baudrateCounter_ti + 1;
			end if;
		end if;
	end process BaudrateGenerator;

	MemMapFrame	: MemMap			-- A frame
	generic map (
		cAddrWidth_ti	=> cAddrWidth_ti,
		cMaxAddress_ti	=> cMaxAddress_ti
	)
	port map (
		resetI				=> resetI,
		resetO				=> i_reset,
		clock168000kI		=> clock168000kI,
		busCsI				=> busCsI,
		busReadI			=> busReadI,
		busWriteI			=> busWriteI,
		busAddrI_t8			=> busAddrI_t8,
		busDataIO_t16		=> busDataIO_t16,
		busInterruptO		=> busInterruptO,

		hdlcO				=> i_hdlcA,
		--dpllEnableO			=> i_dpllEnableA,
		addrMatch_stoplO	=> i_addrMatch_stoplA,
		manchester_oddpO	=> i_manchester_oddpA,
		nrzi_evenpO			=> i_nrzi_evenpA,
		fcsEnableO			=> i_fcsEnableA,
		baudrateConstantO_ti	=> i_baudrateConstantA_ti,

		txEmptyI			=> i_txEmptyBA,
		txHaltI				=> i_txHaltEA,
		txEnableO			=> i_txEnableABF,
		txWriteO			=> i_txWriteBuffAB,
		txAbortO			=> i_txAbortFrameAE,
		txFlagSizeRxHoldSize_t4	=> i_txFlagSizeRxHoldSize_t4,
		txFrameSizeO_tv		=> i_txFrameSizeAB_tv,
		txdO_t16			=> i_txa_t16,

		--flagOpenM			=> i_flagOpenM,
		--txEnableM			=> i_txEnableM,		-- ?????
		--txAvailM			=> i_txAvailBCD,

		rxFullI				=> i_rxFullBA,
		rxOverflowErrorI	=> i_rxOverflowErrorBA,
		--rxOverlapErrorI		=> i_rxOverlapErrorBA,
		rxOverRunErrorI		=> i_rxOverRunErrorEA,
		rxFrameErrorI		=> i_rxFrameErrorA,
		rxFcsErrorI			=> i_rxFcsErrorDA,
		rxParityErrorI		=> i_rxParityErrorCA,
		rxAbortI			=> i_rxAbortDetectEA,
		rxFrameSizeI_tv		=> i_rxFrameSizeBA_tv,
		rxAddressI_t16		=> i_rxHeadDA_t16,
		rxdI_t16			=> i_rxa_t16,
		rxEnableO			=> i_rxEnableAF,
		rxReadO				=> i_rxReadBuffAB
		--rxIgnoreO			=> i_rxIgnoreAB
	);

	TxBuffFrame		: TxBuffer	-- B frame
	generic map (
		cAddrWidth_ti	=> cAddrWidth_ti,
		cMaxAddress_ti	=> cMaxAddress_ti
	)
	port map (
		resetI			=> i_reset,
		clockI			=> clock168000kI,
		txEnableI		=> i_txEnableABF,
		readBuffI		=> i_txReadBuffB,
		writeBuffI		=> i_txWriteBuffAB,
		frameSizeI_tv	=> i_txFrameSizeAB_tv,
		txI_t16			=> i_txa_t16,
		--taddrM_tv		=> taddrM_tv,
		--tbM_t8			=> tbM_t8,
		--enableClockRM	=> enableClockRM,
		txAvailO		=> i_txAvailBCD,
		txEmptyO		=> i_txEmptyBA,
		txO_t8			=> i_txb_t8
	);

	TxAsyncFrame	: TxAsync	-- C Frame
	port map (
		resetI			=> i_reset,
		clock147456kI	=> clock147456kI,
		enableI			=> i_asyncA,		-- not i_hdlcA,
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

	TxFcsFrame		: TxFcs		-- D frame
	generic map (
		cFcsType_ti		=> cFcsType_ti
	)
	port map (
		resetI			=> i_reset,
		clockI			=> clock168000kI,
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

	TxSynchFrame	: TxSynchronizer	-- E frame
	port map (
		resetI			=> i_reset,
		txClockI		=> i_txClock,
		clockI			=> clock168000kI,	--clock147456kI,
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

	TxUnitFrame		: TxChannel		-- F frame
	port map (
		resetI			=> i_reset,
		txClockI		=> i_txClock,
		txI_t8			=> i_txef_t8,
		txEnableI		=> i_txEnableABF,
		writeByteI		=> i_txWriteByteEF,
		validFrameI		=> i_txValidFrameEF,
		abortFrameI		=> i_txAbortFrameEF,
		flagSizeI_t3	=> i_txFlagSizeRxHoldSize_t4(2 downto 0),
		--flagOpenM		=> i_flagOpenM,		-- ?????
		--txEnableM		=> i_txEnableM,		-- ?????
		haltO			=> i_txHaltFE,
		readyO			=> i_txReadyFE,
		frameDefenceO	=> i_txFrameDefenceF,
		txO				=> i_txf
	);

	RxBuffFrame		: RxBuffer		-- B frame
	generic map (
		cFcsType_ti		=> cFcsType_ti,
		cAddrWidth_ti	=> cAddrWidth_ti,
		cMaxAddress_ti	=> cMaxAddress_ti
	)
	port map (
		resetI			=> i_reset,
		clockI			=> clock168000kI,
		--ignoreI			=> i_rxIgnoreAB,
		writeBuffI		=> i_rxWriteBuffB,
		readBuffI		=> i_rxReadBuffAB,
		eotI			=> i_rxEotB,
		rxI_t8			=> i_rxb_t8,
		fullO			=> i_rxFullBA,
		overflowErrorO	=> i_rxOverflowErrorBA,
		--overlapErrorO	=> i_rxOverlapErrorBA,
		frameSizeO_tv	=> i_rxFrameSizeBA_tv,
		rAddrM_tv		=> rAddrM_tv,
		rxO_t16			=> i_rxa_t16
	);

	RxAsyncFrame	: RxAsync	-- C Frame
	port map (
		resetI			=> i_reset,
		clock147456kI	=> clock147456kI,
		enableI			=> i_asyncA,		-- not i_hdlcA,
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
		frameErrorO		=> i_rxFrameErrorC
	);

	RxFcsFrame		: RxFcs		-- D frame
	generic map (
		cFcsType_ti		=> cFcsType_ti
	)
	port map (
		resetI			=> i_reset,
		clockI			=> clock168000kI,
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

	RxSynchFrame	: RxSynchronizer	-- E frame
	port map (
		resetI			=> i_reset,
		rxClockI		=> i_rxClock,
		clockI			=> clock168000kI,
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

	RxUnitFrame 	: RxChannel		-- F frame
	port map (
		resetI			=> i_reset,
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

	TerminalFrame	: Terminal		-- G frame
	port map (
		resetI			=> i_reset,
		clock16I		=> i_clockx16,
		--dpllEnableI		=> i_dpllEnableA,
		hdlcI			=> i_hdlcA,
		manchesterI		=> i_manchester_oddpA,
		nrziI			=> i_nrzi_evenpA,
		txI				=> i_txf,
		rxI				=> i_rxg,
		txClockO		=> i_txClock,
		rxClockO		=> i_rxClock,
		txO				=> i_txg,
		rxO				=> i_rxf
	);

end behavioral;
