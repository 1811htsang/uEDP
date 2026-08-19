from pydantic import BaseModel, field_validator
from typing import List, Optional, Dict

# NOTE - Validation Strategy for lstaxer.vlid 
"""
1. Dangling Alias Check
2. Context-Type Match
3. UST cross Reference
4. Post Resource Existance
5. Policy Alignment Check
"""
