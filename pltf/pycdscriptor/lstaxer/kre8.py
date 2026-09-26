from dataclasses import dataclass
from typing import Any
import yaml
import pprint

from .lukupmodel import (
	lukupmodel_glbda_logic,
	lukupmodel_outexec_logic,
	lukupmodel_sig_resrc,
	lukupmodel_tnorm_logic,
	lukupmodel_tnorm_resrc,
	lukupmodel_tpoll_logic,
	lukupmodel_tpoll_resrc,
)

@dataclass(frozen=True)
class Kre8Project:
	signals: list[Any]
	tnorm_resources: list[Any]
	tpoll_resources: list[Any]
	tnorm_logic: list[Any]
	tpoll_logic: list[Any]
	gda: list[Any]
	outexec: list[Any]

	def to_generator_context(self) -> dict[str, Any]:
		return {
			'signals': [_model_to_dict(item) for item in self.signals],
			'tnorm_resources': [_model_to_dict(item) for item in self.tnorm_resources],
			'tpoll_resources': [_model_to_dict(item) for item in self.tpoll_resources],
			'tnorm_logic': [_model_to_dict(item) for item in self.tnorm_logic],
			'tpoll_logic': [_model_to_dict(item) for item in self.tpoll_logic],
			'glbda_defs': [_model_to_dict(item) for item in self.gda],
			'outexec': [_model_to_dict(item) for item in self.outexec],
		}

def _model_to_dict(model: Any) -> dict[str, Any]:
	return model.model_dump(
		by_alias=True,
		exclude_none=True,
		exclude_defaults=True,
	)

def build_project_ir(yaml_text: str) -> Kre8Project:
	return Kre8Project(
		signals=lukupmodel_sig_resrc(yaml_text),
		tnorm_resources=lukupmodel_tnorm_resrc(yaml_text),
		tpoll_resources=lukupmodel_tpoll_resrc(yaml_text),
		tnorm_logic=lukupmodel_tnorm_logic(yaml_text),
		tpoll_logic=lukupmodel_tpoll_logic(yaml_text),
		gda=lukupmodel_glbda_logic(yaml_text),
		outexec=lukupmodel_outexec_logic(yaml_text),
	)

def build_generator_context(yaml_text: str) -> dict[str, Any]:
	return build_project_ir(yaml_text).to_generator_context()

# STUB - Add sample usage to testing the build_project_ir and build_generator_context functions
# with open('sources/app/lstaxizer.yaml', 'r', encoding='utf-8') as f:
# 	yaml_sample = f.read()

# generator_context = build_generator_context(yaml_sample)
# pprint.pprint(generator_context)