-- Title	: rx buffer
-- Project	: serial communication
-- File		: RxBuff.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

--use work.MemPack.all;
use work.TypicPack.all;

entity RxBuffer is
	generic (
		cFcsType_ti		: integer	:= 2;	-- 2 = FCS 16 /	4 = FCS 32 / 0 = disable FCS
		cAddrWidth_ti	: integer	:= 9;
		cMaxAddress_ti	: integer	:= 512
	);
	port (
		resetI			: in	std_logic;
		clockI			: in	std_logic;
		--ignoreI			: in	std_logic;	-- force reset to rx buffer
		writeBuffI		: in	std_logic;
		readBuffI		: in	std_logic;
		eotI			: in	std_logic;
		rxI_t8			: in	std_logic_vector(7 downto 0);
		fullO			: out	std_logic;
		overflowErrorO	: out	std_logic;
		--overlapErrorO	: out	std_logic;
		frameSizeO_tv	: out	std_logic_vector(cAddrWidth_ti downto 0);
		rAddrM_tv		: out	std_logic_vector(cAddrWidth_ti downto 0);
		--rbM_t8			: out	std_logic_vector(7 downto 0);
		rxO_t16			: out	std_logic_vector(15 downto 0)
	);
end RxBuffer;

architecture behavioral of RxBuffer is
	--signal i_nClock147456		: std_logic;
	signal i_addr_tv		: std_logic_vector(cAddrWidth_ti downto 0);		-- untill 512

	type STATE_TYPES is (
		STATE_IDLE, STATE_WRITE, STATE_READ
	);
	signal i_curState		: STATE_TYPES	:= STATE_IDLE;
	signal i_adjState		: STATE_TYPES	:= STATE_IDLE;

	signal i_frameSize_tv	: std_logic_vector(cAddrWidth_ti downto 0);
	--signal i_regI_t8		: std_logic_vector(7 downto 0);
	--signal i_regO_t16		: std_logic_vector(15 downto 0);

	signal i_enWrite		: std_logic	:= '0';
	signal i_selWrite		: std_logic	:= '0';
	signal i_selRead		: std_logic	:= '0';

	signal i_resetAddr		: std_logic	:= '0';
	--signal i_readBuffF		: std_logic	:= '0';
	signal i_clockInc		: std_logic	:= '0';
	signal i_clearInc		: std_logic	:= '0';
	signal i_incAddr		: std_logic	:= '0';
	signal i_incedAddr		: std_logic	:= '0';
	signal i_latchFrames	: std_logic	:= '0';

	signal i_overflowError	: std_logic	:= '0';
	--signal i_overlapError	: std_logic	:= '0';
	signal i_full			: std_logic	:= '0';

	--signal i_access			: std_logic;
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
	attribute box_type	: string;
	attribute box_type of DpMem8i16o	: component is "black_box";

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
	DpMemFrame	: DpMem8i16o
	port map (
		clka	=> clockI,
		ena		=> i_selWrite,
		wea(0)	=> i_enWrite,
		addra	=> i_addr_tv(cAddrWidth_ti - 1 downto 0),
		dina	=> rxI_t8,		--i_regI_t8,
		clkb	=> clockI,
		enb		=> i_selRead,
		addrb	=> i_addr_tv(cAddrWidth_ti - 1 downto 1),
		doutb	=> rxO_t16		--i_regO_t16
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
	--i_nClock147456k	<= not clock147456kI;
	--i_regI_t8		<= rxI_t8;
	--rxO_t16			<= i_regO_t16;
	frameSizeO_tv	<= i_frameSize_tv;
	--i_access		<= (writeBuffI or readBuffI);

	overflowErrorO	<= i_overflowError;
	--overlapErrorO	<= i_overlapError;
	fullO			<= i_full;

	rAddrM_tv		<= i_addr_tv;

	-- purpose	: byte counter
	-- type		: sequential
	-- inputs	: resetI, clock147456kI
	-- outputs
	AddrIncProc	: process(resetI, i_resetAddr, clockI)	--, i_incedAddr)
	begin
		if resetI = '1' then
			i_addr_tv		<= (others => '0');
			i_clearInc		<= '1';
		elsif rising_edge(clockI) then
			if i_resetAddr = '1' then
				i_addr_tv		<= (others => '0');
				i_clearInc		<= '0';
			elsif i_incAddr = '1' then
				if i_adjState = STATE_READ then
					i_addr_tv		<= i_addr_tv + 2;
				else
					i_addr_tv		<= i_addr_tv + 1;
				end if;
				i_clearInc		<= '1';
			else
				i_clearInc		<= '0';
			end if;
		end if;
	--	if resetI = '1' then
	--		i_addr_tv		<= (others => '0');
	--		--i_incedAddr		<= '0';
	--	elsif rising_edge(clockI) then
	--		if i_resetAddr = '1' then
	--			i_addr_tv		<= (others => '0');
	--			i_incedAddr		<= '0';
	--		elsif i_incAddr = '1' then
	--			if i_incedAddr = '0' then
	--				if i_adjState = STATE_READ then
	--					i_addr_tv		<= i_addr_tv + 2;
	--				else	--if writeBuffI = '1' then
	--					i_addr_tv		<= i_addr_tv + 1;
	--				end if;
	--				i_incedAddr		<= '1';
	--			end if;
	--		else
	--			i_incedAddr		<= '0';
	--		end if;
	--	end if;
	end process AddrIncProc;

	-- purpose	: latch frame size
	-- type		: sequential
	-- input	:
	-- output	:
	LatchFrameSize	: process(resetI, clockI)
	begin
		if resetI = '1' then
			i_frameSize_tv	<= (others => '0');
		elsif rising_edge(clockI) then
			if i_latchFrames = '1' then
				i_frameSize_tv		<= i_addr_tv;
			end if;
		end if;
	end process LatchFrameSize;

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
			--i_incAddr		<= i_rbDelay;
		end if;
	end process ProcTurn;

	-- purpose	: read write
	-- type		: combinational
	-- inputs	: i_prevst, writeBuffI, readBuffI
	-- outputs
	ReadWriteProc	: process(clockI, i_curState, writeBuffI, readBuffI, i_incedAddr, eotI, i_frameSize_tv, i_addr_tv)	-- , ignoreI
	begin
		case i_curState is
			when STATE_IDLE =>
				i_full			<= '0';
				--i_overlapError	<= '0';
				i_overflowError	<= '0';

				if writeBuffI = '1' then			-- from rx controller
					i_adjState		<= STATE_WRITE;
					i_selWrite		<= '1';
					i_resetAddr		<= '0';
					i_enWrite		<= '1';
				else
					i_adjState		<= STATE_IDLE;
					i_resetAddr		<= '1';
					--i_readBuffF		<= '0';
					--i_incAddr		<= '0';
					i_enWrite		<= '0';
					i_selWrite		<= '0';
					i_selRead		<= '0';
				end if;

			when STATE_WRITE =>		-- timeout, abort ???
				i_enWrite		<= writeBuffI;
				--i_readBuffF		<= '0';

				if eotI = '1' then	--or i_addr_tv(cAddrWidth_ti) = '1' then
					i_adjState		<= STATE_READ;
					i_resetAddr		<= '1';
					--i_incAddr		<= '0';
					--i_rbDelay		<= '0';
					i_full			<= '1';
					i_latchFrames	<= '1';
					i_enWrite		<= '0';
					i_selWrite		<= '0';
					i_selRead		<= '1';
				else
					i_adjState		<= STATE_WRITE;
					i_resetAddr		<= '0';
					--i_incAddr		<= not writeBuffI;
					--i_rbDelay		<= not writeBuffI;
					i_full			<= '0';
					i_latchFrames	<= '0';
					i_selWrite		<= '1';

					i_overflowError	<= i_overflowError or (i_addr_tv(cAddrWidth_ti) and writeBuffI);
					--if i_overflowError = '0' then
					--	i_overflowError	<= i_addr_tv(cAddrWidth_ti) and writeBuffI;
					--end if;

				end if;

			when STATE_READ =>
				--if readBuffI = '0' then
				--	if i_readBuffF = '1' then
				--		i_readBuffF		<= '0';
				--		i_incAddr		<= '1';	-- if falling_edge(i_readBuff)	i_incAddr <= '1'
				--	end if;
				--else
				--	i_readBuffF		<= '1';
				--end if;

				--if i_incedAddr = '1' then
				--	i_incAddr		<= '0';		-- i_incAddr <= '0' after increment address
				--end if;

				i_latchFrames	<= '0';

				if i_addr_tv >= i_frameSize_tv then	--or ignoreI = '1' then
					i_adjState		<= STATE_IDLE;
					i_resetAddr		<= '1';
					i_selRead		<= '0';
					i_full			<= '0';
				else
					if writeBuffI = '1' then
						i_adjState		<= STATE_WRITE;
						i_resetAddr		<= '1';		--'0';	!!!! line 197, 207
						i_full			<= '0';
						--i_overlapError	<= '1';
						i_enWrite		<= '1';
						i_selRead		<= '0';
						i_selWrite		<= '1';
					else
						i_adjState		<= STATE_READ;
						i_resetAddr		<= '0';
						i_selRead		<= '1';
						--i_full			<= '0';
						--i_overlapError	<= '0';
					end if;
				end if;

			when others =>
				i_resetAddr		<= '1';
				i_full			<= '0';
				--i_overlapError	<= '0';
				i_enWrite		<= '0';
				i_selWrite		<= '0';
				i_selRead		<= '0';
		end case;
	end process ReadWriteProc;
end behavioral;
