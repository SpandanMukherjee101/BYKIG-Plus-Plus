import os
import sys
import glob
import time
import datetime
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))
project_dir = os.path.abspath(os.path.join(script_dir, ".."))
interpreter_path = os.path.join(project_dir, "Interpreter.exe")

# Allow custom output file path via CLI argument; default to tests/test_output.txt
if len(sys.argv) > 1:
    output_txt_path = os.path.abspath(sys.argv[1])
else:
    output_txt_path = os.path.join(script_dir, "test_output.txt")

test_files = sorted(glob.glob(os.path.join(script_dir, "**", "*.bykig"), recursive=True))

results = []
passed = []
failed = []
hung = []

TIMEOUT_SECONDS = 1.0

print(f"Running {len(test_files)} BYKIG++ tests...")
print(f"Interpreter : {interpreter_path}")
print(f"Output File : {output_txt_path}\n")

for idx, tf in enumerate(test_files, start=1):
    rel_path = os.path.relpath(tf, project_dir).replace("\\", "/")
    start_time = time.time()
    status = "UNKNOWN"
    returncode = None
    stdout = ""
    stderr = ""

    try:
        p = subprocess.run(
            [interpreter_path, tf],
            capture_output=True,
            text=True,
            encoding='utf-8',
            errors='replace',
            timeout=TIMEOUT_SECONDS,
            cwd=project_dir
        )
        duration = time.time() - start_time
        returncode = p.returncode
        stdout = p.stdout
        stderr = p.stderr

        if returncode == 0:
            status = "PASSED"
            passed.append(rel_path)
            print(f"[{idx:02d}/{len(test_files):02d}] PASS: {rel_path} ({duration:.3f}s)")
        else:
            status = "FAILED"
            failed.append(rel_path)
            print(f"[{idx:02d}/{len(test_files):02d}] FAIL: {rel_path} (exit {returncode}, {duration:.3f}s)")

    except subprocess.TimeoutExpired as e:
        duration = time.time() - start_time
        status = "TIMEOUT"
        hung.append(rel_path)
        stdout = e.stdout or ""
        stderr = e.stderr or ""
        print(f"[{idx:02d}/{len(test_files):02d}] HUNG: {rel_path} (timeout {TIMEOUT_SECONDS}s)")

    results.append({
        "index": idx,
        "path": rel_path,
        "status": status,
        "duration": duration,
        "returncode": returncode,
        "stdout": stdout,
        "stderr": stderr
    })

# Clean up transient files produced by file I/O tests in the project directory
for temp_file in ["scanner_dummy.txt", "test.txt", "test_data.txt"]:
    temp_path = os.path.join(project_dir, temp_file)
    if os.path.exists(temp_path):
        try:
            os.remove(temp_path)
        except OSError:
            pass

# Generate structured text report
lines = []
sep = "=" * 80
sub_sep = "-" * 80

lines.append(sep)
lines.append("                           BYKIG++ TEST SUITE REPORT")
lines.append(sep)
lines.append(f"Date / Time   : {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}")
lines.append(f"Interpreter   : {interpreter_path}")
lines.append(f"Working Dir   : {project_dir}")
lines.append(f"Total Tests   : {len(test_files)}")
lines.append(f"Passed        : {len(passed)}")
lines.append(f"Failed        : {len(failed)}")
lines.append(f"Timeout / Hung: {len(hung)}")
pass_rate = (len(passed) / len(test_files) * 100) if test_files else 0.0
lines.append(f"Pass Rate     : {pass_rate:.1f}%")
lines.append(sep)
lines.append("")
lines.append("SUMMARY")
lines.append(sub_sep)
lines.append(f"PASSED ({len(passed)}):")
if passed:
    for p in passed:
        lines.append(f"  + {p}")
else:
    lines.append("  (none)")

lines.append("")
lines.append(f"FAILED ({len(failed)}):")
if failed:
    for f in failed:
        lines.append(f"  - {f}")
else:
    lines.append("  (none)")

lines.append("")
lines.append(f"TIMEOUT / HUNG ({len(hung)}):")
if hung:
    for h in hung:
        lines.append(f"  ? {h}")
else:
    lines.append("  (none)")
lines.append(sub_sep)
lines.append("")
lines.append("DETAILED TEST EXECUTION RESULTS")
lines.append(sep)

for res in results:
    lines.append("")
    status_header = f"[{res['index']:02d}/{len(test_files):02d}] {res['status']}: {res['path']}"
    meta = f"Duration: {res['duration']:.3f}s"
    if res['returncode'] is not None:
        meta += f", Return Code: {res['returncode']}"
    else:
        meta += f", Terminated after {TIMEOUT_SECONDS}s"

    lines.append(status_header)
    lines.append(f"  ({meta})")
    lines.append(sub_sep)

    stdout = res['stdout']
    if isinstance(stdout, bytes):
        stdout = stdout.decode('utf-8', errors='replace')
    stdout = (stdout or "").strip()
    if stdout:
        lines.append("STDOUT:")
        for line in stdout.splitlines():
            lines.append(f"  | {line}")
    else:
        lines.append("STDOUT: (none)")

    stderr = res['stderr']
    if isinstance(stderr, bytes):
        stderr = stderr.decode('utf-8', errors='replace')
    stderr = (stderr or "").strip()
    if stderr:
        lines.append("STDERR:")
        for line in stderr.splitlines():
            lines.append(f"  ! {line}")

    lines.append(sub_sep)

lines.append("")
lines.append(sep)
lines.append("                                END OF REPORT")
lines.append(sep)

report_content = "\n".join(lines) + "\n"

with open(output_txt_path, "w", encoding="utf-8") as f:
    f.write(report_content)

print(f"\nTest execution complete.")
print(f"Total: {len(test_files)} | Passed: {len(passed)} | Failed: {len(failed)} | Hung: {len(hung)}")
print(f"Pass Rate: {pass_rate:.1f}%")
print(f"Report saved to: {output_txt_path}")
