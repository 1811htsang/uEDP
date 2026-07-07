/**
 * @file llist.c
 * @author Shang Huang
 * @brief This file contains the implementation of the linked list (llist) data structure and its associated functions.
 * @version 0.1
 * @date 2026-07-07
 * @copyright MIT License
 */
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "llist.h"

void llist_init(llist_t* list) {
  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
}

bool llist_is_empty(llist_t* list) {
  return (list->size == 0) ? true : false;
}

void llist_append(llist_t* list, void* data) {
  if (data == NULL || list == NULL) {
    return; // Không thêm dữ liệu NULL vào linked list
  }

  llist_node_t* cnter_node = list->head;

  while (cnter_node->next != NULL) {
    if (cnter_node->data == data) {
      return; // Dữ liệu đã tồn tại trong linked list, không thêm lại
      /**
       * @brief Lưu ý rằng phụ thuộc vào logic,
       * ở đây API được thiết kế phục vụ 
       * tính chất idempotency cho OCE,
       * nên nếu dữ liệu đã tồn tại trong linked list, 
       * chúng ta sẽ không thêm lại.
       */
    }
    cnter_node = cnter_node->next;
  }

  llist_node_t* node = list->head;
  node->data = data;
  node->next = NULL;
  cnter_node->next = node;
  list->size++;
}

bool llist_remove(llist_t* list, void* data) {
  if (list == NULL || data == NULL) {
    return false; // Không thể xóa dữ liệu NULL hoặc từ linked list NULL
  }

  llist_node_t* cnter_node = list->head;
  llist_node_t* prev_node = NULL;

  while (cnter_node != NULL) {
    if (cnter_node->data == data) {
      if (prev_node == NULL) {
        // Nếu node cần xóa là node đầu tiên
        list->head = cnter_node->next;
      } else {
        prev_node->next = cnter_node->next;
      }
      cnter_node = NULL; // Xử lý NULL pointer để tránh memory leak
      list->size--;
      return true; // Xóa thành công
    }
    prev_node = cnter_node;
    cnter_node = cnter_node->next;
  }

  return false; // Dữ liệu không tồn tại trong linked list
}