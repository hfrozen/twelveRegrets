-------------------------------------------------------------------------------
-- Title	: hdlc node
-- Project	: serial communication
-- File		: Node.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity Nrz is
	port (
		resetI		: in	std_logic;
		txClockI	: in	std_logic;
		rxClockI	: in	std_logic;
		txI			: in	std_logic;
		rxI			: in	std_logic;
		txO			: out	std_logic;
		rxO			: out	std_logic
	);
end Nrz;

architecture behavioral of Nrz is
	signal i_tx		: std_logic	:= '1';
	signal i_rx		: std_logic	:= '1';

begin
	txO		<= i_tx;
	rxO		<= i_rx;

	process (resetI, txClockI, rxClockI, txI, rxI)
	begin
		if resetI = '1' then
			i_tx	<= '1';
			i_rx	<= '1';
		else
			if rising_edge(txClockI) then
				i_tx	<= txI;
			end if;

			if rising_edge(rxClockI) then
				i_rx	<= rxI;
			end if;
		end if;
	end process;
end behavioral;

library ieee;
use ieee.std_logic_1164.all;

entity Nrzi is
	port (
		resetI		: in	std_logic;
		txClockI	: in	std_logic;
		rxClockI	: in	std_logic;
		txI			: in	std_logic;
		rxI			: in	std_logic;
		txO			: out	std_logic;
		rxO			: out	std_logic
	);
end Nrzi;

architecture behavioral of Nrzi is
	signal i_tx		: std_logic	:= '1';
	signal i_rx		: std_logic	:= '1';

begin
	txO		<= i_tx;
	rxO		<= i_rx;

	process (resetI, txClockI, rxClockI, txI, rxI)
		variable i_prevRx	: std_logic	:= '1';
	begin
		if resetI = '1' then
			i_tx		<= '1';
			i_rx		<= '1';
			i_prevRx	:= '1';
		else
			if rising_edge(txClockI) then
				if txI = '0' then
					i_tx	<= not i_tx;
				end if;
			end if;

			if rising_edge(rxClockI) then
				if rxI = i_prevRx then
					i_rx	<= '1';
				else
					i_rx	<= '0';
				end if;
				i_prevRx	:= rxI;
			end if;
		end if;
	end process;
end behavioral;

library ieee;
use ieee.std_logic_1164.all;

entity Manchester is
	port (
		resetI		: in	std_logic;
		txClockI	: in	std_logic;
		rxClockI	: in	std_logic;
		txI			: in	std_logic;
		rxI			: in	std_logic;
		txO			: out	std_logic;
		--rxClockO	: out	std_logic;
		rxO			: out	std_logic
	);
end Manchester;

architecture behavioral of Manchester is
	signal i_tx			: std_logic	:= '1';
	--signal i_rxClock	: std_logic	:= '0';
	signal i_rxs_t3		: std_logic_vector(2 downto 0)	:= "111";

begin
	txO			<= not (txClockI xor i_tx);
	--rxClockO	<= i_rxClock;

	process(resetI, rxClockI, rxI, i_rxs_t3)
	begin
		if resetI = '1' then
			--i_rxClock	<= '0';
			i_rxs_t3	<= "111";
		else
			if rising_edge(rxClockI) then
				--if i_rxs_t3(2) = '0' and i_rxs_t3(1) = '0' then
				--	i_rxClock	<= '1';
				--else
				--	i_rxClock	<= not i_rxClock;
				--end if;

				i_rxs_t3	<= i_rxs_t3(1 downto 0) & rxI;
			end if;
		end if;
	end  process;

	process(resetI, txClockI, rxClockI, txI, rxI)
	begin
		if resetI = '1' then
			i_tx	<= '1';
			rxO		<= '1';
		else
			if rising_edge(txClockI) then
				i_tx	<= txI;			-- for synch...
			end if;

			if rising_edge(rxClockI) then
				rxO		<= i_rxs_t3(2);
			end if;
		end if;
	end process;
end behavioral;

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

use work.NodePack.all;

entity RxPhaseSynch is
	port (
		resetI		: in	std_logic;
		clock16I	: in	std_logic;
		enableI		: in	std_logic;	-- dpllEnable
		hdlcI		: in	std_logic;
		mancheI		: in	std_logic;	-- manchester = 1
		nrziI		: in	std_logic;
		txClockI	: in	std_logic;
		rxI			: in	std_logic;
		--syncClockM	: out	std_logic;	-- ?????
		--rxaM		: out	std_logic;	-- ?????
		--rxbM		: out	std_logic;	-- ?????
		--clockSyncM_t4	: out	std_logic_vector(3 downto 0);	-- ?????
		syncClockO	: out	std_logic
	);
