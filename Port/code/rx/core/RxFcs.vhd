-------------------------------------------------------------------------------
-- Title	: rx frame check sequence
-- Project	: serial communication
-- File		: RxFcs.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_unsigned.all;

use work.CalcCrc16d8.all;

entity RxFcs is
	generic (
		cFcsType_ti	 : integer	:= 2
	);
	port (
		resetI			: in	std_logic;
		clockI			: in	std_logic;
		enableI			: in	std_logic;
		fcsEnableI		: in	std_logic;
		readyI			: in	std_logic;
		validFrameI		: in	std_logic;
		rxI_t8			: in	std_logic_vector(7 downto 0);
		readByteO		: out	std_logic;
		fcsErrorO		: out	std_logic;
		eotO			: out	std_logic;
		writeBuffO		: out	std_logic;
		--fcsM_t16		: out	std_logic_vector(15 downto 0);	-- ?????
		headO_t16		: out	std_logic_vector(15 downto 0);
		rxO_t8			: out	std_logic_vector(7 downto 0)
	);
end RxFcs;

architecture behavioral of RxFcs is
	type STATE_TYPES is	(STATE_IDLE, STATE_RUN, --STATE_RUN1, STATE_RUN2,
						STATE_READ, STATE_EOF1, STATE_EOF2, STATE_EOFZ);
	signal i_state		: STATE_TYPES;
	signal i_writeBuff	: std_logic;
	signal i_priorWrite	: std_logic;
	signal i_eot		: std_logic;
	--signal i_fcs_t16	: std_logic_vector(15 downto 0);

begin
	--fcsM_t16	<= i_fcs_t16;

	-- purpose	: main
	-- type		: sequential
	-- inputs	: resetI, clockI
	-- outputs
	FiniteStateMachine	: process (resetI, clockI)
		variable i_fcs_t16		: std_logic_vector(15 downto 0)	:= (others => '1');
		variable i_reg_t24		: std_logic_vector(23 downto 0)	:= (others => '1');
		variable i_counter_ti	: integer range 0 to 3	:= 0;

	begin
		if resetI = '1' then
			i_state			<= STATE_IDLE;
			writeBuffO		<= '0';		--'Z';
			i_writeBuff		<= '0';
			i_priorWrite	<= '0';
			headO_t16		<= (others => '0');		--'Z');
			rxO_t8			<= (others => '0');		--'Z');
			eotO			<= '0';		--'Z';
			i_eot			<= '0';
			fcsErrorO		<= '0';					--'Z';
			i_fcs_t16		:= (others => '1');
			i_reg_t24		:= (others => '1');
			i_counter_ti	:= 0;
		elsif rising_edge(clockI) then
			if enableI = '1' then
				if fcsEnableI = '1' then
					rxO_t8			<= i_reg_t24(23 downto 16);
				else
					rxO_t8			<= rxI_t8;
				end if;

				i_writeBuff		<= i_priorWrite;
				writeBuffO		<= i_writeBuff;
				eotO			<= i_eot;
				--fcsM_t16		<= i_fcs_t16;

				case i_state is
					when STATE_IDLE =>
						i_writeBuff		<= '0';
						i_priorWrite	<= '0';
						readByteO		<= '0';
						i_eot			<= '0';
						i_fcs_t16		:= (others => '1');
						i_reg_t24		:= (others => '1');
						i_counter_ti	:= 0;

						if validFrameI = '1' then
							i_state			<= STATE_RUN;
							i_eot			<= '0';
							headO_t16		<= (others => '0');
						else
							i_state			<= STATE_IDLE;
						end if;

					when STATE_RUN =>
						--i_state			<= STATE_RUN1;

					--when STATE_RUN1 =>
						--i_state			<= STATE_RUN2;

					--when STATE_RUN2 =>
						if i_writeBuff = '1' and fcsEnableI = '1' then
							i_fcs_t16		:= NextCrc16d8(i_reg_t24(23 downto 16), i_fcs_t16);
						end if;
						fcsErrorO		<= '0';
						--i_writeBuff		<= '0';
						i_priorWrite	<= '0';

						if validFrameI = '1' then
							if readyI = '1' then
								i_state			<= STATE_READ;
								readByteO		<= '0';
							else
								i_state			<= STATE_RUN;
								readByteO		<= '0';
							end if;
						else
							if i_counter_ti = 0 then
								i_state			<= STATE_IDLE;
							else
								i_state			<= STATE_EOF1;
								readByteO		<= '0';
								i_eot			<= '1';
							end if;
						end if;

					when STATE_EOF1 =>
						i_eot			<= '1';
						if fcsEnableI = '1' then
							i_fcs_t16		:= NextCrc16d8(i_reg_t24(15 downto 8), i_fcs_t16);
							i_state			<= STATE_EOF2;
						else
							i_state			<= STATE_EOFZ;
						end if;

					when STATE_EOF2 =>
						i_eot			<= '1';
						i_fcs_t16		:= NextCrc16d8(i_reg_t24(7 downto 0), i_fcs_t16);
						i_state			<= STATE_EOFZ;

					when STATE_EOFZ =>
						fcsErrorO		<= fcsEnableI and
											not	(not i_fcs_t16(15) and not i_fcs_t16(14) and not i_fcs_t16(13) and     i_fcs_t16(12) and
												     i_fcs_t16(11) and     i_fcs_t16(10) and not i_fcs_t16(9)  and     i_fcs_t16(8)  and
												 not i_fcs_t16(7)  and not i_fcs_t16(6)  and not i_fcs_t16(5)  and not i_fcs_t16(4)  and
												     i_fcs_t16(3)  and     i_fcs_t16(2)  and     i_fcs_t16(1)  and     i_fcs_t16(0));
											-- x"1d0f"
						i_state			<= STATE_IDLE;
						readByteO		<= '0';
						i_writeBuff		<= '0';
						i_priorWrite	<= '0';
						i_eot			<= '0';

					when STATE_READ =>
						--i_eot			<= '0';

						if readyI = '1' then
							i_state			<= STATE_READ;
							i_writeBuff		<= '0';
							i_priorWrite	<= '0';
							readByteO		<= '1';
						else
							i_state			<= STATE_RUN;
							readByteO		<= '0';

							case i_counter_ti is
								when 0 =>	headO_t16(7 downto 0)	<= rxI_t8;
								when 1 =>	headO_t16(15 downto 8)	<= rxI_t8;
								when others =>	null;
							end case;

							if fcsEnableI = '1' then
								i_reg_t24		:= i_reg_t24(15 downto 0) & rxI_t8;
								if i_counter_ti > 1 then
									--i_writeBuff		<= '1';
									i_priorWrite	<= '1';
								end if;
							else
								--i_writeBuff		<= '1';
								i_priorWrite	<= '1';
							end if;

							if i_counter_ti < 2 then
								i_counter_ti	:= i_counter_ti + 1;
							end if;
						end if;

					when others => null;
				end case;
			else
				writeBuffO		<= '0';		--'Z';
				headO_t16		<= (others => '0');		--'Z');
				rxO_t8			<= (others => '0');		--'Z');
				eotO			<= '0';		--'Z';
				fcsErrorO		<= '0';		--'Z';
			end if;
		end if;
	end process FiniteStateMachine;
end behavioral;
