-------------------------------------------------------------------------------
-- Title	: Typical Package
-- Project	: serial communication
-- File		: ToolsPack.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity DpMemWibo is
	generic (
		cAddrWidth_ti	: integer	:= 9
	);
	port (
		clockI		: in	std_logic;
		resetI		: in	std_logic;
		readI		: in	std_logic;
		writeI		: in	std_logic;
		addrI_tv	: in	std_logic_vector(cAddrWidth_ti - 1 downto 0);
		dI_t16		: in	std_logic_vector(15 downto 0);
		dO_t8		: out	std_logic_vector(7 downto 0)
	);
end DpMemWibo;

architecture behavioral of DpMemWibo is
	type D8ARRAY is array (integer range <>) of std_logic_vector(7 downto 0);
	signal i_datas	: D8ARRAY(0 to (2 ** cAddrWidth_ti - 1));

	procedure InitMem(signal mems : inout D8ARRAY) is
	begin
		for i in 0 to (2 ** cAddrWidth_ti - 1) loop
			mems(i)	<= (others => '1');
		end loop;
	end InitMem;

begin
	process (resetI, clockI, readI, writeI, i_datas)
	begin
		if resetI = '1' then
			dO_t8	<= (others => '1');
			InitMem(i_datas);
		elsif rising_edge(clockI) then
			if writeI = '1' then
				--i_datas(conv_integer(addrI_tv(cAddrWidth_ti - 1 downto 1) & "0"))	<= dI_t16(7 downto 0);
				--i_datas(conv_integer(addrI_tv(cAddrWidth_ti - 1 downto 1) & "1"))	<= di_t16(15 downto 8);
				i_datas(conv_integer(addrI_tv))	<= dI_t16(7 downto 0);
				i_datas(conv_integer(addrI_tv) + 1)	<= di_t16(15 downto 8);
				dO_t8	<= (others => '1');
			elsif readI = '0' then
				dO_t8	<= i_datas(conv_integer(addrI_tv));
			else
				dO_t8	<= (others => '1');
			end if;
		end if;
	end process;
end behavioral;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity DpMemBiwo is
	generic (
		cAddrWidth_ti	: integer	:= 9
	);
	port (
		clockI		: in	std_logic;
		resetI		: in	std_logic;
		readI		: in	std_logic;
		writeI		: in	std_logic;
		addrI_tv	: in	std_logic_vector(cAddrWidth_ti - 1 downto 0);
		dI_t8		: in	std_logic_vector(7 downto 0);
		dO_t16		: out	std_logic_vector(15 downto 0)
	);
end DpMemBiwo;

architecture behavioral of DpMemBiwo is
	type D8ARRAY is array (integer range <>) of std_logic_vector(7 downto 0);
	signal i_datas	: D8ARRAY(0 to (2 ** (cAddrWidth_ti - 1)));

	procedure InitMem(signal mems : inout D8ARRAY) is
	begin
		for i in 0 to (2 ** (cAddrWidth_ti - 1)) loop
			mems(i)		<= (others => '1');
		end loop;
	end InitMem;

begin
	process (resetI, clockI, readI, writeI)
	begin
		if resetI = '1' then
			dO_t16	<= (others => '1');
			InitMem(i_datas);
		elsif rising_edge(clockI) then
			if writeI = '1' then
				i_datas(conv_integer(addrI_tv)) <= dI_t8;
			elsif readI = '1' then
				--dO_t16(7 downto 0)	<= i_datas(conv_integer(addrI_tv(cAddrWidth_ti - 1 downto 1) & "0"));
				--dO_t16(15 downto 8)	<= i_datas(conv_integer(addrI_tv(cAddrWidth_ti - 1 downto 1) & "1"));
				dO_t16(7 downto 0)	<= i_datas(conv_integer(addrI_tv));
				dO_t16(15 downto 8)	<= i_datas(conv_integer(addrI_tv) + 1);
			else
				dO_t16	<= (others => '1');
			end if;
		end if;
	end process;
end behavioral;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity Dff is
	port (
		dI, clockI, clearI, resetI	: in	std_logic;
		qO		: out	std_logic
	);
end Dff;

