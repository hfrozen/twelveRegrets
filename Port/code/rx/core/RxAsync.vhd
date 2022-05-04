-------------------------------------------------------------------------------
-- Title	: async. receiver
-- Project	: serial communication
-- File		: RxAsync.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

entity RxAsync is
	port (
		resetI			: in	std_logic;
		clock147456kI	: in	std_logic;
		enableI			: in	std_logic;
		rxClock16I		: in	std_logic;
		evenParityI		: in	std_logic;
		oddParityI		: in	std_logic;
		stopLengI		: in	std_logic;
		eotHoldI_t4		: in	std_logic_vector(3 downto 0);
		rxI				: in	std_logic;
		rxO_t8			: out	std_logic_vector(7 downto 0);
		writeBuffO		: out	std_logic;
		eotO			: out	std_logic;
		parityErrorO	: out	std_logic;
		--eotHoldM_ti		: out	integer;	-- ?????
		--eotCountM_ti	: out	integer;	-- ?????
		--clockM			: out	std_logic;	-- ?????
		--fullM			: out	std_logic;	-- ?????
		--shiftM_ti		: out	integer;
		frameErrorO		: out	std_logic
	);
end RxAsync;

architecture behavioral of RxAsync is
	signal i_writeBuff		: std_logic	:= '0';
	signal i_eotHold_ti		: integer	:= 0;
	signal i_dreg_t8		: std_logic_vector(7 downto 0)	:= (others => '0');
	signal i_shift_t8		: std_logic_vector(7 downto 0)	:= (others => '0');
	signal i_full			: std_logic	:= '0';
	signal i_clock			: std_logic	:= '0';
	signal i_shift_ti		: integer	:= 0;
	signal i_resetSample	: std_logic	:= '0';
	signal i_sampling_t4	: std_logic_vector(3 downto 0)	:= (others => '0');		-- 0xf, 15
	signal i_eotCounter_ti	: integer	:= 0;