end RxPhaseSynch;

architecture behavioral of RxPhaseSynch is
	--signal i_trimClock		: std_logic	:= '0';
	signal i_sampledRx		: std_logic	:= '1';
	signal i_prevRx			: std_logic	:= '1';
	--signal i_syncClock		: std_logic	:= '0';
	signal i_n90Clock		: std_logic	:= '0';
	signal i_prevTxClock	: std_logic	:= '0';
	signal i_clockSync_t4	: std_logic_vector(3 downto 0)	:= (others => '0');

	component Dpll16 is
	port (
		clock16I	: in	std_logic;
		clockI		: in	std_logic;
		clockO		: out	std_logic
	);
	end component;

begin
	--rxaM	<= i_prevRx;
	--rxbM	<= i_sampledRx;
	--clockSyncM_t4	<= i_clockSync_t4;

	process (resetI, clock16I, enableI, hdlcI, mancheI, nrziI, txClockI, i_sampledRx)
		variable i_clockSlow_t4	: std_logic_vector(3 downto 0)	:= (others => '0');
		variable i_count_ti		: integer range 0 to 15	:= 4;
	begin
		if resetI = '1' then
			i_clockSync_t4	<= (others => '0');
			i_clockSlow_t4	:= (others => '0');
			i_count_ti		:= 4;
			i_sampledRx		<= '1';
			i_prevRx		<= '1';
			--i_syncClock		<= '0';
			i_n90Clock		<= '0';
			i_prevTxClock	<= '0';
		elsif rising_edge(clock16I) then
			--if enableI = '1' and hdlcI = '1' then
			--	if rxI = '1' and i_count_ti < 2 then
			--		i_count_ti	:= i_count_ti + 1;
			--	elsif rxI = '0' and i_count_ti > 0 then
			--		i_count_ti	:= i_count_ti - 1;
			--	end if;

			--	if i_count_ti >= 2 then
			--		i_sampledRx	<= '1';
			--	elsif i_count_ti = 0 then
			--		i_sampledRx	<= '0';
			--	end if;

			--	if i_prevRx = i_sampledRx then
			--		i_clockSync_t4	<= i_clockSync_t4 + "0001";
			--	else
			--		if mancheI = '0' and nrziI = '0' then
			--			i_clockSync_t4	<= "0000";		-- nrz, so restart
			--		else
			--			i_clockSync_t4	<= i_clockSync_t4(3) & "000";
			--			i_clockSync_t4(3)	<= not i_clockSync_t4(3);
			--		end if;
			--	end if;

			--	i_prevRx	<= i_sampledRx;
			--	--i_syncClock	<= not i_clockSync_t4(3);
			--	i_syncClock	<= i_clockSync_t4(3);
			--else
				if txClockI = '1' and i_prevTxClock = '0' then
					i_prevTxClock	<= '1';
					i_clockSlow_t4	:= "0001";	-- it should be initialized to "0000"!!!
				else
					i_clockSlow_t4	:= i_clockSlow_t4 + "0001";

					if txClockI = '0' then
						i_prevTxClock	<= '0';
					end if;
				end if;

				i_n90Clock		<= i_clockSlow_t4(3) xor i_clockSlow_t4(2);		-- 90 degree slow phase
			--end if;
		end if;
	end process;

	syncClockO	<= i_n90Clock;
	--syncClockO	<= (i_trimClock	and enableI and hdlci) or (i_n90Clock and not enableI);
	--syncClockM	<= (i_syncClock	and enableI and hdlci) or (i_n90Clock and not enableI);

	--DpllFrame	: Dpll
	--port map (
	--	clockI	=> clock16I,
	--	fI		=> i_syncClock,
	--	fO		=> i_trimClock
	--);

	--DpllFrame	: Dpll16
	--port map (
	--	clock16I	=> clock16I,
	--	clockI		=> i_syncClock,
	--	clockO		=> i_trimClock
	--);

end behavioral;

-- Digital PLL based on the 74LS297 architecture
-- uses jitter minimization

--library ieee;
--use ieee.std_logic_1164.all;
--use ieee.std_logic_arith.all;
--use ieee.std_logic_misc.all;
--use ieee.std_logic_unsigned.all;

--entity Dpll is
--	port (
--		clockI	: in	std_logic;
--		fI		: in	std_logic;
--		fO		: inout	std_logic
--	);
--end Dpll;

