#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TYPE_LEN   40
#define DATA_FILE  "history.txt"
#define NUM_CONVERSIONS 8

// Conversion record structure
typedef struct {
    char type[TYPE_LEN];  // e.g. "Celsius to Fahrenheit"
    double inputValue;
    double result;
} ConversionRecord;

// Global state
ConversionRecord *history = NULL;  // dynamic array of records
int historyCount = 0;              // number of records currently stored

// Function pointer types

// A conversion function takes one value and returns the converted value.
typedef double (*ConversionFunc)(double);

// A precision callback rounds a single record's result to N decimals.
typedef void (*PrecisionFunc)(ConversionRecord *record, int precision);

// A filter callback returns 1 if a record matches a threshold, 0 otherwise.
typedef int (*FilterFunc)(ConversionRecord record, double threshold);

// A compare callback returns 1 if record a should come after record b.
typedef int (*CompareFunc)(ConversionRecord a, ConversionRecord b);


// Utility functions

// Clears leftover input in the buffer after scanf
void clearInputBuffer(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}

// Reads a double safely; returns 1 on success, 0 on failure
int readDouble(double *value) {
    if (scanf("%lf", value) != 1) {
        clearInputBuffer();
        return 0;
    }
    clearInputBuffer();
    return 1;
}

// Reads an integer safely; returns 1 on success, 0 on failure
int readInt(int *value) {
    if (scanf("%d", value) != 1) {
        clearInputBuffer();
        return 0;
    }
    clearInputBuffer();
    return 1;
}

// Conversion functions
// Each function matches the ConversionFunc signature: double f(double)

double celsiusToFahrenheit(double c) { return (c * 9.0 / 5.0) + 32.0; }
double kmToMiles(double km)          { return km * 0.621371; }
double kgToPounds(double kg)         { return kg * 2.20462; }
double cmToInches(double cm)         { return cm / 2.54; }
double kgToGrams(double kg)          { return kg * 1000.0; }
double minutesToSeconds(double min)  { return min * 60.0; }
double hoursToMinutes(double hr)     { return hr * 60.0; }
double monthsToWeeks(double months)  { return months * 4.34524; }

// Conversion menu table
// Pairs each menu option with its name and its function pointer.
// This is what lets the program pick the right conversion function
// at runtime based on the user's menu choice.
typedef struct {
    const char *name;
    ConversionFunc func;
} ConversionEntry;

ConversionEntry conversionTable[NUM_CONVERSIONS] = {
    { "Celsius to Fahrenheit", celsiusToFahrenheit },
    { "Kilometres to Miles",   kmToMiles },
    { "Kilograms to Pounds",   kgToPounds },
    { "Centimetres to Inches", cmToInches },
    { "Kilograms to Grams",    kgToGrams },
    { "Minutes to Seconds",    minutesToSeconds },
    { "Hours to Minutes",      hoursToMinutes },
    { "Months to Weeks",       monthsToWeeks }
};

// File handling

// Loads history records from the text data file at program start.
// Each line is stored as: type|inputValue|result
void loadHistory(void) {
    FILE *fp = fopen(DATA_FILE, "r");
    if (fp == NULL) {
        // No file yet - that's fine, start with empty history
        return;
    }

    char line[200];
    while (fgets(line, sizeof(line), fp) != NULL) {
        ConversionRecord temp;
        char *token;

        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n')
            line[len - 1] = '\0';

        if (strlen(line) == 0)
            continue; // skip blank lines

        token = strtok(line, "|");
        if (token == NULL) continue;
        strncpy(temp.type, token, TYPE_LEN - 1);
        temp.type[TYPE_LEN - 1] = '\0';

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        temp.inputValue = atof(token);

        token = strtok(NULL, "|");
        if (token == NULL) continue;
        temp.result = atof(token);

        ConversionRecord *newArr = realloc(history, (historyCount + 1) * sizeof(ConversionRecord));
        if (newArr == NULL) {
            printf("Error: Memory allocation failed while loading file.\n");
            fclose(fp);
            return;
        }
        history = newArr;
        history[historyCount] = temp;
        historyCount++;
    }

    fclose(fp);
    printf("History loaded successfully (%d records).\n", historyCount);
}

// Saves all history records currently in memory to the text data file.
void saveHistory(void) {
    FILE *fp = fopen(DATA_FILE, "w");
    if (fp == NULL) {
        printf("Error: Could not open file for saving.\n");
        return;
    }

    int i;
    for (i = 0; i < historyCount; i++) {
        fprintf(fp, "%s|%f|%f\n", history[i].type, history[i].inputValue, history[i].result);
    }

    fclose(fp);
    printf("History saved successfully (%d records).\n", historyCount);
}

// Core: performing a conversion

