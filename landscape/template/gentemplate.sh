#!/bin/bash
f=$1
b=$1
cat template_1 > "$b"".json.template"
../../camera "$f" >> "$b"".json.template"
cat template_2 >> "$b"".json.template"
