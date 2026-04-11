### Get all files inside inlucde/src data

    find include/ -type f -print0 | xargs -0 cat > hell.txt
    find api/ -type f -exec tail -n +1 {} + > api_data.txt
    find include/ -type f -exec tail -n +1 {} + > include_data.txt


### check_headers.py
    python3 scripts/check_headers.py


### l10plusplus.py
    python3 scripts/check_headers.py