-- Digital PLL based on the 74LS297 architecture

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_misc.all;
use IEEE.std_logic_unsigned.all;

entity PLL1 is
    port (
        clockI: in STD_LOGIC;		-- clock for k and id counter
        fI: in STD_LOGIC;		-- input signal
        fO: inout STD_LOGIC		-- output signal
    );
end PLL1;

architecture PLL1_arch of PLL1 is

-- enter PLL parameters here

constant i_cKBit_ti:	integer:=8;		-- bitwidth of the K_counter
constant i_cNModul_ti: integer:=9;		-- modul of N_counter
constant i_cFinModul_ti: integer:=8;		-- modul of reference prescaler
constant i_cFoutModul_ti: integer:=8;		-- modul of DCO_postscaler

-- do not change source code below here ((uunless you know what you are doing ;-)

signal 	i_kDirection: std_logic;		-- selects up or down counter of k_counter
signal	i_kCarry: std_logic;			-- carry out of k_counter
signal	i_kBorrow: std_logic;			-- borrow out of k_counter

signal i_nCount_ti: integer range 0 to i_cNModul_ti-1;	-- N counter
signal i_finScale_ti: integer range 0 to i_cFinModul_ti-1;	-- reference prescaler
signal i_foutScale_ti: integer range 0 to i_cFoutModul_ti-1;	-- DCO postscaler
signal i_finScaleOut, i_foutScaleOut:	std_logic;			-- prescaler outputs

signal i_kupCount_tv:	std_logic_vector (i_cKBit_ti-1 downto 0); 	-- up counter in k_counter
signal i_kdownCount_tv:	std_logic_vector (i_cKBit_ti-1 downto 0);     -- down counter in k_counter

signal c1,c2,c3,c4: std_logic;		-- shift register of carry
signal b1,b2,b3,b4: std_logic;		-- shift register of borrow
signal i_idCountOut: std_logic;		-- output of id_counter

begin

-- K_counter

  k_cnt: process (clockI)
  begin
    if falling_edge(clockI) then
      if i_kDirection='1' then
        i_kdownCount_tv<=i_kdownCount_tv+1;
      else
        i_kupCount_tv<=i_kupCount_tv+1;
      end if;
    end if;
  end process;

  i_kCarry<=i_kupCount_tv(i_cKBit_ti-1);
  i_kBorrow<=i_kdownCount_tv(i_cKBit_ti-1);

-- ID_counter

  id_cnt: process (clockI)
  begin

    if clockI='1' and clockI'event then  -- positive edge controlled

      c1<=i_kCarry;
      c2<=c1;
      c3<=c2;
      c4<= ( (not c1) and c2 and i_idCountOut ) or ( (not c2) and c3 and i_idCountOut );

      --  in the schematic of the 74LS297 the c4 uses the invertet output
      --  for some strange reason we can not invert c4 here, this will eliminate 2 FFs

      b1<=i_kBorrow;
      b2<=b1;
      b3<=b2;
      b4<= ( (not b1) and b2 and (not i_idCountOut) ) or ( (not b2) and b3 and (not i_idCountOut) );

      -- here is a JK-FF, maybe we need a better construction in the future
      -- we use the inverted signals of c4 and b4

      if c4='1' and b4='1' then
        i_idCountOut<=i_idCountOut;
      elsif c4='0' and b4='1' then
        i_idCountOut<='1';
      elsif c4='1' and b4='0' then
        i_idCountOut<='0';
      elsif c4='0' and b4='0' then
        i_idCountOut<= not i_idCountOut;
      end if;

      -- end of JK-FF
    end if;
  end process;

  -- The original schematics of the 74LS297 produces a gated clock,
  -- which may result in timing problems
  -- We better use i_idCountOut as an clock enable for the N counter
  -- ID-out is the inverted clock enable; clockI is the inverted clockI


-- simple XOR phase detector

-- i_kDirection<=i_finScaleOut xor i_foutScaleOut;      	-- iff  you want to use the EXOR PD, just uncomment this
					-- but think about the EXOR limitations!!!!

-- edge controlled phase detector
  ECPD: block is
  signal pd1,pd2,rpd: std_logic;
  begin
    rpd<=pd2;
    i_kDirection<=pd1;

  process (rpd, i_foutScaleOut)			-- a leading negative edge of
  begin						-- fO sets the phase detector
    if rpd='1' then
      pd1<='0';
    elsif i_foutScaleOut='0' and i_foutScaleOut'event then
      pd1<='1';
    end if;
  end process;

  process (rpd, i_finScaleOut)					-- a negative edge of fI resets
    begin
      if rpd='1' then					-- the phase detector
        pd2<='0';
      elsif i_finScaleOut='0' and i_finScaleOut'event then
        pd2<='1';
      end if;
    end process;
  end block;

-- prescalers

  process (fI)
  begin
    if fI='1' and fI'event then
      if i_finScale_ti=i_cFinModul_ti-1 then
        i_finScale_ti<=0; i_finScaleOut<='0';
      elsif i_finScale_ti>=(i_cFinModul_ti/2-1) then
        i_finScale_ti<=i_finScale_ti+1; i_finScaleOut<='1';
      else
        i_finScale_ti<=i_finScale_ti+1; i_finScaleOut<='0';
      end if;
    end if;
  end process;

  process (fO)
  begin
    if fO='1' and fO'event then
      if i_foutScale_ti=i_cFoutModul_ti-1 then
        i_foutScale_ti<=0; i_foutScaleOut<='0';
      elsif i_foutScale_ti>=(i_cFoutModul_ti/2-1) then
        i_foutScale_ti<=i_foutScale_ti+1; i_foutScaleOut<='1';
      else
        i_foutScale_ti<=i_foutScale_ti+1; i_foutScaleOut<='0';
      end if;
    end if;
  end process;

-- Divide by N counter

-- this implementation differs a lot fromm  the logic in the 74LS297
-- which uses a gated clock (ID output),  tthat is deadly for very high clock rates
-- (up to 180MHz in Spartan 2)
-- here we use clock enable on the N counntter

  process (clockI, i_idCountOut)
  begin
    if clockI='0' and clockI'event then
      if i_idCountOut='0' then
        if i_nCount_ti=i_cNModul_ti-1 then
          i_nCount_ti<=0; fO<='0';
        elsif i_nCount_ti>=(i_cNModul_ti/2-1) then
          i_nCount_ti<=i_nCount_ti+1; fO<='1';
        else
          i_nCount_ti<=i_nCount_ti+1; fO<='0';
	end if;
      end if;
    end if;
  end process;

end PLL1_arch;
