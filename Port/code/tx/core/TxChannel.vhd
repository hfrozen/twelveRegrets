-------------------------------------------------------------------------------
-- Title	: hdlc tx channel
-- Project	: serial communication
-- File		: TxChannel.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

use work.Packs.all;

entity TxChannel is			-- A frame
	port (
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		txI_t8			: in	std_logic_vector(7 downto 0);
		txEnableI		: in	std_logic;
		writeByteI		: in	std_logic;
		validFrameI		: in	std_logic;
		abortFrameI		: in	std_logic;
		flagSizeI_t3	: in	std_logic_vector(2 downto 0);
		--flagOpenM		: out	std_logic;		-- ?????
		--flagM				: out	std_logic;		-- ?????
		--abortM				: out	std_logic;		-- ?????
		--latchEnableM	: out	std_logic;		-- ?????
		--zeroStateM			: out	std_logic;		-- ?????
		--zeroRegM			: out	std_logic_vector(7 downto 0);	-- ?????
		--zeroDetectM			: out	std_logic;		-- ?????
		--frameSeqM		: out	std_logic;		-- ?????
		--shiftByteM		: out	std_logic;		-- ?????
		--flagStateM_t3		: out	std_logic_vector(2 downto 0);	-- ????
		--flagCounterM_t4		: out	std_logic_vector(3 downto 0);	-- ?????
		--flagShiftM_t4		: out	std_logic_vector(3 downto 0);		-- ?????
		--txEnableM		: out	std_logic;		-- ?????
		haltO			: out	std_logic;
		readyO			: out	std_logic;
		frameDefenceO	: out	std_logic;
		txO				: out	std_logic
	);
end TxChannel;

architecture behavioral of TxChannel is
	signal i_txCD				: std_logic;
	--signal i_enableB			: std_logic;
	signal i_haltCBA			: std_logic;
	signal i_abortSeqBD			: std_logic;
	signal i_frameSeqBD			: std_logic;
	signal i_shiftingByteCB		: std_logic;
	signal i_latchEnableBC		: std_logic;
	signal i_flagOpenBD			: std_logic;
	-- signal _flaging			: std_logic;

begin
	haltO		<= i_haltCBA;
	--frameSeqM			<= i_frameSeqBD;
	--shiftByteM			<= i_shiftingByteCB;
	--latchEnableM		<= i_latchEnableBC;
	--flagOpenM			<= i_flagOpenBD;

	ControlFrame	: TxController		-- B frame
	port map (
		resetI				=> resetI,
		txClockI			=> txClockI,
		txEnableI			=> txEnableI,
		shiftingByteI		=> i_shiftingByteCB,
		validFrameI			=> validFrameI,
		abortFrameI			=> abortFrameI,
		haltI				=> i_haltCBA,
		flagSizeI_t3		=> flagSizeI_t3,
		--flagStateM_t3		=> flagStateM_t3,	-- ????
		--flagCounterM_t4		=> flagCounterM_t4,	-- ?????
		-- enableO				=> i_enableB,
		latchEnableO		=> i_latchEnableBC,
		flagOpenO			=> i_flagOpenBD,
		abortSeqO			=> i_abortSeqBD,
		frameSeqO			=> i_frameSeqBD
	);

	ZeroInsertFrame	: ZeroInsert		-- C frame
	port map (
		resetI				=> resetI,
		txClockI			=> txClockI,
		txI_t8				=> txI_t8,
		txEnableI			=> txEnableI,
		latchEnableI		=> i_latchEnableBC,
		writeByteI			=> writeByteI,
		haltO				=> i_haltCBA,
		shiftingByteO		=> i_shiftingByteCB,
		readyO				=> readyO,
		--detectM				=> ZeroDetectM,		-- ?????
		--zeroStateM			=> zeroStateM,		-- ?????
		--zeroRegM			=> zeroRegM,	-- ?????
		txO					=> i_txCD
	);

	FlagInsertFrame	: FlagInsert		-- D frame
	port map (
		resetI				=> resetI,
		txClockI			=> txClockI,
		txI					=> i_txCD,
		txEnableI			=> txEnableI,
		flagOpenI			=> i_flagOpenBD,
		abortSeqI			=> i_abortSeqBD,
		frameSeqI			=> i_frameSeqBD,
		--flagM				=> flagM,		-- ?????
		--abortM				=> abortM,		-- ?????
		--flagShiftM_t4		=> flagShiftM_t4,		-- ?????
		--txEnableM			=> txEnableM,		-- ?????
		frameDefenceO		=> frameDefenceO,
		txO					=> txO
	);

end behavioral;
