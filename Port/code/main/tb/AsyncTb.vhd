library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

library work;
use work.TypicPack.all;
use work.Packs.all;
use work.TYPES.all;

entity AsyncTb is
end AsyncTb;

architecture behavioral of AsyncTb is
	component Harbor
	generic (
		cBlockLength_ti	: integer	:= 4;
		cFcsType_ti		: integer	:= 2;
		cAddrWidth_ti	: integer	:= 9;
		cMaxAddress_ti	: integer	:= 512
	);
	port (
		clock18432kI	: in	std_logic;
		clock168000kI	: in	std_logic;
		clock147456kO	: out	std_logic;

		rxI_tv			: in	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		txO_tv			: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		rtsO_tv			: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		nInterruptO_tv	: out	LOGIC_ARRAY(0 to cBlockLength_ti - 1);
		--clock16M		: out	std_logic;
		--txEnableM		: out	std_logic;	-- ?????

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
		etxO0			: out	std_logic;
		etxO1			: out	std_logic;

		--clock147456kI	: in	std_logic;
		nResetI			: in	std_logic;
		nCsI			: in	std_logic;
		nReadI			: in	std_logic;
		nWriteI			: in	std_logic;
		addrI_t11		: in	std_logic_vector(10 downto 0);
		dataIO_t16		: inout	std_logic_vector(15 downto 0)
	);
	end component;

	-- connection signals
	signal i_clock18432			: std_logic := '0';
	signal i_clock147456		: std_logic	:= '0';
	signal i_clock168000		: std_logic	:= '0';
	signal i_clock16M			: std_logic	:= '0';
	--signal i_txEnableM		: std_logic	:= '0';
	signal i_rx					: std_logic	:= '0';
	signal i_tx					: std_logic	:= '0';
	signal i_rts				: std_logic	:= '0';
	signal i_interrupt			: std_logic	:= '0';

	signal i_reset				: std_logic := '0';
	signal i_cs					: std_logic := '1';
	signal i_read				: std_logic := '1';
	signal i_write				: std_logic := '1';
	signal i_addr_t11			: std_logic_vector(10 downto 0)	:= (others => '0');
	signal i_data_t16			: std_logic_vector(15 downto 0)	:= (others => '0');
	--signal i_txFsizeM_tv		: std_logic_vector(9 downto 0);
	--signal i_taddrM_tv			: std_logic_vector(9 downto 0);
	--signal i_tbM_t8				: std_logic_vector(7 downto 0);
	--signal i_txdeM_t8			: std_logic_vector(7 downto 0);
	--signal i_treadBuffM			: std_logic;
	--signal i_tavailM			: std_logic;
	--signal i_enableClockRM		: std_logic;
	signal i_rAddrM_tv			: std_logic_vector(9 downto 0);
	signal i_rbM_t8				: std_logic_vector(7 downto 0);
	--signal i_txdeM_t8			: std_logic_vector(7 downto 0);
	signal i_rWriteBuffM		: std_logic;
	signal i_rReadBuffM			: std_logic;

	signal i_clockf				: std_logic	:= '0';
	-- signal i_txAvailM_t4		: LOGIC_ARRAY(0 to 4 - 1);
	--signal i_rxAddrM_tv			: std_logic_vector(9 downto 0);
	--signal i_rxFullM_t4			: LOGIC_ARRAY(0 to 4 - 1);
	--signal i_rxEotM_t4			: LOGIC_ARRAY(0 to 4 - 1);

	signal i_busSeq				: std_logic	:= '0';
	signal i_clockSeq_ti		: integer	:= 0;
	signal i_send_ti			: integer	:= 0;

	signal i_rx_tv				: LOGIC_ARRAY(0 to 3);
	signal i_tx_tv				: LOGIC_ARRAY(0 to 3);
	signal i_rts_tv				: LOGIC_ARRAY(0 to 3);
	signal i_interrupt_tv		: LOGIC_ARRAY(0 to 3);

	constant i_clock18432_period		: time := 54.253 ns;
	constant i_clock168000_period		: time := 11.904 ns;	--5.952 ns;
	constant i_txLength_ti		: integer	:= 24;
	constant i_rxLength_ti		: integer	:= 17;

