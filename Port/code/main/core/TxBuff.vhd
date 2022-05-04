-------------------------------------------------------------------------------
-- Title	: tx buffer
-- Project	: serial communication
-- File		: TxBuff.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

--use work.MemPack.all;
use work.TypicPack.all;

entity TxBuffer is
	generic (
		cAddrWidth_ti	: integer	:= 9;		-- 3ff, 1024byte
		cMaxAddress_ti	: integer	:= 512
	);
	port (
		resetI			: in	std_logic;
		clockI			: in	std_logic;
		txEnableI		: in	std_logic;						-- bus
		readBuffI		: in	std_logic;						-- tx fcs
		writeBuffI		: in	std_logic;						-- bus
		frameSizeI_tv	: in	std_logic_vector(cAddrWidth_ti downto 0);	-- 0x200, "10 0000 0000", 10 bits
		-- transmit byte is 512, need 10 bits, so max size is 1023, but memory size is 512 bytes(0 to 511, 9 bits),
		-- so it can exceed the size.
		-- in bus behavioral, shold be check size that it does not over 512,
		txI_t16			: in	std_logic_vector(15 downto 0);	-- bus
		--taddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);	-- ?????
		--tbM_t8			: out	std_logic_vector(7 downto 0);				-- ?????
		--enableClockRM	: out	std_logic;
		txAvailO		: out	std_logic;						-- tx fcs
		txEmptyO		: out	std_logic;						-- Bus
		--stateM_t2		: out	std_logic_vector(1 downto 0);					-- ?????
		txO_t8			: out	std_logic_vector(7 downto 0)	-- tx fcs
	);
end TxBuffer;

architecture behavioral of TxBuffer is
	--signal i_nClock147456		: std_logic;
	signal i_addr_tv		: std_logic_vector(cAddrWidth_ti downto 0)	:= (others => '0');		-- untill 512

	type STATE_TYPES is (STATE_IDLE, STATE_WRITE, STATE_READ);	--, STATE_READ2);
	signal i_curState		: STATE_TYPES	:= STATE_IDLE;
	signal i_adjState		: STATE_TYPES	:= STATE_IDLE;

	--signal i_frameSize_tv	: std_logic_vector(cAddrWidth_ti downto 0);
	signal i_regI_t16		: std_logic_vector(15 downto 0);
	signal i_regO_t8		: std_logic_vector(7 downto 0);
	signal i_resetAddr		: std_logic	:= '0';
	signal i_clockInc		: std_logic	:= '0';
	signal i_clearInc		: std_logic	:= '0';
	signal i_incAddr		: std_logic := '0';
	signal i_enWrite		: std_logic	:= '0';
	signal i_selWrite		: std_logic	:= '0';
	signal i_selRead		: std_logic	:= '0';
	--signal i_latchFrames	: std_logic	:= '0';

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
	attribute box_type	: string;
	attribute box_type of DpMem16i8o	: component is "black_box";

	component Dff
	port (
		dI		: in	std_logic;
		clockI	: in	std_logic;
		clearI	: in	std_logic;
		resetI	: in	std_logic;
		qO		: out	std_logic
	);
	end component;

