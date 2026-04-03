#!/bin/bash

MODULE_PATH=$1

if [ -z "$MODULE_PATH" ]; then
  echo "Usage: ./show_module.sh <module_path>"
  exit 1
fi

echo "🔍 Showing module: $MODULE_PATH"
echo "----------------------------------------"

for f in $(find "$MODULE_PATH" -type f \( -name "*.h" -o -name "*.c" \) | sort); do
  echo ""
  echo "===== $f ====="
  cat "$f"
done

# chmod +x show_module.sh
# next 
# NEXT STEP (EXTREME PERFORMANCE)

# Say:

# "next: make jump_table cache-line aligned + per-core optimized"

# That’s where you hit:

# 💀 ultra low latency
# 💀 NUMA scaling
# 💀 cloud-scale readiness