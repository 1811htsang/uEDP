from .gnnerate import generate_ustab_from_kconfig
from .xportstax import ustab_xport
kconfig = generate_ustab_from_kconfig(".config")
ustab_xport()
