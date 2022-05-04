-------------------------------------------------------------------------------
-- Title	: hdlc tx synchronizer
-- Project	: serial communication
-- File		: TxSynch.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity TxSynchronizer is
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
end TxSynchronizer;

architecture behavioral of TxSynchronizer is
begin
	txO_t8	<= txI_t8;

	-- purpose	: ready signal
	-- type		: sequential
	-- inputs	: clockI, resetI
	-- outputs
	-- clockI -> readyI(tx controller) -> i_temp -> readyO(tx fcs)
	ReadySignal	: process(resetI, clockI)
		variable i_temp	: std_logic;
	begin
		if resetI = '1' then
			i_temp		:= '0';
			readyO		<= '0';
		elsif rising_edge(clockI) then
			readyO		<= i_temp;
			i_temp		:= readyI;
		end if;
	end process ReadySignal;

	-- purpose	: write bytes signal
	-- type		: sequential
	-- inputs	: txClockI, resetI
	-- outputs
	-- txClockI -> writeByteI(tx fcs) -> i_temp -> writeByteO(tx controller)
	WriteSignal	: process(resetI, txClockI)
		variable i_temp	: std_logic;
	begin
		if resetI = '1' then
			i_temp		:= '0';
			writeByteO	<= '0';
		elsif rising_edge(txClockI) then
			writeByteO	<= i_temp;
			i_temp		:= writeByteI;
		end if;
	end process WriteSignal;

	-- purpose	: abort frame
	-- type		: sequential
	-- inputs	: txClockI, resetI
	-- outputs
	-- txClockI -> abortFrameI(tx fcs) -> i_temp -> abortFrameO(tx controller)
	AbortFrameSignal	: process(resetI, txClockI)
		variable i_temp	: std_logic;
	begin
		if resetI = '1' then
			i_temp		:= '0';
			abortFrameO	<= '0';
		elsif rising_edge(txClockI) then
			abortFrameO	<= i_temp;
			i_temp		:= abortFrameI;
		end if;
	end process AbortFrameSignal;

	-- purpose	: valid frame
	-- type		: sequential
	-- inputs	: txClockI, resetI
	-- outputs
	-- txClockI -> validFrameI(tx fcs) -> i_temp -> validFrameO(tx controller)
	ValidFrameSignal	: process(resetI, txClockI)
		variable i_temp	: std_logic;
	begin
		if resetI = '1' then
			i_temp		:= '0';
			validFrameO	<= '0';
		elsif rising_edge(txClockI) then
			validFrameO	<= i_temp;
			i_temp		:= validFrameI;
		end if;
	end process ValidFrameSignal;

	-- purpose	: abort transmit
	-- type		: sequential
	-- inputs	: clockI, resetI
	-- outputs
	-- clockI -> haltI(tx controller) -> i_temp -> haltO(tx fcs)
	AbortTransmitSignal	: process(resetI, clockI)
		variable i_temp	: std_logic;
	begin
		if resetI = '1' then
			i_temp		:= '0';
			haltO		<= '0';
		elsif rising_edge(clockI) then
			haltO		<= i_temp;
			i_temp		:= haltI;
		end if;
	end process AbortTransmitSignal;
end behavioral;
