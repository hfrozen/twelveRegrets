-------------------------------------------------------------------------------
-- Title	: main
-- Project	: serial communication
-- File		: Harbor.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

library work;
use work.TypicPack.all;
use work.Packs.all;
use work.TYPES.all;

entity Harbor is
	generic (
		constant cDebugRx_b			: boolean	:= false;
		constant cBlockLength_ti	: integer	:= 4;
		constant cFcsType_ti		: integer	:= 2;
		constant cAddrWidth_ti		: integer	:= 9;
		constant cMaxAddress_ti		: integer	:= 512
	);
	port (
		clock168000kI	: in	std_logic;
		clock18432kI	: in	std_logic;
		clock147456kO	: out	std_logic;
		--clockM			: out	std_logic;

		rxI_tv			: in	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		txO_tv			: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		rtsO_tv			: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		nInterruptO_tv	: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		--txEnableM_tv	: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);	-- ?????
		--taddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		--txFsizeM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		--rxAddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		--tbM_t8			: out	std_logic_vector(7 downto 0);
		--txdeM_t8		: out	std_logic_vector(7 downto 0);
		--treadBuffM		: out	std_logic;
		--tavailM			: out	std_logic;
		--enableClockRM	: out	std_logic;
		rAddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		rbM_t8			: out	std_logic_vector(7 downto 0);
		rWriteBuffM		: out	std_logic;
		rReadBuffM		: out	std_logic;
		--txAvailM_tv		: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		--rxFullM_tv		: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		--rxEotM_tv		: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		--clock16M		: out	std_logic;
		--clockM			: out	std_logic;
		etxO0			: out	std_logic;		-- for tx6, tx7
		etxO1			: out	std_logic;
		--erts0			: out	std_logic;
		--erts1			: out	std_logic;
		--eresI0			: in	std_logic;
		--eresI1			: in	std_logic;

		--clock147456kI	: in	std_logic;
		nResetI			: in	std_logic;
		nCsI			: in	std_logic;
		nReadI			: in	std_logic;
		nWriteI			: in	std_logic;
		addrI_t11		: in	std_logic_vector(10 downto 0);
		dataIO_t16		: inout	std_logic_vector(15 downto 0)
	);
end Harbor;

architecture behavioral of Harbor is
	signal i_clock147456k	: std_logic;
	--ignal i_clock16_tv		: LOGIC_ARRAY(0 to cBlockLength_ti - 1)	:= (others => '0');

	signal i_reset			: std_logic	:= '0';
	signal i_cs				: std_logic	:= '0';
	signal i_read			: std_logic	:= '0';
	signal i_write			: std_logic	:= '0';
	signal i_cs_tv			: LOGIC_ARRAY(0 to cBlockLength_ti - 1)	:= (others => '0');
	signal i_interrupt_tv	: LOGIC_ARRAY(0 to cBlockLength_ti - 1)	:= (others => '0');

	component ClockGenB
	port (
		CLKIN_IN	: in    std_logic;
		RST_IN		: in    std_logic;
		CLKFX_OUT	: out   std_logic
	);
	end component;

