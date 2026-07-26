def hardware_api_declaration(num_hw_api):
  # 1. Tạo nội dung kconfig mới
  kconfig_content = []
  kconfig_content.append('menu "Hardware API configuration"\n')

  for i in range(1, num_hw_api + 1):
    kconfig_content.append(f'\tmenu \"Hardware API #{i} configuration\"')

    # Config API name
    kconfig_content.append(f'\t\tconfig PAL_HW_API_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of Hardware API #{i}"')
    kconfig_content.append(f'\t\t\tdefault "HW_API_{i}"\n')

    kconfig_content.append('\t\tendmenu\n')
  
  kconfig_content.append('endmenu\n')

  with open("sources/app/kconfig/pal.kconfig", "a", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))