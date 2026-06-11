/**
 * @file fsm.c
 * @author Shang Huang
 * @brief Triển khai các hàm và logic liên quan đến Finite State Machine (FSM) trong hệ thống CIEDPC
 * @version 0.1
 * @date 2026-04-16
 * 
 * @copyright MIT License
 * 
 */
#include "ciedpc_fsm.h"
#include "ciedpc_msg.h"

void ciedpc_fsm_go_next(ciedpc_fsm_t* me, state_handler target) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi thay đổi trạng thái của FSM
  pal_enter_critical();

  // Kiểm tra tính hợp lệ của con trỏ FSM và trạng thái mục tiêu
  if (!me || !target) {
    pal_exit_critical();
    return; 
  }

  // Gửi tín hiệu EXIT đến trạng thái hiện tại trước khi chuyển đổi
  if (me->state) {
    ciedpc_msg_t exit_msg;
    exit_msg.sig = CIEDPC_FSM_SIG_EXIT; // Giả định có hàm tạo msg chuẩn
    me->state(&exit_msg);
  }

  // Lưu trạng thái hiện tại vào lịch sử trước khi chuyển đổi
  me->history[me->history_index] = me->state; // Lưu trạng thái hiện tại vào lịch sử
  me->history_index = (me->history_index + 1) % CIEDPC_FSM_HIS_MAX; // Cập nhật chỉ số lịch sử, đảm bảo không vượt quá giới hạn

  // Cập nhật trạng thái hiện tại của FSM thành trạng thái mục tiêu
  me->state = target; 

  // Gửi tín hiệu ENTRY đến trạng thái mục tiêu sau khi chuyển đổi
  ciedpc_msg_t entry_msg;
  entry_msg.sig = CIEDPC_FSM_SIG_ENTRY; // Giả định có hàm tạo msg chuẩn

  // Gửi tín hiệu ENTRY đến trạng thái mục tiêu
  target(&entry_msg);

  // Thoát critical section sau khi hoàn thành việc chuyển đổi trạng thái
  pal_exit_critical();
}

void ciedpc_fsm_go_back(ciedpc_fsm_t* me) {
  // Bảo vệ critical section để đảm bảo tính nhất quán khi thay đổi trạng thái của FSM
  pal_enter_critical();

  // Kiểm tra tính hợp lệ của con trỏ FSM và đảm bảo có trạng thái trong lịch sử để quay lại
  if (!me || !me->state || me->history_count == 0) {
    pal_exit_critical();
    return; 
  }

  // Cập nhật chỉ số lịch sử để trỏ đến trạng thái trước đó
  ui8 prev_index = (me->history_index + CIEDPC_FSM_HIS_MAX - 1) % CIEDPC_FSM_HIS_MAX; // Tính chỉ số của trạng thái trước đó

  // Lấy trạng thái trước đó từ lịch sử
  state_handler previous_state = me->history[me->history_index]; 

  // Đảm bảo rằng trạng thái trước đó không phải là NULL trước khi quay lại
  if (!previous_state) {
    return; 
  }

  // Gửi tín hiệu EXIT đến trạng thái hiện tại trước khi quay lại
  ciedpc_msg_t exit_msg;
  exit_msg.sig = CIEDPC_FSM_SIG_EXIT; // Giả định có hàm tạo msg chuẩn
  me->state(&exit_msg);

  // Cập nhật trạng thái hiện tại của FSM thành trạng thái trước đó
  me->state = previous_state;
  me->history_index = prev_index; // Cập nhật chỉ số lịch sử sau khi quay lại
  me->history_count--; // Giảm số lượng trạng thái đã lưu trong lịch sử sau

  // Gửi tín hiệu ENTRY đến trạng thái trước đó sau khi quay lại
  ciedpc_msg_t entry_msg;
  entry_msg.sig = CIEDPC_FSM_SIG_ENTRY; // Giả định có hàm tạo msg chuẩn

  // Gửi tín hiệu ENTRY đến trạng thái trước đó
  previous_state(&entry_msg);

  // Thoát critical section sau khi hoàn thành việc quay lại trạng thái
  pal_exit_critical();
}
