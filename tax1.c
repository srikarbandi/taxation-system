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
