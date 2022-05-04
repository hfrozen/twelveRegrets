-------------------------------------------------------------------------------
-- Title	: tx frame check sequence
-- Project	: serial communication
-- File		: TxFcs.vhd
-------------------------------------------------------------------------------

library ieee;
use ieee.std_logic_1164.all;

use work.CalcCrc16d8.all;

entity TxFcs is
	generic (
		cFcsType_ti		: integer	:= 2
	);
	port (
		resetI			: in	std_logic;
		clockI	: in	std_logic;
		enableI			: in	std_logic;
		fcsEnableI		: in	std_logic;						-- bus
		readyI			: in	std_logic;						-- tx synch
		txAvailI		: in	std_logic;						-- tx buff
		txI_t8			: in	std_logic_vector(7 downto 0);	-- tx buff
		validFrameO		: out	std_logic;						-- tx synch
		writeByteO		: out	std_logic;						-- tx synch
		readBuffO		: out	std_logic;						-- tx buff
		--stateM_t4		: out	std_logic_vector(3 downto 0);	-- ?????
		txO_t8			: out	std_logic_vector(7 downto 0)	-- tx synch
	);
end TxFcs;

architecture behavioral of TxFcs is
begin
	-- purpose	: main
	-- type		: sequential
	-- inputs	: resetI, clockI
	-- outputs
	FiniteStateMachine	: process(resetI, clockI)
		variable i_fcs_t16	: std_logic_vector(15 downto 0);
		type STATE_TYPES is (
			STATE_IDLE, STATE_READ, STATE_READ1, --STATE_READ2,
			STATE_READYLOW, STATE_WAIT, STATE_CLOSEBODY, STATE_FCS0,
			STATE_CLOSEFCS0, STATE_FCS1, STATE_CLOSEFCS1
		);
		variable i_state	: STATE_TYPES;
		variable i_dreg_t8	: std_logic_vector(7 downto 0);		-- input data to fcs register

	begin
		if resetI = '1' then
			i_dreg_t8		:= (others => '0');
			i_fcs_t16		:= (others => '1');
			validFrameO		<= '0';		--'Z';
			writeByteO		<= '0';		--'Z';
			txO_t8			<= (others => '0');		--'Z');
			readBuffO		<= '0';		--'Z';
			i_state			:= STATE_IDLE;
			--stateM_t4		<= "0000";
		elsif rising_edge(clockI) then
			if enableI = '1' then
				case i_state is
					when STATE_IDLE =>
						i_fcs_t16	:= (others => '1');
						i_dreg_t8	:= (others => '1');
						validFrameO	<= txAvailI;			--'0';
						writeByteO	<= '0';
						txO_t8		<= (others => '0');

						if readyI = '1' and txAvailI = '1' then		-- !!!!!
							i_state		:= STATE_READ;
							--stateM_t4	<= "0001";
							readBuffO	<= '1';			-- to tx buffer
						else
							i_state		:= STATE_IDLE;
							--stateM_t4	<= "0000";
							readBuffO	<= '0';
						end if;

					when STATE_READ =>
						--readBuffO	<= '0';
						--i_state		:= STATE_READ1;		-- if this is not, step A may be ignored, then txO_t8 is validated in step B

					--when STATE_READ1 =>
						--i_state		:= STATE_READ2;

					--when STATE_READ2 =>
						i_dreg_t8	:= txI_t8;
						txO_t8		<= txI_t8;		-- step A
						i_fcs_t16	:= NextCrc16d8(i_dreg_t8, i_fcs_t16);	-- fcs calc.
						validFrameO	<= '1';
						writeByteO	<= '1';			-- to zero insert
						readBuffO	<= '0';			-- 190517, this step not occurs inc
						i_state		:= STATE_READYLOW;
						--stateM_t4	<= "0010";

					when STATE_READYLOW =>	-- this state does nothing but registers all output signals till ack is valid
						--txO_t8		<= txI_t8;		-- step A
						readBuffO	<= '0';
						if readyI = '0' then
							i_state	:= STATE_WAIT;
							--stateM_t4	<= "0011";
						else
							i_state	:= STATE_READYLOW;
							--stateM_t4	<= "0010";
						end if;

					when STATE_WAIT =>
						validFrameO	<= '1';
						writeByteO	<= '0';

						if txAvailI = '1' then
							--txO_t8		<= txI_t8;		-- step B
							--i_dreg_t8	:= txI_t8;

							if readyI = '1' then
								readBuffO	<= '1';
								i_state		:= STATE_READ;
								--stateM_t4	<= "0001";
							else
								readBuffO	<= '0';
								i_state		:= STATE_WAIT;
								--stateM_t4	<= "0011";
							end if;
						else
							if fcsEnableI = '1' then	-- tx end & fcs enable then put crc16
								txO_t8		<= (others => '1');
								i_dreg_t8	:= (others => '1');
								readBuffO	<= '0';
								i_state		:= STATE_CLOSEBODY;
								--stateM_t4	<= "0100";
							else
								txO_t8		<= (others => '1');
								i_dreg_t8	:= (others => '1');
								i_fcs_t16	:= (others => '1');
								readBuffO	<= '0';
								i_state		:= STATE_IDLE;
								--stateM_t4	<= "0000";
							end if;
						end if;

					when STATE_CLOSEBODY =>
						i_dreg_t8	:= (others => '1');
						validFrameO	<= '1';
						writeByteO	<= '0';
						txO_t8		<= (others => '1');
						readBuffO	<= '0';
						i_state		:= STATE_FCS0;
						--stateM_t4	<= "0101";

					when STATE_FCS0 =>
						i_dreg_t8	:= (others => '1');
						validFrameO	<= '1';
						readBuffO	<= '0';
						txO_t8(0)	<= not i_fcs_t16(15);
						txO_t8(1)	<= not i_fcs_t16(14);
						txO_t8(2)	<= not i_fcs_t16(13);
						txO_t8(3)	<= not i_fcs_t16(12);
						txO_t8(4)	<= not i_fcs_t16(11);
						txO_t8(5)	<= not i_fcs_t16(10);
						txO_t8(6)	<= not i_fcs_t16(9);
						txO_t8(7)	<= not i_fcs_t16(8);

						if readyI = '1' then
							writeByteO	<= '1';
							i_state		:= STATE_CLOSEFCS0;
							--stateM_t4	<= "0110";
						else
							writeByteO	<= '0';
							i_state		:= STATE_FCS0;
							--stateM_t4	<= "0101";
						end if;

					when STATE_CLOSEFCS0 =>	-- this state does nothing but registers all output signals till ack is valid
						if readyI = '0' then
							i_state	:= STATE_FCS1;
							--stateM_t4	<= "0111";
						else
							i_state	:= STATE_CLOSEFCS0;
							--stateM_t4	<= "0110";
						end if;

					when STATE_FCS1	=>
						i_dreg_t8	:= (others => '1');
						validFrameO	<= '1';
						readBuffO	<= '0';
						txO_t8(0)	<= not i_fcs_t16(7);
						txO_t8(1)	<= not i_fcs_t16(6);
						txO_t8(2)	<= not i_fcs_t16(5);
						txO_t8(3)	<= not i_fcs_t16(4);
						txO_t8(4)	<= not i_fcs_t16(3);
						txO_t8(5)	<= not i_fcs_t16(2);
						txO_t8(6)	<= not i_fcs_t16(1);
						txO_t8(7)	<= not i_fcs_t16(0);

						if readyI = '1' then
							writeByteO	<= '1';
							i_state		:= STATE_CLOSEFCS1;
							--stateM_t4	<= "1000";
						else
							writeByteO	<= '0';
							i_state		:= STATE_FCS1;
							--stateM_t4	<= "0111";
						end if;

					when STATE_CLOSEFCS1 =>	-- this state does nothing but registers all output signals till ack is valid
						if readyI = '0' then
							i_state	:= STATE_IDLE;
							--stateM_t4	<= "0000";
						else
							i_state	:= STATE_CLOSEFCS1;
							--stateM_t4	<= "1000";
						end if;

					when others => null;
				end case;
			else
				validFrameO		<= '0';		--'Z';
				writeByteO		<= '0';		--'Z';
				txO_t8			<= (others => '0');		--'Z');
				readBuffO		<= '0';		--'Z';
			end if;
		end if;		-- rising_edge(clockI)
	end process FiniteStateMachine;
end behavioral;
