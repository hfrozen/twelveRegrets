-------------------------------------------------------------------------------
-- Title	: hdlc terminal
-- Project	: serial communication
-- File		: Terminal.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

use work.NodePack.all;

entity Terminal is
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
		--rxSyncM_t4	: out	std_logic_vector(3 downto 0);	-- ?????
		--rxSyncM		: out	std_logic;	-- ?????
		rxClockO	: out	std_logic;
		txO			: out	std_logic;
		rxO			: out	std_logic
	);
end Terminal;

architecture behavioral of Terminal is
	signal i_rxNrz			: std_logic	:= '1';
	signal i_rxNrzi			: std_logic	:= '1';
	signal i_rxManche		: std_logic	:= '1';

	signal i_txNrz			: std_logic	:= '1';
	signal i_txNrzi			: std_logic	:= '1';
	signal i_txManche		: std_logic	:= '1';

	signal i_txClock		: std_logic	:= '0';
	signal i_rxClock		: std_logic	:= '0';
	--signal i_rxClockManche	: std_logic	:= '0';

	signal i_rx				: std_logic	:= '1';

begin
	i_rx		<= rxI;
	txO			<= resetI or (hdlcI	and	not manchesterI and	not nrziI and	i_txNrz) or
							(hdlcI	and	not manchesterI and		nrziI and	i_txNrzi) or
							(hdlcI and		manchesterI and	not nrziI and	i_txManche);
	rxO			<= resetI or (hdlcI	and	not manchesterI and	not nrziI and	i_rxNrz) or
							(hdlcI	and	not manchesterI and		nrziI and	i_rxNrzi) or
							(hdlcI and		manchesterI and	not nrziI and	i_rxManche);
	txClockO	<= i_txClock;
	rxClockO	<= i_rxClock;
	--rxClockO	<= (i_rxClockManche and manchesterI) or (i_rxClock and not manchesterI);

	NrzFrame	: Nrz
	port map (
		resetI		=> resetI,
		txClockI	=> i_txClock,
		rxClockI	=> i_rxClock,
		txI			=> txI,
		rxI			=> i_rx,
		txO			=> i_txNrz,
		rxO			=> i_rxNrz
	);

	NrziFrame	: Nrzi
	port map (
		resetI		=> resetI,
		txClockI	=> i_txClock,
		rxClockI	=> i_rxClock,
		txI			=> txI,
		rxI			=> i_rx,
		txO			=> i_txNrzi,
		rxO			=> i_rxNrzi
	);

	ManchesterFrame	: Manchester
	port map (
		resetI		=> resetI,
		txClockI	=> i_txClock,
		rxClockI	=> i_rxClock,
		txI			=> txI,
		rxI			=> i_rx,
		txO			=> i_txManche,
		--rxClockO	=> i_rxClockManche,
		rxO			=> i_rxManche
	);

	--RxPhaseSynchFrame	: RxPhaseSynch
	--port map (
	--	resetI		=> resetI,
	--	clock16I	=> clock16I,	--i_clockRef,
	--	enableI		=> dpllEnableI,
	--	hdlcI		=> hdlcI,
	--	mancheI		=> manchesterI,
	--	nrziI		=> nrziI,
	--	txClockI	=> i_txClock,
	--	rxI			=> rxI,
	--	--syncClockM	=> rxSyncM,	-- ?????
	--	--rxaM		=> rxaM,	-- ?????
	--	--rxbM		=> rxbM,	-- ?????
	--	--clockSyncM_t4	=> rxSyncM_t4,	-- ?????
	--	syncClockO	=> i_rxClock
	--);

	-- tx clock = x / 16
	process(resetI, clock16I)
		variable i_count_t4	: std_logic_vector(3 downto 0)	:= (others => '0');
	begin
		if resetI = '1' then
			i_count_t4	:= (others => '0');
			i_txClock	<= '0';
			i_rxClock	<= '0';
		elsif rising_edge(clock16I) then
			--i_clock8	<= not i_clock8;
			i_count_t4	:= i_count_t4 + "0001";
			i_txClock	<= not i_count_t4(3);
			i_rxClock	<= not i_count_t4(3);
		end if;
	end process;
end behavioral;
