-------------------------------------------------------------------------------
-- Title	: async. transmitter
-- Project	: serial communication
-- File		: TxAsync.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

entity TxAsync is
	port (
		resetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		enableI			: in	std_logic;
		txClock16I		: in	std_logic;
		evenParityI		: in	std_logic;
		oddParityI		: in	std_logic;
		stopLengI		: in	std_logic;
		txAvailI		: in	std_logic;
		txI_t8			: in	std_logic_vector(7 downto 0);
		readBuffO		: out	std_logic;
		--txClockM		: out	std_logic;		-- ?????
		--fullM			: out	std_logic;		-- ?????
		--shiftM			: out	std_logic;		-- ?????
		--dregM_t8		: out	std_logic_vector(7 downto 0);	-- ?????
		----shiftM_t8		: out	std_logic_vector(7 downto 0);	-- ?????
		frameDefenceO	: out	std_logic;
		txO				: out	std_logic
	);
end TxAsync;

architecture behavioral of TxAsync is
	signal i_dreg_t8		: std_logic_vector(7 downto 0)	:= (others => '1');
	signal i_shift_t8		: std_logic_vector(7 downto 0)	:= (others => '1');
	signal i_txClock		: std_logic;
	signal i_full			: std_logic;
	signal i_shift			: std_logic;

begin
	--txClockM	<= i_txClock;
	--fullM		<= i_full;
	--shiftM		<= i_shift;
	--dregM_t8	<= i_dreg_t8;
	--shiftM_t8	<= i_shift_t8;

	LatchProc	: process(resetI, clock147456kI)
		type STATE_LATCHTYPES is (LATCH_IDLE, LATCH_WAITSHIFTEND, LATCH_WAITSHIFTBEGIN, LATCH_READREADY, LATCH_READDROP, LATCH_NEXT);
		variable i_state		: STATE_LATCHTYPES;
	begin
		if resetI = '1' then
			i_dreg_t8	<= (others => '1');
			i_full		<= '0';
			readBuffO	<= '0';		--'Z';
			i_state		:= LATCH_IDLE;
		elsif clock147456kI'event and clock147456kI = '1' then
			if enableI = '1' then
				case i_state is
				when LATCH_IDLE =>
					i_dreg_t8	<= (others => '1');
					i_full		<= '0';
					readBuffO	<= '0';

					if txAvailI = '1' then
						readBuffO	<= '0';
						i_state		:= LATCH_WAITSHIFTEND;		-- 0's data is prepared at txBuffer;
					else
						readBuffO	<= '0';
						i_state		:= LATCH_IDLE;
					end if;

				when LATCH_WAITSHIFTEND =>
					if i_shift = '0' then
						if txAvailI = '1' then
							i_dreg_t8	<= txI_t8;
							i_full		<= '1';
							i_state		:= LATCH_WAITSHIFTBEGIN;
						else
							i_state		:= LATCH_IDLE;
						end if;
					else
						i_state		:= LATCH_WAITSHIFTEND;
					end if;

				when LATCH_WAITSHIFTBEGIN =>
					if i_shift = '1' then
						i_full		<= '0';
						i_state		:= LATCH_READREADY;
					else
						i_state		:= LATCH_WAITSHIFTBEGIN;
					end if;

				when LATCH_READREADY =>
					readBuffO	<= '1';
					i_state		:= LATCH_READDROP;

				when LATCH_READDROP =>
					readBuffO	<= '0';
					i_state		:= LATCH_NEXT;

				when LATCH_NEXT =>
					if txAvailI = '1' then
						i_state		:= LATCH_WAITSHIFTEND;
					else
						i_state		:= LATCH_IDLE;
					end if;

				when others =>	null;
				end case;
			else
				readBuffO	<= '0';		--'Z';
			end if;
		end if;
	end process LatchProc;

	ClockProc	: process(resetI, txClock16I)
		variable i_div_ti	: integer;
	begin
		if resetI = '1' then
			i_div_ti	:= 0;
			i_txClock	<= '0';		-- very important !!!!!
		elsif txClock16I'event and txClock16I = '1' then
			if enableI = '1' then
				if i_div_ti = 7 then
					i_txClock	<= not i_txClock;
					i_div_ti	:= 0;
				else
					i_div_ti	:= i_div_ti + 1;
				end if;
			end if;
		end if;
	end process ClockProc;

	ShiftProc	: process(resetI, i_txClock, i_full)
		type STATE_SHIFTTYPES is (SHIFT_IDLE, SHIFT_SYNC, SHIFT_TEXT, SHIFT_PARITY, SHIFT_STOP, SHIFT_STOPEX);
		variable i_state			: STATE_SHIFTTYPES;
		variable i_shift_ti			: integer	:= 0;
		variable i_closeCount_ti	: integer range 0 to 511	:= 0;
		variable i_parity			: std_logic	:= '0';
	begin
		if resetI = '1' then
			i_shift_t8		<= (others => '1');
			i_shift_ti		:= 0;
			i_closeCount_ti	:= 0;
			i_shift			<= '0';
			frameDefenceO	<= '0';		--'Z';
			txO				<= '1';		--'Z';
			i_state			:= SHIFT_IDLE;
		elsif i_txClock'event and i_txClock = '1' then
			if enableI = '1' then
				case i_state is
				when SHIFT_IDLE =>
					i_shift_t8		<= (others => '1');

					if i_closeCount_ti = 0 then
						frameDefenceO	<= '0';
					else
						i_closeCount_ti	:= i_closeCount_ti - 1;
					end if;

					if i_full = '1' then
						i_state			:= SHIFT_SYNC;
					else
						i_state			:= SHIFT_IDLE;
					end if;

				when SHIFT_SYNC =>
					if i_full = '1' then
						i_shift_t8		<= i_dreg_t8;
						i_shift			<= '1';
						frameDefenceO	<= '1';
						txO				<= '0';
						i_parity		:= '0';
						i_shift_ti		:= 0;
						i_closeCount_ti	:= 16;
						i_state			:= SHIFT_TEXT;
					else
						i_state			:= SHIFT_IDLE;
					end if;

				when SHIFT_TEXT =>
					txO				<= i_shift_t8(i_shift_ti);
					i_parity		:= i_parity xor i_shift_t8(i_shift_ti);

					if i_shift_ti /= 7 then
						i_shift_ti		:= i_shift_ti + 1;
					else
						i_shift_t8	<= (others => '1');
						if evenParityI = '1' or oddParityI = '1' then
							i_state			:= SHIFT_PARITY;
						else
							i_state			:= SHIFT_STOP;
						end if;
					end if;

				when SHIFT_PARITY =>
					if (evenParityI = '1' and i_parity = '1') or (oddParityI = '1' and i_parity = '0') then
						txO				<= '1';
					else
						txO				<= '0';
					end if;

					i_state			:= SHIFT_STOP;

				when SHIFT_STOP =>
					txO				<= '1';
					i_shift			<= '0';
					if stopLengI = '1' then
						i_state			:= SHIFT_STOPEX;
					else
						i_state			:= SHIFT_SYNC;
					end if;

				when SHIFT_STOPEX =>
					txO				<= '1';
					i_shift			<= '0';
					i_state			:= SHIFT_SYNC;

				end case;
			else
				txO				<= '1';		--'Z';
				frameDefenceO	<= '0';		--'Z';
			end if;
		end if;
	end process ShiftProc;
end behavioral;