begin
	i_reset			<= '1' after 800 ns;
	i_clock18432	<= not i_clock18432 after i_clock18432_period / 2;
	i_clock168000	<= not i_clock168000 after i_clock168000_period / 2;
	i_tx			<= i_tx_tv(0);
	i_rx			<= i_tx;
	i_rx_tv(0)		<= i_rx;
	i_rts			<= i_rts_tv(0);
	i_interrupt		<= i_interrupt_tv(0);

	process(i_reset, i_clock168000, i_clock147456, i_interrupt)
		--variable i_busSeq		: std_logic	:= '0';
		--variable i_clockSeq_ti	: integer	:= 0;
		variable i_frameSeq_ti	: integer	:= 0;
		--variable i_send_ti		: integer	:= 0;
		--variable i_sd_t16		: std_logic_vector(15 downto 0)	:= x"f101";
		variable i_upper_t8		: std_logic_vector(7 downto 0)	:= (others => '0');
		variable i_lower_t8		: std_logic_vector(7 downto 0)	:= (others => '0');
		--variable i_data_t16	: std_logic_vector(15 downto 0)	:= (others => '0');
		variable i_addr_t8		: std_logic_vector(7 downto 0)	:= (others => '0');
		variable i_readSeq		: std_logic	:= '0';
		variable i_ready_ti		: integer	:= 0;
	begin
		if i_reset = '0' then
			i_cs			<= '1';
			i_read			<= '1';
			i_write			<= '1';
			i_frameSeq_ti	:= 0;
			i_send_ti		<= 0;
			i_busSeq		<= '0';
			i_ready_ti		:= 0;
			i_clockf		<= '0';
		elsif i_clock168000 = '1' then
			i_clockf		<= '1';
		elsif i_clock168000 = '0' then
			if i_clockf = '1' then
				i_clockf		<= '0';
		--elsif falling_edge(i_clock168000) then
		--elsif falling_edge(i_clock147456) then
			if i_busSeq = '0' then
				i_clockSeq_ti	<= 0;
				i_busSeq		<= '1';

				case i_frameSeq_ti is
				when 0 =>
					i_addr_t8		:= x"06";
					i_data_t16		<= x"0001";	-- mode
					i_readSeq		:= '0';
					i_frameSeq_ti	:= 1;
				when 1 =>
					i_addr_t8		:= x"0e";
					i_data_t16		<= x"1234";	-- address
					i_frameSeq_ti	:= 2;
				when 2 =>
					i_addr_t8		:= x"14";
					i_data_t16		<= x"0077";	-- 38400
					i_frameSeq_ti	:= 3;
				when 3 =>
					i_addr_t8		:= x"10";
					i_data_t16		<= conv_std_logic_vector(i_txLength_ti, 16);	-- tx length
					i_frameSeq_ti	:= 4;
				when 4 =>
				--	i_addr_t8		:= x"12";
				--	i_data_t16		<= x"0000";	-- sync length
				--	i_frameSeq_ti	:= 5;
				--when 5 =>
					i_addr_t8		:= x"0a";
					i_data_t16		<= x"00fd";	-- interrupt
					i_frameSeq_ti	:= 6;
				when 6 =>
					--i_addr_t8		:= x"06";
					--i_data_t16		<= x"0001";
					--i_sd_t16		:= x"f101";
					i_lower_t8		:= x"01";
					i_upper_t8		:= x"02";
					i_send_ti		<= 0;
					i_frameSeq_ti	:= 7;
				when 7 =>
					if i_send_ti >= i_txLength_ti then
						i_frameSeq_ti	:= 8;
						i_busSeq		<= '0';
					else
						i_addr_t8		:= x"00";
						--i_data_t16		<= i_sd_t16;
						i_data_t16		<= i_upper_t8 & i_lower_t8;
						i_upper_t8		:= i_upper_t8 + x"02";
						i_lower_t8		:= i_lower_t8 + x"02";
						--i_sd_t16		:= i_sd_t16 + x"0001";
						i_send_ti		<= i_send_ti + 2;
					end if;
				when 8 =>
					--i_addr_t8		:= x"06";
					--i_data_t16		<= (others => 'Z');
					--i_frameSeq_ti	:= 10;
					--i_readSeq		:= '1';
				--when 9 =>
					i_addr_t8		:= x"16";
					i_frameSeq_ti	:= 10;
				when 10 =>
					i_addr_t8		:= x"04";
					i_data_t16		<= x"0008";	-- tx enable
					i_readSeq		:= '0';
					i_frameSeq_ti	:= 11;
				when 11 =>
					if i_interrupt = '0' then
						i_frameSeq_ti	:= 12;
					end if;
					i_busSeq		<= '0';
				when 12 =>
					i_addr_t8		:= x"04";
					i_data_t16		<= x"0900";
					i_frameSeq_ti	:= 13;
				when 13 =>
					i_readSeq		:= '1';
					i_addr_t8		:= x"0c";	-- interrupt status
					i_data_t16		<= (others => 'Z');
					i_frameSeq_ti	:= 14;
				when 14 =>
					i_addr_t8		:= x"02";	-- rx length
					i_send_ti		<= 0;
					i_frameSeq_ti	:= 15;
				when 15 =>
					if i_send_ti >= i_txLength_ti then
						i_frameSeq_ti	:= 16;
						i_busSeq		<= '0';
					else
						i_addr_t8		:= x"00";
						i_send_ti		<= i_send_ti + 2;
					end if;
				when 16 =>
					i_busSeq		<= '0';
				when others => null;
				end case;
			else
				case i_clockSeq_ti is
				when 0 =>
					i_addr_t11		<= "000" & i_addr_t8;
					i_cs			<= '0';

					if i_readSeq = '1' then
						i_write			<= '1';
						i_read			<= '0';
					else
						i_read			<= '1';
						i_write			<= '0';
					end if;
					i_clockSeq_ti	<= 1;

				when 1 =>
					i_clockSeq_ti	<= 2;
				when 2 =>
					i_clockSeq_ti	<= 3;
				when 3 =>
					i_clockSeq_ti	<= 4;
				when 4 =>
					--i_cs			<= '1';
					i_read			<= '1';
					i_write			<= '1';
					--i_busSeq		<= '0';
					i_clockSeq_ti	<= 5;
				when 5 =>
					i_cs			<= '1';
					i_clockSeq_ti	<= 10;
				when 6 =>
					i_clockSeq_ti	<= 7;
				when 7 =>
					i_clockSeq_ti	<= 10;
				when 8 =>
					i_clockSeq_ti	<= 10;
				when 9 =>
					i_clockSeq_ti	<= 10;
				when 10 =>
					i_cs			<= '1';
					i_read			<= '1';
					i_write			<= '1';
					i_busSeq		<= '0';
				when others =>	null;
				end case;
			end if;
			end if;
		end if;
	end process;

	uut: Harbor
	--generic map (
	--	cBlockLength_ti	=> cBlockLength_ti,
	--	cFcsType_ti		=> cFcsType_ti,
	--	cAddrWidth_ti	=> cAddrWidth_ti,
	--	cMaxAddress_ti	=> cMaxAddress_ti
	--);
	port map (
		clock168000kI	=> i_clock168000,
		clock18432kI	=> i_clock18432,
		clock147456kO	=> i_clock147456,

		rxI_tv			=> i_rx_tv,
		txO_tv			=> i_tx_tv,
		rtsO_tv			=> i_rts_tv,
		nInterruptO_tv	=> i_interrupt_tv,

		--clock168000kI	=> i_clock16M,
		--txEnableM_tv	=> i_txEnableM_tv,
		--taddrM_tv		=> i_taddrM_tv,
		---txFsizeM_tv		=> i_txFsizeM_tv,
		--rxAddrM_tv		=> i_rxAddrM_tv,
		--tbM_t8			=> i_tbM_t8,
		--txdeM_t8		=> i_txdeM_t8,
		--treadBuffM		=> i_treadBuffM,
		--tavailM			=> i_tavailM,
		--enableClockRM	=> i_enableClockRM,
		rAddrM_tv		=> i_rAddrM_tv,
		rbM_t8			=> i_rbM_t8,
		rWriteBuffM		=> i_rWriteBuffM,
		rReadBuffM		=> i_rReadBuffM,
		--txAvailM_tv		=> i_txAvailM_t4,
		--rxFullM_tv		=> i_rxFullM_t4,
		--rxEotM_tv		=> i_rxEotM_t4,

		nResetI			=> i_reset,
		nCsI			=> i_cs,
		nReadI			=> i_read,
		nWriteI			=> i_write,
		addrI_t11		=> i_addr_t11,
		dataIO_t16		=> i_data_t16
	);
end;
