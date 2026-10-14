#!/usr/bin/env python3
"""Compare two Atlantis output directories numerically.

The script reports, for every comparable array:

- maximum absolute difference
- maximum relative difference
- mean absolute difference
- RMSE
- number and fraction of values that differ

Relative difference uses max(|a|, |b|, REL_FLOOR) as the denominator so a
zero or tiny value does not inflate the ratio.

Exit status is 0 only when every compared array is exactly equal.
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

import numpy as np
from netCDF4 import Dataset


## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##
## ~ Files compared for the NESP biology run ~ ##
## ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ ##

NETCDF_FILES = (
    "output.nc",
    "outputTOT.nc",
    "outputPROD.nc",
)

TEXT_FILES = (
    "outputBiomIndx.txt",
    "outputBoxBiomass.txt",
    "outputMort.txt",
    "outputSpecificMort.txt",
    "outputMortPerPred.txt",
    "outputSpecificPredMort.txt",
    "outputPredPropCheck.txt",
    "outputDietCheck.txt",
    "outputMigrationArray.txt",
)

# Log text, converted XML, and the copied geometry are not model state.
SKIP_NAMES = {
    "log.txt",
    "delete_to_halt_run",
}

REL_FLOOR = 1.0e-30
REL_THRESHOLDS = (0.0, 1.0e-12, 1.0e-9, 1.0e-6)

# Global attributes that change between runs without changing the science.
IGNORED_ATTRIBUTE_NAMES = {
    "history",
    "History",
    "date_created",
    "date_modified",
    "creation_date",
}


def array_metrics(left: np.ndarray, right: np.ndarray) -> dict:
    """Summarise the difference between two numeric arrays of the same shape."""
    left_values = np.asarray(left, dtype=np.float64)
    right_values = np.asarray(right, dtype=np.float64)

    left_finite = np.isfinite(left_values)
    right_finite = np.isfinite(right_values)
    both_finite = left_finite & right_finite
    nan_mismatch = left_finite != right_finite

    n_values = left_values.size
    if n_values == 0:
        return {
            "n_values": 0,
            "n_differ": 0,
            "fraction_differ": 0.0,
            "max_abs": 0.0,
            "mean_abs": 0.0,
            "rmse": 0.0,
            "max_rel": 0.0,
            "n_rel_above": {threshold: 0 for threshold in REL_THRESHOLDS},
            "n_nonfinite_mismatch": int(np.count_nonzero(nan_mismatch)),
            "identical": not bool(np.any(nan_mismatch)),
        }

    difference = np.zeros(left_values.shape, dtype=np.float64)
    difference[both_finite] = left_values[both_finite] - right_values[both_finite]
    absolute_difference = np.abs(difference)

    equal_mask = np.zeros(left_values.shape, dtype=bool)
    equal_mask[both_finite] = left_values[both_finite] == right_values[both_finite]
    equal_mask[~both_finite] = ~nan_mismatch[~both_finite]
    n_differ = int(np.count_nonzero(~equal_mask))

    if np.any(both_finite):
        finite_abs = absolute_difference[both_finite]
        max_abs = float(np.max(finite_abs))
        mean_abs = float(np.mean(finite_abs))
        rmse = float(np.sqrt(np.mean(finite_abs ** 2)))
        denominator = np.maximum(
            np.maximum(np.abs(left_values[both_finite]), np.abs(right_values[both_finite])),
            REL_FLOOR,
        )
        relative = finite_abs / denominator
        max_rel = float(np.max(relative))
        n_rel_above = {
            threshold: int(np.count_nonzero(relative > threshold))
            for threshold in REL_THRESHOLDS
        }
    else:
        max_abs = 0.0
        mean_abs = 0.0
        rmse = 0.0
        max_rel = 0.0
        n_rel_above = {threshold: 0 for threshold in REL_THRESHOLDS}

    identical = n_differ == 0 and not bool(np.any(nan_mismatch))
    return {
        "n_values": int(n_values),
        "n_differ": n_differ,
        "fraction_differ": n_differ / n_values,
        "max_abs": max_abs,
        "mean_abs": mean_abs,
        "rmse": rmse,
        "max_rel": max_rel,
        "n_rel_above": n_rel_above,
        "n_nonfinite_mismatch": int(np.count_nonzero(nan_mismatch)),
        "identical": identical,
    }


def format_metrics_line(name: str, metrics: dict) -> str:
    """One readable line for a compared array."""
    thresholds = " ".join(
        f"n_rel>{threshold:g}={metrics['n_rel_above'][threshold]}"
        for threshold in REL_THRESHOLDS
    )
    return (
        f"{name}\n"
        f"  n={metrics['n_values']}  n_differ={metrics['n_differ']}  "
        f"fraction={metrics['fraction_differ']:.6g}  identical={metrics['identical']}\n"
        f"  max_abs={metrics['max_abs']:.6g}  mean_abs={metrics['mean_abs']:.6g}  "
        f"rmse={metrics['rmse']:.6g}  max_rel={metrics['max_rel']:.6g}\n"
        f"  {thresholds}  nonfinite_mismatch={metrics['n_nonfinite_mismatch']}"
    )


def is_numeric_dtype(dtype: np.dtype) -> bool:
    """True for integer and floating NetCDF variables."""
    return np.issubdtype(dtype, np.number)


def compare_numeric_arrays(name: str, left: np.ndarray, right: np.ndarray, report: list[str]) -> bool:
    """Append metrics. Return True when the arrays match exactly."""
    if left.shape != right.shape:
        report.append(
            f"{name}\n  SHAPE MISMATCH left={left.shape} right={right.shape}"
        )
        return False
    if left.dtype != right.dtype and not (
        is_numeric_dtype(left.dtype) and is_numeric_dtype(right.dtype)
    ):
        report.append(
            f"{name}\n  DTYPE MISMATCH left={left.dtype} right={right.dtype}"
        )
        return False
    metrics = array_metrics(left, right)
    report.append(format_metrics_line(name, metrics))
    return bool(metrics["identical"])


def load_text_numbers(path: Path) -> np.ndarray:
    """Read every floating-point token from an Atlantis diagnostic text file.

    Header words that are not numbers are ignored. This matches the
    whitespace tables Atlantis writes (BiomIndx, BoxBiomass, mortality).
    """
    if path.stat().st_size == 0:
        return np.array([], dtype=np.float64)
    values: list[float] = []
    with path.open("r", encoding="utf-8", errors="replace") as handle:
        for line in handle:
            for token in line.split():
                try:
                    values.append(float(token))
                except ValueError:
                    continue
    return np.asarray(values, dtype=np.float64)


def compare_text_file(left_dir: Path, right_dir: Path, filename: str, report: list[str]) -> bool:
    """Compare one diagnostic text file. Missing on both sides is a skip."""
    left_path = left_dir / filename
    right_path = right_dir / filename
    left_exists = left_path.is_file()
    right_exists = right_path.is_file()
    if not left_exists and not right_exists:
        report.append(f"{filename}\n  skipped: absent from both directories")
        return True
    if left_exists != right_exists:
        # The archived live run was cleaned by the calibration tool, so some
        # diagnostics exist only in a fresh timed run. That is not a model change.
        # A file present in the baseline and missing from the candidate is a failure.
        if left_exists and not right_exists:
            report.append(f"{filename}\n  MISSING from the candidate")
            return False
        report.append(
            f"{filename}\n  not in the baseline archive, so it was not compared"
        )
        return True
    left_values = load_text_numbers(left_path)
    right_values = load_text_numbers(right_path)
    return compare_numeric_arrays(filename, left_values, right_values, report)


def compare_netcdf_file(left_dir: Path, right_dir: Path, filename: str, report: list[str]) -> bool:
    """Compare numeric variables in one NetCDF file. Attributes are not science."""
    left_path = left_dir / filename
    right_path = right_dir / filename
    left_exists = left_path.is_file()
    right_exists = right_path.is_file()
    if not left_exists and not right_exists:
        report.append(f"{filename}\n  skipped: absent from both directories")
        return True
    if left_exists != right_exists:
        if left_exists and not right_exists:
            report.append(f"{filename}\n  MISSING from the candidate")
            return False
        report.append(
            f"{filename}\n  not in the baseline archive, so it was not compared"
        )
        return True

    all_identical = True
    with Dataset(left_path, "r") as left_nc, Dataset(right_path, "r") as right_nc:
        left_names = set(left_nc.variables)
        right_names = set(right_nc.variables)
        only_left = sorted(left_names - right_names)
        only_right = sorted(right_names - left_names)
        if only_left or only_right:
            report.append(
                f"{filename} variable set differs\n"
                f"  only left: {only_left}\n"
                f"  only right: {only_right}"
            )
            all_identical = False

        for variable_name in sorted(left_names & right_names):
            left_variable = left_nc.variables[variable_name]
            right_variable = right_nc.variables[variable_name]
            qualified = f"{filename}:{variable_name}"
            if not is_numeric_dtype(left_variable.dtype):
                left_text = np.asarray(left_variable[:])
                right_text = np.asarray(right_variable[:])
                same = left_text.shape == right_text.shape and np.array_equal(left_text, right_text)
                report.append(f"{qualified}\n  non-numeric identical={same}")
                all_identical = all_identical and same
                continue
            left_values = np.asarray(left_variable[:])
            right_values = np.asarray(right_variable[:])
            # Masked fill values become NaN so a fill mismatch is visible.
            if np.ma.isMaskedArray(left_values):
                left_values = left_values.filled(np.nan)
            if np.ma.isMaskedArray(right_values):
                right_values = right_values.filled(np.nan)
            matched = compare_numeric_arrays(qualified, left_values, right_values, report)
            all_identical = all_identical and matched
    return all_identical


def compare_output_directories(left_dir: Path, right_dir: Path) -> tuple[bool, str]:
    """Compare the NESP output set. Return (identical, report text)."""
    if not left_dir.is_dir():
        raise FileNotFoundError(f"Left output directory does not exist: {left_dir}")
    if not right_dir.is_dir():
        raise FileNotFoundError(f"Right output directory does not exist: {right_dir}")

    report = [
        "Atlantis output comparison",
        f"left:  {left_dir}",
        f"right: {right_dir}",
        "",
    ]
    all_identical = True
    for filename in NETCDF_FILES:
        file_matches = compare_netcdf_file(left_dir, right_dir, filename, report)
        all_identical = file_matches and all_identical
    for filename in TEXT_FILES:
        file_matches = compare_text_file(left_dir, right_dir, filename, report)
        all_identical = file_matches and all_identical

    report.append("")
    report.append(f"IDENTICAL={all_identical}")
    return all_identical, "\n".join(report) + "\n"


def main() -> int:
    parser = argparse.ArgumentParser(
        description="Compare two Atlantis output directories numerically."
    )
    parser.add_argument("left", type=Path, help="Baseline output directory")
    parser.add_argument("right", type=Path, help="Candidate output directory")
    parser.add_argument(
        "--report",
        type=Path,
        default=None,
        help="Optional path to write the full comparison report",
    )
    args = parser.parse_args()

    identical, report_text = compare_output_directories(args.left, args.right)
    sys.stdout.write(report_text)
    if args.report is not None:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(report_text, encoding="utf-8")
    return 0 if identical else 1


if __name__ == "__main__":
    raise SystemExit(main())
