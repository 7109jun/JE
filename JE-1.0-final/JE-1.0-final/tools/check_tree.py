from pathlib import Path

EXCLUDED_DIRS = {".git"}

def is_excluded(path):
    return any(part == ".git" or part == "build" or part.startswith("build-") or part == "target" for part in path.parts)

files = [p for p in Path(".").rglob("*") if p.is_file() and not is_excluded(p)]
if len(files) > 60:
    raise SystemExit(f"project has {len(files)} source/package files; limit is 60")
print(f"JE project: {len(files)} files (limit 60)")