begin
	rxO_t8			<= i_dreg_t8 when enableI = '1'	else (others => '0');	--'Z');
	writeBuffO		<= i_writeBuff when enableI = '1'	else '0';	--'Z';
	i_eotHold_ti	<= (conv_integer(eotHoldI_t4) + 1) * 128;	--8 * 16, 8bit * iClock16;
	--eotHoldM_ti		<= i_eotHold_ti;		-- ?????
	--eotCountM_ti	<= i_eotCounter_ti;		-- ?????
	--clockM			<= i_clock;				-- ?????
	--fullM			<= i_full;				-- ?????
	--shiftM_ti		<= i_shift_ti;

	LatchProc	: process(resetI, clock147456kI)
		variable i_fulled	: std_logic	:= '0';
	begin
		if resetI = '1' then
			i_fulled	:= '0';
			i_writeBuff	<= '0';
		elsif clock147456kI'event and clock147456kI = '1' then
			if enableI = '1' then
				if i_full = '1' then
					if i_fulled = '0' then
						i_fulled	:= '1';
						i_dreg_t8	<= i_shift_t8;
						i_writeBuff	<= '1';
					else
						i_writeBuff	<= '0';
					end if;
				else
					i_fulled	:= '0';
					i_writeBuff	<= '0';
				end if;
			end if;
		end if;
	end process LatchProc;

	ClockProc	: process(resetI, rxClock16I)
		variable i_eoting			: std_logic	:= '0';
		--variable i_eotCounter_ti	: integer	:= 0;
	begin
		if resetI = '1' then
			eotO			<= '0';		--'Z';
			i_clock			<= '0';
			i_sampling_t4	<= (others => '0');
			i_eotCounter_ti	<= 0;
			i_eoting		:= '0';
		elsif rxClock16I'event and rxClock16I = '1' then
			if enableI = '1' then
				if i_resetSample = '1' then
					i_sampling_t4	<= (others => '0');
					i_clock			<= '0';
				else
					i_sampling_t4	<= i_sampling_t4 + 1;
					i_clock			<= i_sampling_t4(3) and i_sampling_t4(2) and i_sampling_t4(1) and i_sampling_t4(0);
				end if;

				if i_full = '0' then
					i_eotCounter_ti	<= 0;
					i_eoting		:= '0';
					eotO			<= '0';
				else
					if i_eoting = '0' then
						if i_eotCounter_ti = i_eotHold_ti then
							eotO			<= '1';
							i_eoting		:= '1';
						else
							i_eotCounter_ti	<= i_eotCounter_ti + 1;
						end if;
					end if;
				end if;
			else
				eotO		<= '0';		--'Z';
			end if;
		end if;
	end process ClockProc;

	ShiftProc	: process(resetI, rxClock16I)
		type STATE_SHIFTTYPES is (SHIFT_IDLE, SHIFT_DETECT, SHIFT_TEXT, SHIFT_PARITY, SHIFT_STOP, SHIFT_STOPEX);
		variable i_state		: STATE_SHIFTTYPES;
		variable i_parity		: std_logic	:= '0';
		--variable i_shift_ti		: integer	:= 0;
	begin
		if resetI = '1' then
			parityErrorO	<= '0';		--'Z';
			frameErrorO		<= '0';		--'Z';
			i_full			<= '0';
			i_shift_t8		<= (others => '0');
			i_shift_ti		<= 0;
			i_resetSample	<= '1';
			i_parity		:= '0';
			i_state			:= SHIFT_IDLE;
		elsif rxClock16I'event and rxClock16I = '1' then
			if enableI = '1' then
				case i_state is
				when SHIFT_IDLE =>
					i_shift_ti		<= 0;
					i_resetSample	<= '1';

					if rxI = '0' then
						i_resetSample	<= '0';
						i_state			:= SHIFT_DETECT;
					else
						i_state			:= SHIFT_IDLE;
					end if;

				when SHIFT_DETECT =>
					if rxI = '1' then
						i_state			:= SHIFT_IDLE;
					elsif i_sampling_t4 = "0111" then
						parityErrorO	<= '0';
						frameErrorO		<= '0';
						i_full			<= '0';
						i_shift_ti		<= 0;
						i_resetSample	<= '1';
						i_state			:= SHIFT_TEXT;
					end if;

				when SHIFT_TEXT =>
					i_resetSample	<= '0';

					if i_clock = '1' then
						i_shift_t8		<= rxI & i_shift_t8(7 downto 1);
						i_parity		:= i_parity xor rxI;

						if i_shift_ti = 7 then
							if evenParityI = '1' or oddParityI = '1' then
								i_state			:= SHIFT_PARITY;
							else
								if stopLengI = '1' then
									i_state			:= SHIFT_STOP;
								else
									i_state			:= SHIFT_STOPEX;
								end if;
							end if;
						else
							i_shift_ti		<= i_shift_ti + 1;
						end if;
					end if;

				when SHIFT_PARITY =>
					if i_clock = '1' then
						i_parity		:= i_parity xor rxI;

						if stopLengI = '1' then
							i_state			:= SHIFT_STOP;
						else
							i_state			:= SHIFT_STOPEX;
						end if;
					end if;

				when SHIFT_STOP =>
					if i_clock = '1' then
						if rxI = '0' then
							frameErrorO		<= '1';
						end if;

						i_state			:= SHIFT_STOPEX;
					end if;

				when SHIFT_STOPEX =>
					if i_clock = '1' then
						if rxI = '0' then
							frameErrorO		<= '1';
						elsif (evenParityI = '1' and i_parity = '1') or (oddParityI = '1' and i_parity = '0') then
							parityErrorO	<= '1';
						end if;

						i_full			<= '1';
						i_state			:= SHIFT_IDLE;
					end if;

				when others =>	null;
				end case;
			else
				parityErrorO	<= '0';		--'Z';
				frameErrorO		<= '0';		--'Z';
			end if;
		end if;
	end process ShiftProc;
end behavioral;
