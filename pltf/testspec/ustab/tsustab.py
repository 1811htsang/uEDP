from .gnnerate_ustab import generate_ustab_from_kconfig
from .xportstax_ustab import ustab_xport
kconfig = generate_ustab_from_kconfig(".config")
ustab_xport()