--architecture behavioral of Dpll is
--	constant i_cKBit_ti			: integer	:= 8;	-- k counter bit width
--	constant i_cNModulo_ti		: integer	:= 9;	-- n counter modulo
--	constant i_cFinModulo_ti	: integer	:= 8;	-- reference prescaler modulo
--	constant i_cFoutModulo_ti	: integer	:= 8;	-- dco_postscaler modulo

--	signal i_kDirection		: std_logic;	-- k counter up down
--	signal i_kCarry			: std_logic;
--	signal i_kBorrow		: std_logic;

--	signal i_nCount_ti		: integer range 0 to i_cNModulo_ti - 1;
--	signal i_finScale_ti	: integer range 0 to i_cFinModulo_ti - 1;	-- reference
--	signal i_foutScale_ti	: integer range 0 to i_cFoutModulo_ti - 1;

--	signal i_finScaleOut	: std_logic;
--	signal i_foutScaleOut	: std_logic;
--	signal i_kupCount_tv	: std_logic_vector(i_cKBit_ti - 1 downto 0);
--	signal i_kdownCount_tv	: std_logic_vector(i_cKBit_ti - 1 downto 0);

--	signal i_c1, i_c2, i_c3, i_c4	: std_logic;	-- carry shift
--	signal i_b1, i_b2, i_b3, i_b4	: std_logic;	-- borrow shift
--	signal i_idCountOut		: std_logic;
--begin

--	kCounter	: process(clockI)
--	begin
--		if clockI'event and clockI = '0' then
--			if i_kDirection = '1' then
--				i_kdownCount_tv	<= i_kdownCount_tv + 1;
--			else
--				i_kupCount_tv	<= i_kupCount_tv + 1;
--			end if;
--		end if;
--	end process;

--	i_kCarry	<= i_kupCount_tv(i_cKBit_ti - 1);
--	i_kBorrow	<= i_kdownCount_tv(i_cKBit_ti - 1);

--	idCounter	: process(clockI)
--	begin
--		if clockI'event and clockI = '1' then	-- positive edge control
--			i_c1	<= i_kCarry;
--			i_c2	<= i_c1;
--			i_c3	<= i_c2;
--			i_c4	<= ((not i_c1) and i_c2 and i_idCountOut) or ((not i_c2) and i_c3 and i_idCountOut);

--			i_b1	<= i_kBorrow;
--			i_b2	<= i_b1;
--			i_b3	<= i_b2;
--			i_b4	<= ((not i_b1) and i_b2 and (not i_idCountOut)) or ((not i_b2) and i_b3 and (not i_idCountOut));

--			-- JK-FF
--			if i_c4 = '1' and i_b4 = '1' then
--				i_idCountOut	<= i_idCountOut;
--			elsif i_c4 = '0' and i_b4 = '1' then
--				i_idCountOut	<= '1';
--			elsif i_c4 = '1' and i_b4 = '0' then
--				i_idCountOut	<= '0';
--			elsif i_c4 = '0' and i_b4 = '0' then
--				i_idCountOut	<= not i_idCountOut;
--			end if;
--		end if;
--	end process;

--	-- edge controlled phase detector
--	Ecpd	: block is
--		signal	i_pd1	: std_logic;
--		signal	i_pd2	: std_logic;
--		signal	i_rpd	: std_logic;

--	begin
--		i_rpd			<= i_pd2;
--		i_kDirection	<= i_pd1;

--		process(i_rpd, i_foutScaleOut)	-- a leading negative egde of
--		begin
--			if i_rpd = '1' then
--				i_pd1	<= '0';
--			elsif i_foutScaleOut'event and i_foutScaleOut = '0' then
--				i_pd1	<= '1';
--			end if;
--		end process;

--		process(i_rpd, i_finScaleOut)	-- a negative edge of fI reset
--		begin
--			if i_rpd = '1' then
--				i_pd2	<= '0';
--			elsif i_finScaleOut'event and i_finScaleOut = '0' then
--				i_pd2	<= '1';
--			end if;
--		end process;
--	end block;

--	-- prescaler
--	process(fI)
--	begin
--		if fI'event and fI = '1' then
--			if i_finScale_ti = i_cFinModulo_ti - 1 then
--				i_finScale_ti	<= 0;
--				i_finScaleOut	<= '0';
--			elsif i_finScale_ti >= (i_cFinModulo_ti / 2 - 1) then
--				i_finScale_ti	<= i_finScale_ti + 1;
--				i_finScaleOut	<= '1';
--			else
--				i_finScale_ti	<= i_finScale_ti + 1;
--				i_finScaleOut	<= '0';
--			end if;
--		end if;
--	end process;

