library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity memTb is
end memTb;

architecture behavioral of memTb is
	component Harbor
	port (
		clock18432kI	: in	std_logic;
		clock147456M	: out	std_logic;	-- ?????
		clockM			: out	std_logic;
		cs4M			: out	std_logic;

		nResetI			: in	std_logic;
		nCsI			: in	std_logic;
		nReadI			: in	std_logic;
		nWriteI			: in	std_logic;
		addrI_t11		: in	std_logic_vector(10 downto 0);
		dataIO_t16		: inout	std_logic_vector(15 downto 0)
	);
	end component;

	signal i_clock		: std_logic	:= '0';
	signal i_reset		: std_logic	:= '0';
	signal i_cs			: std_logic	:= '0';
	signal i_read		: std_logic	:= '0';
	signal i_write		: std_logic	:= '0';
	signal i_addr_t11	: std_logic_vector(10 downto 0)	:= (others => '0');
	signal i_data_t16	: std_logic_vector(15 downto 0)	:= (others => 'Z');

	constant clock18432_period	: time	:= 54.253 ns;
	constant bus_period	: time	:= 30 ns;

begin
	i_reset	<= '1' after 100 ns;
	i_clock	<= not i_clock after clock18432_period / 2;

	process
	begin
		i_cs		<= '1';
		i_read		<= '1';
		i_write		<= '1';
		wait for 200 ns;
		i_addr_t11	<= "10000000000";
		i_data_t16	<= x"375a";
		i_cs		<= '0';
		i_write		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_write		<= '1';
		i_data_t16	<= (others => 'Z');
		wait for bus_period;

		i_addr_t11	<= i_addr_t11 + "00000000010";
		i_data_t16	<= x"37a5";
		i_cs		<= '0';
		i_write		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_write		<= '1';
		wait for bus_period;

		i_addr_t11	<= i_addr_t11 + "00000000010";
		i_data_t16	<= x"735a";
		i_cs		<= '0';
		i_write		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_write		<= '1';
		wait for bus_period;

		i_addr_t11	<= i_addr_t11 + "00000000010";
		i_data_t16	<= x"73a5";
		i_cs		<= '0';
		i_write		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_write		<= '1';
		wait for bus_period;

		i_addr_t11	<= "10000000000";
		i_data_t16	<= (others => 'Z');
		i_cs		<= '0';
		i_read		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_read		<= '1';
		wait for bus_period;

		i_addr_t11	<= i_addr_t11 + "00000000010";
		i_cs		<= '0';
		i_read		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_read		<= '1';
		wait for bus_period;

		i_addr_t11	<= i_addr_t11 + "00000000010";
		i_cs		<= '0';
		i_read		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_read		<= '1';
		wait for bus_period;

		i_addr_t11	<= i_addr_t11 + "00000000010";
		i_cs		<= '0';
		i_read		<= '0';
		wait for bus_period;
		i_cs		<= '1';
		i_read		<= '1';
		wait for bus_period;

	end process;

	uut: Harbor
	port map (
		clock18432kI	=> i_clock,
		nResetI			=> i_reset,
		nCsI			=> i_cs,
		nReadI			=> i_read,
		nWriteI			=> i_write,
		addrI_t11		=> i_addr_t11,
		dataIO_t16		=> i_data_t16
	);

end;