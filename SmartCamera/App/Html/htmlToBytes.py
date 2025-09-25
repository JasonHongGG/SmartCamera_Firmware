import gzip
import os

base_dir = os.path.dirname(os.path.abspath(__file__))
html_path = os.path.join(base_dir, "index.html")
web_index_path = os.path.join(base_dir, "../WebIndex.h")

with open(html_path, "rb") as f:
    html = f.read()
gz = gzip.compress(html)

with open(web_index_path, "w") as out:
    out.write(f"#define index_ov2640_html_gz_len {len(gz)}\n")
    out.write("const unsigned char index_ov2640_html_gz[] = {\n  ")
    out.write(", ".join(f"0x{b:02x}" for b in gz))
    out.write("\n};\n")