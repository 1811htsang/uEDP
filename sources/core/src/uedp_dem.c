/**
 * @file uedp_dem.c
 * @brief Implementation of Diagnostic Event Management (DEM) in UEDP
 * @version 0.1
 * @copyright MIT License
 */
#include <stddef.h>
#include "uedp_dem.h"

typedef struct internal_dem_slot_t {
  const uedp_dem_cfg_t* cfg;
  ui8                   count;
  uedp_dem_status_t     status;
} internal_dem_slot_t;

sta ui16 g_dem_count = 0;
sta internal_dem_slot_t g_dem_slots[UEDP_DEM_MAX_EVENTS];
sta ui32 g_cond_flags[UEDP_COND_KIND_COUNT];

sta internal_dem_slot_t* internal_dem_find(uedp_fcr_code_t code) {
  for (ui16 i = 0; i < g_dem_count; i++) {
    if (g_dem_slots[i].cfg->code == code) {
      return &g_dem_slots[i];
    }
  }
  return NULL;
}

void uedp_dem_set_table(const uedp_dem_cfg_t* table, ui16 count) {
  g_dem_count = 0;

  if (table == NULL || count > UEDP_DEM_MAX_EVENTS) {
    UEDP_FCR_RAISE_MSG(UEDP_FCR_MSG_INVALID_PARAM, "dem: table NULL or exceeds UEDP_DEM_MAX_EVENTS");
    return;
  }

  for (ui16 i = 0; i < count; i++) {
    g_dem_slots[i] = (internal_dem_slot_t){ .cfg = &table[i], .count = 0, .status = UEDP_DEM_STATUS_IDLE };
  }
  g_dem_count = count;
}

bool uedp_dem_report(uedp_fcr_code_t code) {
  internal_dem_slot_t* slot = internal_dem_find(code);
  if (slot == NULL) {
    return true;
  }

  // FATAL bỏ qua ECS/DBS: panic hoặc reset không thể chờ debounce
  if (uedp_fcr_lookup(code)->severity == UEDP_FCR_SEV_FATAL) {
    return true;
  }

  const uedp_dem_cfg_t* cfg = slot->cfg;
  if (!cfg->enabled) {
    return false;
  }

  if (slot->status == UEDP_DEM_STATUS_CONFIRMED) {
    return true;
  }

  if (slot->count < cfg->debounce_cnt) {
    slot->count++;
  }
  if (slot->count < cfg->debounce_cnt) {
    slot->status = UEDP_DEM_STATUS_DEBOUNCING;
    return false;
  }

  slot->status = UEDP_DEM_STATUS_CONFIRMED;
  g_cond_flags[cfg->cond_kind] |= (1u << (cfg->cond_bit & 31u));
  return true;
}

void uedp_dem_heal(uedp_fcr_code_t code) {
  internal_dem_slot_t* slot = internal_dem_find(code);
  if (slot == NULL) {
    return;
  }

  if (slot->status == UEDP_DEM_STATUS_CONFIRMED) {
    g_cond_flags[slot->cfg->cond_kind] &= ~(1u << (slot->cfg->cond_bit & 31u));
  }
  slot->count = 0;
  slot->status = UEDP_DEM_STATUS_IDLE;
}

uedp_dem_status_t uedp_dem_get_status(uedp_fcr_code_t code) {
  internal_dem_slot_t* slot = internal_dem_find(code);
  return (slot == NULL) ? UEDP_DEM_STATUS_IDLE : slot->status;
}

bool uedp_cond_is_set(uedp_cond_kind_t kind, ui8 bit) {
  if (kind >= UEDP_COND_KIND_COUNT || bit > 31u) {
    return false;
  }
  return ((g_cond_flags[kind] >> bit) & 1u) != 0u;
}
