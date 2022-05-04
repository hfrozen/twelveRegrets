-------------------------------------------------------------------------------
-- Title	: hdlc tx controller
-- Project	: serial communication
-- File		: TxController.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity TxController is
	port (
		resetI				: in	std_logic;
		txClockI			: in	std_logic;
		txEnableI			: in	std_logic;		-- from TxChannel(txEnableI)
		shiftingByteI		: in	std_logic;		-- from ZeroInsert(shifting8bitO)
		validFrameI			: in	std_logic;		-- from TxChannel(validFrameI)
		abortFrameI			: in	std_logic;		-- from TxChannel(abortFrameI)
		haltI				: in	std_logic;		-- from ZeroInsert
		flagSizeI_t3		: in	std_logic_vector(2 downto 0);
		--flagStateM_t3		: out	std_logic_vector(2 downto 0);	-- ????
		--flagCounterM_t4		: out	std_logic_vector(3 downto 0);	-- ?????
		-- enableO				: out	std_logic;		-- to TxChannel(enableO)
		latchEnableO		: out	std_logic;		-- to ZeroInsert(enableOfBackendI)
		flagOpenO			: out	std_logic;		-- to FlagInsert
		abortSeqO			: out	std_logic;		-- to FlagInsert(abortSeqI)
		frameSeqO			: out	std_logic		-- to FlagInsert(flagSeqI)
	);
end TxController;

architecture behavioral of TxController is
	signal	i_flagSize_t3		: std_logic_vector(2 downto 0)	:= (others => '0');
	signal	i_flagSize_ti		: integer range 0 to 7	:= 0;
	signal	i_flagOpen_ti		: integer;

