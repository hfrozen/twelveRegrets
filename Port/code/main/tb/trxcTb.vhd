library ieee;
use ieee.std_logic_1164.all;

entity trxcTb is
end trxcTb;

architecture behavior of trxcTb is

	component MemMap
	port (
		resetI					: in  std_logic;
		clock147456kI			: in  std_logic;
		busCsI					: in  std_logic;
		busReadI				: in  std_logic;
		busWriteI				: in  std_logic;
		busAddrI_t8				: in  std_logic_vector(7 downto 0);
		busDataIO_t16			: inout  std_logic_vector(15 downto 0);
		busInterruptO			: out  std_logic;
		hdlcO					: out  std_logic;
		dpllEnableO				: out  std_logic;
		addrMatch_stoplO		: out  std_logic;
		manchester_oddpO		: out  std_logic;
		nrzi_evenpO				: out  std_logic;
		fcsEnableO				: out  std_logic;
		baudrateConstantO_ti	: out  std_logic_vector(0 to 7);
		txEmptyI				: in  std_logic;
		txHaltI					: in  std_logic;
		txEnableO				: out  std_logic;
		txWriteO				: out  std_logic;
		txAbortO				: out  std_logic;
		txFlagSizeRxHoldSize_t4	: out  std_logic_vector(3 downto 0);
		txFrameSizeO_tv			: out  std_logic_vector(9 downto 0);
		txdO_t16				: out  std_logic_vector(15 downto 0);
		rxFullI					: in  std_logic;
		rxOverflowErrorI		: in  std_logic;
		rxOverlapErrorI			: in  std_logic;
		rxOverRunErrorI			: in  std_logic;
		rxFrameErrorI			: in  std_logic;
		rxFcsErrorI				: in  std_logic;
		rxParityErrorI			: in  std_logic;
		rxAbortI				: in  std_logic;
		rxFrameSizeI_tv			: in  std_logic_vector(9 downto 0);
		rxAddressI_t16			: in  std_logic_vector(15 downto 0);
		rxdI_t16				: in  std_logic_vector(15 downto 0);
		rxEnableO				: out  std_logic;
		rxReadO					: out  std_logic;
		rxIgnoreO				: out  std_logic
	);
	end component;

	--Inputs
	signal resetI : std_logic := '0';
	signal clock147456kI : std_logic := '0';
	signal busCsI : std_logic := '0';
	signal busReadI : std_logic := '0';
	signal busWriteI : std_logic := '0';
	signal busAddrI_t8 : std_logic_vector(7 downto 0) := (others => '0');
	signal txEmptyI : std_logic := '0';
	signal txHaltI : std_logic := '0';
	signal rxFullI : std_logic := '0';
	signal rxOverflowErrorI : std_logic := '0';
	signal rxOverlapErrorI : std_logic := '0';
	signal rxOverRunErrorI : std_logic := '0';
	signal rxFrameErrorI : std_logic := '0';
	signal rxFcsErrorI : std_logic := '0';
	signal rxParityErrorI : std_logic := '0';
	signal rxAbortI : std_logic := '0';
	signal rxFrameSizeI_tv : std_logic_vector(9 downto 0) := (others => '0');
	signal rxAddressI_t16 : std_logic_vector(15 downto 0) := (others => '0');
	signal rxdI_t16 : std_logic_vector(15 downto 0) := (others => '0');

	--BiDirs
	signal busDataIO_t16 : std_logic_vector(15 downto 0);

 	--Outputs
	signal busInterruptO : std_logic;
	signal hdlcO : std_logic;
	signal dpllEnableO : std_logic;
	signal addrMatch_stoplO : std_logic;
	signal manchester_oddpO : std_logic;
	signal nrzi_evenpO : std_logic;
	signal fcsEnableO : std_logic;
	signal baudrateConstantO_ti : std_logic_vector(0 to 7);
	signal txEnableO : std_logic;
	signal txWriteO : std_logic;
	signal txAbortO : std_logic;
	signal txFlagSizeRxHoldSize_t4 : std_logic_vector(3 downto 0);
	signal txFrameSizeO_tv : std_logic_vector(9 downto 0);
	signal txdO_t16 : std_logic_vector(15 downto 0);
	signal rxEnableO : std_logic;
	signal rxReadO : std_logic;
	signal rxIgnoreO : std_logic;
	-- No clocks detected in port list. Replace <clock> below with
	-- appropriate port name

	constant <clock>_period : time := 10 ns;

begin

	-- Instantiate the Unit Under Test (UUT)
	uut: MemMap
	port map (
		resetI					=> resetI,
		clock147456kI			=> clock147456kI,
		busCsI					=> busCsI,
		busReadI				=> busReadI,
		busWriteI				=> busWriteI,
		busAddrI_t8				=> busAddrI_t8,
		busDataIO_t16			=> busDataIO_t16,
		busInterruptO			=> busInterruptO,
		hdlcO					=> hdlcO,
		dpllEnableO				=> dpllEnableO,
		addrMatch_stoplO		=> addrMatch_stoplO,
		manchester_oddpO		=> manchester_oddpO,
		nrzi_evenpO				=> nrzi_evenpO,
		fcsEnableO				=> fcsEnableO,
		baudrateConstantO_ti	=> baudrateConstantO_ti,
		txEmptyI				=> txEmptyI,
		txHaltI					=> txHaltI,
		txEnableO				=> txEnableO,
		txWriteO				=> txWriteO,
		txAbortO				=> txAbortO,
		txFlagSizeRxHoldSize_t4	=> txFlagSizeRxHoldSize_t4,
		txFrameSizeO_tv			=> txFrameSizeO_tv,
		txdO_t16				=> txdO_t16,
		rxFullI					=> rxFullI,
		rxOverflowErrorI		=> rxOverflowErrorI,
		rxOverlapErrorI			=> rxOverlapErrorI,
		rxOverRunErrorI			=> rxOverRunErrorI,
		rxFrameErrorI			=> rxFrameErrorI,
		rxFcsErrorI				=> rxFcsErrorI,
		rxParityErrorI			=> rxParityErrorI,
		rxAbortI				=> rxAbortI,
		rxFrameSizeI_tv			=> rxFrameSizeI_tv,
		rxAddressI_t16			=> rxAddressI_t16,
		rxdI_t16				=> rxdI_t16,
		rxEnableO				=> rxEnableO,
		rxReadO					=> rxReadO,
		rxIgnoreO				=> rxIgnoreO
	);
end;
