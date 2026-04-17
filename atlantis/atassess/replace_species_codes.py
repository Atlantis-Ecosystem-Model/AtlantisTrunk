#!/usr/bin/env python3
"""
Replace species codes in parameter names in a .prm file.

This script reads a code mapping from an Excel file and replaces species codes
embedded within parameter names in a parameter file, leaving values, comments,
and other content unchanged.
"""

import pandas as pd
import re
import sys
from pathlib import Path


def load_code_mapping(excel_path, model='CEPPRM'):
    """
    Load code mapping from Excel file for specified model.
    
    Returns a dictionary {old_code: new_code} sorted by key length (longest first).
    """
    df = pd.read_excel(excel_path)
    
    # Filter for specified model
    model_df = df[df['Model'] == model].copy()
    
    print(f"Loaded {len(model_df)} code mappings for model '{model}'")
    
    # Create mapping dictionary
    mapping = dict(zip(model_df['Code'].str.strip(), model_df['NewCode'].str.strip()))
    
    # Sort by code length (longest first) to avoid partial matches
    # e.g., process "FPS" before "PS" to avoid replacing PS within FPS
    sorted_mapping = {k: mapping[k] for k in sorted(mapping.keys(), key=len, reverse=True)}
    
    return sorted_mapping


def replace_codes_in_parameter_name(param_name, code_mapping):
    """
    Replace species codes in a parameter name.
    
    Uses regex to ensure exact code matches.
    Codes typically appear as suffixes at the end of parameter names.
    """
    result = param_name
    
    for old_code, new_code in code_mapping.items():
        # Skip if no change needed
        if old_code == new_code:
            continue
            
        # Pattern: match the code at the end of the parameter name
        # The code must either:
        # 1. Be at the very end of the string, OR
        # 2. Be followed by an underscore and then more characters
        # This handles cases like:
        # - flagdemFPS (code at end)
        # - Kmax_coefft_FPS (code at end after underscore)
        # - C_CEP_T15 (code in middle followed by underscore)
        
        # First, try to match code at the absolute end
        pattern_end = rf'{re.escape(old_code)}$'
        if re.search(pattern_end, result):
            result = re.sub(pattern_end, new_code, result)
            continue
        
        # Then, try to match code followed by underscore (for cases like C_CEP_T15)
        pattern_middle = rf'{re.escape(old_code)}(?=_)'
        if re.search(pattern_middle, result):
            result = re.sub(pattern_middle, new_code, result)
    
    return result


def process_prm_file(input_path, output_path, code_mapping):
    """
    Process the .prm file, replacing species codes in parameter names only.
    """
    lines_processed = 0
    lines_modified = 0
    
    with open(input_path, 'r', encoding='utf-8') as infile, \
         open(output_path, 'w', encoding='utf-8') as outfile:
        
        for line_num, line in enumerate(infile, 1):
            lines_processed += 1
            
            # Strip newline for processing but preserve it
            line_content = line.rstrip('\n\r')
            
            # Skip empty lines and comment lines
            if not line_content.strip() or line_content.strip().startswith('#'):
                outfile.write(line)
                continue
            
            # Split line into tokens
            tokens = line_content.split(None, 1)  # Split on first whitespace
            
            if not tokens:
                outfile.write(line)
                continue
            
            # First token is the parameter name
            param_name = tokens[0]
            rest_of_line = tokens[1] if len(tokens) > 1 else ''
            
            # Replace codes in parameter name
            new_param_name = replace_codes_in_parameter_name(param_name, code_mapping)
            
            # Track modifications
            if new_param_name != param_name:
                lines_modified += 1
                if lines_modified <= 10:  # Show first 10 changes
                    print(f"  Line {line_num}: {param_name} → {new_param_name}")
            
            # Reconstruct line
            if rest_of_line:
                # Preserve original whitespace between param name and rest
                # Find where the param name ends in original line
                param_end_pos = line_content.find(param_name) + len(param_name)
                original_space = line_content[param_end_pos:param_end_pos + (len(line_content) - param_end_pos - len(rest_of_line))]
                new_line = new_param_name + original_space + rest_of_line + '\n'
            else:
                new_line = new_param_name + '\n'
            
            outfile.write(new_line)
    
    print(f"\nProcessed {lines_processed} lines")
    print(f"Modified {lines_modified} parameter names")
    
    return lines_modified


def main():
    # File paths
    prm_file = "/Users/ful083/Work/blue_econ/NESP_offshore_wind_cum_effects_project/Model_relevant_NESP_Atlantis_EwE/Atlantis_redo/CEP_biol_standard_CC_BF_burn-in_to_recode.prm"
    mapping_file = "/Users/ful083/Work/blue_econ/NESP_offshore_wind_cum_effects_project/Model_relevant_NESP_Atlantis_EwE/Atlantis_redo/code_mapping.xlsx"
    
    # Create output filename
    input_path = Path(prm_file)
    output_path = input_path.parent / f"{input_path.stem}_recoded{input_path.suffix}"
    
    print("=" * 70)
    print("Species Code Replacement in Parameter Names")
    print("=" * 70)
    print(f"Input file:  {prm_file}")
    print(f"Mapping file: {mapping_file}")
    print(f"Output file: {output_path}")
    print()
    
    # Load code mapping
    code_mapping = load_code_mapping(mapping_file, model='CEPPRM')
    
    # Show some example mappings
    print("\nSample mappings:")
    for i, (old, new) in enumerate(list(code_mapping.items())[:10]):
        print(f"  {old} → {new}")
    if len(code_mapping) > 10:
        print(f"  ... and {len(code_mapping) - 10} more")
    print()
    
    # Process the file
    print("Processing file...")
    print("First 10 modifications:")
    modifications = process_prm_file(prm_file, output_path, code_mapping)
    
    print("=" * 70)
    print(f"✓ Complete! Output written to: {output_path}")
    print("=" * 70)
    
    return 0


if __name__ == "__main__":
    sys.exit(main())
