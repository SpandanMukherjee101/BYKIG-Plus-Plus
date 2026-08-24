import os
import subprocess
import glob

script_dir = os.path.dirname(os.path.abspath(__file__))
interpreter_path = os.path.join(script_dir, "..", "Interpreter.exe")
test_files = glob.glob(os.path.join(script_dir, "**", "*.bykig"), recursive=True)

passed = []
failed = []
hung = []

for tf in test_files:
    try:
        p = subprocess.run([interpreter_path, tf], capture_output=True, text=True, timeout=1)
        if p.returncode == 0:
            passed.append(tf)
        else:
            failed.append(tf)
    except subprocess.TimeoutExpired:
        hung.append(tf)

print(f"Total: {len(test_files)}")
print(f"Passed ({len(passed)}): {', '.join(passed)}")
print(f"Failed ({len(failed)}): {', '.join(failed)}")
print(f"Hung ({len(hung)}): {', '.join(hung)}")
