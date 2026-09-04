try:
  from . import appcfgpgen, corecfgpgen, palcfgpgen, appdeclpgen, archdirpgen, archhpgen, archcpgen, cfpcall
except ImportError:
  import appcfgpgen, corecfgpgen, palcfgpgen, appdeclpgen, archdirpgen, archhpgen, archcpgen, cfpcall

if __name__ == "__main__":
  print("[INFO] testspec.gen is called")
  print("[INFO] generating context from .config")
  context = cfpcall.main()
  print("[INFO] context is generated")
  appcfgpgen.main(context)
  print("[INFO] app configuration is generated")
  corecfgpgen.main(context)
  print("[INFO] core configuration is generated")
  palcfgpgen.main(context)
  print("[INFO] pal configuration is generated")
  appdeclpgen.main(context)
  print("[INFO] app declaration is generated")
  archdirpgen.main(context)
  print("[INFO] architecture directory is generated")
  archhpgen.main(context)
  print("[INFO] architecture header file is generated")
  archcpgen.main(context)
  print("[INFO] architecture source file is generated")
  print("[INFO] testspec.gen has done")