#!/bin/bash

BACKUP_ROOT="$HOME/my_backup_manager/backups"
LOG_FILE="$HOME/my_backup_manager/activity.log"
RECORD_FILE="$HOME/my_backup_manager.records.db"

mkdir -p "$BACKUP_ROOT"
touch "LOG_FILE" "$RECORD_FILE"


log_activity()
{
    local message="$1"
    local timestamp
    timestamp=$(date "+%Y-%m-%d %H:%M:%S")
    echo "[$timestamp] $message" >> "$LOG_FILE"
}

press_enter_to_continue()
{
    echo ""
    read -rp "Press <Enter> to continue..." _
}

is_valid_directory()
{
    local path="$1"
    [ -d "$path" ]
}

show_disk_space()
{
    local path="$1"
    echo "Available disk space:"
    df -h "$path" | awk 'NR==1 || NR==2'
}

#==========

create_backup()
{
    read -rp "Enter the full path of the directory to be backed up: " src_dir

    if ! is_valid_directory "$src_dir"; then
        echo "Error: '$src_dir' is not a valid directory"
        log_activity "Failed backup attempt - invalid directory: $src_dir"
        press_enter_to_continue
        return 1
    fi 

    # Show the disk space on the backup destination before proceeding
    show_disk_space "$BACKUP_ROOT"

    read -rp "Proceed with backup? (y/n): " confirm
    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        echo "Backup cancelled."
        log_activity "Backup cancelled by user for: $src_dir"
        press_enter_to_continue
        return 0
    fi

    local timestamp
    timestamp=$(date "+%Y%m%d_%H%M%S")
    local dir_name
    dir_name=$(basename "$src_dir")
    local backup_name="${dir_name}_${timestamp}.tar.gz"
    local backup_path="$BACKUP_ROOT/$backup_name"

    echo "Creating backup, please wait..."
    if tar -czf "$backup_path" -C "$(dirname "$src_dir")" "$dir_name" 2>/tmp/backup_err.log; then
        local size
        size=$(du -h "$backup_path" | cut -f1)
        echo "Backup created successfully at: $backup_path ($size)"

        # Record: name | source |timestamp | size
        echo "$backup_name|$src_dir|$timestamp|$size" >> "$RECORD_FILE"
        log_activity "Backup created at: $backup_name (source: $src_dir, size: $size)"
    else
        echo "Error: Backup failed. see reason below:"
        cat /tmp/backup_err.log
        log_activity "Failed backup for: $src_dir"
    fi

    press_enter_to_continue
}


list_backups()
{
    BACKUP_LINES=()
    if [ ! -s "$RECORD_FILE" ]; then
        echo "No backups found"
        return 0
    fi
    
    printf "%-4s %-35s %-30s %-20s %-8s\n" "No." "Backup Name" "Source Directory" "Timestamp" "Size"


    local i=1
    while IFS='|' read -r name source ts size; do 
        [ -z "$name" ] && continue 
        # COnfirm and list backup that exists
        if [ -f "$BACKUP_ROOT/$name" ]; then 
            printf "%-4s %-35s %-30s %-20s %-8s\n" "$i" "$name" "$source" "$ts" "$size"
            BACKUP_LINES+=("$name|$source|$ts|$size")
            ((i++))
        fi 
    done < "$RECORD_FILE"

    if [ "${#BACKUP_LINES[@]}" -eq 0 ]; then 
        echo "No backups found"
    fi 
}

view_backup_history()
{
    echo "==== Showing backup history ====="
    list_backups
    press_enter_to_continue
}

