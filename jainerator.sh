echo "[ENTRY] call pydscriptor.lstaxer.vlid"
python -m pltf.pycdscriptor.lstaxer.vlid
echo "[ENTRY] call pycdscriptor.jnerator.postgen.cgen"
python -m pltf.pycdscriptor.jnerator.postgen.cgen \
  --yaml sources/app/lstaxizer.yaml \
  --output sources/app/app.c