begin
	ClockGenFrame	: ClockGenB
	port map (
		CLKIN_IN	=> clock18432kI,
		RST_IN		=> '0',		--resetI,
		CLKFX_OUT	=> i_clock147456k
	);

	clock147456kO	<= i_clock147456k;
	i_reset			<= not nResetI;
	i_cs			<= not nCsI;
	i_read			<= not nReadI;
	i_write			<= not nWriteI;
	etxO0			<= '1';
	etxO1			<= '1';

	AddrDec	: process(i_reset, i_cs, addrI_t11)
	begin
		if i_reset = '0' and i_cs = '1' then
			case addrI_t11(10 downto 8) is
				when "000" =>	i_cs_tv(0)	<= '1';
				when "001" =>	i_cs_tv(1)	<= '1';
				when "010" =>	i_cs_tv(2)	<= '1';
				when "011" =>	i_cs_tv(3)	<= '1';
				--when "100" =>	i_cs_tv(4)	<= '1';
				--when "101" =>	i_cs_tv(5)	<= '1';
				--when "110" =>	i_cs_tv(6)	<= '1';
				--when "111" =>	i_cs_tv(7)	<= '1';
				when others =>	i_cs_tv		<= (others => '0');
		end case;
		else
			i_cs_tv	<= (others => '0');
		end if;
	end process AddrDec;

	InterruptDec	: process(i_interrupt_tv)
	begin
		for i in 0 to cBlockLength_ti - 1 loop
			nInterruptO_tv(i)	<= not i_interrupt_tv(i);
		end loop;
	end process InterruptDec;

	RealLocate : if not cDebugRx_b generate
		Locate	: for i in 0 to cBlockLength_ti - 1 generate
			PlatformFrames	: Platform
			generic map (
				cFcsType_ti		=> cFcsType_ti,
				cAddrWidth_ti	=> cAddrWidth_ti,
				cMaxAddress_ti	=> cMaxAddress_ti
			)
			port map (
				clock147456kI	=> i_clock147456k,
				clock168000kI	=> clock168000kI,
				rxI				=> rxI_tv(i),
				txO				=> txO_tv(i),
				rtsO			=> rtsO_tv(i),
				resetI			=> i_reset,
				busCsI			=> i_cs_tv(i),
				busReadI		=> i_read,
				busWriteI		=> i_write,
				busAddrI_t8		=> addrI_t11(7 downto 0),
				busDataIO_t16	=> dataIO_t16,
				busInterruptO	=> i_interrupt_tv(i)
			);
		end generate Locate;
	end generate RealLocate;

	DebugLocate	: if cDebugRx_b generate
		Locate	: for i in 1 to cBlockLength_ti - 1 generate
			PlatformFrames	: Platform
			generic map (
				cFcsType_ti		=> cFcsType_ti,
				cAddrWidth_ti	=> cAddrWidth_ti,
				cMaxAddress_ti	=> cMaxAddress_ti
			)
			port map (
				clock147456kI	=> i_clock147456k,
				clock168000kI	=> clock168000kI,
				rxI				=> rxI_tv(i),
				txO				=> txO_tv(i),
				rtsO			=> rtsO_tv(i),
				--clock16M		=> clock16M,	-- ?????
				--clockM			=> clockM,
				--taddrM_tv		=> taddrM_tv,
				--tbM_t8			=> tbM_t8,
				--treadBuffM		=> treadBuffM,
				--tavailM			=> tavailM,
				--enableClockRM	=> enableClockRM,
				--txEnableM		=> txEnableM_tv(i),	-- ?????
				--txAvailM		=> txAvailM_tv(i),	-- ?????
				--rxFullM			=> rxFullM_tv(i),	-- ?????
				--rxEotM			=> rxEotM_tv(i),	-- ?????
				resetI			=> i_reset,
				busCsI			=> i_cs_tv(i),
				busReadI		=> i_read,
				busWriteI		=> i_write,
				busAddrI_t8		=> addrI_t11(7 downto 0),
				busDataIO_t16	=> dataIO_t16,
				busInterruptO	=> i_interrupt_tv(i)
			);
		end generate Locate;

		PlatformFrame0	: Platform
		generic map (
			cFcsType_ti		=> cFcsType_ti,
			cAddrWidth_ti	=> cAddrWidth_ti,
			cMaxAddress_ti	=> cMaxAddress_ti
		)
		port map (
			clock147456kI	=> i_clock147456k,
			clock168000kI	=> clock168000kI,
			rxI				=> rxI_tv(0),
			txO				=> txO_tv(0),
			rtsO			=> rtsO_tv(0),
			--clock16M		=> clock16M,	-- ?????
			--clockM			=> clockM,
			---taddrM_tv		=> taddrM_tv,
			--tbM_t8			=> tbM_t8,
			--treadBuffM		=> treadBuffM,
			--tavailM			=> tavailM,
			--enableClockRM	=> enableClockRM,

			rAddrM_tv		=> rAddrM_tv,
			rbM_t8			=> rbM_t8,
			rWriteBuffM		=> rWriteBuffM,
			rReadBuffM		=> rReadBuffM,

			--txEnableM		=> txEnableM_tv(i),	-- ?????
			--txAvailM		=> txAvailM_tv(i),	-- ?????
			--rxFullM			=> rxFullM_tv(i),	-- ?????
			--rxEotM			=> rxEotM_tv(i),	-- ?????
			resetI			=> i_reset,
			busCsI			=> i_cs_tv(0),
			busReadI		=> i_read,
			busWriteI		=> i_write,
			busAddrI_t8		=> addrI_t11(7 downto 0),
			busDataIO_t16	=> dataIO_t16,
			busInterruptO	=> i_interrupt_tv(0)
		);
	end generate DebugLocate;

	--TestMemFrames	: Mem16_16
	--port map (
	--	resetI		=> i_reset,
	--	clockI		=> i_clock147456k,
	--	csI			=> i_cs_tv(4),
	--	readI		=> i_read,
	--	writeI		=> i_write,
	--	addrI_t4	=> addrI_t11(4 downto 1),
	--	dataIO_t16	=> dataIO_t16
	--);

	--TestRomFrame	: Rom16_16
	--port map (
	--	resetI		=> i_reset,
	--	clockI		=> i_clock147456k,
	--	csI			=> i_cs_tv(5),
	--	readI		=> i_read,
	--	addrI_t4	=> addrI_t11(4 downto 1),
	--	dataIO_t16	=> dataIO_t16
	--);

end behavioral;
