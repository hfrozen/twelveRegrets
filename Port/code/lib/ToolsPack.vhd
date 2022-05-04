-------------------------------------------------------------------------------
-- Title	: Tools Package
-- Project	: Utility library
-- File		: ToolsPack.vhd
-------------------------------------------------------------------------------
library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;


package ToolsPack is
-- Types
-- Memory arraye type of std_logic_vector
--	type std_memory_array_typ is array (integer range <>) of std_logic_vector(5 downto 0);	--integer range <>);
-- Memory arraye type of std_ulogic_vector
--	type stdu_memory_array_typ is array (integer range <>) of std_ulogic_vector(integer range <>);
-- Sign magnitude numbers based on std_logic_vector (The msb represents the sign)

	type _SIGN_MAG is array (natural range <>) of std_logic;

	function Log2(in_ti : integer) return integer; 							-- log2 functions
	function Slv2Int(slv_tv : std_logic_vector) return integer;				--std_logic_vector to integer
	function Int2Slv (in_ti, size_ti : integer) return std_logic_vector;
	function "+"(a, b : _SIGN_MAG) return _SIGN_MAG;						-- sign_magnitude addition
	function "-"(a, b : _SIGN_MAG) return _SIGN_MAG;						-- sign_magnitude subtraction (based on complement operations)
	function LeftShift(reg_tv, size_tv : std_logic_vector) return std_logic_vector;
	function RightShift(reg_tv, size_tv : std_logic_vector) return std_logic_vector;
end ToolsPack;