restore_backup()
{
    echo "-====== Restore backup ======"
    list_backups

    if [ "${#BACKUP_LINES[@]}" -eq 0 ]; then
        press_enter_to_continue
        return 0
    fi

    read -rp "Enter the number of the backup to restore (or 0 to cancel): " choice
 
    if ! [[ "$choice" =~ ^[0-9]+$ ]]; then
        echo "Invalid input. Please enter a number."
        press_enter_to_continue
        return 1
    fi

    if [ "$choice" -eq 0 ]; then
        echo "Restore cancelled."
        press_enter_to_continue
        return 0
    fi

    if [ "$choice" -lt 1 ] || [ "$choice" -gt "${#BACKUP_LINES[@]}" ]; then
        echo "Error: Invalid selection."
        press_enter_to_continue
        return 1
    fi

    local selected="${BACKUP_LINES[$((choice-1))]}"
    IFS='|' read -r name source ts size <<< "$selected"
    local backup_path="$BACKUP_ROOT/$name"
 
    if [ ! -f "$backup_path" ]; then
        echo "Error: Backup file is missing on disk: $backup_path"
        log_activity "FAILED restore - missing backup file: $name"
        press_enter_to_continue
        return 1
    fi
 
    read -rp "Enter destination directory for restore [default: $source]: " dest_dir
    dest_dir="${dest_dir:-$source}"
 
    if [ ! -d "$dest_dir" ]; then
        read -rp "Destination '$dest_dir' does not exist. Create it? (y/n): " make_dir
        if [[ "$make_dir" =~ ^[Yy]$ ]]; then
            mkdir -p "$dest_dir"
        else
            echo "Restore cancelled."
            log_activity "Restore cancelled - destination missing: $dest_dir"
            press_enter_to_continue
            return 0
        fi
    fi
 
    echo "Restoring backup, please wait..."
    if tar -xzf "$backup_path" -C "$dest_dir" 2>/tmp/restore_err.log; then
        echo "Restore completed successfully to: $dest_dir"
        log_activity "Backup restored: $name -> $dest_dir"
    else
        echo "Error: Restore failed. See details below:"
        cat /tmp/restore_err.log
        log_activity "FAILED restore: $name -> $dest_dir"
    fi
 
    press_enter_to_continue
}


delete_backup() {
    echo "===== Delete a Backup ====="
    list_backups
 
    if [ "${#BACKUP_LINES[@]}" -eq 0 ]; then
        press_enter_to_continue
        return 0
    fi
 
    read -rp "Enter the number of the backup to delete (or 0 to cancel): " choice
 
    if ! [[ "$choice" =~ ^[0-9]+$ ]]; then
        echo "Invalid input. Please enter a number."
        press_enter_to_continue
        return 1
    fi
 
    if [ "$choice" -eq 0 ]; then
        echo "Deletion cancelled."
        press_enter_to_continue
        return 0
    fi
 
    if [ "$choice" -lt 1 ] || [ "$choice" -gt "${#BACKUP_LINES[@]}" ]; then
        echo "Error: Invalid selection."
        press_enter_to_continue
        return 1
    fi
 
    local selected="${BACKUP_LINES[$((choice-1))]}"
    IFS='|' read -r name source ts size <<< "$selected"
 
    read -rp "Are you sure you want to delete '$name'? (y/n): " confirm
    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        echo "Deletion cancelled."
        press_enter_to_continue
        return 0
    fi
 
    rm -f "$BACKUP_ROOT/$name"
    # Rebuild the record file without the deleted entry
    grep -v "^${name}|" "$RECORD_FILE" > "${RECORD_FILE}.tmp" && mv "${RECORD_FILE}.tmp" "$RECORD_FILE"
 
    echo "Backup '$name' deleted."
    log_activity "Backup deleted: $name"
    press_enter_to_continue
}


view_or_clear_log() {
    echo "===== Activity Log ====="
    if [ ! -s "$LOG_FILE" ]; then
        echo "Log is empty."
    else
        cat "$LOG_FILE"
    fi
 
    read -rp "Clear the log? (y/n): " clear_choice
    if [[ "$clear_choice" =~ ^[Yy]$ ]]; then
        read -rp "This will permanently erase all log entries. Confirm? (y/n): " confirm
        if [[ "$confirm" =~ ^[Yy]$ ]]; then
            > "$LOG_FILE"
            echo "Log cleared."
            log_activity "Activity log cleared by user."
        else
            echo "Log not cleared."
        fi
    fi
 
    press_enter_to_continue
}


show_menu() {
    echo "=========================================="
    echo "Linux File Backup and Recovery Manager"
    echo "=========================================="
    echo "1. Create a backup"
    echo "2. Restore a backup"
    echo "3. View backup history"
    echo "4. Delete an existing backup"
    echo "5. View or clear the activity log"
    echo "6. Exit"
    echo "=========================================="
}


main() {
    while true; do
        show_menu
        read -rp "Select an option [1-6]: " option
 
        if [ "$option" == "1" ]; then
            create_backup
        elif [ "$option" == "2" ]; then
            restore_backup
        elif [ "$option" == "3" ]; then
            view_backup_history
        elif [ "$option" == "4" ]; then
            delete_backup
        elif [ "$option" == "5" ]; then
            view_or_clear_log
        elif [ "$option" == "6" ]; then
            echo "Exiting Backup Manager. Goodbye!"
            log_activity "Program exited by user."
            exit 0
        else
            echo "Invalid option. Please choose a number between 1 and 6."
            press_enter_to_continue
        fi
    done
}

# Entry point for the program
main
