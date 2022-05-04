-------------------------------------------------------------------------------
-- Title	: hdlc flag insertion
-- Project	: serial communication
-- File		: FlagInsert.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;

entity FlagInsert is
	port (
		resetI			: in	std_logic;
		txClockI		: in	std_logic;
		txEnableI		: in	std_logic;		-- from TxChannel(txEnableI)
		flagOpenI		: in	std_logic;
		abortSeqI		: in	std_logic;		-- from TxController(abortTransmitO)
		frameSeqI		: in	std_logic;		-- from TxController(validTransmitO)
		txI				: in	std_logic;		-- from TxO of ZeroInsert
		--flagM			: out	std_logic;		-- ?????
		--abortM			: out	std_logic;		-- ?????
		--flagShiftM_t4	: out	std_logic_vector(3 downto 0);		-- ?????
		--txEnableM		: out	std_logic;		-- ?????
		frameDefenceO	: out	std_logic;
		txO				: out	std_logic		-- to TxChannel(txO)
	);
end FlagInsert;

architecture behavioral of FlagInsert is
begin
	-- purpose	: flag insertion
	-- type		: sequential
	-- inputs	: resetI, txClockI
	-- output
	process(resetI, txClockI, txEnableI)
		variable i_reg_t8			: std_logic_vector(7 downto 0)	:= (others => '1');
		variable i_state			: std_logic := '0';
		variable i_flagSeq_ti		: integer	:= 0;
		variable i_flagShift_ti		: integer	:= 0;
		variable i_closeCount_ti	: integer range 0 to 511	:= 0;
		--variable i_shiftM_ti		: integer	:= 0;	-- ?????
		--variable i_flagM		: std_logic;		-- ?????
		--variable i_abortM		: std_logic;		-- ?????

	begin
		--flagM		<= i_flagM;		-- ?????
		--abortM		<= i_abortM;	-- ?????
		--flagShiftM_t4	<= conv_std_logic_vector(i_flagShift_ti, 4);
		--txEnableM		<= txEnableI;

		if resetI = '1' then
			frameDefenceO	<= '0';
			txO				<= '1';		--'Z';
			i_reg_t8		:= (others => '1');
			i_state			:= '0';
			i_flagSeq_ti	:= 0;
			i_flagShift_ti	:= 0;
			--i_shiftM_ti		:= 0;		-- ?????
			--i_flagM			:= '0';		-- ?????
			--i_abortM		:= '0';		-- ?????

		elsif rising_edge(txClockI) then
			--if i_flagM = '1' or i_abortM = '1' then	----- ?????
			--	i_shiftM_ti	:= i_shiftM_ti + 1;
			--	if i_shiftM_ti = 8 then
			--		i_shiftM_ti	:= 0;
			--		i_flagM		:= '0';
			--		i_abortM	:= '0';
			--	end if;
			--end if;	------------------------------------- ?????

			if txEnableI = '1' then
				case i_state is
					when '0' =>			-- idle state
						-- when flagOpenI = 0 and frameSeqI = 1, 1 clock is delayed.
						-- so "00111111" is output.
						-- but at first, "10111111" should be output
						-- "10111111", ..., "00111111", "0"(delay clock), "01111110"
						if flagOpenI = '1' then
							if i_flagShift_ti < 2 then
								if i_flagShift_ti = 0 and i_flagSeq_ti = 0 then
									i_flagSeq_ti	:= 1;
									txO				<= '1';
								else
									txO				<= '0';
								end if;
							else
								txO		<= '1';
							end if;

							i_flagShift_ti	:= i_flagShift_ti + 1;
							if i_flagShift_ti > 7 then
								i_flagShift_ti	:= 0;
							end if;

							frameDefenceO	<= '1';
						end if;

						if frameSeqI = '1' and abortSeqI = '0' then
							i_state		:= '1';
							i_reg_t8	:= "01111110";
							if i_flagSeq_ti /= 0 then
								txO			<= '0';
							end if;

							i_flagSeq_ti	:= 0;
							frameDefenceO	<= '1';
							--i_shiftM_ti		:= 0;	-- ?????
							--i_flagM			:= '1';	-- ?????
						end if;

						if flagOpenI = '0' and frameSeqI = '0' then
							txO						<= i_reg_t8(0);
							i_reg_t8(7 downto 0)	:= '1' & i_reg_t8(7 downto 1);
							i_flagShift_ti			:= 0;

							if i_closeCount_ti = 0 then
								frameDefenceO	<= '0';
							else
								i_closeCount_ti	:= i_closeCount_ti - 1;
							end if;
						end if;

					when '1' =>			-- normal operation
						txO						<= i_reg_t8(0);
						i_reg_t8(7 downto 0)	:= txI & i_reg_t8(7 downto 1);

						if abortSeqI = '1' then
							i_reg_t8		:= "11111110";
							i_state			:= '0';
							i_closeCount_ti	:= 16;
							--i_abortM	:= '1';		-- ?????
							--i_shiftM_ti	:= 0;	-- ?????
						elsif frameSeqI = '0' then
							i_reg_t8		:= "01111110";
							i_state			:= '0';
							i_closeCount_ti	:= 16;
							--i_flagM		:= '1';	-- ?????
							--i_shiftM_ti	:= 0;	-- ?????
						end if;

					when others => null;
				end case;
			else
				txO		<= '1';
			end if;		-- txEnableI
		end if;		-- txClockI
	end process;
end behavioral;
