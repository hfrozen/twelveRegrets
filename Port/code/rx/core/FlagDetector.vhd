-------------------------------------------------------------------------------
-- Title	: hdlc flag detection
-- Project	: serial communication
-- File		: FlagDectector.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity	FlagDetector is
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
end FlagDetector;

architecture behavioral of FlagDetector is
	signal	i_shift_t8	: std_logic_vector(7 downto 0);

begin
	-- purpose	: flag detection
	-- type		: sequential
	-- inputs	: resetI, rxClockI
	-- outputs	:
	process(resetI, rxClockI)
		variable i_detect	: std_logic;
		variable i_block_t8	: std_logic_vector(7 downto 0);
		variable i_reg_t8	: std_logic_vector(7 downto 0);

	begin
		if resetI = '1' then
			flagDetectO		<= '0';
			abortDetectO	<= '0';
			rxO				<= '0';
			i_detect		:= '0';
			i_shift_t8		<= (others => '1');
			i_block_t8		:= (others => '0');
			rxEnableO		<= '0';
			i_reg_t8		:= (others => '1');
		elsif rising_edge(rxClockI) then	-- rising edge
			i_detect		:= not i_shift_t8(0) and i_shift_t8(1) and i_shift_t8(2) and i_shift_t8(3) and
									i_shift_t8(4) and i_shift_t8(5) and i_shift_t8(6) and not i_shift_t8(7);
			flagDetectO		<= i_detect;

			abortDetectO	<= not i_shift_t8(0) and i_shift_t8(1) and i_shift_t8(2) and i_shift_t8(3) and
									i_shift_t8(4) and i_shift_t8(5) and i_shift_t8(6) and i_shift_t8(7) and
									not (i_block_t8(0) or i_block_t8(1) or i_block_t8(2) or i_block_t8(3) or
										i_block_t8(4) or i_block_t8(5) or i_block_t8(6) or i_block_t8(7));

			i_shift_t8		<= rxi & i_shift_t8(7 downto 1);
			rxO				<= i_shift_t8(0);		-- eighth RXI

			i_block_t8		:= i_detect & i_block_t8(7 downto 1);

			rxEnableO		<= i_reg_t8(0);
			i_reg_t8		:= rxEnableI & i_reg_t8(7 downto 1);
		end if;	-- rxClockI
	end process;
end behavioral;
