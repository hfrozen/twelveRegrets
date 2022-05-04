-------------------------------------------------------------------------------
-- Title	: hdlc node package
-- Project	: serial communication
-- File		: NodePack.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

package NodePack is
	component Nrz
	port (
		resetI		: in	std_logic;
		txClockI	: in	std_logic;
		rxClockI	: in	std_logic;
		txI			: in	std_logic;
		rxI			: in	std_logic;
		txO			: out	std_logic;
		rxO			: out	std_logic
	);
	end component;

	component Nrzi
	port (
		resetI		: in	std_logic;
		txClockI	: in	std_logic;
		rxClockI	: in	std_logic;
		txI			: in	std_logic;
		rxI			: in	std_logic;
		txO			: out	std_logic;
		rxO			: out	std_logic
	);
	end component;

	component Manchester
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
	end component;

	component RxPhaseSynch
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
	end component;

	--component Dpll
	--port (
	--	clockI	: in	std_logic;
	--	fI		: in	std_logic;
	--	fO		: inout	std_logic
	--);
	--end component;

	--component Dpll16
	--port (
	--	clock16I	: in	std_logic;
	--	clockI		: in	std_logic;
	--	clockO		: out	std_logic
	--);
	--end component;

	--component Adjuster
	--port (
	--	clock16I	: in	std_logic;
	--	clockInI	: in	std_logic;
	--	clockOutI	: in	std_logic;
	--	enableO		: out	std_logic;
	--	accelO		: out	std_logic
	--);
	--end component;

	--component Divider16
	--port (
	--	clock16I	: in	std_logic;
	--	enableI		: in	std_logic;
	--	accelI		: in	std_logic;
	--	clockO		: out	std_logic
	--);
	--end component;
end package;
