#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "tax_data.dat"
#define REPORT_FILE "tax_report.txt"

typedef struct {
    char name[100];
    int account_type; // 1: Individual, 2: Senior, 3: Business
    float income;
    float taxable_income;
    float tax;
    float penalty;
    float total_tax;
    int tax_paid; // 0 = not paid, 1 = paid
} TaxPayer;

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

float calculateTax(TaxPayer tp) {
    float income = tp.taxable_income;
    float tax = 0;
    
    if (tp.account_type == 1) { // Individual
        if (income <= 250000) tax = 0;
        else if (income <= 500000) tax = (income - 250000) * 0.05;
        else if (income <= 1000000)
            tax = (250000 * 0.05) + (income - 500000) * 0.20;
        else
            tax = (250000 * 0.05) + (500000 * 0.20) + (income - 1000000) * 0.30;
    }
    else if (tp.account_type == 2) { // Senior Citizen
        if (income <= 300000) tax = 0;
        else if (income <= 500000) tax = (income - 300000) * 0.05;
        else if (income <= 1000000)
            tax = (200000 * 0.05) + (income - 500000) * 0.20;
        else
            tax = (200000 * 0.05) + (500000 * 0.20) + (income - 1000000) * 0.30;
    }
    else if (tp.account_type == 3) { // Business
        if (income <= 250000) tax = 0;
        else tax = (income - 250000) * 0.25; // Flat 25%
    }

    return tax;
}

float calculatePenalty(float tax, int tax_paid) {
    if (tax_paid == 0)
        return tax * 0.02; // flat 2% penalty if unpaid
    return 0;
}

void displayTaxPayer(TaxPayer tp) {
    char *type_str[] = {"Invalid", "Individual", "Senior Citizen", "Business"};
    printf("\n-------------------------------\n");
    printf("Name: %s\n", tp.name);
    printf("Account Type: %s\n", type_str[tp.account_type]);
    printf("Total Income: ₹%.2f\n", tp.income);
    printf("Taxable Income: ₹%.2f\n", tp.taxable_income);
    printf("Tax: ₹%.2f\n", tp.tax);
    printf("Penalty: ₹%.2f\n", tp.penalty);
    printf("Total Payable: ₹%.2f\n", tp.total_tax);
    printf("Status: %s\n", tp.tax_paid ? "Paid" : "Unpaid");
    printf("-------------------------------\n");
}

void addTaxPayer() {
    TaxPayer tp;
    printf("Enter Name: ");
    clear_input();
    fgets(tp.name, sizeof(tp.name), stdin);
    tp.name[strcspn(tp.name, "\n")] = 0;

    printf("Select Account Type:\n");
    printf("1. Individual (<60 years)\n2. Senior Citizen (60+)\n3. Business\n");
    do {
        printf("Enter type (1/2/3): ");
        scanf("%d", &tp.account_type);
    } while (tp.account_type < 1 || tp.account_type > 3);

    printf("Enter Total Income: ");
    scanf("%f", &tp.income);

    tp.taxable_income = tp.income;
    tp.tax = calculateTax(tp);
    tp.penalty = calculatePenalty(tp.tax, 0);
    tp.total_tax = tp.tax + tp.penalty;
    tp.tax_paid = 0;

    FILE *fp = fopen(DATA_FILE, "ab");
    if (fp) {
        fwrite(&tp, sizeof(TaxPayer), 1, fp);
        fclose(fp);
        printf("Taxpayer added successfully.\n");
    } else {
        printf("Error saving taxpayer.\n");
    }
}

void viewAll() {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) {
        printf("No records found.\n");
        return;
    }
    TaxPayer tp;
    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1)
        displayTaxPayer(tp);
    fclose(fp);
}

void payTax() {
    char name[100];
    printf("Enter name to mark as paid: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    if (!fp || !temp) {
        printf("File error.\n");
        return;
    }

    TaxPayer tp;
    int found = 0;
    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) == 0) {
            if (tp.tax_paid) {
                printf("Already marked as paid.\n");
            } else {
                tp.tax_paid = 1;
                tp.penalty = 0;
                tp.total_tax = tp.tax;
                printf("Payment marked for '%s'.\n", tp.name);
            }
            found = 1;
        }
        fwrite(&tp, sizeof(TaxPayer), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    remove(DATA_FILE);
    rename("temp.dat", DATA_FILE);

    if (!found)
        printf("Taxpayer not found.\n");
}

