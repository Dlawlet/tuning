find . -type f | while read -r file; do 
	sed -i "s|/home/students|$HOME|g; s|/home/students|$HOME|g" "$file"
done

echo "Replacement with HOME done."