--	process(fO)
--	begin
--		if fO'event and fO = '1' then
--			if i_foutScale_ti = i_cFoutModulo_ti - 1 then
--				i_foutScale_ti	<= 0;
--				i_foutScaleOut	<= '0';
--			elsif i_foutScale_ti >= (i_cFoutModulo_ti / 2 - 1) then
--				i_foutScale_ti	<= i_foutScale_ti + 1;
--				i_foutScaleOut	<= '1';
--			else
--				i_foutScale_ti	<= i_foutScale_ti + 1;
--				i_foutScaleOut	<= '0';
--			end if;
--		end if;
--	end process;

--	-- divide by N counter
--	process(clockI, i_idCountOut)
--	begin
--		if clockI'event and clockI = '0' then
--			if i_idCountOut = '0' then
--				if i_nCount_ti = i_cNModulo_ti - 1 then
--					i_nCount_ti	<= 0;
--					fO			<= '0';
--				elsif i_nCount_ti >= (i_cNModulo_ti / 2 - 1) then
--					i_nCount_ti	<= i_nCount_ti + 1;
--					fO			<= '1';
--				else
--					i_nCount_ti	<= i_nCount_ti + 1;
--					fO			<= '0';
--				end if;
--			end if;
--		end if;
--	end process;
--end behavioral;

--library ieee;
--use ieee.std_logic_1164.all;

--use work.NodePack.all;

-- 16bit Digital Phase-Locked Loop
--entity Dpll16 is
--	port (
--		clock16I	: in	std_logic;
--		clockI		: in	std_logic;
--		clockO		: out	std_logic
--	);
--end Dpll16;

--architecture behavioral of Dpll16 is
--	component Adjuster
--	port (
--		clock16I	: in	std_logic;
--		clockInI	: in	std_logic;
--		clockOutI	: in	std_logic;
--		enableO		: out	std_logic;
--		accelO		: out	std_logic
--	);
--	end component;

--	component Divider16
--	port (
--		clock16I	: in	std_logic;
--		enableI		: in	std_logic;
--		accelI		: in	std_logic;
--		clockO		: out	std_logic
--	);
--	end component;

--	signal i_enable	: std_logic;
--	signal i_accel	: std_logic;
--	signal i_clock	: std_logic;

--begin
--	clockO		<= i_clock;

--	Adjustframe		: Adjuster
--	port map (
--		clock16I	=> clock16I,
--		clockInI	=> clockI,
--		clockOutI	=> i_clock,
--		enableO		=> i_enable,
--		accelO		=> i_accel
--	);

--	Divide16Frame	: Divider16
--	port map (
--		clock16I	=> clock16I,
--		enableI		=> i_enable,
--		accelI		=> i_accel,
--		clockO		=> i_clock
--	);
--end behavioral;

--library ieee;
--use ieee.std_logic_1164.all;

--entity Adjuster is
--	port (
--		clock16I	: in	std_logic;
--		clockInI	: in	std_logic;
--		clockOutI	: in	std_logic;
--		enableO		: out	std_logic;
--		accelO		: out	std_logic
--	);
--end Adjuster;

--architecture behavioral of Adjuster is
--begin
--	process(clock16I)
--	begin
--		if clock16I'event and clock16I = '1' then
--			if clockInI = clockOutI then
--				enableO	<= '0';
--			else
--				enableO	<= '1';

--				if ClockOutI = '0' then
--					accelO	<= '1';		-- leading
--				else
--					accelO	<= '0';		-- lagging
--				end if;
--			end if;
--		end if;
--	end process;
--end behavioral;

--library ieee;
--use ieee.std_logic_1164.all;
--use ieee.std_logic_unsigned.all;

--entity Divider16 is
--	port (
--		clock16I	: in	std_logic;
--		enableI		: in	std_logic;
--		accelI		: in	std_logic;
--		clockO		: out	std_logic
--	);
--end Divider16;

--architecture behavioral of Divider16 is
--	signal i_mod_t4		: std_logic_vector(3 downto 0)	:= (others => '0');

--begin
--	clockO	<= not i_mod_t4(3);

--	process(clock16I)
--	begin
--		if clock16I'event and clock16I = '1' then
--			if enableI = '1' then
--				if accelI = '1' then	-- leading
--					i_mod_t4	<= i_mod_t4 + "0010";
--				--else					-- non, lagging
--				end if;
--			else
--				i_mod_t4	<= i_mod_t4 + "0001";
--			end if;
--		end if;
--	end process;
--end behavioral;