void searchByName() {
    char name[100];
    printf("Enter name to search: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) {
        printf("File error.\n");
        return;
    }

    TaxPayer tp;
    int found = 0;
    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) == 0) {
            displayTaxPayer(tp);
            found = 1;
        }
    }

    fclose(fp);
    if (!found)
        printf("No record found for '%s'\n", name);
}

void deleteTaxPayer() {
    char name[100];
    printf("Enter name to delete: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    if (!fp || !temp) {
        printf("File error.\n");
        return;
    }

    TaxPayer tp;
    int deleted = 0;
    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) != 0) {
            fwrite(&tp, sizeof(TaxPayer), 1, temp);
        } else {
            deleted = 1;
        }
    }

    fclose(fp);
    fclose(temp);
    remove(DATA_FILE);
    rename("temp.dat", DATA_FILE);
    if (deleted)
        printf("Taxpayer deleted.\n");
    else
        printf("Name not found.\n");
}

void updateIncome() {
    char name[100];
    printf("Enter name to update income: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");

    TaxPayer tp;
    int updated = 0;
    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) == 0) {
            printf("Enter new income: ");
            scanf("%f", &tp.income);
            tp.taxable_income = tp.income;
            tp.tax = calculateTax(tp);
            tp.penalty = calculatePenalty(tp.tax, tp.tax_paid);
            tp.total_tax = tp.tax + tp.penalty;
            updated = 1;
        }
        fwrite(&tp, sizeof(TaxPayer), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    remove(DATA_FILE);
    rename("temp.dat", DATA_FILE);
    if (updated)
        printf("Income updated.\n");
    else
        printf("Taxpayer not found.\n");
}

void exportReport() {
    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *out = fopen(REPORT_FILE, "w");
    if (!fp || !out) {
        printf("Export error.\n");
        return;
    }

    TaxPayer tp;
    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        fprintf(out, "-------------------------------\n");
        fprintf(out, "Name: %s\n", tp.name);
        fprintf(out, "Account Type: %d\n", tp.account_type);
        fprintf(out, "Total Income: ₹%.2f\n", tp.income);
        fprintf(out, "Taxable Income: ₹%.2f\n", tp.taxable_income);
        fprintf(out, "Tax: ₹%.2f\n", tp.tax);
        fprintf(out, "Penalty: ₹%.2f\n", tp.penalty);
        fprintf(out, "Total Payable: ₹%.2f\n", tp.total_tax);
        fprintf(out, "Status: %s\n", tp.tax_paid ? "Paid" : "Unpaid");
    }

    fclose(fp);
    fclose(out);
    printf("Report saved to '%s'.\n", REPORT_FILE);
}

void viewRichTaxPayers() {
    FILE *fp = fopen(DATA_FILE, "rb");
    if (!fp) {
        printf("No records.\n");
        return;
    }
    TaxPayer tp;
    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (tp.income > 1000000)
            displayTaxPayer(tp);
    }
    fclose(fp);
}

int main() {
    int choice;
    do {
        printf("\n============= TAXATION SYSTEM =============\n");
        printf("1. Add New Taxpayer\n");
        printf("2. View All Taxpayers\n");
        printf("3. Search by Name\n");
        printf("4. Delete Taxpayer\n");
        printf("5. Update Taxpayer Income\n");
        printf("6. Pay Tax\n");
        printf("7. View Rich Taxpayers (> ₹10L)\n");
        printf("8. Export Report to Text File\n");
        printf("0. Exit\n");
        printf("===========================================\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addTaxPayer(); break;
            case 2: viewAll(); break;
            case 3: searchByName(); break;
            case 4: deleteTaxPayer(); break;
            case 5: updateIncome(); break;
            case 6: payTax(); break;
            case 7: viewRichTaxPayers(); break;
            case 8: exportReport(); break;
            case 0: printf("Exiting.\n"); break;
            default: printf("Invalid option.\n");
        }
    } while (choice != 0);

    return 0;
}






#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define DATA_FILE "tax_data.dat"
#define REPORT_FILE "tax_report.txt"

typedef struct {
    char name[100];
    int account_type; // 1: Individual, 2: Senior, 3: Business
    float income;
    float taxable_income;
    float tax;
    float penalty;
    float total_tax;
    int tax_paid;         // 0 = not paid, 1 = paid
    char due_date[20];    // YYYY-MM-DD
    char paid_date[20];   // YYYY-MM-DD or blank
} TaxPayer;

void clear_input() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void get_current_date(char *buffer) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, 20, "%Y-%m-%d", t);
}

