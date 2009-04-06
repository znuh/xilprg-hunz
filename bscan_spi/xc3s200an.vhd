library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;

entity top is
end top;

architecture Behavioral of top is
component BSCAN_SPARTAN3A
    port
    (
        CAPTURE : out std_ulogic;
        DRCK1 : out std_ulogic;
        DRCK2 : out std_ulogic;
        RESET : out std_ulogic;
        SEL1 : out std_ulogic;
        SEL2 : out std_ulogic;
        SHIFT : out std_ulogic;
        TCK : out std_ulogic;
        TDI : out std_ulogic;
        TMS : out std_ulogic;
        UPDATE : out std_ulogic;
        TDO1 : in std_ulogic;
        TDO2 : in std_ulogic
    );
end component;

component SPI_ACCESS
    generic
    (
        SIM_DEVICE : string := "UNSPECIFIED";
        SIM_FACTORY_ID : bit_vector := X"0000000000000000";
        SIM_MEM_FILE : string := "UNSPECIFIED";
        SIM_USER_ID : bit_vector := X"0000000000000000"
    );
    port
    (
        MISO : out std_ulogic;
        CLK : in std_ulogic;
        CSB : in std_ulogic;
        MOSI : in std_ulogic
    );
end component;

	signal CAPTURE: std_logic;
	signal UPDATE: std_logic;
	signal DRCK1: std_logic;
	signal TDI: std_logic;
	signal TDO1: std_logic;
	signal CSB: std_logic := '1';
	signal header: std_logic_vector(47 downto 0);
	signal have_header : std_logic := '0';
	signal tdoreg: std_logic_vector(6 downto 0);
	signal MISO: std_logic;
begin

   BSCAN_SPARTAN3A_inst : BSCAN_SPARTAN3A
   port map (
      CAPTURE => CAPTURE, -- CAPTURE output from TAP controller
      DRCK1 => DRCK1,     -- Data register output for USER1 functions
      DRCK2 => open,     -- Data register output for USER2 functions
      RESET => open,     -- Reset output from TAP controller
      SEL1 => open,       -- USER1 active output
      SEL2 => open,       -- USER2 active output
      SHIFT => open,     -- SHIFT output from TAP controller
      TCK => open,         -- TCK output from TAP controller
      TDI => TDI,         -- TDI output from TAP controller
      TMS => open,         -- TMS output from TAP controller
      UPDATE => UPDATE,   -- UPDATE output from TAP controller
      TDO1 => TDO1,       -- Data input for USER1 function
      TDO2 => '0'        -- Data input for USER2 function
   );

	SPI_ACCESS_inst : SPI_ACCESS
   generic map (
      SIM_DEVICE => "3S200AN" -- "3S50AN", "3S200AN", "3S400AN", "3S700AN", "3S1400AN" 
	)
   port map (
      MISO => MISO, --TDO1,  -- Serial output data from SPI PROM
      CLK => DRCK1,    -- SPI PROM clock input
      CSB => CSB,    -- SPI PROM enable input
      MOSI => TDI   -- Serial input data to SPI PROM
   );

	process(DRCK1, CAPTURE, UPDATE)
		
	begin
	
		if CAPTURE='1' or UPDATE='1' then
			header <= (others => '0');
			have_header <= '0';
			CSB <= '1';
		
		elsif rising_edge(DRCK1) then
		
			tdoreg <= tdoreg(5 downto 0) & MISO;
		
			if have_header='0' then
				
				header <= header(46 downto 0) & TDI;
				
				if header(46 downto 15) = x"59a659a6" then
					
					have_header <= '1';
					
					if (header(14 downto 0) & TDI) /= x"0000" then
						CSB <= '0';
					end if;
					
				end if;
				
			elsif header(15 downto 0) = x"0001" then
				CSB <= '1';
				
			else
				header(15 downto 0) <= header(15 downto 0) - 1;
			
			end if;
				
		end if;
	
	end process;
	
	TDO1 <= tdoreg(6);
	
end Behavioral;

