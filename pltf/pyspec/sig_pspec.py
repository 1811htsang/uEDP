# Function to generate blank signal declarations based on user input for kconfig
# Signal declarations include:
# - Signal name
# - Signal value (auto generated from 0x01u, 0x02u, ... to 0xFFu)
def signal_declaration(num_signals):
  # NOTE -Generate signal declarations in Kconfig format
  kconfig_content = []
  kconfig_content.append('menu "Signal configuration"\n')

  for i in range(1, num_signals + 1):
    kconfig_content.append(f'\tmenu \"Signal #{i} configuration\"')
    kconfig_content.append(f'\t\tconfig DECL_SIG_{i}_NAME')
    kconfig_content.append(f'\t\t\tstring "Name of signal #{i}"')
    kconfig_content.append(f'\t\t\tdefault "SIG_{i}_ID"\n')
    kconfig_content.append(f'\t\t\thelp\n')
    kconfig_content.append(f'\t\t\t\tThis is the name of signal, please use upper case.\n')
    kconfig_content.append(f'\t\t\t\tPrefer like eg. `SIG_A_ID`, `SIG_USR_ID`\n')
    kconfig_content.append(f'\t\t\t\tThis is only used for internal identification.\n')
    kconfig_content.append(f'\tendmenu\n')

  kconfig_content.append(f'endmenu\n')
  
  with open("sources/app/kconfig/decl.kconfig", "a", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))