package body ToolsPack is

	function Log2(in_ti : integer) return integer is
		variable _tmp_ti, _log_ti	: integer;
	begin
		assert in_ti /= 0
			report "Error : function missuse : log2(zero)"
			severity failure;

		_tmp_ti	:= in_ti;
		_log_ti	:= 0;
		while(_tmp_ti /= 0) loop
			_tmp_ti	:= _tmp_ti / 2;
			_log_ti	:= _log_ti + 1;
		end loop;
		return _log_ti;
	end Log2;

	function Slv2Int(slv_tv : std_logic_vector) return integer is
		variable _res_ti : integer := 0;		 -- conversion result
	begin
		for i in slv_tv'range loop
			_res_ti	:= _res_ti * 2;	-- shift the variable to left
			case slv_tv(i) is
				when '1' | 'H' =>	_res_ti	:= _res_ti + 1;
				when '0' | 'L' =>	_res_ti	:= _res_ti + 0;
				when others	=>	null;
			end case;
		end loop;
		return _res_ti;
	end Slv2Int;

	function Int2Slv(in_ti, size_ti : integer) return std_logic_vector is
		variable _res_tv	: std_logic_vector(size_ti - 1 downto 0);
		variable _tmp_ti	: integer := in_ti;
	begin
		assert size_ti > 1
			report "Error : function missuse : in_2_slv(val, negative size)"
			severity failure;
		for i in 0 to _res_tv'length - 1 loop
			if (_tmp_ti mod 2) = 0 then
				_res_tv(i)	:= '0';
			else
				_res_tv(i)	:= '1';
			end if;
			_tmp_ti	:= _tmp_ti / 2;
		end loop;
		return _res_tv;
	end Int2Slv;

	function "+"(a, b : _SIGN_MAG) return _SIGN_MAG is
		variable _va_tui, _vb_tui, _vr_tui	: unsigned(a'length - 1 downto 0);
		variable _sa, _sb, _sr	: std_logic;
		variable _tmp_ar, _res_ar	: _SIGN_MAG(a'length - 1 downto 0);
		variable _var_t2	: std_logic_vector(1 downto 0);
		variable _std_ar	: std_logic_vector(a'length - 1 downto 0)	:= (others => '0');
	begin
		assert a'length = b'length
			report "Error : length mismatch"
			severity failure;
		_tmp_ar	:= a;
		_sa		:= _tmp_ar(a'length - 1);
		_va_tui	:= '0' & unsigned(_tmp_ar(a'length - 2 downto 0));
		_tmp_ar	:= b;
		_sb		:= _tmp_ar(b'length - 1);
		_vb_tui	:= '0' & unsigned(_tmp_ar(b'length - 2 downto 0));
		_var_t2	:= _sa & _sb;
		case _var_t2 is
			when "00" |"11" =>
				_vr_tui	:= _va_tui + _vb_tui;
				_sr		:= _sa;

			when "01" =>
				_vr_tui	:= _va_tui - _vb_tui;
				_sr		:= _vr_tui(_vr_tui'length - 1);

				if _sr = '1' then
					_std_ar(_vr_tui'length - 2 downto 0)	:= std_logic_vector(_vr_tui(_vr_tui'length - 2 downto 0));
					_std_ar									:= not _std_ar;
					_vr_tui(_vr_tui'length - 2 downto 0)	:= unsigned(_std_ar(_vr_tui'length - 2 downto 0));
					_vr_tui(_vr_tui'length - 2 downto 0)	:= _vr_tui(_vr_tui'length - 2 downto 0) + 1;
				end if;

			when "10" =>
				_vr_tui	:= _vb_tui - _va_tui;
				_sr		:= _vr_tui(_vr_tui'length - 1);

				if _sr = '1' then
					_std_ar(_vr_tui'length - 2 downto 0)	:= std_logic_vector(_vr_tui(_vr_tui'length - 2 downto 0));
					_std_ar									:= not _std_ar;
					_vr_tui(_vr_tui'length - 2 downto 0)	:= unsigned(_std_ar(_vr_tui'length - 2 downto 0));
					_vr_tui(_vr_tui'length - 2 downto 0)	:= _vr_tui(_vr_tui'length - 2 downto 0) + 1;
				end if;

			when others => null;
		end case;
		_res_ar	:= _SIGN_MAG(_sr & _vr_tui(_vr_tui'length - 2 downto 0));
		return _res_ar;
	end "+";

	function "-"(a, b : _SIGN_MAG) return _SIGN_MAG is
		variable _tmp_ar	: _SIGN_MAG(a'length - 1 downto 0);
	begin
		assert a'length = b'length
			report "Error : length mismach"
			severity failure;
		_tmp_ar					:= b;
		_tmp_ar(b'length - 1)	:= not _tmp_ar(b'length - 1);
		return a + _tmp_ar;
	end "-";

	function LeftShift(reg_tv, size_tv : std_logic_vector)	return std_logic_vector is
		constant _cSize_ti		: integer	:= reg_tv'length;
		variable _var_tv		: std_logic_vector(reg_tv'length - 1 downto 0);
		constant _cShiftSize_ti	: integer	:= Log2(reg_tv'length);
	begin
		_var_tv	:= reg_tv;
		for i in 0 to _cShiftSize_ti - 2 loop
			if size_tv(i) = '1' then
				_var_tv(_cSize_ti - 1 downto 0)	:= _var_tv((_cSize_ti - (2 ** i) - 1) downto 0) & ((2 ** i) - 1 downto 0	=> '0');
			end if;
		end loop;

		if size_tv(_cShiftSize_ti - 1) = '1' then
			_var_tv	:= (others => '0');
		end if;
		return _var_tv;
	end LeftShift;

	function RightShift(reg_tv, size_tv : std_logic_vector)	return std_logic_vector is
		constant _cSize_ti		: integer	:= reg_tv'length;
		variable _var_tv		: std_logic_vector(reg_tv'length - 1 downto 0);
		constant _cShiftSize_ti	: integer := Log2(reg_tv'length);
	begin
		_var_tv	:= reg_tv;
		for i in 0 to _cShiftSize_ti - 2 loop
			if size_tv(i) = '1' then
				_var_tv(_cSize_ti - 1 downto 0) := (_cSize_ti - 1 downto _cSize_ti-(2 ** i)	=> '0') & _var_tv(_cSize_ti - 1 downto (2 ** i));
			end if;
		end loop;

		if size_tv(_cShiftSize_ti - 1) = '1' then
			_var_tv	:= (others => '0');
		end if;
		return _var_tv;
	end RightShift;
end ToolsPack;
