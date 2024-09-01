find . -type f | while read -r file; do
    # Skip the home.sh file
    if [ "$(basename "$file")" = "home.sh" ]; then
        continue
    fi

    # Perform the replacement in other files
    sed -i "s|/home/students|$HOME|g; s|/home/students|$HOME|g" "$file"
done

echo "Replacement with HOME done."
