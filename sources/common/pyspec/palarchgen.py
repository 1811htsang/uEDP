"""Prototype generator for PAL architecture folders.

This module turns `PAL_HW_API_{i}_NAME` entries into architecture folders and
simple `.h/.c` stubs that follow the same shape as the existing `stm32_f103`
example, but with a dynamic API prefix.
"""

from __future__ import annotations

import argparse
import os
import re
from pathlib import Path

try:
	from sources.common.kconfiglib import kconfiglib
except Exception:  # pragma: no cover - keep prototype import-friendly
	kconfiglib = None


DEFAULT_OUTPUT_ROOT = Path("sources/pal/arch")


def _normalize_name(raw_name: str) -> str:
	"""Convert a Kconfig string into a filesystem and identifier friendly name."""

	cleaned = raw_name.strip().lower()
	cleaned = re.sub(r"[^a-z0-9]+", "_", cleaned)
	cleaned = re.sub(r"_+", "_", cleaned).strip("_")
	return cleaned or "unnamed_hw"


def _extract_hw_api_names(kconf) -> list[str]:
	hw_api_names: list[str] = []

	for sym in getattr(kconf, "unique_defined_syms", []):
		if not getattr(sym, "name", ""):
			continue

		if not sym.name.startswith("PAL_HW_API_") or not sym.name.endswith("_NAME"):
			continue

		value = getattr(sym, "str_value", "") or getattr(sym, "config_string", "")
		if not value:
			continue

		normalized = _normalize_name(value)
		if normalized not in hw_api_names:
			hw_api_names.append(normalized)

	return hw_api_names


def _extract_pal_name(kconf) -> str:
	for sym in getattr(kconf, "unique_defined_syms", []):
		if getattr(sym, "name", "") != "PAL_NAME":
			continue

		value = getattr(sym, "str_value", "") or getattr(sym, "config_string", "")
		if value:
			return _normalize_name(value)

	return "pal"


def _header_guard(hw_name: str) -> str:
	return f"__{hw_name.upper()}_ARCH_H__"


def _function_prefix(hw_name: str) -> str:
	return f"pal_{hw_name}"


def _api_function_name(prefix: str, api_name: str) -> str:
	return f"{prefix}_{api_name}"


def _render_empty_function(signature: str) -> str:
	return f"{signature} {{\n\n}}"


def _render_header(hw_name: str, pal_name: str, api_names: list[str]) -> str:
	guard = _header_guard(hw_name)
	prefix = pal_name
	function_declarations = "\n".join(
		f"\tvoid {_api_function_name(prefix, api_name)}(void);"
		for api_name in api_names
	)

	return f'''/**
 * @file {hw_name}_arch.h
 * @brief Header file for {hw_name.upper()} Architecture Abstraction Layer in UEDP
 */

#ifndef {guard}
	#define {guard}

	#include "pal_core.h"

{function_declarations}

#endif // {guard}
'''


def _render_source(hw_name: str, pal_name: str, api_names: list[str]) -> str:
	prefix = pal_name
	core_prototypes = [
		"void pal_core_init(void);",
		"void pal_enter_critical(void);",
		"void pal_exit_critical(void);",
		"ui8 pal_math_get_highest_bit32(ui32 mask);",
		"ui32 pal_sys_get_tick(void);",
		"void pal_sys_reset(void);",
		"void pal_sys_fatal(const char* file, ui32 line, const char* msg);",
	]
	api_prototypes = [f"void {_api_function_name(prefix, api_name)}(void);" for api_name in api_names]
	function_blocks = "\n\n".join(
		_render_empty_function(signature)
		for signature in core_prototypes + api_prototypes
	)

	return f'''/**
 * @file {hw_name}_arch.c
 * @brief Implementation of {hw_name.upper()} Architecture Abstraction Layer for UEDP
 */

#include <stdint.h>
#include "{hw_name}_arch.h"
#include "pal_core.h"

static ui8 is_inited = 0x0u;

extern void uedp_timer_tick(void);

static void internal_hardfault_decoder(uint32_t *stack);

/*
 * Empty function bodies for user implementation.
 */
{function_blocks}
'''


def generate_pal_arch_folder(hw_name: str, pal_name: str, api_names: list[str], output_root: Path = DEFAULT_OUTPUT_ROOT) -> Path:
	normalized_name = _normalize_name(hw_name)
	target_dir = output_root / normalized_name
	target_dir.mkdir(parents=True, exist_ok=True)

	header_path = target_dir / f"{normalized_name}_arch.h"
	source_path = target_dir / f"{normalized_name}_arch.c"

	header_path.write_text(_render_header(normalized_name, pal_name, api_names), encoding="utf-8")
	source_path.write_text(_render_source(normalized_name, pal_name, api_names), encoding="utf-8")

	return target_dir


def pal_arch_gen(kconf, output_root: str | os.PathLike[str] = DEFAULT_OUTPUT_ROOT) -> list[Path]:
	generated_dirs: list[Path] = []
	target_root = Path(output_root)
	pal_name = _extract_pal_name(kconf)
	api_names = _extract_hw_api_names(kconf)

	if api_names:
		generated_dirs.append(generate_pal_arch_folder(pal_name, pal_name, api_names, target_root))

	return generated_dirs


def pal_arch_gen_from_kconfig_file(kconfig_path: str, output_root: str | os.PathLike[str] = DEFAULT_OUTPUT_ROOT) -> list[Path]:
	if kconfiglib is None:
		raise RuntimeError("kconfiglib is not available in this environment")

	kconf = kconfiglib.Kconfig(kconfig_path)
	return pal_arch_gen(kconf, output_root)


def _build_arg_parser() -> argparse.ArgumentParser:
	parser = argparse.ArgumentParser(description="Generate PAL arch folders from PAL_HW_API_* Kconfig values.")
	parser.add_argument("--kconfig", required=True, help="Path to the Kconfig file to read.")
	parser.add_argument("--output-root", default=str(DEFAULT_OUTPUT_ROOT), help="Output root folder for generated arch folders.")
	return parser


def main() -> int:
	args = _build_arg_parser().parse_args()
	generated_dirs = pal_arch_gen_from_kconfig_file(args.kconfig, args.output_root)

	for generated_dir in generated_dirs:
		print(f"[OK] Generated {generated_dir}")

	return 0


if __name__ == "__main__":
	raise SystemExit(main())