begin
	DpMemFrame	: DpMem16i8o
	port map (
		clka	=> clockI,
		ena		=> i_selWrite,
		wea(0)	=> i_enWrite,
		addra	=> i_addr_tv(cAddrWidth_ti - 1 downto 1),
		dina	=> i_regI_t16,	--txI_t16,
		clkb	=> clockI,		--i_nClock147456,
		enb		=> i_selRead,
		addrb	=> i_addr_tv(cAddrWidth_ti - 1 downto 0),
		doutb	=> i_regO_t8
	);

	IncAddrFrame	: Dff
	port map (
		dI		=> '1',
		clockI	=> i_clockInc,
		clearI	=> i_clearInc,
		resetI	=> '0',
		qO		=> i_incAddr
	);

	i_clockInc		<= not (readBuffI or writeBuffI);
	--taddrM_tv	<= i_addr_tv;
	--enableClockRM	<= i_selRead;
	--tbM_t8		<= i_regO_t8;
	--i_regO_t8	<= i_addr_tv(7 downto 0);
	--i_nClock147456k	<= not clock147456kI;
	--i_frameSize_tv	<= frameSizeI_tv;
	i_regI_t16		<= txI_t16;
	--i_regI_t16		<= "00010001" & i_addr_tv(8 downto 1);
	txO_t8			<= i_regO_t8;
	--txO_t8			<= i_addr_tv(7 downto 0);
	--overflowO	<= '1'	when conv_integer(i_addr_tv) > i_max_addr_tv	else '0';
	--stateM_t2	<= i_stateM_t2;	-- ?????

	-- purpose	: byte counter
	-- type		: sequential
	-- inputs	: resetI, clock147456kI
	-- outputs
	AddrIncProc	: process(resetI, i_resetAddr, clockI)	--i_access)
	begin
		if resetI = '1' then
			i_addr_tv		<= (others => '0');
			i_clearInc		<= '1';
		elsif rising_edge(clockI) then
			if i_resetAddr = '1' then
				i_addr_tv		<= (others => '0');
				i_clearInc		<= '0';
			elsif i_incAddr = '1' then
				if i_adjState = STATE_WRITE then
					i_addr_tv		<= i_addr_tv + 2;
				else	--if i_adjState = STATE_READ then
					i_addr_tv		<= i_addr_tv + 1;
				end if;
				i_clearInc		<= '1';
			else
				i_clearInc		<= '0';
			end if;
		end if;
	end process AddrIncProc;

	-- purpose	: latch frame size
	-- type		: sequential
	-- input	: resetI, clockI
	-- output	:
	--LatchFrameSize	: process(resetI, clockI)
	--begin
	--	if resetI = '1' then
	--		i_frameSize_tv	<= (others => '0');
	--	elsif rising_edge(clockI) then
	--		if i_latchFrames = '1' then
	--			--if frameSizeI_tv = x"000" then
	--				i_frameSize_tv	<= i_addr_tv;
	--			--else
	--			--	i_frameSize_tv	<= frameSizeI_tv;
	--			--end if;
	--		end if;
	--	end if;
	--end process LatchFrameSize;

	-- purpose	: process turn
	-- type		: sequential
	-- input	: resetI, clockI
	-- output	:
	ProcTurn	: process(resetI, clockI)
	begin
		if resetI = '1' then
			i_curState		<= STATE_IDLE;
		elsif rising_edge(clockI) then
			i_curState		<= i_adjState;
		end if;
	end process ProcTurn;

	-- purpose	: read write
	-- type		: combimational
	-- inputs	: strobe
	-- outputs
	ReadWriteProc	: process(clockI, txEnableI, writeBuffI, readBuffI, i_addr_tv, i_curState, frameSizeI_tv)	--, i_frameSize_tv)
	begin
		case i_curState is
			when STATE_IDLE =>
				txEmptyO		<= '1';
				txAvailO		<= '0';

				if writeBuffI = '1' then		-- by bus
					i_adjState		<= STATE_WRITE;
					i_selWrite		<= '1';
					i_resetAddr		<= '0';
				else
					i_adjState		<= STATE_IDLE;
					i_resetAddr		<= '1';
					--i_incAddr		<= '0';
					i_enWrite		<= '0';
					i_selWrite		<= '0';
					i_selRead		<= '0';
				end if;

			when STATE_WRITE =>		-- write cycle by bus
				txEmptyO		<= '0';
				txAvailO		<= '0';
				--i_incAddr		<= not writeBuffI;
				i_enWrite		<= writeBuffI;

				if txEnableI = '1' then	-- or i_addr_tv = i_max_addr_tv then
					i_adjState		<= STATE_READ;
					i_resetAddr		<= '1';
					i_enWrite		<= '0';
					i_selWrite		<= '0';
					i_selRead		<= '1';
				else
					i_adjState		<= STATE_WRITE;
					i_resetAddr		<= '0';
					i_selWrite		<= '1';
				end if;

			when STATE_READ =>
				txAvailO		<= '1';
				--i_incAddr		<= readBuffI;

				if i_addr_tv = frameSizeI_tv then
					i_adjState		<= STATE_IDLE;
					i_resetAddr		<= '1';
					txEmptyO		<= '1';
					i_selRead		<= '0';
					txAvailO		<= '0';
				else
					i_resetAddr		<= '0';
					i_adjState		<= STATE_READ;
					txEmptyO		<= '0';
					i_selRead		<= '1';
				end if;

			when others =>
				txAvailO		<= '0';
				txEmptyO		<= '0';
				i_resetAddr		<= '1';
				i_enWrite		<= '0';
				i_selWrite		<= '0';
				i_selRead		<= '0';

		end case;
	end process ReadWriteProc;
end behavioral;
