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

#define LLIST_MAX_NODES 64U

static llist_node_t llist_node_pool[LLIST_MAX_NODES];
static llist_node_t* llist_free_list = NULL;

static void llist_pool_init(void) {
  if (llist_free_list != NULL) {
    return;
  }

  for (uint32_t i = 0; i < (LLIST_MAX_NODES - 1U); i++) {
    llist_node_pool[i].next = &llist_node_pool[i + 1U];
  }

  llist_node_pool[LLIST_MAX_NODES - 1U].next = NULL;
  llist_free_list = &llist_node_pool[0];
}

static llist_node_t* llist_alloc_node(void) {
  llist_pool_init();

  if (llist_free_list == NULL) {
    return NULL;
  }

  llist_node_t* node = llist_free_list;
  llist_free_list = llist_free_list->next;
  node->next = NULL;
  return node;
}

static void llist_free_node(llist_node_t* node) {
  if (node == NULL) {
    return;
  }

  node->next = llist_free_list;
  llist_free_list = node;
}

void llist_init(llist_t* list) {
  if (list == NULL) {
    return;
  }

  list->head = NULL;
  list->tail = NULL;
  list->size = 0;
}

bool llist_is_empty(llist_t* list) {
  if (list == NULL) {
    return true;
  }

  return (list->size == 0) ? true : false;
}

void llist_append(llist_t* list, void* data) {
  if (data == NULL || list == NULL) {
    return;
  }

  llist_node_t* cnter_node = list->head;

  while (cnter_node != NULL) {
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

  llist_node_t* node = llist_alloc_node();
  if (node == NULL) {
    return;
  }

  node->data = data;
  node->next = NULL;
  if (list->head == NULL) {
    list->head = node;
    list->tail = node;
  } else {
    list->tail->next = node;
    list->tail = node;
  }
  list->size++;
}

bool llist_remove(llist_t* list, void* data) {
  if (list == NULL || data == NULL) {
    return false;
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

      if (cnter_node == list->tail) {
        list->tail = prev_node;
      }

      llist_free_node(cnter_node);
      list->size--;

      if (list->size == 0) {
        list->head = NULL;
        list->tail = NULL;
      }

      return true;
    }
    prev_node = cnter_node;
    cnter_node = cnter_node->next;
  }

  return false;
}