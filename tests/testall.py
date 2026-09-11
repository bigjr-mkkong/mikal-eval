from pathlib import Path
import subprocess

tests = Path(__file__).resolve().parent
for test in sorted(tests.glob("*-test")):
    print(test.name, flush=True)
    with test.open() as source:
        subprocess.run([str(tests.parent / "run"), "-q"], stdin=source, check=True)
