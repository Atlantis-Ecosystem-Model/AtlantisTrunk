"""Summarise ThreadSanitizer data-race reports from the Atlantis box loop.

Each report is reduced to one line:
    variable | access 1 (type, file:line, function) | access 2 (type, file:line, function)

The variable is the global name when TSan knows it, otherwise the allocation
site of the heap block. Duplicate lines are counted.

Usage:
    python3 performance/summarise_tsan_races.py <tsan_log> [<tsan_log> ...]
"""

import re
import sys
from collections import Counter


## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Patterns for the parts of a report we need ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
access_pattern = re.compile(r"^\s*(?:Previous |Atomic )?(read|write|Read|Write)\b.*of size \d+", re.IGNORECASE)
frame_pattern = re.compile(r"^\s*#(\d+) (\S+) (\S+):(\d+)")
global_pattern = re.compile(r"Location is global '([^']+)'")
heap_pattern = re.compile(r"Location is heap block")
atlantis_root = "BethAtlantis_fix/atlantis/"


def short_path(path):
    if atlantis_root in path:
        return path.split(atlantis_root, 1)[1]
    return path.rsplit("/", 1)[-1]


def frame_text(function_name, path, line):
    return f"{short_path(path)}:{line} {function_name}"


def summarise_report(report_lines):
    accesses = []
    variable = "unknown"
    index = 0
    while index < len(report_lines):
        line = report_lines[index]
        access_match = access_pattern.match(line)
        if access_match and len(accesses) < 2:
            access_type = "W" if "rite" in line else "R"
            first_frame = "no stack"
            if index + 1 < len(report_lines):
                frame_match = frame_pattern.match(report_lines[index + 1])
                if frame_match:
                    first_frame = frame_text(frame_match.group(2), frame_match.group(3), frame_match.group(4))
            accesses.append(f"{access_type} {first_frame}")
        global_match = global_pattern.search(line)
        if global_match:
            variable = global_match.group(1)
        elif heap_pattern.search(line):
            # The allocation stack follows; skip allocator frames and keep the first Atlantis caller.
            look_ahead = index + 1
            while look_ahead < len(report_lines):
                frame_match = frame_pattern.match(report_lines[look_ahead])
                if not frame_match:
                    break
                function_name = frame_match.group(2)
                if function_name not in ("malloc", "calloc", "realloc") and not function_name.startswith(("d_alloc", "i_alloc", "l_alloc", "alloc", "Util_Alloc")):
                    variable = "heap@" + frame_text(function_name, frame_match.group(3), frame_match.group(4))
                    break
                look_ahead += 1
        index += 1
    while len(accesses) < 2:
        accesses.append("?")
    return f"{variable} | {accesses[0]} | {accesses[1]}"


## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Split logs into reports and count them ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
if len(sys.argv) < 2:
    raise SystemExit(__doc__)

race_counts = Counter()
for log_path in sys.argv[1:]:
    with open(log_path) as log_file:
        current_report = None
        for raw_line in log_file:
            if "WARNING: ThreadSanitizer: data race" in raw_line:
                current_report = []
            elif raw_line.startswith("SUMMARY: ThreadSanitizer") and current_report is not None:
                race_counts[summarise_report(current_report)] += 1
                current_report = None
            elif current_report is not None:
                current_report.append(raw_line.rstrip("\n"))

for summary_line in sorted(race_counts):
    print(f"{race_counts[summary_line]:4d}  {summary_line}")
