-------------------------------------------------------------------------------
-- Title	: hdlc zero detection
-- Project	: serial communication
-- File		: ZeroDetector.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity ZeroDetector is
	port (
		resetI			: in	std_logic;
		rxClockI		: in	std_logic;
		rxI				: in	std_logic;		-- from FlagDetector
		validFrameI		: in	std_logic;		-- from RxController
		enableI			: in	std_logic;		-- from RxController
		startFrameI		: in	std_logic;		-- from RxController
		readByteI		: in	std_logic;		-- from RxChannel
		availableO		: out	std_logic;		-- to RxController
		readyO			: out	std_logic;		-- to RxController
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
end ZeroDetector;

architecture behavioral of ZeroDetector is
	signal i_reg_t8	: std_logic_vector(7 downto 0);
	signal i_eight	: std_logic;			-- 8bits data ready

begin
	-- purpose	: zero detection
	-- type		: sequential
	-- inputs	: resetI, rxClockI
	-- outputs	:
	DetectProc	: process(resetI, rxClockI)
		variable i_detected		: std_logic;
		variable i_temp_t8		: std_logic_vector(7 downto 0);
		variable i_counter_ti	: integer range 0 to 7;
		variable i_check_t6		: std_logic_vector(5 downto 0);

	begin
		if resetI = '1' then
			i_counter_ti	:= 0;
			i_temp_t8		:= (others => '0');
			i_reg_t8		<= (others => '0');
			i_eight			<= '0';
			i_detected		:= '0';
			i_check_t6		:= (others => '0');
		elsif rising_edge(rxClockI) then
			if enableI = '1' then
				if startFrameI = '0' then	-- add new bit to the check register
					i_check_t6				:= rxI & i_check_t6(5 downto 1);
					i_temp_t8(i_counter_ti)	:= rxI;
				else						-- reset the check register
					i_check_t6				:= (rxI, others => '0');
					i_counter_ti			:= 0;
					i_temp_t8(i_counter_ti)	:= rxI;
				end if;

				-- check if got 5 ones
				i_detected	:=  not i_check_t6(5) and i_check_t6(4) and i_check_t6(3) and
									 i_check_t6(2) and i_check_t6(1) and i_check_t6(0);
								-- 01 1111
				if i_detected = '1' then		-- zero detected, following operation is skipped.
					i_eight	 	<= '0';
				else							-- zero not detect.
					if i_counter_ti = 7 then
						i_reg_t8		<= i_temp_t8;
						i_counter_ti	:= 0;
						i_eight			<= '1';
					else
						i_counter_ti	:= i_counter_ti + 1;
						i_eight			<= '0';
					end if;		-- i_counter_ti
				end if;		-- i_detected
			end if;		-- enableI
		end if;		-- rxClockI
	end process DetectProc;

	-- purpose	: bus interface
	-- type		: sequential
	-- inputs	: resetI, rxClockI
	-- outputs	:
	LatchProc	: process(resetI, rxClockI)
		variable i_ready	: std_logic;

	begin
		if resetI = '1' then
			rxO_t8		<= (others => '0');
			availableO	<= '1';
			i_ready		:= '0';
			readyO		<= '0';
		elsif rising_edge(rxClockI) then
			if enableI = '1' then
				if i_eight = '1' then
					rxO_t8		<= i_reg_t8;
					i_ready		:= '1';
				end if;
			end if;

			if readByteI = '1' then
				i_ready		:= '0';
			end if;
			readyO		<= i_ready;

			if validFrameI = '0' then
				availableO	<= '1';
			else
				availableO	<= not i_ready;
			end if;		-- validFrameI
		end if;		-- rxClockI
	end process LatchProc;
end behavioral;
