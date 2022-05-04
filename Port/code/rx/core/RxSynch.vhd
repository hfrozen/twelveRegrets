-------------------------------------------------------------------------------
-- Title	: hdlc rx synchronizer
-- Project	: serial communication
-- File		: RxSynch.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity RxSynchronizer is	-- D1 Domain 1 = Serial line	-- D2 Domain 2 = System interface
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
end RxSynchronizer;

architecture behavioral of RxSynchronizer is
begin
	rxO_t8	<= rxI_t8;

	-- purpose	: ready signal
	-- type		: sequential
	-- inputs	: resetI, clockI
	-- outputs
	-- clockI -> readyI -> i_temp -> readyO
	ReadySignal	 : process(resetI, clockI)
		variable i_temp	: std_logic;

	begin
		if resetI = '1' then
			i_temp	:= '0';
			readyO	<= '0';
		elsif rising_edge(clockI) then
			readyO	<= i_temp;
			i_temp	:= readyI;
		end if;

	end process ReadySignal;

	-- purpose	: read bytes signal
	-- type		: sequential
	-- inputs	: resetI, rxClockI
	-- outputs
	-- rxClockI -> readByteI -> i_temp -> readByteO
	ReadSignal		: process(resetI, rxClockI)
		variable i_temp	: std_logic;

	begin
		if resetI = '1' then
			i_temp		:= '0';
			readByteO	<= '0';
		elsif rising_edge(rxClockI) then
			readByteO	<= i_temp;
			i_temp		:= readByteI;
		end if;
	end process ReadSignal;

	-- purpose	: valid frame signal
	-- type		: sequential
	-- inputs	: resetI, clockI
	-- outputs	:
	-- clockI -> validFrameI -> i_temp -> validFrameO
	ValidFrameSignal	: process(resetI, clockI)
		variable i_temp	: std_logic;

	begin
		if resetI = '1' then
			i_temp		:= '0';
			validFrameO	<= '0';
		elsif rising_edge(clockI) then
			validFrameO	<= i_temp;
			i_temp		:= validFrameI;
		end if;
	end process ValidFrameSignal;

	-- purpose	: abort signal
	-- type		: sequential
	-- inputs	: resetI, clockI
	-- outputs
	-- clockI -> abortDetectI -> i_temp -> abortDetectO
	AbortDetect		: process(resetI, clockI)
		variable i_temp	: std_logic;

	begin
		if resetI = '1' then
			i_temp			:= '0';
			abortDetectO	<= '0';
		elsif rising_edge(clockI) then
			abortDetectO	<= i_temp;
			i_temp			:= abortDetectI;
		end if;
	end process AbortDetect;

	-- purpose	: overrun error signal
	-- type		: sequential
	-- inputs	: resetI, clockI
	-- outputs
	-- clockI -> overRunErrorI -> i_temp -> overRunErrorO
	OverRunErrorSignal	: process(resetI, clockI)
		variable i_temp	: std_logic;

	begin
		if resetI = '1' then
			i_temp			:= '0';
			overRunErrorO	<= '0';
		elsif rising_edge(clockI) then
			overRunErrorO	<= i_temp;
			i_temp			:= overRunErrorI;
		end if;
	end process OverRunErrorSignal;

	-- purpose	: frame error signal
	-- type		: sequential
	-- inputs	: resetI, clockI
	-- outputs
	-- clockI -> frameErrorI -> i_temp -> frameErrorO
	FrameErrorSignal	: process(resetI, clockI)
		variable i_temp	: std_logic;

	begin
		if resetI = '1' then
			i_temp		:= '0';
			frameErrorO	<= '0';
		elsif rising_edge(clockI) then
			frameErrorO	<= i_temp;
			i_temp		:= frameErrorI;
		end if;
	end process FrameErrorSignal;
end behavioral;