// Adds a new record to the dynamic history array
void addRecord(const char *type, double inputValue, double result) {
    ConversionRecord *newArr = realloc(history, (historyCount + 1) * sizeof(ConversionRecord));
    if (newArr == NULL) {
        printf("Error: Memory allocation failed. Record not saved.\n");
        return;
    }
    history = newArr;
    strncpy(history[historyCount].type, type, TYPE_LEN - 1);
    history[historyCount].type[TYPE_LEN - 1] = '\0';
    history[historyCount].inputValue = inputValue;
    history[historyCount].result = result;
    historyCount++;
}

// Shows the conversion menu, lets the user pick one, then uses a
// function pointer to call the matching conversion function.
void performConversion(void) {
    int i, choice;

    printf("\n--- Choose a Conversion ---\n");
    for (i = 0; i < NUM_CONVERSIONS; i++) {
        printf("%d. %s\n", i + 1, conversionTable[i].name);
    }
    printf("Select an option [1-%d]: ", NUM_CONVERSIONS);

    if (!readInt(&choice) || choice < 1 || choice > NUM_CONVERSIONS) {
        printf("Invalid choice.\n");
        return;
    }

    double inputValue;
    printf("Enter value to convert: ");
    if (!readDouble(&inputValue)) {
        printf("Invalid input.\n");
        return;
    }

    // Look up the chosen entry and call its function through the
    // function pointer stored in the table.
    ConversionEntry entry = conversionTable[choice - 1];
    ConversionFunc convert = entry.func;
    double result = convert(inputValue);

    printf("Result: %.4f %s -> %.4f\n", inputValue, entry.name, result);

    addRecord(entry.name, inputValue, result);
    printf("Record added to history.\n");
}

// View history

void viewHistory(void) {
    if (historyCount == 0) {
        printf("No conversion history available.\n");
        return;
    }

    int i;
    printf("\n%-4s %-25s %-12s %-12s\n", "No.", "Type", "Input", "Result");
    printf("---------------------------------------------------------\n");
    for (i = 0; i < historyCount; i++) {
        printf("%-4d %-25s %-12.4f %-12.4f\n",
               i + 1, history[i].type, history[i].inputValue, history[i].result);
    }
}

// Search functions (manual)

// Manual linear search by conversion type (substring match)
void searchByType(void) {
    char keyword[TYPE_LEN];
    int i, found = 0;

    printf("Enter conversion type to search (e.g. Celsius): ");
    if (fgets(keyword, TYPE_LEN, stdin) != NULL) {
        size_t len = strlen(keyword);
        if (len > 0 && keyword[len - 1] == '\n')
            keyword[len - 1] = '\0';
    }

    for (i = 0; i < historyCount; i++) {
        if (strstr(history[i].type, keyword) != NULL) {
            if (!found) {
                printf("\nMatching Records:\n");
                found = 1;
            }
            printf("%-25s Input: %-10.4f Result: %-10.4f\n",
                   history[i].type, history[i].inputValue, history[i].result);
        }
    }

    if (!found) {
        printf("No records found for \"%s\".\n", keyword);
    }
}

// Manual linear search by converted value (closest match within tolerance)
void searchByValue(void) {
    double target;
    int i, found = 0;
    double tolerance = 0.01;

    printf("Enter converted value to search for: ");
    if (!readDouble(&target)) {
        printf("Invalid input.\n");
        return;
    }

    for (i = 0; i < historyCount; i++) {
        if (fabs(history[i].result - target) <= tolerance) {
            if (!found) {
                printf("\nMatching Records:\n");
                found = 1;
            }
            printf("%-25s Input: %-10.4f Result: %-10.4f\n",
                   history[i].type, history[i].inputValue, history[i].result);
        }
    }

    if (!found) {
        printf("No records found close to %.4f.\n", target);
    }
}

// Sort functions (manual bubble sort using a compare callback)

// Compare callback: returns 1 if a's type should come after b's type
int compareByType(ConversionRecord a, ConversionRecord b) {
    return strcmp(a.type, b.type) > 0;
}

// Compare callback: returns 1 if a's result should come after b's result
int compareByValue(ConversionRecord a, ConversionRecord b) {
    return a.result > b.result;
}

// Generic manual bubble sort that uses whichever compare callback is passed in.
void manualSort(CompareFunc cmp) {
    int i, j;
    ConversionRecord temp;

    for (i = 0; i < historyCount - 1; i++) {
        for (j = 0; j < historyCount - 1 - i; j++) {
            if (cmp(history[j], history[j + 1])) {
                temp = history[j];
                history[j] = history[j + 1];
                history[j + 1] = temp;
            }
        }
    }
}

// Callback operations

// Precision callback: rounds a record's result to a given number of decimals
void roundToPrecision(ConversionRecord *record, int precision) {
    double factor = pow(10, precision);
    record->result = round(record->result * factor) / factor;
}

