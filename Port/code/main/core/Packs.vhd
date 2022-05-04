-------------------------------------------------------------------------------
-- Title	: components package
-- Project	: serial communication
-- File		: Packs.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

package TYPES is
	type LOGIC_ARRAY is		array (natural range <>) of	std_logic;
	type WORD_ARRAY is		array (integer range <>) of std_logic_vector(15 downto 0);
	type BYTE_ARRAY is		array (integer range <>) of std_logic_vector(7 downto 0);
	type ADDRESS_ARRAY is	array (integer range <>) of std_logic_vector(9 downto 0);
end TYPES;

library ieee;
use ieee.std_logic_1164.all;

package Packs is
	--component Rom16_16
	--port (
	--	resetI		: in	std_logic;
	--	clockI		: in	std_logic;
	--	csI			: in	std_logic;
	--	readI		: in	std_logic;
	--	addrI_t4	: in	std_logic_vector(3 downto 0);
	--	dataIO_t16	: inout	std_logic_vector(15 downto 0)
	--);
	--end component;

	--component Mem16_16
	--port (
	--	resetI		: in	std_logic;
	--	clockI		: in	std_logic;
	--	csI			: in	std_logic;
	--	readI		: in	std_logic;
	--	writeI		: in	std_logic;
	--	addrI_t4	: in	std_logic_vector(3 downto 0);
	--	dataIO_t16	: inout	std_logic_vector(15 downto 0)
	--);
	--end component;

	component Platform
	generic (
		cFcsType_ti		: integer	:= 2;
		cAddrWidth_ti	: integer	:= 10;
		cMaxAddress_ti	: integer	:= 512
	);
	port (
		clock168000kI	: in	std_logic;
		clock147456kI	: in	std_logic;
		rxI				: in	std_logic;
		txO				: out	std_logic;
		rtsO			: out	std_logic;
		--clock16M		: out	std_logic;	-- ?????
		--clockM			: out	std_logic;

		--taddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);	-- ?????
		--tbM_t8			: out	std_logic_vector(7 downto 0);				-- ?????
		--treadBuffM		: out	std_logic;
		--tavailM			: out	std_logic;
		--enableClockRM	: out	std_logic;
		rAddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);	-- ?????
		rbM_t8			: out	std_logic_vector(7 downto 0);				-- ?????
		rWriteBuffM		: out	std_logic;
		rReadBuffM		: out	std_logic;

		--txEnableM		: out	std_logic;	-- ?????
		--txAvailM		: out	std_logic;	--?????
		--rxFullM			: out	std_logic;	--?????
		--rxEotM			: out	std_logic;	--?????

		resetI			: in	std_logic;
		busCsI			: in	std_logic;
		busReadI		: in	std_logic;
		busWriteI		: in	std_logic;
		busAddrI_t8		: in	std_logic_vector(7 downto 0);
		busDataIO_t16	: inout	std_logic_vector(15 downto 0);
		busInterruptO	: out	std_logic
	);
	end component;

	component MemMap is
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
		--busAddrDataIO_t16	: inout	std_logic_vector(15 downto 0);
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
	end component;

	component TxBuffer
	generic (
		cAddrWidth_ti	: integer;
		cMaxAddress_ti	: integer
	);
	port (
		resetI			: in	std_logic;
		clockI			: in	std_logic;
		txEnableI		: in	std_logic;						-- bus
		readBuffI		: in	std_logic;						-- tx fcs
		writeBuffI		: in	std_logic;						-- bus
		frameSizeI_tv	: in	std_logic_vector(cAddrWidth_ti downto 0);
		txI_t16			: in	std_logic_vector(15 downto 0);	-- bus
		--taddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);	-- ?????
		--tbM_t8			: out	std_logic_vector(7 downto 0);				-- ?????
		--enableClockRM	: out	std_logic;
		txAvailO		: out	std_logic;						-- tx fcs
		txEmptyO		: out	std_logic;						-- Bus
		--stateM_t2		: out	std_logic_vector(1 downto 0);
		txO_t8			: out	std_logic_vector(7 downto 0)	-- tx fcs
	);
	end component;

	component TxAsync
	port (
		resetI			: in	std_logic;
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
		resetI			: in	std_logic;
		clockI			: in	std_logic;
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
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		clockI			: in	std_logic;

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
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		txI_t8			: in	std_logic_vector(7 downto 0);
		txEnableI		: in	std_logic;
		writeByteI		: in	std_logic;
		validFrameI		: in	std_logic;
		abortFrameI		: in	std_logic;
		flagSizeI_t3	: in	std_logic_vector(2 downto 0);
		--flagOpenM		: out	std_logic;	-- ?????
		--txEnableM		: out	std_logic;	-- ?????
		haltO			: out	std_logic;
		readyO			: out	std_logic;
		frameDefenceO	: out	std_logic;
		txO				: out	std_logic
	);
	end component;

	component TxController
	port (
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		txEnableI		: in	std_logic;		-- from TxChannel(txEnableI)
		shiftingByteI	: in	std_logic;		-- from ZeroInsert(shifting8bitO)
		validFrameI		: in	std_logic;		-- from TxChannel(validFrameI)
		abortFrameI		: in	std_logic;		-- from TxChannel(abortFrameI)
		haltI			: in	std_logic;		-- from ZeroInsert(abortTransmitO)
		flagSizeI_t3	: in	std_logic_vector(2 downto 0);
		latchEnableO	: out	std_logic;		-- to ZeroInsert(enableOfBackendI)
		flagOpenO		: out	std_logic;		-- to FlagInsert
		abortSeqO		: out	std_logic;		-- to FlagInsert(abortSeqI)
		frameSeqO		: out	std_logic		-- to FlagInsert(flagSeqI)
	);
	end component;

	component ZeroInsert
	port (
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		txI_t8			: in	std_logic_vector(7 downto 0);	-- from TxChannel(txI_t8)
		txEnableI		: in	std_logic;		-- from TxChannel(enableI)
		latchEnableI	: in	std_logic;		-- from TxController(enableOfBackendO)
		writeByteI		: in	std_logic;		-- from TxChannel(writeByteI)
		haltO			: out	std_logic;		-- to TxController(abortedTransI)
		shiftingByteO	: out	std_logic;		-- to TxController(shifting8bitI)
		readyO			: out	std_logic;		-- to TxChannel(readyO)
		txO				: out	std_logic		-- to FlagInsert(txI)
	);
	end component;

	component FlagInsert
	port (
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		txEnableI		: in	std_logic;		-- from TxChannel(txEnableI)
		flagOpenI		: in	std_logic;
		abortSeqI		: in	std_logic;		-- from TxController(abortTransmitO)
		frameSeqI		: in	std_logic;		-- from TxController(validTransmitO)
		txI				: in	std_logic;		-- from TxO of ZeroInsert
		--txEnableM		: out	std_logic;		-- ?????
		frameDefenceO	: out	std_logic;
		txO				: out	std_logic		-- to TxChannel(txO)
	);
	end component;

	component RxBuffer
	generic (
		cFcsType_ti		: integer;
		cAddrWidth_ti	: integer;
		cMaxAddress_ti	: integer
	);
	port (
		resetI			: in	std_logic;
		clockI			: in	std_logic;
		--ignoreI			: in	std_logic;	-- force reset to rx buffer
		writeBuffI		: in	std_logic;
		readBuffI		: in	std_logic;
		eotI			: in	std_logic;
		rxI_t8			: in	std_logic_vector(7 downto 0);
		fullO			: out	std_logic;
		overflowErrorO	: out	std_logic;
		--overlapErrorO	: out	std_logic;
		frameSizeO_tv	: out	std_logic_vector(cAddrWidth_ti downto 0);
		rAddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		rxO_t16			: out	std_logic_vector(15 downto 0)
	);
	end component;

	component RxAsync
	port (
		resetI			: in	std_logic;
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
		frameErrorO		: out	std_logic
	);
	end component;

	component RxFcs
	generic (
		cFcsType_ti	 	: integer
	);
	port (
		resetI			: in	std_logic;
		clockI			: in	std_logic;
		enableI			: in	std_logic;
		fcsEnableI		: in	std_logic;
		readyI			: in	std_logic;
		validFrameI		: in	std_logic;
		rxI_t8			: in	std_logic_vector(7 downto 0);
		readByteO		: out	std_logic;
		fcsErrorO		: out	std_logic;
		eotO			: out	std_logic;
		writeBuffO		: out	std_logic;
		--fcsM_t16		: out	std_logic_vector(15 downto 0);
		headO_t16		: out	std_logic_vector(15 downto 0);
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
	end component;

	component RxSynchronizer
	port (
		resetI			: in	std_logic;
		rxClockI		: in	std_logic;
		clockI			: in	std_logic;

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
		resetI			: in	std_logic;
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

	component RxController
	port (
		resetI			: in	std_logic;
		rxClockI		: in	std_logic;
		rxEnableI		: in	std_logic;		-- from FlagDetector
		flagDetectI		: in	std_logic;		-- from FlagDetector
		availableI		: in	std_logic;		-- from ZeroDetector(assem. byte)
		abortDetectI	: in	std_logic;		-- from FlagDetector
		initZeroO		: out	std_logic;		-- init zero detect block, to ZeroDetector
		enableZeroO		: out	std_logic;		-- to ZeroDetector
		validFrameO		: out	std_logic;		-- to ZeroDetector
		abortDetectO	: out	std_logic;		-- to controller's abortSignalO
		--frameStatusM	: out	std_logic;		-- ?????
		--frameMonM		: out	std_logic;		-- ?????
		--frameSampM		: out	std_logic;		-- ?????
		--flagCounterM_ti	: out	integer;		-- ?????
		overRunErrorO	: out	std_logic;
		frameErrorO		: out	std_logic		-- error in next byte at the bus
	);
	end component;

	component ZeroDetector
	port (
		resetI			: in	std_logic;
		rxClockI		: in	std_logic;
		rxI				: in	std_logic;		-- from FlagDetector
		validFrameI		: in	std_logic;		-- from RxController
		enableI			: in	std_logic;		-- from RxController
		startFrameI		: in	std_logic;		-- from RxController
		readByteI		: in	std_logic;		-- from RxChannel
		availableO		: out	std_logic;		-- to RxController
		readyO			: out	std_logic;		-- to RxController
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
	end component;

	component FlagDetector
	port (
		resetI			: in	std_logic;
		rxClockI		: in	std_logic;
		rxI				: in	std_logic;		-- rx input
		rxEnableI		: in	std_logic;		-- from bus
		rxO				: out	std_logic;		-- to ZeroDetector
		rxEnableO		: out	std_logic;		-- to RxController
		flagDetectO		: out	std_logic;		-- to RxController
		abortDetectO	: out	std_logic		-- to RxController
	);
	end component;

	component Terminal
	port (
		resetI		: in	std_logic;
		clock16I	: in	std_logic;
		hdlcI		: in	std_logic;
		--dpllEnableI	: in	std_logic;
		manchesterI	: in	std_logic;
		nrziI		: in	std_logic;
		txI			: in	std_logic;
		rxI			: in	std_logic;
		txClockO	: out	std_logic;
		--rxaM		: out	std_logic;	-- ?????
		--rxbM		: out	std_logic;	-- ?????
		--rxSyncM		: out	std_logic;	-- ?????
		rxClockO	: out	std_logic;
		txO			: out	std_logic;
		rxO			: out	std_logic
	);
	end component;
end package;