architecture behavioral of Dff is
begin
	process (dI, clockI, clearI, resetI)
	begin
		if clearI = '1' or resetI = '1' then
			qO	<= '0';
		elsif rising_edge(clockI) then
			qO	<= dI;
		end if;
	end process;
end behavioral;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

entity Count4D is
	port (
		dI		: in	std_logic;
		clockI	: in	std_logic;
		resetI	: in	std_logic;
		dO_t4	: out	std_logic_vector(3 downto 0)
	);
end Count4D;

architecture behavioral of Count4D is
	signal i_count_t4	: std_logic_vector(3 downto 0)	:= "0000";
begin
	dO_t4	<= i_count_t4;

	process (dI, clockI, resetI)
	begin
		if resetI = '1' then
			i_count_t4	<= (others => '0');
		elsif rising_edge(clockI) then
			if dI = '1' then
				i_count_t4	<= i_count_t4 + "0001";
			else
				i_count_t4	<= (others => '0');
			end if;
		end if;
	end process;
end behavioral;

library ieee;
use ieee.std_logic_1164.all;

package TypicPack is
	component DpMemWibo
	generic (
		cAddrWidth_ti	: integer
	);
	port (
		clockI		: in	std_logic;
		resetI		: in	std_logic;
		readI		: in	std_logic;
		writeI		: in	std_logic;
		addrI_tv	: in	std_logic_vector(cAddrWidth_ti - 1 downto 0);
		dI_t16		: in	std_logic_vector(15 downto 0);
		dO_t8		: out	std_logic_vector(7 downto 0)
	);
	end component;

	component DpMemBiwo
	generic (
		cAddrWidth_ti	: integer
	);
	port (
		clockI		: in	std_logic;
		resetI		: in	std_logic;
		readI		: in	std_logic;
		writeI		: in	std_logic;
		addrI_tv	: in	std_logic_vector(cAddrWidth_ti - 1 downto 0);
		dI_t8		: in	std_logic_vector(7 downto 0);
		dO_t16		: out	std_logic_vector(15 downto 0)
	);
	end component;

	component Dff
	port (
		di, clockI, clearI, resetI	: in	std_logic;
		qO		: out	std_logic
	);
	end component;

	component Count4D
	port (
		dI		: in	std_logic;
		clockI	: in	std_logic;
		resetI	: in	std_logic;
		dO_t4	: out	std_logic_vector(3 downto 0)
	);
	end component;

	component ClockGenerator
	port (
		U1_CLKIN_IN			: in    std_logic;
		U1_RST_IN			: in    std_logic;
		U1_U2_SELECT_IN		: in    std_logic;
		U2_CLKIN_IN			: in    std_logic;
		U2_RST_IN			: in    std_logic;
		CLKFX_OUT			: out   std_logic
	);
	end component;

	component DpMem16i8o
	port (
		clka		: in	std_logic;
		ena			: in	std_logic;
		wea			: in	std_logic_vector(0 downto 0);
		addra		: in	std_logic_vector(7 downto 0);
		dina		: in	std_logic_vector(15 downto 0);
		clkb		: in	std_logic;
		enb			: in	std_logic;
		addrb		: in	std_logic_vector(8 downto 0);
		doutb		: out	std_logic_vector(7 downto 0)
	);
	end component;

	component DpMem8i16o
	port (
		clka		: in	std_logic;
		ena			: in	std_logic;
		wea			: in	std_logic_vector(0 downto 0);
		addra		: in	std_logic_vector(8 downto 0);
		dina		: in	std_logic_vector(7 downto 0);
		clkb		: in	std_logic;
		enb			: in	std_logic;
		addrb		: in	std_logic_vector(7 downto 0);
		doutb		: out	std_logic_vector(15 downto 0)
	);
	end component;

	component DpMem16i16o
	port (
		clka		: in	std_logic;
		ena			: in	std_logic;
		wea			: in	std_logic_vector(0 downto 0);
		addra		: in	std_logic_vector(3 downto 0);
		dina		: in	std_logic_vector(15 downto 0);
		clkb		: in	std_logic;
		enb			: in	std_logic;
		addrb		: in	std_logic_vector(3 downto 0);
		doutb		: out	std_logic_vector(15 downto 0)
	);
	end component;

end TypicPack;
