# Function to generate blank task poll declarations based on user input for kconfig
# Task poll declarations include:
# - Task poll name
# - Task poll ability
# - Task poll handler (auto generated based on task poll name)
def task_poll_declaration(num_tasks_poll):
  # Generate task declarations in Kconfig format
  kconfig_content = []
  kconfig_content.append('menu "Task Poll configuration"\n')

  for i in range(1, num_tasks_poll + 1):
    kconfig_content.append(f'\tmenu \"Task Poll #{i} configuration\"')

    # Config name
    kconfig_content.append(f'\t\tconfig DECL_TASK_POLL_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of task poll #{i}"')
    kconfig_content.append(f'\t\t\tdefault "TASK_POLL_{i}_ID"\n') 

    # Config ability
    kconfig_content.append(f'\t\tconfig DECL_TASK_POLL_{i}_ABILITY') 
    kconfig_content.append(f'\t\t\tbool "Ability of task poll #{i}"')
    kconfig_content.append(f'\t\t\tdefault "n"')
    kconfig_content.append(f'\t\t\tdepends on DECL_TASK_POLL_{i}_NAME != ""\n')

    # Config handler name
    kconfig_content.append(f'\t\tconfig DECL_POLL_HANDLER_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of handler #{i}"')
    kconfig_content.append(f'\t\t\tdefault "POLL_HANDLER_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on DECL_TASK_POLL_{i}_NAME != ""\n')

    kconfig_content.append(f'\tendmenu\n')

  kconfig_content.append(f'endmenu\n')
  
  with open("sources/app/kconfig/decl.kconfig", "a", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))