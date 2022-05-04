-------------------------------------------------------------------------------
-- Title	: crc 16 calcuration
-- Project	: serial communication
-- File		: CalcCrc16d8.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;

package CalcCrc16d8 is
	-- polynomial	: (0 5 12 16)
	-- data width	: 8
	-- convention	: the first serial data bit is D(7)
	function NextCrc16d8(
		data	: std_logic_vector(7 downto 0);
		crc		: std_logic_vector(15 downto 0)
	)
	return std_logic_vector;
end CalcCrc16d8;

library ieee;
use ieee.std_logic_1164.all;

package body CalcCrc16d8 is
	function NextCrc16d8(
		data	: std_logic_vector(7 downto 0);
		crc		: std_logic_vector(15 downto 0)
	)
	return std_logic_vector is

	variable i_d_t8		: std_logic_vector(7 downto 0);
	variable i_c_t16	: std_logic_vector(15 downto 0);
	variable i_crc_t16	: std_logic_vector(15 downto 0);

	begin
		i_d_t8(0)	:= data(7);
		i_d_t8(1)	:= data(6);
		i_d_t8(2)	:= data(5);
		i_d_t8(3)	:= data(4);
		i_d_t8(4)	:= data(3);
		i_d_t8(5)	:= data(2);
		i_d_t8(6)	:= data(1);
		i_d_t8(7)	:= data(0);
		i_c_t16		:= crc;

		i_crc_t16(0)	:= i_d_t8(4) xor i_d_t8(0) xor i_c_t16(8) xor i_c_t16(12);
		i_crc_t16(1)	:= i_d_t8(5) xor i_d_t8(1) xor i_c_t16(9) xor i_c_t16(13);
		i_crc_t16(2)	:= i_d_t8(6) xor i_d_t8(2) xor i_c_t16(10) xor i_c_t16(14);
		i_crc_t16(3)	:= i_d_t8(7) xor i_d_t8(3) xor i_c_t16(11) xor i_c_t16(15);
		i_crc_t16(4)	:= i_d_t8(4) xor i_c_t16(12);
		i_crc_t16(5)	:= i_d_t8(5) xor i_d_t8(4) xor i_d_t8(0) xor i_c_t16(8) xor i_c_t16(12) xor i_c_t16(13);
		i_crc_t16(6)	:= i_d_t8(6) xor i_d_t8(5) xor i_d_t8(1) xor i_c_t16(9) xor i_c_t16(13) xor i_c_t16(14);
		i_crc_t16(7)	:= i_d_t8(7) xor i_d_t8(6) xor i_d_t8(2) xor i_c_t16(10) xor i_c_t16(14) xor i_c_t16(15);
		i_crc_t16(8)	:= i_d_t8(7) xor i_d_t8(3) xor i_c_t16(0) xor i_c_t16(11) xor i_c_t16(15);
		i_crc_t16(9)	:= i_d_t8(4) xor i_c_t16(1) xor i_c_t16(12);
		i_crc_t16(10)	:= i_d_t8(5) xor i_c_t16(2) xor i_c_t16(13);
		i_crc_t16(11)	:= i_d_t8(6) xor i_c_t16(3) xor i_c_t16(14);
		i_crc_t16(12)	:= i_d_t8(7) xor i_d_t8(4) xor i_d_t8(0) xor i_c_t16(4) xor i_c_t16(8) xor i_c_t16(12) xor i_c_t16(15);
		i_crc_t16(13)	:= i_d_t8(5) xor i_d_t8(1) xor i_c_t16(5) xor i_c_t16(9) xor i_c_t16(13);
		i_crc_t16(14)	:= i_d_t8(6) xor i_d_t8(2) xor i_c_t16(6) xor i_c_t16(10) xor i_c_t16(14);
		i_crc_t16(15)	:= i_d_t8(7) xor i_d_t8(3) xor i_c_t16(7) xor i_c_t16(11) xor i_c_t16(15);
		return i_crc_t16;
	end NextCrc16d8;
end CalcCrc16d8;
