try:
  from . import appcfg_tsgen, corecfg_tsgen, palcfg_tsgen, appdecl_tsgen, arch_dir_tsgen, arch_h_tsgen, arch_c_tsgen, cfpcall_tsgen
except ImportError:
  import appcfg_tsgen, corecfg_tsgen, palcfg_tsgen, appdecl_tsgen, arch_dir_tsgen, arch_h_tsgen, arch_c_tsgen, cfpcall_tsgen

if __name__ == "__main__":
  print("[INFO] testspec.gen is called")
  print("[INFO] generating context from .config")
  context = cfpcall_tsgen.main()
  print("[INFO] context is generated")
  appcfg_tsgen.main(context)
  print("[INFO] app configuration is generated")
  corecfg_tsgen.main(context)
  print("[INFO] core configuration is generated")
  palcfg_tsgen.main(context)
  print("[INFO] pal configuration is generated")
  appdecl_tsgen.main(context)
  print("[INFO] app declaration is generated")
  arch_dir_tsgen.main(context)
  print("[INFO] architecture directory is generated")
  arch_h_tsgen.main(context)
  print("[INFO] architecture header file is generated")
  arch_c_tsgen.main(context)
  print("[INFO] architecture source file is generated")
  print("[INFO] testspec.gen has done")