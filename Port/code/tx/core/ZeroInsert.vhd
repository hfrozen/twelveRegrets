-------------------------------------------------------------------------------
-- Title	: hdlc zero insertion
-- Project	: serial communication
-- File		: ZeroInsert.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

entity ZeroInsert is
	port (
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		txI_t8			: in	std_logic_vector(7 downto 0);	-- from TxChannel(txI_t8)
		-- validFrameI		: in	std_logic;
		txEnableI		: in	std_logic;		-- from TxChannel(enableI)
		latchEnableI	: in	std_logic;		-- from TxController(enableOfBackendO)
		writeByteI		: in	std_logic;		-- from TxChannel(writeByteI)
		haltO			: out	std_logic;		-- to TxController
		shiftingByteO	: out	std_logic;		-- to TxController(shifting8bitI)
		readyO			: out	std_logic;		-- to TxChannel(readyO)
		--detectM			: out	std_logic;		-- ?????
		--zeroStateM		: out	std_logic;		-- ?????
		--zeroRegM		: out	std_logic_vector(7 downto 0);	-- ?????
		txO				: out	std_logic		-- to FlagInsert(txI)
	);
end ZeroInsert;

architecture behavioral of ZeroInsert is
	signal i_reg_t8		: std_logic_vector(7 downto 0);
	signal i_shiftEnd	: std_logic;
	signal i_tx			: std_logic;

begin
	-- purpose	: parallel to serial
	-- type		: sequential
	-- inputs	: resetI, txClockI
	-- outputs	:
	P2sProc		: process(resetI, txClockI)
		variable i_temp_t16		: std_logic_vector(15 downto 0);
		variable i_counter_ti	: integer range 0 to 8;
		variable i_detected		: std_logic;

	begin
		if resetI = '1' then
			i_temp_t16		:= (others => '0');
			i_counter_ti	:= 0;
			i_shiftEnd		<= '1';
			i_detected		:= '0';
			txO				<= '1';
			i_tx			<= '1';
			shiftingByteO	<= '0';
		elsif rising_edge(txClockI) then
			if txEnableI = '1' then
				i_detected	:= i_temp_t16(0) and i_temp_t16(1) and i_temp_t16(2) and i_temp_t16(3) and i_temp_t16(4);
				i_tx		<= i_temp_t16(0);
				txO			<= i_tx;

				if i_detected = '1' then		-- zero insertion
					i_temp_t16(4 downto 0)	:= '0' & i_temp_t16(4 downto 1);
				else						-- total shift, msb is filled from i_reg_t8 when i_counter_ti is 8
					i_temp_t16(15 downto 0)	:= '0' & i_temp_t16(15 downto 1);
					i_counter_ti			:= i_counter_ti + 1;
				end if;

				if i_counter_ti = 8 then
					i_counter_ti			:= 0;
					i_shiftEnd				<= '1';		-- 8bit shift end, to readyO = 1
					shiftingByteO			<= '0';
					i_temp_t16(15 downto 8)	:= i_reg_t8;
				else
					shiftingByteO			<= '1';
					i_shiftEnd				<= '0';
				end if;
			end if;		-- txEnableI
			--detectM		<= i_detected;
		end if;		-- txClockI
	end process P2sProc;

	-- purpose	: bus interface
	-- type		: sequential
	-- inputs	: resetI, txClockI
	-- outputs	:
	LatchProc	: process(resetI, txClockI)
		variable i_state		: std_logic;

	begin
		--zeroStateM	<= i_state;
		--zeroRegM	<= i_reg_t8;
		if resetI = '1' then
			i_state			:= '0';
			i_reg_t8		<= (others => '0');
			readyO			<= '0';
			haltO			<= '0';
		elsif rising_edge(txClockI) then
			if txEnableI = '1' then
				if latchEnableI = '1' then
					case i_state is
						when '0' =>							-- wait for reading register
							if i_shiftEnd = '1' then		-- register has been read
								i_state		:= '1';
								readyO		<= '1';
								i_reg_t8	<= "00000000";	-- set register to known pattern to avoid invalid read
															-- (upon valid read this value will be overwritten)
							end if;

						when '1' =>
							if writeByteI = '1' then		-- to validFrame
								i_state			:= '0';
								readyO			<= '0';
								i_reg_t8		<= txI_t8;
							elsif i_shiftEnd = '1' then
								i_state			:= '0';
								readyO			<= '0';
								i_reg_t8		<= "00000000";
								haltO			<= '1';		-- shiftEnd & !writeByte => ummm...
							end if;

						when others => null;
					end case;
				else
					readyO			<= '0';
					i_state			:= '0';
					haltO			<= '0';
					--i_reg_t8		<= (others => '0');
				end if;		-- latchEnableI
			end if;		-- txEnableI
		end if;		-- txClockI
	end process LatchProc;
end behavioral;
