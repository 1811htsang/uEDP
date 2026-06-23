# Function to get user input for task generation parameters
# Task generation parameters include:
# - Number of tasks norm to generate
# - Number of tasks poll to generate
# - Number of signals to generate
# - Whether to use FSM/TSM
# - Number of TSM states to generate (if TSM is used)
# - Number of FSM states to generate (if FSM is used)
def user_input(DEFAULT_VALS):
  # Number of tasks to generate
  print(f'[INFO] Number of tasks norm to generate (default: {DEFAULT_VALS["num_tasks_norm"]}): ', end='')
  val = input().strip()
  num_tasks_norm = int(val) if val != '' else DEFAULT_VALS["num_tasks_norm"]

  # Number of tasks poll to generate
  print(f'[INFO] Number of tasks poll to generate (default: {DEFAULT_VALS["num_tasks_poll"]}): ', end='')
  val = input().strip()
  num_tasks_poll = int(val) if val != '' else DEFAULT_VALS["num_tasks_poll"]

  # Number of signals to generate
  print(f'[INFO] Number of signals to generate (default: {DEFAULT_VALS["num_signals"]}): ', end='')
  val = input().strip()
  num_signals = int(val) if val != '' else DEFAULT_VALS["num_signals"]

  # Ask user if they want to use FSM/TSM
  print('[INFO] Do you want to use FSM? (y/n, default: n): ', end='')
  if input().strip().lower() == 'y':
    is_use_fsm = True
  else:
    is_use_fsm = False

  print('[INFO] Do you want to use TSM? (y/n, default: n): ', end='')
  if input().strip().lower() == 'y':
    is_use_tsm = True
  else:
    is_use_tsm = False

  num_tsm_states = 0
  if is_use_tsm:
    print(f'[INFO] Number of TSM states to generate (default: {num_tasks_norm}): ', end='')
    val = input().strip()
    num_tsm_states = int(val) if val != '' else num_tasks_norm

  num_fsm_states = 0
  if is_use_fsm:
    print(f'[INFO] Number of FSM states to generate (default: {num_tasks_norm}): ', end='')
    val = input().strip()
    num_fsm_states = int(val) if val != '' else num_tasks_norm

  # TRẢ VỀ CÁC GIÁ TRỊ ĐÃ NHẬP
  return num_tasks_norm, num_tasks_poll, num_signals, is_use_fsm, is_use_tsm, num_tsm_states, num_fsm_states