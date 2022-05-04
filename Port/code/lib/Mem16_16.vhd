-------------------------------------------------------------------------------
-- Title	: bus test memory
-- Project	: serial communication
-- File		: Mem16_16.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

library work;
use work.Packs.all;
use work.TYPES.all;
use work.TypicPack.all;

entity Mem16_16 is
	port (
		resetI		: in	std_logic;
		clockI		: in	std_logic;
		csI			: in	std_logic;
		readI		: in	std_logic;
		writeI		: in	std_logic;
		addrI_t4	: in	std_logic_vector(3 downto 0);
		dataIO_t16	: inout	std_logic_vector(15 downto 0)
	);
end Mem16_16;

architecture behavioral of Mem16_16 is
	signal i_read		: std_logic	:= '0';
	signal i_write		: std_logic	:= '0';
	signal i_regO_t16	: std_logic_vector(15 downto 0)	:= (others => '0');

begin
	DpMemFrame	: DpMem16i16o
	port map (
		clka	=> clockI,
		ena		=> i_write,
		wea		=> "1",
		addra	=> addrI_t4,
		dina	=> dataIO_t16,
		clkb	=> clockI,
		enb		=> i_read,
		addrb	=> addrI_t4,
		doutb	=> i_regO_t16
	);

	i_read	<= csI and readI;
	i_write	<= csI and writeI;
	dataIO_t16	<= i_regO_t16 when i_read = '1'	else (others => 'Z');

end behavioral;