void get_due_date(char *buffer) {
    time_t now = time(NULL);
    now += 30 * 24 * 60 * 60; // Add 30 days
    struct tm *t = localtime(&now);
    strftime(buffer, 20, "%Y-%m-%d", t);
}

// Helper to convert date string to tm struct
void parse_date(const char *date_str, struct tm *tm_out) {
    strptime(date_str, "%Y-%m-%d", tm_out);
}

// Calculate days between two dates (returns >0 if late)
int days_between(const char *start, const char *end) {
    struct tm tm1 = {0}, tm2 = {0};
    parse_date(start, &tm1);
    parse_date(end, &tm2);

    time_t t1 = mktime(&tm1);
    time_t t2 = mktime(&tm2);

    return (int)((t2 - t1) / (60 * 60 * 24));
}

float calculateTax(TaxPayer tp) {
    float income = tp.taxable_income;
    float tax = 0;
    
    if (tp.account_type == 1) {
        if (income <= 250000) tax = 0;
        else if (income <= 500000) tax = (income - 250000) * 0.05;
        else if (income <= 1000000)
            tax = (250000 * 0.05) + (income - 500000) * 0.20;
        else
            tax = (250000 * 0.05) + (500000 * 0.20) + (income - 1000000) * 0.30;
    }
    else if (tp.account_type == 2) {
        if (income <= 300000) tax = 0;
        else if (income <= 500000) tax = (income - 300000) * 0.05;
        else if (income <= 1000000)
            tax = (200000 * 0.05) + (income - 500000) * 0.20;
        else
            tax = (200000 * 0.05) + (500000 * 0.20) + (income - 1000000) * 0.30;
    }
    else if (tp.account_type == 3) {
        if (income <= 250000) tax = 0;
        else tax = (income - 250000) * 0.25;
    }

    return tax;
}
void displayTaxPayer(TaxPayer tp) {
    char *type_str[] = {"Invalid", "Individual", "Senior Citizen", "Business"};
    printf("\n-------------------------------\n");
    printf("Name: %s\n", tp.name);
    printf("Account Type: %s\n", type_str[tp.account_type]);
    printf("Income: ₹%.2f\n", tp.income);
    printf("Tax: ₹%.2f\n", tp.tax);
    printf("Penalty: ₹%.2f\n", tp.penalty);
    printf("Total to Pay: ₹%.2f\n", tp.total_tax);
    printf("Due Date: %s\n", tp.due_date);
    printf("Paid Date: %s\n", tp.tax_paid ? tp.paid_date : "Not Paid");
    printf("Status: %s\n", tp.tax_paid ? "Paid" : "Unpaid");
    printf("-------------------------------\n");
}

void addTaxPayer() {
    TaxPayer tp;
    printf("Enter Name: ");
    clear_input();
    fgets(tp.name, sizeof(tp.name), stdin);
    tp.name[strcspn(tp.name, "\n")] = 0;

    printf("Select Account Type:\n");
    printf("1. Individual (<60)\n2. Senior Citizen (60+)\n3. Business\n");
    do {
        printf("Enter choice (1-3): ");
        scanf("%d", &tp.account_type);
    } while (tp.account_type < 1 || tp.account_type > 3);

    printf("Enter Total Income: ");
    scanf("%f", &tp.income);

    tp.taxable_income = tp.income;
    tp.tax = calculateTax(tp);
    tp.penalty = 0;
    tp.total_tax = tp.tax;
    tp.tax_paid = 0;
    strcpy(tp.paid_date, ""); // not paid yet
    get_due_date(tp.due_date); // due in 30 days

    FILE *fp = fopen(DATA_FILE, "ab");
    if (fp) {
        fwrite(&tp, sizeof(TaxPayer), 1, fp);
        fclose(fp);
        printf("✅ Taxpayer added successfully.\n");
    } else {
        printf("❌ File error.\n");
    }
}

