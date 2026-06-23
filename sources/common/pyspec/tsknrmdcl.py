# Function to generate blank task poll declarations based on user input for kconfig
# Task declarations include:
# - Task name
# - Task priority
# - Task message queue name
# - Task handler name
# - Task TSM use flag
# - Task TSM name
# - Task TSM state name
# - Task FSM use flag
# - Task FSM name
# - Task FSM state name
def task_norm_declaration(num_tasks_norm, num_tsm_states, num_fsm_states, is_tsm_enabled, is_fsm_enabled):
  # Generate task declarations in Kconfig format
  kconfig_content = []
  kconfig_content.append('menu "Task Norm configuration"\n')

  for i in range(1, num_tasks_norm + 1):
    kconfig_content.append(f'\tmenu \"Task #{i} configuration\"')

    # Config name
    kconfig_content.append(f'\t\tconfig DECL_TASK_NORM_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of task #{i}"')
    kconfig_content.append(f'\t\t\tdefault "TASK_NORM_{i}_ID"\n') 

    # Config message queue name
    kconfig_content.append(f'\t\tconfig DECL_MSG_QUEUE_{i}_NAME')
    kconfig_content.append(f'\t\t\tstring "Name of message queue #{i}"')
    kconfig_content.append(f'\t\t\tdefault "MSG_QUEUE_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')

    # Config handler name
    kconfig_content.append(f'\t\tconfig DECL_NORM_HANDLER_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of handler #{i}"')
    kconfig_content.append(f'\t\t\tdefault "NORM_HANDLER_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')

    # Config TSM use flag
    if is_tsm_enabled == True:
      kconfig_content.append('menu "TSM Configuration"\n')
      # Config TSM name
      kconfig_content.append(f'\tconfig APPCFG_TSM_TASK_{i}') 
      kconfig_content.append(f'\t\tstring "Name of TSM task #{i}"')
      kconfig_content.append(f'\t\tdefault "TSM_TASK_{i}_ID"')
      kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
      # Config TSM state name
      for j in range(1, num_tsm_states + 1):
        kconfig_content.append(f'\tconfig APPCFG_TSM_TASK_{i}_STATE_{j}')
        kconfig_content.append(f'\t\tstring "Name of TSM task #{i} state #{j}"')
        kconfig_content.append(f'\t\tdefault "TSM_TASK_{i}_STATE_{j}_ID"')
        kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')

      kconfig_content.append('\t\tendmenu\n')

    if is_fsm_enabled == True:
      kconfig_content.append('menu "FSM Configuration"\n')
      # Config FSM name
      kconfig_content.append(f'\tconfig APPCFG_FSM_TASK_{i}')
      kconfig_content.append(f'\t\tstring "Name of FSM task #{i}"')
      kconfig_content.append(f'\t\tdefault "FSM_TASK_{i}_ID"')
      kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
      # Config FSM state name
      for j in range(1, num_fsm_states + 1):
        kconfig_content.append(f'\tconfig APPCFG_FSM_TASK_{i}_STATE_{j}') 
        kconfig_content.append(f'\t\tstring "Name of FSM task #{i} state #{j}"')
        kconfig_content.append(f'\t\tdefault "FSM_TASK_{i}_STATE_{j}_ID"')
        kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')

      kconfig_content.append('\t\tendmenu\n')

    kconfig_content.append(f'\tendmenu\n')

  kconfig_content.append(f'endmenu\n')
  
  with open("sources/app/kconfig/decl.kconfig", "w", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))