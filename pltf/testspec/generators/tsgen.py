import appcfg_tsgen
import corecfg_tsgen
import palcfg_tsgen
import appdecl_tsgen
import arch_dir_tsgen
import arch_h_tsgen
import arch_c_tsgen

if __name__ == "__main__":
  print("[INFO] testspec.gen is called")
  appcfg_tsgen.main()
  print("[INFO] app configuration is generated")
  corecfg_tsgen.main()
  print("[INFO] core configuration is generated")
  palcfg_tsgen.main()
  print("[INFO] pal configuration is generated")
  appdecl_tsgen.main()
  print("[INFO] app declaration is generated")
  arch_dir_tsgen.main()
  print("[INFO] architecture directory is generated")
  arch_h_tsgen.main()
  print("[INFO] architecture header file is generated")
  arch_c_tsgen.main()
  print("[INFO] architecture source file is generated")
  print("[INFO] testspec.gen has done")