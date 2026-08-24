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
  # NOTE - Generate task declarations in Kconfig format
  # TODO - Consider forward task to Minh
  """
  Add task specific generation logic for TSM and FSM states
  in order to remove the situation where some task only use FSM or TSM, but not both. 
  This will help to reduce the number of task declarations and make the configuration more efficient.
  """
  kconfig_content = []
  kconfig_content.append('menu "Task Norm configuration"\n')

  for i in range(1, num_tasks_norm + 1):
    kconfig_content.append(f'\tmenu \"Task #{i} configuration\"')

    # NOTE - Config name
    kconfig_content.append(f'\t\tconfig DECL_TASK_NORM_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of task #{i}"')
    kconfig_content.append(f'\t\t\tdefault "TASK_NORM_{i}_ID"\n')
    kconfig_content.append(f'\t\t\thelp\n')
    kconfig_content.append(f'\t\t\t\tThis is the name of task norm, please use upper case.\n') 
    kconfig_content.append(f'\t\t\t\tPrefer like eg. `TASK_NORM_A_ID`, `TASK_NORM_USR_ID`\n')
    kconfig_content.append(f'\t\t\t\tThis is only used for internal identification.\n')

    # NOTE - Config message queue name
    kconfig_content.append(f'\t\tconfig DECL_MSG_QUEUE_{i}_NAME')
    kconfig_content.append(f'\t\t\tstring "Name of message queue #{i}"')
    kconfig_content.append(f'\t\t\tdefault "MSG_QUEUE_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
    kconfig_content.append(f'\t\t\thelp\n')
    kconfig_content.append(f'\t\t\t\tThis is the name of task norm message queue.\n')
    kconfig_content.append(f'\t\t\t\tFormat output as extern uedp_msg_t* <value>_msgq[UEDP_MSG_BLANK_QUEUE_SIZE];.\n')
    kconfig_content.append(f'\t\t\t\tUsually prefer eg. `tnorm_a` to produce `tnorm_a_msgq[UEDP_MSG_BLANK_QUEUE_SIZE]`.\n')

    # NOTE - Config handler name
    kconfig_content.append(f'\t\tconfig DECL_NORM_HANDLER_{i}_NAME') 
    kconfig_content.append(f'\t\t\tstring "Name of handler #{i}"')
    kconfig_content.append(f'\t\t\tdefault "NORM_HANDLER_{i}_ID"')
    kconfig_content.append(f'\t\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
    kconfig_content.append(f'\t\t\thelp\n')
    kconfig_content.append(f'\t\t\t\tThis is the name of task norm handler.\n')
    kconfig_content.append(f'\t\t\t\tFormat output as void <value>_nhler(uedp_msg_t* msg);.\n')
    kconfig_content.append(f'\t\t\t\tUsually prefer eg. `tnorm_a` to produce `tnorm_a_nhler(uedp_msg_t* msg);`.\n')

    # NOTE - Config TSM
    if is_tsm_enabled == True:
      kconfig_content.append('menu "TSM Configuration"\n')
      # NOTE - Config TSM name
      kconfig_content.append(f'\tconfig APPCFG_TSM_TASK_{i}') 
      kconfig_content.append(f'\t\tstring "Name of TSM task #{i}"')
      kconfig_content.append(f'\t\tdefault "TSM_TASK_{i}_ID"')
      kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
      kconfig_content.append(f'\t\thelp\n')
      kconfig_content.append(f'\t\t\tThis is the name of TSM task.\n')
      kconfig_content.append(f'\t\t\tFormat output as extern uedp_tsm_t <value>;\n')
      kconfig_content.append(f'\t\t\tUsually prefer eg. `tsm_task_1` to produce `extern uedp_tsm_t tsm_task_1;` in tsm objects.\n')
      kconfig_content.append(f'\t\t\tUsually prefer eg. `tsm_task_1` to produce `extern tsm_state_desc_t tsm_task_1_tbl[];` in tsm objects table which contains list of states.\n')
      # NOTE - Config TSM state name
      for j in range(1, num_tsm_states + 1):
        kconfig_content.append(f'\tconfig APPCFG_TSM_TASK_{i}_STATE_{j}')
        kconfig_content.append(f'\t\tstring "Name of TSM task #{i} state #{j}"')
        kconfig_content.append(f'\t\tdefault "TSM_TASK_{i}_STATE_{j}_ID"')
        kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
        kconfig_content.append(f'\t\thelp\n')
        kconfig_content.append(f'\t\t\tThis is the name of TSM task state.\n')
        kconfig_content.append(f'\t\t\tFormat output as tsm_trans_t <value>_trans[];\n')
        kconfig_content.append(f'\t\t\tAlso used for ntry/actv/exit function names\n')
        kconfig_content.append(f'\t\t\tUsually prefer eg. `tsm_task_1_state_1` to produce `tsm_task_1_state_1_trans[]` in tsm state transition table.\n')
        kconfig_content.append(f'\t\t\tUsually prefer eg. `tsm_task_1_state_1` to produce `tsm_task_1_state_1_ntry()`, `tsm_task_1_state_1_actv()`, `tsm_task_1_state_1_exit()` in tsm state transition table.\n')

      kconfig_content.append('\t\tendmenu\n')

    # NOTE - Config FSM
    if is_fsm_enabled == True:
      kconfig_content.append('menu "FSM Configuration"\n')
      # NOTE - Config FSM name
      kconfig_content.append(f'\tconfig APPCFG_FSM_TASK_{i}')
      kconfig_content.append(f'\t\tstring "Name of FSM task #{i}"')
      kconfig_content.append(f'\t\tdefault "FSM_TASK_{i}_ID"')
      kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
      kconfig_content.append(f'\t\thelp\n')
      kconfig_content.append(f'\t\t\tThis is the name of FSM task.\n')
      kconfig_content.append(f'\t\t\tFormat output as extern uedp_fsm_t <value>;\n')
      kconfig_content.append(f'\t\t\tUsually prefer eg. `fsm_task_1` to produce `extern uedp_fsm_t fsm_task_1;` in fsm objects.\n')
      # NOTE - Config FSM state name
      for j in range(1, num_fsm_states + 1):
        kconfig_content.append(f'\tconfig APPCFG_FSM_TASK_{i}_STATE_{j}') 
        kconfig_content.append(f'\t\tstring "Name of FSM task #{i} state #{j}"')
        kconfig_content.append(f'\t\tdefault "FSM_TASK_{i}_STATE_{j}_ID"')
        kconfig_content.append(f'\t\tdepends on DECL_TASK_NORM_{i}_NAME != ""\n')
        kconfig_content.append(f'\t\thelp\n')
        kconfig_content.append(f'\t\t\tThis is the name of FSM task state.\n')
        kconfig_content.append(f'\t\t\tFormat output as fsm_state_desc_t <value>_trans[];\n')
        kconfig_content.append(f'\t\t\tUsually prefer eg. `fsm_task_1_state_1` to produce `fsm_task_1_state_1_onst()` in fsm state transition function.\n')

      kconfig_content.append('\t\tendmenu\n')

    kconfig_content.append(f'\tendmenu\n')

  kconfig_content.append(f'endmenu\n')
  
  with open("sources/app/kconfig/decl.kconfig", "w", encoding="utf-8") as f:
    f.write("\n".join(kconfig_content))