begin
	i_flagSize_ti	<= conv_integer(unsigned(i_flagSize_t3));
	i_flagOpen_ti	<= 7 - i_flagSize_ti;
	i_flagSize_t3	<= flagSizeI_t3;

	-- purpose	: abort controller
	-- type		: sequential
	-- inputs	: resetI, txClockI
	-- outputs	:
	AbortProc	: process(resetI, txClockI)
		variable i_counter_ti	: integer range 0 to 14;
		variable i_state		: std_logic;

	begin
		if resetI = '1' then
			abortSeqO		<= '0';
			i_counter_ti	:= 0;
			-- enableO			<= '1';
			i_state			:= '0';
		elsif rising_edge(txClockI) then
			if txEnableI = '1' then
				case i_state is
					when '0' =>				-- no abort signal
						if haltI = '1' or abortFrameI = '1' then
							i_state			:= '1';
							i_counter_ti	:= 0;
						end if;
						abortSeqO		<= '0';

					when '1' =>				-- abort signal
						if i_counter_ti = 8 then
							i_counter_ti	:= 0;

							if haltI = '0' and abortFrameI = '0' then
								i_state			:= '0';
								abortSeqO		<= '0';
							else
								abortSeqO		<= '1';
							end if;
						else
							i_counter_ti	:= i_counter_ti + 1;
						end if;

					when others => null;
				end case;
			end if;
			-- enableO <= txEnableI;

		end if;
	end process AbortProc;

	-- purpose	: flag controller
	-- type		: sequential
	-- inputs	: resetI, txClockI
	-- outputs	:
	--					        ___________________________________________________________
	--	validFrameI		_______|
	--					____   ____ 0 ____ 1 ____ 2 ____ 3 ____ 4 ____ 5 ____ 6 ____ 7 ____
	--	shiftByteI		    |_|    |_|    |_|    |_|    |_|    |_|    |_|    |_|    |_|
	--                                                            _________________________
	--	latchEnableO	_________________________________________|
	--					                                                               ____
	--	frameSeqO(s0)	______________________________________________________________|    	r1
	--					                                                        ______
	--	flagOpenO(s1)	_______________________________________________________|      |____	r2
	--					                                                 _____________
	--	flagOpenO(s2)	________________________________________________|             |____	r3
	--					                                          ____________________
	--	flagOpenO(s3)	_________________________________________|                    |____	r4
	--					                                   ___________________________
	--	flagOpenO(s4)	__________________________________|                           |____	r5
	--					                            __________________________________
	--	flagOpenO(s5)	___________________________|                                  |____	r6
	--					                     _________________________________________
	--	flagOpenO(s6)	____________________|                                         |____	r7
	--					              ________________________________________________
	--	flagOpenO(s7)	_____________|                                                |____	r8

	FlagProc	: process(resetI, txClockI)
		type STATE_TYPES is (STATE_IDLE, STATE_INTRO, STATE_VALID, STATE_EMPTY2, STATE_EMPTYLAST);
		variable i_state			: STATE_TYPES	:= STATE_IDLE;
		--variable i_clone_t3			: std_logic_vector(2 downto 0);		-- ?????
		variable i_counter_ti		: integer range 0 to 16;

	begin
		if resetI = '1' then
			flagOpenO		<= '0';
			frameSeqO		<= '0';
			i_state			:= STATE_IDLE;
			--i_clone_t3		:= "000";		-- ?????
			i_counter_ti	:= 0;
			latchEnableO	<= '0';
		elsif rising_edge(txClockI) then
			if txEnableI = '1' then
				case i_state is
					when STATE_IDLE =>						-- check valid frame
						flagOpenO		<= '0';
						frameSeqO		<= '0';
						if validFrameI = '1' then
							i_state			:= STATE_INTRO;
							--i_clone_t3		:= "001";		-- ?????
							if i_flagSize_ti < 4 then
								i_counter_ti	:= 4;
							else
								i_counter_ti	:= 7 - i_flagSize_ti;
							end if;
						end if;
						latchEnableO	<= '0';

					when STATE_INTRO =>
						if shiftingByteI = '0' then
							if i_counter_ti = 4 then
								latchEnableO	<= '1';
							end if;

							if i_counter_ti = 7 then
								i_state			:= STATE_VALID;
								--i_clone_t3		:= "010";	-- ?????
								flagOpenO		<= '0';
								frameSeqO		<= '1';
							elsif i_counter_ti = i_flagOpen_ti then
								flagOpenO		<= '1';
							end if;

							i_counter_ti	:= i_counter_ti + 1;
						end if;

						--if shiftingByteI = '0' then
						--	if i_counter_ti = 2 then		-- !! first byte, and wait for two more byte, -> three bytes
						--		i_state		:= STATE_UNTILVALID;
						--		i_clone_t3	:= "010";		-- ?????
						--		frameSeqO	<= '1';			-- after fill to two byte or at push 3th byte
						--	else
						--		frameSeqO	<= '0';
						--	end if;

						--	i_counter_ti	:= i_counter_ti + 1;
						--end if;
						--latchEnableO	<= '1';

					when STATE_VALID =>
						frameSeqO		<= '1';				-- start tx after 2 clock

						--if validFrameI = '0' then				-- untill valid frame (done tx writing)
						-- 180403, extend the latchEnableO to shiftByteI(i_byteLoaded of ZeroInsert)
						-- to zero dummy in to last register in ZeroInsert.
						if validFrameI = '0' and shiftingByteI = '0' then	-- 180403
							i_state			:= STATE_EMPTY2;
							--i_clone_t3		:= "011";		-- ?????
							latchEnableO	<= '0';
						else
							latchEnableO	<= '1';
						end if;
						i_counter_ti	:= 0;

					when STATE_EMPTY2 =>
						frameSeqO		<= '1';
						latchEnableO	<= '0';

						if shiftingByteI = '0' then
							if i_counter_ti > 0 then			-- wait push 2 byte to shift register
								i_state			:= STATE_EMPTYLAST;
								--i_clone_t3		:= "100";		-- ?????
							else
								i_counter_ti	:= i_counter_ti + 1;
							end if;
						end if;

					when STATE_EMPTYLAST =>
						if i_counter_ti = 8 then
							i_counter_ti	:= 0;
							i_state			:= STATE_IDLE;
							--i_clone_t3		:= "000";		-- ?????
							frameSeqO		<= '0';
						else
							i_counter_ti	:= i_counter_ti + 1;
							frameSeqO		<= '1';
						end if;
						latchEnableO	<= '0';

					when others => null;
				end case;
			end if;		-- txEnableI
		end if;		-- txClockI

		--flagStateM_t3		<= i_clone_t3;	-- ?????
		--flagCounterM_t4		<= conv_std_logic_vector(i_counter_ti, 4);	-- ?????
	end process FlagProc;
end behavioral;
