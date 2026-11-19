# Automake Options: Use 'foreign' to relax GNU standards compliance.
AUTOMAKE_OPTIONS = foreign

# --- 1. CORE SUBDIRECTORIES ---
# These directories are built unconditionally.
SUBDIRS = ConvertAtlantis \
	atassess \
	atecology \
	ateconomic \
	atlantisUtil \
    atharvest \
	atmanage \
	atimplementation \
	atphysics \
	sjwlib \
	atSS3Link

# --- 2. CONDITIONAL LINKING SUBDIRECTORIES ---
# These directories are added only if their respective feature flags are enabled (usually via configure).

if BROKER_LINK_ENABLED
	SUBDIRS += atbrokerlink
endif	 

if LINK_ENABLED
	SUBDIRS += atlink
endif

if CLAM_LINK_ENABLED
	SUBDIRS += atCLAMLink
endif

#if SS3_LINK_ENABLED
#	SUBDIRS += atSS3Link
#endif

# --- 3. MAIN EXECUTABLE DIRECTORY ---
# The main program directory is typically built last.
SUBDIRS += atlantismain