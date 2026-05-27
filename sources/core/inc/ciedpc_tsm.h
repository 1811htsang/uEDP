/**
 * @file ciedpc_tsm.h
 * @author Shang Huang
 * @brief Header file for Transition State Machine (TSM) management in CIEDPC system
 * @version 0.1
 * @date 2026-04-17
 * 
 * @copyright MIT License
 * 
 */
#ifndef __CIEDPC_TSM_H__
	#define __CIEDPC_TSM_H__

	/**
	 * @brief Khai báo các thư viện sử dụng
	 */
	#include <stdint.h>
	#include "ciedpc_core.h"

	/**
	 * @brief Định nghĩa kiểu dữ liệu để quản lý trạng thái trong máy trạng thái chuyển tiếp (TSM)
	 */
	typedef ui16 tsm_state_id_t;

	/**
	 * @brief Khai báo kiểu dữ liệu để quản lý tin nhắn trong hệ thống CIEDPC
	 * @attention `ciedpc_msg_t` được gọi ở đây để thực thi forward declaration, 
	 * 						cho phép sử dụng con trỏ đến `ciedpc_msg_t` trong các khai báo sau này 
	 * 						mà không cần phải định nghĩa chi tiết của `ciedpc_msg_t` tại thời điểm này, 
	 * 						giúp tránh các vấn đề về phụ thuộc lẫn nhau giữa các cấu trúc dữ liệu trong hệ thống CIEDPC.
	 */
	typedef struct ciedpc_msg_t ciedpc_msg_t;

	/**
	 * @brief Khai báo con trỏ hàm để quản lý handler và callback
	 */
	typedef void (*tsm_func_f)(ciedpc_msg_t*); // Hàm xử lý trạng thái
	typedef void (*tsm_on_state_f)(tsm_state_id_t); // Hàm callback khi trạng thái thay đổi

	/**
	 * @brief Cấu trúc cơ bản định nghĩa một transition trong TSM
	 * @param sig: Tín hiệu của transition
	 * @param next_state: Trạng thái tiếp theo sau khi xử lý tín hiệu
	 * @param tsm_func: Hàm xử lý khi transition kích hoạt
	 */
	typedef struct tsm_trans_t {
		ui8 sig;							
		tsm_state_id_t next_state;		
		tsm_func_f tsm_func;			
	} tsm_trans_t;

	/**
	 * @brief Cấu trúc mô tả thông tin của một state trong TSM
	 * @param state_id: ID của trạng thái, nên tuân thủ tương ứng index theo thứ tự khi khởi tạo state table
	 * @param on_entry: Hàm được gọi khi vào trạng thái
	 * @param on_exit: Hàm được gọi khi thoát trạng thái
	 * @param transitions: Mảng các transition được xử lý trong trạng thái này
	 * @param tran_count: Số lượng transition trong mảng transitions
	 */
	typedef struct tsm_state_desc_t {
		tsm_state_id_t 			state_id;     
		tsm_func_f  				on_entry;     
		tsm_func_f  				on_exit;      
		const tsm_trans_t* 	transitions; 
		ui8      						trans_count;  
	} tsm_state_desc_t;

	/**
	 * @brief Cấu trúc quản lý thông tin của máy trạng thái chuyển tiếp (TSM)
	 * @param cur_state: Trạng thái hiện tại của TSM, sử dụng state_id từ tsm_state_desc_t
	 * @param prev_state: Trạng thái trước đó của TSM, sử dụng state_id từ tsm_state_desc_t
	 * @param state_table: Bảng chứa các mô tả trạng thái, mỗi phần tử trong bảng là một tsm_state_desc_t
	 * @param state_count: Số lượng trạng thái trong bảng state_table
	 * @param on_state_changed: Hàm callback sẽ được gọi khi trạng thái thay đổi, nhận vào ID của trạng thái mới
	 */
	typedef struct tsm_trans_tbl_t {
		tsm_state_id_t  cur_state;    
		tsm_state_id_t  prev_state;   
		const tsm_state_desc_t* state_table;
		ui8      state_count;
		tsm_on_state_f on_state_changed;
	} ciedpc_tsm_t;

	/**
	 * @brief Hàm khởi tạo TSM
	 * @param tsm_table Bảng chứa thông tin về các trạng thái và transition của TSM
	 * @param state_des_table Bảng chứa mô tả chi tiết về các trạng thái của TSM, mỗi phần tử là một tsm_state_desc_t
	 * @param state_count Số lượng trạng thái trong bảng state_des_table
	 * @param initial_state_id ID của trạng thái ban đầu của TSM
	 * @param on_state_changed Hàm callback sẽ được gọi khi trạng thái thay đổi, nhận vào ID của trạng thái mới
	 * @attention Khi khởi tạo trạng thái ban đầu thì initial_state_id được set là 1 state tự trỏ vào chính nó
	 */
	void ciedpc_tsm_init(
		ciedpc_tsm_t* tsm_table, const tsm_state_desc_t* state_des_table, 
		ui8 state_count, tsm_state_id_t initial_state_id, 
		tsm_on_state_f on_state_changed
	);

	/**
	 * @brief Hàm để thực hiện chuyển đổi trạng thái trong TSM dựa trên tín hiệu nhận được
	 * 
	 * @param tsm_table Bảng chứa thông tin về các trạng thái và transition của TSM
	 * @param state_id ID của trạng thái mục tiêu mà TSM sẽ chuyển đến
	 */
	void ciedpc_tsm_trans(ciedpc_tsm_t* tsm_table, tsm_state_id_t state_id);

	/**
	 * @brief Hàm để xử lý tín hiệu và điều hướng trạng thái trong TSM
	 * 
	 * @param tsm_table Bảng chứa thông tin về các trạng thái và transition của TSM
	 * @param msg Con trỏ đến tin nhắn chứa tín hiệu cần xử lý để điều hướng trạng thái trong TSM
	 */
	void ciedpc_tsm_dispatch(ciedpc_tsm_t* tsm_table, ciedpc_msg_t* msg); 

#endif //__CIEDPC_TSM_H__

