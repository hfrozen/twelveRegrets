-------------------------------------------------------------------------------
-- Title	: hdlc rx channel
-- Project	: serial communication
-- File		: RxChannel.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

use work.Packs.all;

entity RxChannel is			-- A frame
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
		--frameSampM		: out	std_logic;		-- ?????
		--enableZeroM		: out	std_logic;		-- ?????
		--flagCounterM_ti	: out	integer;
		--flagDetectM		: out	std_logic;		-- ?????
		readyO			: out	std_logic;
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
end RxChannel;

architecture behavioral of RxChannel is
	signal i_rxDC			: std_logic;
	signal i_enableBC		: std_logic;
	signal i_availableCB	: std_logic;
	signal i_flagDetectDB	: std_logic;
	signal i_abortDetectDB	: std_logic;
	signal i_initZeroCB		: std_logic;
	signal i_rxEnableDB		: std_logic;
	signal i_validFrameBCA	: std_logic;

begin
	validFrameO		<= i_validFrameBCA;
	--enableZeroM		<= i_enableBC;		-- ?????
	--flagDetectM		<= i_flagDetectDB;	-- ?????

	ControlFrame	: RxController		-- B frame
	port map (
		resetI			=> resetI,
		rxClockI		=> rxClockI,
		rxEnableI		=> i_rxEnableDB,
		flagDetectI		=> i_flagDetectDB,
		availableI		=> i_availableCB,
		abortDetectI	=> i_abortDetectDB,
		initZeroO		=> i_initZeroCB,
		enableZeroO		=> i_enableBC,
		validFrameO		=> i_validFrameBCA,
		abortDetectO	=> abortDetectO,
		--frameStatusM	=> frameStatusM,	-- ?????
		--frameMonM		=> frameMonM,		-- ?????
		--frameSampM		=> frameSampM,		-- ?????
		--flagCounterM_ti	=> flagCounterM_ti,
		overRunErrorO	=> overRunErrorO,
		frameErrorO		=> frameErrorO
	);

	ZeroDetectFrame	: ZeroDetector		-- C frame
	port map (
		resetI			=> resetI,
		rxClockI		=> rxClockI,
		rxI				=> i_rxDC,
		validFrameI		=> i_validFrameBCA,
		enableI			=> i_enableBC,
		startFrameI		=> i_initZeroCB,
		readByteI		=> readByteI,
		availableO		=> i_availableCB,
		readyO			=> readyO,
		rxO_t8			=> rxO_t8
	);

	FlagDetectFrame	: FlagDetector		-- D frame
	port map (
		resetI			=> resetI,
		rxClockI		=> rxClockI,
		rxI				=> rxI,
		rxEnableI		=> rxEnableI,
		rxO				=> i_rxDC,
		rxEnableO		=> i_rxEnableDB,
		flagDetectO		=> i_flagDetectDB,
		abortDetectO	=> i_abortDetectDB
	);

end behavioral;
