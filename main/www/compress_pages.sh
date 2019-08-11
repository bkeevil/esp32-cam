#!/bin/bash
for file in `ls *.html`; do
    echo "Compressing: $file"
    cp "$file" "copy_$file" && \
    gzip -f "$file" && \
    mv "copy_$file" "$file"
done
for file in `ls *.css`; do
    echo "Compressing: $file"
    cp "$file" "copy_$file" && \
    gzip -f "$file" && \
    mv "copy_$file" "$file"
done
for file in `ls *.js`; do
    echo "Compressing: $file"
    cp "$file" "copy_$file" && \
    gzip -f "$file" && \
    mv "copy_$file" "$file"
done