#include "main.h"

void print_schedule(OptimizationResult *result) {
    printf("=== OPTIMAL BAKIM PLANI ===\n");
    printf("Toplam Maliyet: %d TL\n", (int)llround(result->total_cost));
    printf("Toplam Bakim Sayisi: %d\n", result->total_maintenance);
    printf("Toplam Duruş Süresi: %d saat\n", result->total_downtime);
    printf("Enerji Tasarrufu: %d kWh\n", result->energy_savings);
    printf("\n");
    
    if (result->schedule_size > 0) {
        printf("BAKIM PROGRAMI:\n");
        printf("Gun | Makine ID | Bakim Tipi | Maliyet (TL)\n");
        printf("-------------------------------------------\n");
        
        for (int i = 0; i < result->schedule_size; i++) {
            MaintenanceTask task = result->schedule[i];
            const char *type_str;
            
            switch (task.maintenance_type) {
                case 0: type_str = "Planli"; break;
                case 1: type_str = "Acil"; break;
                case 2: type_str = "Onleyici"; break;
                default: type_str = "Bilinmiyor"; break;
            }
            
            printf("%3d | %9d | %10s | %12d\n",
                   task.maintenance_time,
                   task.machine_id + 1,
                   type_str,
                   task.cost);
        }
    } else {
        printf("Bu planlamada bakim ongorulmemistir.\n");
    }
}

void free_optimization_result(OptimizationResult *result) {
    if (result->schedule != NULL) {
        free(result->schedule);
    }
    free(result);
}

void save_results_to_file(OptimizationResult *result, const char *filename) {
    FILE *file = fopen(filename, "w");
    if (file == NULL) {
        printf("Dosya acilamadi!\n");
        return;
    }
    
    fprintf(file, "MAKINE BAKIM PLANLAMA OPTIMIZASYON SONUCLARI\n");
    fprintf(file, "============================================\n\n");
    fprintf(file, "Toplam Maliyet: %d TL\n", (int)llround(result->total_cost));
    fprintf(file, "Toplam Bakim Sayisi: %d\n", result->total_maintenance);
    fprintf(file, "Enerji Tasarrufu: %d kWh\n\n", result->energy_savings);
    
    if (result->schedule_size > 0) {
        fprintf(file, "BAKIM PROGRAMI:\n");
        fprintf(file, "Gun\tMakine ID\tBakim Tipi\tMaliyet (TL)\n");
        fprintf(file, "-----------------------------------------------\n");
        
        for (int i = 0; i < result->schedule_size; i++) {
            MaintenanceTask task = result->schedule[i];
            const char *type_str;
            
            switch (task.maintenance_type) {
                case 0: type_str = "Planli"; break;
                case 1: type_str = "Acil"; break;
                case 2: type_str = "Onleyici"; break;
                default: type_str = "Bilinmiyor"; break;
            }
            
            fprintf(file, "%d\t%d\t\t%s\t\t%d\n",
                   task.maintenance_time,
                   task.machine_id + 1,
                   type_str,
                   task.cost);
        }
    }
    
    fclose(file);
}