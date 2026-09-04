def hardware_api_declaration(num_hw_api):
  # NOTE - Generate hardware API declarations in Kconfig format
  kconfig_content = []
  kconfig_content.append('menu "Hardware API configuration"\n')

  for i in range(1, num_hw_api + 1):
    kconfig_content.append(f'\tmenu \"Hardware API #{i} configuration\"')

    # NOTE - Config API name
    kconfig_content.append(f'\t\tconfig PAL_HW_API_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of Hardware API #{i}"')
    kconfig_content.append(f'\t\t\tdefault "hw_api_{i}"\n')
    kconfig_content.append(f'\t\t\thelp\n')
    kconfig_content.append(f'\t\t\t\tThis is the name of hardware API, please use lower case.\n')
    kconfig_content.append(f'\t\t\t\tPrefer like eg. `hw_api_1`, `hw_api_2`\n')

    kconfig_content.append('\t\tendmenu\n')
  
  kconfig_content.append('endmenu\n')

  with open("sources/app/kconfig/pal.kconfig", "a", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))