// Applies the precision callback to every record in history
void applyPrecisionToAll(PrecisionFunc func) {
    int precision, i;

    printf("Enter number of decimal places (0-6): ");
    if (!readInt(&precision) || precision < 0 || precision > 6) {
        printf("Invalid precision value.\n");
        return;
    }

    for (i = 0; i < historyCount; i++) {
        func(&history[i], precision);
    }

    printf("Applied %d decimal place(s) to all %d record(s).\n", precision, historyCount);
}

// Filter callback: returns 1 if a record's result is greater than the threshold
int filterGreaterThan(ConversionRecord record, double threshold) {
    return record.result > threshold;
}

// Filter callback: returns 1 if a record's result is less than the threshold
int filterLessThan(ConversionRecord record, double threshold) {
    return record.result < threshold;
}

// Applies a chosen filter callback across all records and prints matches
void filterRecords(void) {
    if (historyCount == 0) {
        printf("No history to filter.\n");
        return;
    }

    int choice, i, found = 0;
    double threshold;
    FilterFunc filter;

    printf("Filter by:\n1. Result greater than a value\n2. Result less than a value\n");
    printf("Select an option [1-2]: ");
    if (!readInt(&choice) || (choice != 1 && choice != 2)) {
        printf("Invalid choice.\n");
        return;
    }

    printf("Enter threshold value: ");
    if (!readDouble(&threshold)) {
        printf("Invalid input.\n");
        return;
    }

    filter = (choice == 1) ? filterGreaterThan : filterLessThan;

    printf("\nFiltered Records:\n");
    for (i = 0; i < historyCount; i++) {
        if (filter(history[i], threshold)) {
            printf("%-25s Input: %-10.4f Result: %-10.4f\n",
                   history[i].type, history[i].inputValue, history[i].result);
            found = 1;
        }
    }

    if (!found) {
        printf("No records matched the filter.\n");
    }
}

// Submenus

void searchMenu(void) {
    int choice;
    printf("\n--- Search Records ---\n");
    printf("1. Search by conversion type\n");
    printf("2. Search by converted value\n");
    printf("Select an option [1-2]: ");

    if (!readInt(&choice)) {
        printf("Invalid input.\n");
        return;
    }

    if (choice == 1) {
        searchByType();
    } else if (choice == 2) {
        searchByValue();
    } else {
        printf("Invalid choice.\n");
    }
}

void sortMenu(void) {
    if (historyCount == 0) {
        printf("No history to sort.\n");
        return;
    }

    int choice;
    printf("\n--- Sort Records ---\n");
    printf("1. Sort by conversion type\n");
    printf("2. Sort by converted value\n");
    printf("Select an option [1-2]: ");

    if (!readInt(&choice)) {
        printf("Invalid input.\n");
        return;
    }

    if (choice == 1) {
        manualSort(compareByType);
        printf("Records sorted by conversion type.\n");
    } else if (choice == 2) {
        manualSort(compareByValue);
        printf("Records sorted by converted value.\n");
    } else {
        printf("Invalid choice.\n");
    }
}

void callbackMenu(void) {
    int choice;
    printf("\n--- Callback Operations ---\n");
    printf("1. Apply precision to all results\n");
    printf("2. Filter records\n");
    printf("Select an option [1-2]: ");

    if (!readInt(&choice)) {
        printf("Invalid input.\n");
        return;
    }

    if (choice == 1) {
        if (historyCount == 0) {
            printf("No history available.\n");
        } else {
            applyPrecisionToAll(roundToPrecision);
        }
    } else if (choice == 2) {
        filterRecords();
    } else {
        printf("Invalid choice.\n");
    }
}

// Main menu

void showMenu(void) {
    printf("\n");
    printf("==========================================\n");
    printf("Smart Calculator - Unit Conversion Toolkit\n");
    printf("\n");
    printf("1. Perform a conversion\n");
    printf("2. View conversion history\n");
    printf("3. Search records\n");
    printf("4. Sort records\n");
    printf("5. Apply callback operations\n");
    printf("6. Save history\n");
    printf("7. Load history\n");
    printf("8. Exit\n");
    printf("==========================================\n");
    printf("Select an option [1-8]: ");
}

int main() {
    int choice;

    loadHistory();

    while (1) {
        showMenu();

        if (!readInt(&choice)) {
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        if (choice == 1) {
            performConversion();
        } else if (choice == 2) {
            viewHistory();
        } else if (choice == 3) {
            searchMenu();
        } else if (choice == 4) {
            sortMenu();
        } else if (choice == 5) {
            callbackMenu();
        } else if (choice == 6) {
            saveHistory();
        } else if (choice == 7) {
            // Free existing in-memory history before reloading from file
            free(history);
            history = NULL;
            historyCount = 0;
            loadHistory();
        } else if (choice == 8) {
            printf("Saving history and exiting. Goodbye!\n");
            saveHistory();
            free(history);
            break;
        } else {
            printf("Invalid option. Please choose a number between 1 and 8.\n");
        }
    }

    return 0;
}

