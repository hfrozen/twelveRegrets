-------------------------------------------------------------------------------
-- Title	: hdlc rx controller
-- Project	: serial communication
-- File		: RxController.vhd
-------------------------------------------------------------------------------
--
--		  0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   _   _   _   _   0   1   2   3   4   5   6   7   8   9   0   1   2   3   4   5   6   7
--clock	_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |_| |
--
--
--cnt	_|0|_|1|_|2|_|3|_|4|_|5|_|6|_|7|_|0|_____________________________________________|0|_|1|_|2|_|3|_|4|_|5|_|6|_|7|_|0|
--		  _                                                                               _
--dtct	_| |_____________________________________________________________________________| |____________________________________________________________________
--
--		  ___________________________                                                     ___________________________
--init	_|                           |___________________________________________________|                           |__________________________________________
--
--		                              ___________________________________________________                             __________________________________________
--st	\____________________________|                                                   |___________________________|
--
--
--		 |0| |0| |0| |0| |0| |0| |0| |8| |c| |e| |f| |f| |f| |f| |f| |f| |f| |f| |f| |7| |3| |1| |0| |0| |0| |0| |0| |8| |c| |e| |f| |f| |f| |f| |f|
--reg	_|0|_|0|_|0|_|0|_|0|_|0|_|0|_|0|_|0|_|0|_|0|_|8|_|c|_|e|_|f|_|f|_|f|_|f|_|f|_|f|_|f|_|f|_|f|_|7|_|3|_|1|_|0|_|0|_|0|_|0|_|0|_|8|_|c|_|e|_|f|
--
--		                                                          ___________________________________________________                             ______________
--valid	_________________________________________________________|                                                   |___________________________|
--

library ieee;
use ieee.std_logic_1164.all;

entity RxController is
	port (
		resetI			: in	std_logic;
		rxClockI		: in	std_logic;
		rxEnableI		: in	std_logic;		-- from FlagDetector
		flagDetectI		: in	std_logic;		-- from FlagDetector
		availableI		: in	std_logic;		-- from ZeroDetector(assem. byte)
		abortDetectI	: in	std_logic;		-- from FlagDetector
		initZeroO		: out	std_logic;		-- init zero detect block, to ZeroDetector
		enableZeroO		: out	std_logic;		-- to ZeroDetector
		validFrameO		: out	std_logic;		-- to ZeroDetector
		abortDetectO	: out	std_logic;		-- to controller's abortSignalO
		--frameStatusM	: out	std_logic;		-- ?????
		--frameMonM		: out	std_logic;		-- ?????
		--frameSampM		: out	std_logic;		-- ?????
		--flagCounterM_ti	: out	integer;
		overRunErrorO	: out	std_logic;
		frameErrorO		: out	std_logic		-- error in next byte at the bus
	);
end RxController;

architecture behavioral of RxController is
begin
	-- purpose	: enable controller
	-- type		: sequential
	-- input	: resetI, rxClockI
	-- output
	EnableProc	: process(resetI, rxClockI)
		variable i_counter_ti		: integer range 0 to 7;
		variable i_flagCounter_ti	: integer range 0 to 8;
		variable i_frameStatus		: std_logic;
		variable i_frameSample		: std_logic;
		variable i_flagInit			: std_logic;
		variable i_reg_t8			: std_logic_vector(7 downto 0);
		variable i_validFrame		: std_logic;
		variable i_toutCounter_ti	: integer range 0 to 8193;	-- 8192 = 1024 * 8 bit,

	begin
		--frameStatusM	<= i_frameStatus;	-- ?????
		--flagCounterM_ti	<= i_flagCounter_ti;

		if resetI = '1' then
			enableZeroO			<= '0';
			i_frameStatus		:= '0';
			i_frameSample		:= '0';
			--frameMonM			<= '0';
			--frameSampM			<= '0';
			validFrameO			<= '0';
			i_validFrame		:= '0';
			abortDetectO		<= '0';
			i_counter_ti		:= 0;
			i_flagInit			:= '0';
			initZeroO			<= '0';
			i_reg_t8			:= (others => '0');
			overRunErrorO		<= '0';
			frameErrorO			<= '0';
			i_flagCounter_ti	:= 0;
			i_toutCounter_ti	:= 0;
		elsif rising_edge(rxClockI) then
			if flagDetectI = '1' then		-- start
				i_flagInit			:= '1';
				i_frameStatus		:= '0';
				--frameMonM			<= '0';
				i_flagCounter_ti	:= 0;
				abortDetectO		<= '0';

				if i_validFrame = '0' then	-- begin flag
					overRunErrorO		<= '0';
					frameErrorO			<= '0';
				end if;
			end if;

			if i_flagInit = '1' then
				if i_flagCounter_ti = 7 then
					--i_frameStatus		:= '1';		-- combined 8bit after i_flagInit(= flagDetectI)
					--frameMonM			<= '1';
					i_frameStatus		:= not i_validFrame;
					--frameMonM			<= not i_validFrame;
					i_flagCounter_ti	:= 0;
					initZeroO			<= '1';		-- combined 8bit after i_flagInit(= flagDetectI)
					i_flagInit			:= '0';
				else
					if i_flagCounter_ti = 5 then
						i_frameSample		:= i_validFrame;
						--frameSampM			<= i_frameSample;
					end if;

					i_flagCounter_ti	:= i_flagCounter_ti + 1;
					initZeroO			<= '0';
				end if;
			else
				initZeroO		<= '0';
			end if;

			--if abortDetectI = '1' and i_frameStatus = '1' then	-- prevent abort that appear after end flag
			--	i_frameStatus	:= '0';
			--	abortDetectO	<= '1';
			--end if;

			--validFrameO				<= i_reg_t8(0);		-- eighth of i_frameStatus
			i_validFrame			:= i_reg_t8(0);		-- eighth of i_frameStatus
			validFrameO				<= i_validFrame;
			i_reg_t8(7 downto 0)	:= i_frameStatus & i_reg_t8(7 downto 1);

			-- enable
			if rxEnableI = '1' then
				if i_frameStatus = '1' then
					if abortDetectI = '1' then
						i_frameStatus	:= '0';
						abortDetectO	<= '1';
					end if;

					if availableI = '1' then		-- shifting at ZeroDetector
						enableZeroO		<= '1';
						i_counter_ti	:= 0;
						--frameErrorO		<= '0';
					else
						if i_counter_ti = 5 then
							enableZeroO		<= '0';
							i_counter_ti	:= 0;
							frameErrorO		<= '1';	-- strange code... availableI(0) * 5 => i_full8bit(1) * 5 => non read byte from RxFcs ???
						else
							enableZeroO		<= '1';
							i_counter_ti	:= i_counter_ti + 1;
							--frameErrorO		<= '0';
						end if;
					end if;

					i_toutCounter_ti	:= i_toutCounter_ti + 1;
					if i_toutCounter_ti = 8192 then
						overRunErrorO		<= '1';
					end if;
				else
					--frameErrorO		<= '0';
					enableZeroO			<= '0';
					i_counter_ti		:= 0;
					i_toutCounter_ti	:= 0;
				end if;
			else
				overRunErrorO	<= '0';
				frameErrorO		<= '0';
				enableZeroO		<= '0';
				i_counter_ti	:= 0;
			end if;		-- rxEnableI
		end if;		-- rxClockI
	end process EnableProc;
end behavioral;
