# Function to get user input for task generation parameters
# Task generation parameters include:
# - Number of tasks norm to generate
# - Number of tasks poll to generate
# - Number of signals to generate
# - Per-task: whether task uses FSM/TSM, and how many states each uses
# - Number of Hardware API need to be generated (if Hardware API is used)
def user_input(DEFAULT_VALS):
  # NOTE - Number of tasks to generate
  print(f'[INFO] Number of tasks norm to generate (default: {DEFAULT_VALS["num_tasks_norm"]}): ', end='')
  val = input().strip()
  num_tasks_norm = int(val) if val != '' else DEFAULT_VALS["num_tasks_norm"]

  # NOTE - Number of tasks poll to generate
  print(f'[INFO] Number of tasks poll to generate (default: {DEFAULT_VALS["num_tasks_poll"]}): ', end='')
  val = input().strip()
  num_tasks_poll = int(val) if val != '' else DEFAULT_VALS["num_tasks_poll"]

  # NOTE - Number of signals to generate
  print(f'[INFO] Number of signals to generate (default: {DEFAULT_VALS["num_signals"]}): ', end='')
  val = input().strip()
  num_signals = int(val) if val != '' else DEFAULT_VALS["num_signals"]

  # NOTE - Ask FSM/TSM usage & state count PER TASK, so each norm task can
  # independently opt into FSM/TSM with its own number of states, instead of
  # a single global flag/count shared by every task.
  fsm_flags = []
  tsm_flags = []
  num_fsm_states_list = []
  num_tsm_states_list = []

  for i in range(1, num_tasks_norm + 1):
    print(f'[INFO] Task #{i} - Do you want to use FSM? (y/n, default: n): ', end='')
    use_fsm_i = input().strip().lower() == 'y'
    fsm_flags.append(use_fsm_i)

    num_fsm_states_i = 0
    if use_fsm_i:
      print(f'[INFO] Task #{i} - Number of FSM states to generate (default: 1): ', end='')
      val = input().strip()
      num_fsm_states_i = int(val) if val != '' else 1
    num_fsm_states_list.append(num_fsm_states_i)

    print(f'[INFO] Task #{i} - Do you want to use TSM? (y/n, default: n): ', end='')
    use_tsm_i = input().strip().lower() == 'y'
    tsm_flags.append(use_tsm_i)

    num_tsm_states_i = 0
    if use_tsm_i:
      print(f'[INFO] Task #{i} - Number of TSM states to generate (default: 1): ', end='')
      val = input().strip()
      num_tsm_states_i = int(val) if val != '' else 1
    num_tsm_states_list.append(num_tsm_states_i)

  # NOTE - Number of hardware API to generate
  num_hw_api = 0
  print('[INFO] Do you want to generate Hardware API? (y/n, default: n): ', end='')
  if input().strip().lower() == 'y':
    print(f'[INFO] Number of Hardware API to generate (default: {DEFAULT_VALS["num_hw_api"]}): ', end='')
    val = input().strip()
    num_hw_api = int(val) if val != '' else DEFAULT_VALS["num_hw_api"]

  # NOTE - Return type: tsm/fsm flags và state counts giờ là list, mỗi phần tử
  # tương ứng với 1 task norm (list[i-1] ứng với task #i).
  return num_tasks_norm, num_tasks_poll, num_signals, fsm_flags, tsm_flags, num_tsm_states_list, num_fsm_states_list, num_hw_api