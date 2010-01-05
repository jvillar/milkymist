--
-- Definition of a dual port ROM for KCPSM2 or KCPSM3 program defined by progctrl.psm
-- and assmbled using KCPSM2 or KCPSM3 assembler.
--
-- Standard IEEE libraries
--
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.STD_LOGIC_ARITH.ALL;
use IEEE.STD_LOGIC_UNSIGNED.ALL;
--
-- The Unisim Library is used to define Xilinx primitives. It is also used during
-- simulation. The source can be viewed at %XILINX%\vhdl\src\unisims\unisim_VCOMP.vhd
--  
library unisim;
use unisim.vcomponents.all;
--
--
entity progctrl is
    Port (      address : in std_logic_vector(9 downto 0);
            instruction : out std_logic_vector(17 downto 0);
             proc_reset : out std_logic;
                    clk : in std_logic);
    end progctrl;
--
architecture low_level_definition of progctrl is
--
-- Declare signals internal to this module
--
signal jaddr     : std_logic_vector(10 downto 0);
signal jparity   : std_logic_vector(0 downto 0);
signal jdata     : std_logic_vector(7 downto 0);
signal doa       : std_logic_vector(7 downto 0);
signal dopa      : std_logic_vector(0 downto 0);
signal tdo1      : std_logic;
signal tdo2      : std_logic;
signal update    : std_logic;
signal shift     : std_logic;
signal reset     : std_logic;
signal tdi       : std_logic;
signal sel1      : std_logic;
signal drck1     : std_logic;
signal drck1_buf : std_logic;
signal sel2      : std_logic;
signal drck2     : std_logic;
signal capture   : std_logic;
signal tap5      : std_logic;
signal tap11     : std_logic;
signal tap17     : std_logic;
--
-- Attributes to define ROM contents during implementation synthesis. 
-- The information is repeated in the generic map for functional simulation
--
attribute INIT_00 : string; 
attribute INIT_01 : string; 
attribute INIT_02 : string; 
attribute INIT_03 : string; 
attribute INIT_04 : string; 
attribute INIT_05 : string; 
attribute INIT_06 : string; 
attribute INIT_07 : string; 
attribute INIT_08 : string; 
attribute INIT_09 : string; 
attribute INIT_0A : string; 
attribute INIT_0B : string; 
attribute INIT_0C : string; 
attribute INIT_0D : string; 
attribute INIT_0E : string; 
attribute INIT_0F : string; 
attribute INIT_10 : string; 
attribute INIT_11 : string; 
attribute INIT_12 : string; 
attribute INIT_13 : string; 
attribute INIT_14 : string; 
attribute INIT_15 : string; 
attribute INIT_16 : string; 
attribute INIT_17 : string; 
attribute INIT_18 : string; 
attribute INIT_19 : string; 
attribute INIT_1A : string; 
attribute INIT_1B : string; 
attribute INIT_1C : string; 
attribute INIT_1D : string; 
attribute INIT_1E : string; 
attribute INIT_1F : string; 
attribute INIT_20 : string; 
attribute INIT_21 : string; 
attribute INIT_22 : string; 
attribute INIT_23 : string; 
attribute INIT_24 : string; 
attribute INIT_25 : string; 
attribute INIT_26 : string; 
attribute INIT_27 : string; 
attribute INIT_28 : string; 
attribute INIT_29 : string; 
attribute INIT_2A : string; 
attribute INIT_2B : string; 
attribute INIT_2C : string; 
attribute INIT_2D : string; 
attribute INIT_2E : string; 
attribute INIT_2F : string; 
attribute INIT_30 : string; 
attribute INIT_31 : string; 
attribute INIT_32 : string; 
attribute INIT_33 : string; 
attribute INIT_34 : string; 
attribute INIT_35 : string; 
attribute INIT_36 : string; 
attribute INIT_37 : string; 
attribute INIT_38 : string; 
attribute INIT_39 : string; 
attribute INIT_3A : string; 
attribute INIT_3B : string; 
attribute INIT_3C : string; 
attribute INIT_3D : string; 
attribute INIT_3E : string; 
attribute INIT_3F : string; 
attribute INITP_00 : string;
attribute INITP_01 : string;
attribute INITP_02 : string;
attribute INITP_03 : string;
attribute INITP_04 : string;
attribute INITP_05 : string;
attribute INITP_06 : string;
attribute INITP_07 : string;
--
-- Attributes to define ROM contents during implementation synthesis.
--
attribute INIT_00 of ram_1024_x_18 : label is  "502C4042502A4045002201310F3E019301930193023301990193C001011A0027";
attribute INIT_01 of ram_1024_x_18 : label is  "013101310F3F019350D540535003404850C2404950A34052508C405750474050";
attribute INIT_02 of ram_1024_x_18 : label is  "011F02D401930904402D09FEA000C0080006A000013E10F00131011F40070131";
attribute INIT_03 of ram_1024_x_18 : label is  "02F14007022C5C38C902004101310F2E07000800019302090193543F4F590131";
attribute INIT_04 of ram_1024_x_18 : label is  "B0004B01006700574007022C004C01E20193A00000F700EA01D000EA01204007";
attribute INIT_05 of ram_1024_x_18 : label is  "B0004F0AB0004F0D011F54584F3A011F0E2B404C00776A2B0193016D504C4B04";
attribute INIT_06 of ram_1024_x_18 : label is  "8D02B4004B0450724B007BD08D037CD00D2B405B8E01F0E0017412F0011F13F0";
attribute INIT_07 of ram_1024_x_18 : label is  "032F00EAC10111A05077208000E000EA01E8A00078D0CD0177D0CD01A00079D0";
attribute INIT_08 of ram_1024_x_18 : label is  "1900588C018D02FEA00000F700EA01D05480CA018301A900A800870100EA7130";
attribute INIT_09 of ram_1024_x_18 : label is  "110000EA01404007022C0193009D5896018D030E1700588C018D1800588C018D";
attribute INIT_0A of ram_1024_x_18 : label is  "022C00B10193170058A3018D180058A3018D190058A3018D02FEA00000F700EA";
attribute INIT_0B of ram_1024_x_18 : label is  "54B2C60154B6C5010167A900A800870100E0019605100196016D019306104007";
attribute INIT_0C of ram_1024_x_18 : label is  "07020196016700E000EA0190070008000900019601310F3D02C60193A0000193";
attribute INIT_0D of ram_1024_x_18 : label is  "400700F40193016700E0019300EA0170070008000900400700F40193016700E0";
attribute INIT_0E of ram_1024_x_18 : label is  "C1080100C110C720C840C980A000C108400201060106C1080105C720C840C980";
attribute INIT_0F of ram_1024_x_18 : label is  "A00000F450F9208000E0AE008D010D000E00A00000EA01FFA000C10801060106";
attribute INIT_10 of ram_1024_x_18 : label is  "A000550CC10101070128A0005508C001000BA0000193016710D0016710E00193";
attribute INIT_11 of ram_1024_x_18 : label is  "C000A000551BC40101150414A0005516C30101100314A0005511C201010B0219";
attribute INIT_12 of ram_1024_x_18 : label is  "51204F114F014129552D20084000A000C00151294F134F014120552420084000";
attribute INIT_13 of ram_1024_x_18 : label is  "B8004061A000803AC1015D38C00A81010130A000CF0441315135200140004129";
attribute INIT_14 of ram_1024_x_18 : label is  "02069200020602061200B80001447010A000C0F6B80080C6A000A0DFBC00407B";
attribute INIT_15 of ram_1024_x_18 : label is  "0162A00F101012000162000E000E000E000E1100A0009200B800014470108101";
attribute INIT_16 of ram_1024_x_18 : label is  "108001671090A00001311F1001311F200156A000803A80075965C00AA0001100";
attribute INIT_17 of ram_1024_x_18 : label is  "D030B8000181102003060306030603061300B80001811030A000016710700167";
attribute INIT_18 of ram_1024_x_18 : label is  "002213000022A000800AA000C0F6B80080075D8BC011B800C0E9B80080B9A000";
attribute INIT_19 of ram_1024_x_18 : label is  "0F6301310F6901310F5001930193A00001310F20A00001310F0DA00001741200";
attribute INIT_1A of ram_1024_x_18 : label is  "01310F4E019601310F6501310F7A01310F6101310F6C01310F4201310F6F0131";
attribute INIT_1B of ram_1024_x_18 : label is  "019601310F4801310F5301310F4101310F4C01310F46019601310F5201310F4F";
attribute INIT_1C of ram_1024_x_18 : label is  "01310F6D01310F6D01310F6101310F7201310F6701310F6F01310F7201310F50";
attribute INIT_1D of ram_1024_x_18 : label is  "019301310F3001310F3001310F2E01310F3101310F76019601310F7201310F65";
attribute INIT_1E of ram_1024_x_18 : label is  "01310F6701310F6E01310F6901310F7401310F6901310F6101310F57A0000193";
attribute INIT_1F of ram_1024_x_18 : label is  "0F46019601310F5301310F4301310F4D019601310F7201310F6F01310F660196";
attribute INIT_20 of ram_1024_x_18 : label is  "0F50019601310F6E01310F690220A000019301310F6501310F6C01310F690131";
attribute INIT_21 of ram_1024_x_18 : label is  "A0000193013101310F7301310F6501310F7201310F6701310F6F01310F720131";
attribute INIT_22 of ram_1024_x_18 : label is  "0F4B01310F4F0193A000019601310F6501310F7301310F6101310F7201310F45";
attribute INIT_23 of ram_1024_x_18 : label is  "0F420193013101310F6C01310F61022001310F2D01310F450193A00001930131";
attribute INIT_24 of ram_1024_x_18 : label is  "01310F7301310F6B01310F6301310F6F01310F6C01310F62022001310F2D0131";
attribute INIT_25 of ram_1024_x_18 : label is  "01310F7201310F5001310F2D01310F50019301310F3301310F2D01310F310196";
attribute INIT_26 of ram_1024_x_18 : label is  "0F5701310F2D01310F5701F701310F6D01310F6101310F7201310F6701310F6F";
attribute INIT_27 of ram_1024_x_18 : label is  "01310F2D01310F52019302CB019601310F6501310F7401310F6901310F720131";
attribute INIT_28 of ram_1024_x_18 : label is  "019601310F3601310F3501310F32019601310F6401310F6101310F6501310F52";
attribute INIT_29 of ram_1024_x_18 : label is  "01310F6901310F7601310F6501310F4401310F2D01310F49019301310F7302CB";
attribute INIT_2A of ram_1024_x_18 : label is  "0F6C01310F6501310F4801310F2D01310F48019302C6019601310F6501310F63";
attribute INIT_2B of ram_1024_x_18 : label is  "01310F7401310F6101310F7401310F5301310F2D01310F53019301310F700131";
attribute INIT_2C of ram_1024_x_18 : label is  "0F7401310F7901310F62A00001310F4401310F49A000019301310F7301310F75";
attribute INIT_2D of ram_1024_x_18 : label is  "0F7201310F6901310F6601310F6E01310F6F01310F430193A00001310F650131";
attribute INIT_2E of ram_1024_x_18 : label is  "019601310F2901310F6E01310F2F01310F5901310F280220019601310F6D0131";
attribute INIT_2F of ram_1024_x_18 : label is  "0F610193A000019301310F7401310F7201310F6F01310F6201310F410193A000";
attribute INIT_30 of ram_1024_x_18 : label is  "0F640193A00001310F3D013101310F7301310F6501310F72013101310F640131";
attribute INIT_31 of ram_1024_x_18 : label is  "00000000000000000000000000000000430B01310F6101310F7401310F610131";
attribute INIT_32 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_33 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_34 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_35 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_36 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_37 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_38 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_39 of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_3A of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_3B of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_3C of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_3D of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_3E of ram_1024_x_18 : label is  "0000000000000000000000000000000000000000000000000000000000000000";
attribute INIT_3F of ram_1024_x_18 : label is  "43F580016000C004001143FC001353FB20104000E00000000000000000000000";
attribute INITP_00 of ram_1024_x_18 : label is "34DF2118674436CC99F73CFD9FFFEF33FF7C0FF7FCCA2CFFF3DDDDDDDDF3FFFF";
attribute INITP_01 of ram_1024_x_18 : label is "BDD42CA08AAA022AFFFC03FF3FC03CFBDDD5F3F3FCF3CFEF33FFF3CF3FBCD55C";
attribute INITP_02 of ram_1024_x_18 : label is "2CAA2CB332CCE5D8C0EA89B19A2C998999752BD3D3D2F4F4EDCB72DCB72D2F33";
attribute INITP_03 of ram_1024_x_18 : label is "3CCCF333CCCCCCCBF33333CCCCCCCCCCF33333CCCF333333333ECB2CCE667666";
attribute INITP_04 of ram_1024_x_18 : label is "CCFF3333333CCCCCCCCCF333CCCCCCF33F33CCEF33BCCCCCBF3333333CCEF333";
attribute INITP_05 of ram_1024_x_18 : label is "3BCCCCCEF33333F3333333B3332CCBCCCCCCCCF333333FCCCCCCCCF3F333CCCC";
attribute INITP_06 of ram_1024_x_18 : label is "0000000000000000000000000000000000000000000000000000F3333B3CCCF3";
attribute INITP_07 of ram_1024_x_18 : label is "F233480000000000000000000000000000000000000000000000000000000000";
--
begin
--
  --Instantiate the Xilinx primitive for a block RAM
  ram_1024_x_18: RAMB16_S9_S18
  --synthesis translate_off
  --INIT values repeated to define contents for functional simulation
  generic map (INIT_00 => X"502C4042502A4045002201310F3E019301930193023301990193C001011A0027",
               INIT_01 => X"013101310F3F019350D540535003404850C2404950A34052508C405750474050",
               INIT_02 => X"011F02D401930904402D09FEA000C0080006A000013E10F00131011F40070131",
               INIT_03 => X"02F14007022C5C38C902004101310F2E07000800019302090193543F4F590131",
               INIT_04 => X"B0004B01006700574007022C004C01E20193A00000F700EA01D000EA01204007",
               INIT_05 => X"B0004F0AB0004F0D011F54584F3A011F0E2B404C00776A2B0193016D504C4B04",
               INIT_06 => X"8D02B4004B0450724B007BD08D037CD00D2B405B8E01F0E0017412F0011F13F0",
               INIT_07 => X"032F00EAC10111A05077208000E000EA01E8A00078D0CD0177D0CD01A00079D0",
               INIT_08 => X"1900588C018D02FEA00000F700EA01D05480CA018301A900A800870100EA7130",
               INIT_09 => X"110000EA01404007022C0193009D5896018D030E1700588C018D1800588C018D",
               INIT_0A => X"022C00B10193170058A3018D180058A3018D190058A3018D02FEA00000F700EA",
               INIT_0B => X"54B2C60154B6C5010167A900A800870100E0019605100196016D019306104007",
               INIT_0C => X"07020196016700E000EA0190070008000900019601310F3D02C60193A0000193",
               INIT_0D => X"400700F40193016700E0019300EA0170070008000900400700F40193016700E0",
               INIT_0E => X"C1080100C110C720C840C980A000C108400201060106C1080105C720C840C980",
               INIT_0F => X"A00000F450F9208000E0AE008D010D000E00A00000EA01FFA000C10801060106",
               INIT_10 => X"A000550CC10101070128A0005508C001000BA0000193016710D0016710E00193",
               INIT_11 => X"C000A000551BC40101150414A0005516C30101100314A0005511C201010B0219",
               INIT_12 => X"51204F114F014129552D20084000A000C00151294F134F014120552420084000",
               INIT_13 => X"B8004061A000803AC1015D38C00A81010130A000CF0441315135200140004129",
               INIT_14 => X"02069200020602061200B80001447010A000C0F6B80080C6A000A0DFBC00407B",
               INIT_15 => X"0162A00F101012000162000E000E000E000E1100A0009200B800014470108101",
               INIT_16 => X"108001671090A00001311F1001311F200156A000803A80075965C00AA0001100",
               INIT_17 => X"D030B8000181102003060306030603061300B80001811030A000016710700167",
               INIT_18 => X"002213000022A000800AA000C0F6B80080075D8BC011B800C0E9B80080B9A000",
               INIT_19 => X"0F6301310F6901310F5001930193A00001310F20A00001310F0DA00001741200",
               INIT_1A => X"01310F4E019601310F6501310F7A01310F6101310F6C01310F4201310F6F0131",
               INIT_1B => X"019601310F4801310F5301310F4101310F4C01310F46019601310F5201310F4F",
               INIT_1C => X"01310F6D01310F6D01310F6101310F7201310F6701310F6F01310F7201310F50",
               INIT_1D => X"019301310F3001310F3001310F2E01310F3101310F76019601310F7201310F65",
               INIT_1E => X"01310F6701310F6E01310F6901310F7401310F6901310F6101310F57A0000193",
               INIT_1F => X"0F46019601310F5301310F4301310F4D019601310F7201310F6F01310F660196",
               INIT_20 => X"0F50019601310F6E01310F690220A000019301310F6501310F6C01310F690131",
               INIT_21 => X"A0000193013101310F7301310F6501310F7201310F6701310F6F01310F720131",
               INIT_22 => X"0F4B01310F4F0193A000019601310F6501310F7301310F6101310F7201310F45",
               INIT_23 => X"0F420193013101310F6C01310F61022001310F2D01310F450193A00001930131",
               INIT_24 => X"01310F7301310F6B01310F6301310F6F01310F6C01310F62022001310F2D0131",
               INIT_25 => X"01310F7201310F5001310F2D01310F50019301310F3301310F2D01310F310196",
               INIT_26 => X"0F5701310F2D01310F5701F701310F6D01310F6101310F7201310F6701310F6F",
               INIT_27 => X"01310F2D01310F52019302CB019601310F6501310F7401310F6901310F720131",
               INIT_28 => X"019601310F3601310F3501310F32019601310F6401310F6101310F6501310F52",
               INIT_29 => X"01310F6901310F7601310F6501310F4401310F2D01310F49019301310F7302CB",
               INIT_2A => X"0F6C01310F6501310F4801310F2D01310F48019302C6019601310F6501310F63",
               INIT_2B => X"01310F7401310F6101310F7401310F5301310F2D01310F53019301310F700131",
               INIT_2C => X"0F7401310F7901310F62A00001310F4401310F49A000019301310F7301310F75",
               INIT_2D => X"0F7201310F6901310F6601310F6E01310F6F01310F430193A00001310F650131",
               INIT_2E => X"019601310F2901310F6E01310F2F01310F5901310F280220019601310F6D0131",
               INIT_2F => X"0F610193A000019301310F7401310F7201310F6F01310F6201310F410193A000",
               INIT_30 => X"0F640193A00001310F3D013101310F7301310F6501310F72013101310F640131",
               INIT_31 => X"00000000000000000000000000000000430B01310F6101310F7401310F610131",
               INIT_32 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_33 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_34 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_35 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_36 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_37 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_38 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_39 => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_3A => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_3B => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_3C => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_3D => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_3E => X"0000000000000000000000000000000000000000000000000000000000000000",
               INIT_3F => X"43F580016000C004001143FC001353FB20104000E00000000000000000000000",    
               INITP_00 => X"34DF2118674436CC99F73CFD9FFFEF33FF7C0FF7FCCA2CFFF3DDDDDDDDF3FFFF",
               INITP_01 => X"BDD42CA08AAA022AFFFC03FF3FC03CFBDDD5F3F3FCF3CFEF33FFF3CF3FBCD55C",
               INITP_02 => X"2CAA2CB332CCE5D8C0EA89B19A2C998999752BD3D3D2F4F4EDCB72DCB72D2F33",
               INITP_03 => X"3CCCF333CCCCCCCBF33333CCCCCCCCCCF33333CCCF333333333ECB2CCE667666",
               INITP_04 => X"CCFF3333333CCCCCCCCCF333CCCCCCF33F33CCEF33BCCCCCBF3333333CCEF333",
               INITP_05 => X"3BCCCCCEF33333F3333333B3332CCBCCCCCCCCF333333FCCCCCCCCF3F333CCCC",
               INITP_06 => X"0000000000000000000000000000000000000000000000000000F3333B3CCCF3",
               INITP_07 => X"F233480000000000000000000000000000000000000000000000000000000000")
  --synthesis translate_on
  port map(    DIB => "0000000000000000",
              DIPB => "00",
               ENB => '1',
               WEB => '0',
              SSRB => '0',
              CLKB => clk,
             ADDRB => address,
               DOB => instruction(15 downto 0),
              DOPB => instruction(17 downto 16),
               DIA => jdata,
              DIPA => jparity,
               ENA => sel1,
               WEA => '1',
              SSRA => '0',
              CLKA => update,
              ADDRA=> jaddr,
               DOA => doa(7 downto 0),
              DOPA => dopa); 
  v2_bscan: BSCAN_VIRTEX2 
  port map(   TDO1 => tdo1,
         TDO2 => tdo2,
            UPDATE => update,
             SHIFT => shift,
             RESET => reset,
               TDI => tdi,
              SEL1 => sel1,
             DRCK1 => drck1,
              SEL2 => sel2,
             DRCK2 => drck2,
      CAPTURE => capture);
  --buffer signal used as a clock
  upload_clock: BUFG
  port map( I => drck1,
            O => drck1_buf);
  -- Assign the reset to be active whenever the uploading subsystem is active
  proc_reset <= sel1;
  srlC1: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => tdi,
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jaddr(10),
            Q15 => jaddr(8));
  flop1: FD
  port map ( D => jaddr(10),
             Q => jaddr(9),
             C => drck1_buf);
  srlC2: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => jaddr(8),
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jaddr(7),
            Q15 => tap5);
  flop2: FD
  port map ( D => jaddr(7),
             Q => jaddr(6),
             C => drck1_buf);
  srlC3: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => tap5,
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jaddr(5),
            Q15 => jaddr(3));
  flop3: FD
  port map ( D => jaddr(5),
             Q => jaddr(4),
             C => drck1_buf);
  srlC4: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => jaddr(3),
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jaddr(2),
            Q15 => tap11);
  flop4: FD
  port map ( D => jaddr(2),
             Q => jaddr(1),
             C => drck1_buf);
  srlC5: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => tap11,
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jaddr(0),
            Q15 => jdata(7));
  flop5: FD
  port map ( D => jaddr(0),
             Q => jparity(0),
             C => drck1_buf);
  srlC6: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => jdata(7),
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jdata(6),
            Q15 => tap17);
  flop6: FD
  port map ( D => jdata(6),
             Q => jdata(5),
             C => drck1_buf);
  srlC7: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => tap17,
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jdata(4),
            Q15 => jdata(2));
  flop7: FD
  port map ( D => jdata(4),
             Q => jdata(3),
             C => drck1_buf);
  srlC8: SRLC16E
  --synthesis translate_off
  generic map (INIT => X"0000")
  --synthesis translate_on
  port map(   D => jdata(2),
             CE => '1',
            CLK => drck1_buf,
             A0 => '1',
             A1 => '0',
             A2 => '1',
             A3 => '1',
              Q => jdata(1),
            Q15 => tdo1);
  flop8: FD
  port map ( D => jdata(1),
             Q => jdata(0),
             C => drck1_buf);
end low_level_definition;
--
------------------------------------------------------------------------------------
--
-- END OF FILE progctrl.vhd
--
------------------------------------------------------------------------------------
