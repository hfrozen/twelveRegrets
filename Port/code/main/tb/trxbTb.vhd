--------------------------------------------------------------------------------
-- Company: 
-- Engineer:
--
-- Create Date:   14:22:52 06/29/2018
-- Design Name:   
-- Module Name:   D:/Vhdlsx/Port4/code/main/tb/trxbTb.vhd
-- Project Name:  Port
-- Target Device:  
-- Tool versions:  
-- Description:   
-- 
-- VHDL Test Bench Created by ISE for module: Platform
-- 
-- Dependencies:
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
--
-- Notes: 
-- This testbench has been automatically generated using types std_logic and
-- std_logic_vector for the ports of the unit under test.  Xilinx recommends
-- that these types always be used for the top-level I/O of a design in order
-- to guarantee that the testbench will bind correctly to the post-implementation 
-- simulation model.
--------------------------------------------------------------------------------
LIBRARY ieee;
USE ieee.std_logic_1164.ALL;
 
-- Uncomment the following library declaration if using
-- arithmetic functions with Signed or Unsigned values
--USE ieee.numeric_std.ALL;
 
ENTITY trxbTb IS
END trxbTb;
 
ARCHITECTURE behavior OF trxbTb IS 
 
    -- Component Declaration for the Unit Under Test (UUT)
 
    COMPONENT Platform
    PORT(
         clock147456kI : IN  std_logic;
         rxI : IN  std_logic;
         txO : OUT  std_logic;
         rtsO : OUT  std_logic;
         rxlpO : OUT  std_logic;
         txlpO : OUT  std_logic;
         txAddrM_tv : OUT  std_logic_vector(9 downto 0);
         txbM_t8 : OUT  std_logic_vector(7 downto 0);
         txAvailM : OUT  std_logic;
         resetI : IN  std_logic;
         busCsI : IN  std_logic;
         busReadI : IN  std_logic;
         busWriteI : IN  std_logic;
         busAddrI_t8 : IN  std_logic_vector(7 downto 0);
         busDataIO_t16 : INOUT  std_logic_vector(15 downto 0);
         busInterruptO : OUT  std_logic
        );
    END COMPONENT;
    

   --Inputs
   signal clock147456kI : std_logic := '0';
   signal rxI : std_logic := '0';
   signal resetI : std_logic := '0';
   signal busCsI : std_logic := '0';
   signal busReadI : std_logic := '0';
   signal busWriteI : std_logic := '0';
   signal busAddrI_t8 : std_logic_vector(7 downto 0) := (others => '0');

	--BiDirs
   signal busDataIO_t16 : std_logic_vector(15 downto 0);

 	--Outputs
   signal txO : std_logic;
   signal rtsO : std_logic;
   signal rxlpO : std_logic;
   signal txlpO : std_logic;
   signal txAddrM_tv : std_logic_vector(9 downto 0);
   signal txbM_t8 : std_logic_vector(7 downto 0);
   signal txAvailM : std_logic;
   signal busInterruptO : std_logic;

   -- Clock period definitions
   constant clock147456kI_period : time := 10 ns;
 
BEGIN
 
	-- Instantiate the Unit Under Test (UUT)
   uut: Platform PORT MAP (
          clock147456kI => clock147456kI,
          rxI => rxI,
          txO => txO,
          rtsO => rtsO,
          rxlpO => rxlpO,
          txlpO => txlpO,
          txAddrM_tv => txAddrM_tv,
          txbM_t8 => txbM_t8,
          txAvailM => txAvailM,
          resetI => resetI,
          busCsI => busCsI,
          busReadI => busReadI,
          busWriteI => busWriteI,
          busAddrI_t8 => busAddrI_t8,
          busDataIO_t16 => busDataIO_t16,
          busInterruptO => busInterruptO
        );

   -- Clock process definitions
   clock147456kI_process :process
   begin
		clock147456kI <= '0';
		wait for clock147456kI_period/2;
		clock147456kI <= '1';
		wait for clock147456kI_period/2;
   end process;
 

   -- Stimulus process
   stim_proc: process
   begin		
      -- hold reset state for 100 ns.
      wait for 100 ns;	

      wait for clock147456kI_period*10;

      -- insert stimulus here 

      wait;
   end process;

END;
