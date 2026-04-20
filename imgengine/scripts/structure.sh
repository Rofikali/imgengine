# #!/bin/bash
# # Recursively list all directories and files, sorted A-Z
# ls -R | grep ":$" | sed -e 's/:$//' -e 's/[^-][^\/]*\//--/g' -e 's/^/   /' -e 's/-/|/'

# #!/bin/bash
# # Updated structure script to show BOTH files and folders A-Z
# find "${1:-.}" -not -path '*/.*' | sort | sed -e 's/[^-][^\/]*\// |  /g' -e 's/|  \([^|]\)/|-- \1/'



#!/bin/bash
# Lists all files as "path/to/file.ext", sorted A to Z
# -type f: only shows files (folders are implied by the path)
# -not -path '*/.*': ignores hidden files/folders (like .git)
# find "${1:-.}" -type f -not -path '*/.*' | sed 's|^\./||' | sort


# #!/bin/bash
# OUTPUT_FILE="project_structure.txt"

# find "${1:-.}" -not -path '*/.*' | sort | sed -e 's/[^-][^\/]*\// |  /g' -e 's/|  \([^|]\)/|-- \1/' > "$OUTPUT_FILE"

# echo "Structure saved to $OUTPUT_FILE"


#!/bin/bash
OUTPUT_FILE="project_structure.md"

# Add folder names you want to skip inside the \( \) brackets
# Separate them with -o (which means "OR")
find "${1:-.}" \
    \( -name "build" -o -name "__pycache__" -o -name "bin" -o -name "lib" -o -name "scripts" \) -prune \
    -o -not -path '*/.*' -print | \
    sort | sed -e 's/[^-][^\/]*\// |  /g' -e 's/|  \([^|]\)/|-- \1/' > "$OUTPUT_FILE"

echo "Structure saved to $OUTPUT_FILE (excluding build, pycache, bin, and lib)"
