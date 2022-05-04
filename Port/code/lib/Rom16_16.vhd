-------------------------------------------------------------------------------
-- Title	: bus test memory
-- Project	: serial communication
-- File		: Rom16_16.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

library work;
use work.Packs.all;
use work.TYPES.all;

entity Rom16_16 is
	port (
		resetI		: in	std_logic;
		clockI		: in	std_logic;
		csI			: in	std_logic;
		readI		: in	std_logic;
		addrI_t4	: in	std_logic_vector(3 downto 0);
		dataIO_t16	: inout	std_logic_vector(15 downto 0)
	);
end Rom16_16;

architecture behavioral of Rom16_16 is
	signal i_cs		: std_logic	:= '0';
	signal i_read	: std_logic	:= '0';
	constant wordMem	: WORD_ARRAY(0 to 15)  := ( "0000000000000001",
													"0000000000000010",
													"0000000000000100",
													"0000000000001000",
													"0000000000010000",
													"0000000000100000",
													"0000000001000000",
													"0000000010000000",
													"0000000100000000",
													"0000001000000000",
													"0000010000000000",
													"0000100000000000",
													"0001000000000000",
													"0010000000000000",
													"0100000000000000",
													"1000000000000000" );
	--signal i_reg_t16	: std_logic_vector(15 downto 0)	:= (others => '0');

begin
	i_read	<= csI and readI;

	--busDataIO_t16	<= i_reg_t16 when i_read = '1'	else (others => 'Z');

	process (resetI, clockI, i_cs, i_read, addrI_t4)
	begin
		if resetI = '0' and rising_edge(clockI) then
			if i_read = '1' then
				dataIO_t16	<= wordMem(conv_integer(addrI_t4));
			else
				dataIO_t16	<= (others => 'Z');
			end if;
		end if;
	end process;

end behavioral;