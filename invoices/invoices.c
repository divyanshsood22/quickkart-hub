#include "../panels/quickkarthub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if defined(_MSC_VER)
  #define HAVE_LOCALTIME_S 1
#elif defined(_POSIX_VERSION) || defined(__unix__) || defined(__APPLE__)
  #define HAVE_LOCALTIME_R 1
#endif

static void portable_localtime(const time_t *ts, struct tm *out) {
#if defined(_MSC_VER)
    localtime_s(out, ts);
#elif defined(HAVE_LOCALTIME_R)
    localtime_r(ts, out);
#else
    struct tm *tmp = localtime(ts);
    if (tmp) *out = *tmp;
#endif
}

void appendBillToGlobalFile(int orderId, int custId, const char *custName, Product *p, int qty, float amount, time_t ts) {
    FILE *f = fopen(BILLS_FILE, "a");
    if (!f) {
        f = fopen(BILLS_FILE, "w");
        if (!f) {
            perror("Failed to open bills file");
            return;
        }
    }

    char timestr[64];
    struct tm tm_info;
    portable_localtime(&ts, &tm_info);
    strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &tm_info);

    fprintf(f, "%d,%d,%s,%d,%s,%d,%.2f,%d,%.2f,%s\n",
            orderId, custId, custName ? custName : "Unknown",
            p ? p->id : 0, p ? p->name : "Unknown",
            qty, p ? p->price : 0.0f, p ? p->discount : 0, amount, timestr);

    fclose(f);
}

void generateAndSaveBill(int orderId, int custId, Product *p, int qty) {
    if (!p) return;

    int custIdx = findCustomerById(custId);
    const char *custName = (custIdx != -1) ? customers[custIdx].name : "Unknown";

    char filename[64];
    snprintf(filename, sizeof(filename), "invoices/bills/bill_%d.txt", custId);
    FILE *f = fopen(filename, "a");
    if (!f) {
        printf("Error opening bill file %s for writing.\n", filename);
        return;
    }

    time_t t = time(NULL);
    struct tm tm_info;
    portable_localtime(&t, &tm_info);
    char dateStr[64];
    strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", &tm_info);

    float discounted = p->price * (1.0f - p->discount / 100.0f);
    float subtotal = discounted * qty;
    float gst = subtotal * 0.18f;
    float total = subtotal + gst;

    fprintf(f, "\n=================================================\n");
    fprintf(f, "                  QUICKKARTHUB BILL\n");
    fprintf(f, "=================================================\n");
    fprintf(f, "Date/Time    : %s\n", dateStr);
    fprintf(f, "Order ID     : %d\n", orderId);
    fprintf(f, "Customer ID  : %d\n", custId);
    fprintf(f, "Customer Name: %s\n", custName);
    fprintf(f, "-------------------------------------------------\n");
    fprintf(f, "Product Details:\n");
    fprintf(f, "  Product Name  : %s\n", p->name);
    fprintf(f, "  Product ID    : %d\n", p->id);
    fprintf(f, "  Unit Price    : %.2f\n", p->price);
    fprintf(f, "  Discount      : %d%%\n", p->discount);
    fprintf(f, "  Final Price   : %.2f\n", discounted);
    fprintf(f, "  Quantity      : %d\n", qty);
    fprintf(f, "-------------------------------------------------\n");
    fprintf(f, "Subtotal (after discount): %.2f\n", subtotal);
    fprintf(f, "GST (18%%)                : %.2f\n", gst);
    fprintf(f, "-------------------------------------------------\n");
    fprintf(f, "TOTAL PAYABLE            : %.2f\n", total);
    fprintf(f, "=================================================\n");
    fprintf(f, "Thank you for shopping with QuickKartHub!\n");
    fprintf(f, "Visit again soon!\n");
    fprintf(f, "=================================================\n\n");

    fclose(f);

    appendBillToGlobalFile(orderId, custId, custName, p, qty, total, t);

    printf("Invoice generated and saved as %s (Total: %.2f)\n", filename, total);
}
