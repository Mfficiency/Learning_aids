import os
import re

# ============= CONFIGURATION =============
DRY_RUN = True  # Set to False to actually rename folders
TARGET_DIRECTORY = "screenshots"  # Path to folder containing subfolders to rename
# =========================================

def rename_folders(directory=".", dry_run=DRY_RUN):
    """
    Renames folders by:
    - Removing 'ahl-' prefix
    - Converting format to 'x.xx name' (spaces instead of hyphens)
    - Removing '-ai-hl-paper-3' suffix
    
    Args:
        directory: Path to the directory containing folders to rename
        dry_run: If True, only shows what would be renamed without actually renaming
    """
    
    # Get all items in directory
    items = os.listdir(directory)
    
    # Filter only directories
    folders = [item for item in items if os.path.isdir(os.path.join(directory, item))]
    
    renamed_count = 0
    
    if dry_run:
        print("=" * 60)
        print("DRY RUN MODE - No actual changes will be made")
        print("=" * 60 + "\n")
    
    for folder in folders:
        # Check if folder starts with 'ahl-'
        if folder.startswith('ahl-'):
            # Remove 'ahl-' prefix
            without_prefix = folder[4:]
            
            # Remove '-ai-hl-paper-3' or similar suffixes
            without_suffix = re.sub(r'-ai-hl-paper-\d+.*$', '', without_prefix)
            
            # Replace hyphens with spaces
            new_name = without_suffix.replace('-', ' ')
            
            # Build full paths
            old_path = os.path.join(directory, folder)
            new_path = os.path.join(directory, new_name)
            
            # Rename the folder (or just show what would happen)
            if dry_run:
                print(f"Would rename: '{folder}' -> '{new_name}'")
                renamed_count += 1
            else:
                try:
                    os.rename(old_path, new_path)
                    print(f"Renamed: '{folder}' -> '{new_name}'")
                    renamed_count += 1
                except Exception as e:
                    print(f"Error renaming '{folder}': {e}")
    
    print(f"\nTotal folders {'that would be ' if dry_run else ''}renamed: {renamed_count}")
    
    if dry_run:
        print("\n" + "=" * 60)
        print("To actually rename folders, set DRY_RUN = False at the top")
        print("=" * 60)

if __name__ == "__main__":
    print(f"Renaming folders in: {os.path.abspath(TARGET_DIRECTORY)}\n")
    
    # Ask for confirmation only if not in dry run mode
    if not DRY_RUN:
        response = input("Do you want to proceed with renaming? (yes/no): ")
        
        if response.lower() in ['yes', 'y']:
            rename_folders(TARGET_DIRECTORY, dry_run=False)
        else:
            print("Operation cancelled.")
    else:
        rename_folders(TARGET_DIRECTORY, dry_run=True)