void payTax() {
    char name[100];
    printf("Enter name to pay tax: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");

    TaxPayer tp;
    int found = 0;

    char today[20];
    get_current_date(today);

    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) == 0) {
            found = 1;

            if (tp.tax_paid) {
                printf("✅ Already paid on %s\n", tp.paid_date);
            } else {
                int days_late = days_between(tp.due_date, today);
                float penalty = 0;

                if (days_late > 0) {
                    int weeks = days_late / 7;
                    penalty = 500 + (tp.tax * 0.01 * weeks);
                }

                tp.penalty = penalty;
                tp.total_tax = tp.tax + penalty;
                printf("🧾 You owe: ₹%.2f (Tax: ₹%.2f + Penalty: ₹%.2f)\n", tp.total_tax, tp.tax, tp.penalty);

                char choice;
                printf("Do you want to pay now? (Y/N): ");
                scanf(" %c", &choice);
                if (choice == 'Y' || choice == 'y') {
                    tp.tax_paid = 1;
                    strcpy(tp.paid_date, today);
                    printf("✅ Tax marked as paid.\n");
                } else {
                    printf("⚠  Tax remains unpaid.\n");
                }
            }
        }
        fwrite(&tp, sizeof(TaxPayer), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    remove(DATA_FILE);
    rename("temp.dat", DATA_FILE);
    if (!found) printf("❌ Taxpayer not found.\n");
}

void searchTaxPayer() {
    char name[100];
    printf("Enter name to search: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    TaxPayer tp;
    int found = 0;

    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) == 0) {
            displayTaxPayer(tp);
            found = 1;
            break;
        }
    }

    fclose(fp);
    if (!found) printf("❌ Taxpayer not found.\n");
}

void deleteTaxPayer() {
    char name[100];
    printf("Enter name to delete: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    TaxPayer tp;
    int deleted = 0;

    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) != 0) {
            fwrite(&tp, sizeof(TaxPayer), 1, temp);
        } else {
            deleted = 1;
        }
    }

    fclose(fp);
    fclose(temp);
    remove(DATA_FILE);
    rename("temp.dat", DATA_FILE);

    if (deleted) printf("✅ Taxpayer deleted successfully.\n");
    else printf("❌ Taxpayer not found.\n");
}

void updateTaxPayer() {
    char name[100];
    printf("Enter name to update: ");
    clear_input();
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *temp = fopen("temp.dat", "wb");
    TaxPayer tp;
    int found = 0;

    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        if (strcmp(tp.name, name) == 0) {
            found = 1;
            printf("Enter new income: ");
            scanf("%f", &tp.income);
            tp.taxable_income = tp.income;
            tp.tax = calculateTax(tp);
            tp.penalty = 0;
            tp.total_tax = tp.tax;
            tp.tax_paid = 0;
            get_due_date(tp.due_date);
            strcpy(tp.paid_date, "");
            printf("✅ Taxpayer updated.\n");
        }
        fwrite(&tp, sizeof(TaxPayer), 1, temp);
    }

    fclose(fp);
    fclose(temp);
    remove(DATA_FILE);
    rename("temp.dat", DATA_FILE);

    if (!found) printf("❌ Taxpayer not found.\n");
}

void exportReport() {
    FILE *fp = fopen(DATA_FILE, "rb");
    FILE *out = fopen(REPORT_FILE, "w");

    if (!fp || !out) {
        printf("❌ File error during export.\n");
        return;
    }

    TaxPayer tp;
    fprintf(out, "%-20s %-10s %-10s %-10s %-10s %-10s %-15s %-15s\n",
            "Name", "Type", "Income", "Tax", "Penalty", "Total", "Due Date", "Paid Date");
    fprintf(out, "-----------------------------------------------------------------------------------------------------------\n");

    char *type_str[] = {"Invalid", "Individual", "Senior", "Business"};

    while (fread(&tp, sizeof(TaxPayer), 1, fp) == 1) {
        fprintf(out, "%-20s %-10s %-10.2f %-10.2f %-10.2f %-10.2f %-15s %-15s\n",
                tp.name, type_str[tp.account_type], tp.income, tp.tax,
                tp.penalty, tp.total_tax, tp.due_date,
                tp.tax_paid ? tp.paid_date : "Not Paid");
    }

    fclose(fp);
    fclose(out);
    printf("✅ Report exported to '%s'.\n", REPORT_FILE);
}

void menu() {
    int choice;
    while (1) {
        printf("\n\n------ Tax Management System ------\n");
        printf("1. Add Taxpayer\n");
        printf("2. Pay Tax\n");
        printf("3. Search Taxpayer\n");
        printf("4. Delete Taxpayer\n");
        printf("5. Update Taxpayer\n");
        printf("6. Export Report\n");
        printf("0. Exit\n");
        printf("Enter choice: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1: addTaxPayer(); break;
            case 2: payTax(); break;
            case 3: searchTaxPayer(); break;
            case 4: deleteTaxPayer(); break;
            case 5: updateTaxPayer(); break;
            case 6: exportReport(); break;
            case 0: exit(0);
            default: printf("❌ Invalid choice.\n");
        }
    }
}

int main() {
    menu();
    return 0;
}




