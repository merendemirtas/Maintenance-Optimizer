#include "main.h"

// Greedy algoritma implementasyonu
OptimizationResult* greedy_optimization(Machine *machines, int n, int horizon) {
    OptimizationResult *result = (OptimizationResult*)malloc(sizeof(OptimizationResult));
    result->total_cost = 0;
    result->total_maintenance = 0;
    result->total_downtime = 0;
    result->energy_savings = 0;
    
    // Maksimum bakım görev sayısını tahmin et
    result->schedule_size = horizon * MAINTENANCE_CAPACITY;
    result->schedule = (MaintenanceTask*)malloc(result->schedule_size * sizeof(MaintenanceTask));
    
    int schedule_index = 0;
    int machine_state = (1 << n) - 1;  // Tüm makineler çalışıyor
    
    for (int t = 1; t <= horizon; t++) {
        // Bakım gerektiren makineleri belirle (greedy: en yüksek arıza riski)
        int maintenance_mask = 0;
        int machines_to_maintain = 0;
        
        // Arıza riski en yüksek 3 makineyi seç
        for (int priority = 100; priority >= 0 && machines_to_maintain < MAINTENANCE_CAPACITY; priority -= 10) {
            for (int i = 0; i < n && machines_to_maintain < MAINTENANCE_CAPACITY; i++) {
                if ((machine_state & (1 << i)) && 
                    machines[i].failure_probability >= priority &&
                    !(maintenance_mask & (1 << i))) {
                    
                    maintenance_mask |= (1 << i);
                    machines_to_maintain++;
                    
                    // Bakım görevini kaydet
                    result->schedule[schedule_index].machine_id = i;
                    result->schedule[schedule_index].maintenance_time = t;
                    result->schedule[schedule_index].maintenance_type = 
                        (machines[i].failure_probability > 60) ? 1 : 0;
                    result->schedule[schedule_index].cost = machines[i].maintenance_cost;
                    
                    result->total_cost += machines[i].maintenance_cost;
                    result->total_cost += machines[i].production_loss * 8; // 8 saat duruş
                    result->total_maintenance++;
                    result->total_downtime += 8;
                    
                    schedule_index++;
                }
            }
        }
        
        // Durumu güncelle
        if (maintenance_mask > 0) {
            machine_state = apply_maintenance(machines, n, machine_state, maintenance_mask);
        }
        
        // Operasyonel maliyet
        result->total_cost += calculate_operational_cost(machines, n, machine_state, t);
    }
    
    result->schedule_size = schedule_index;
    
    return result;
}

// Sabit periyotlu algoritma
OptimizationResult* fixed_schedule_optimization(Machine *machines, int n, int horizon) {
    OptimizationResult *result = (OptimizationResult*)malloc(sizeof(OptimizationResult));
    result->total_cost = 0;
    result->total_maintenance = 0;
    result->total_downtime = 0;
    result->energy_savings = 0;
    
    int schedule_size = (horizon / 7) * n;  // Haftada bir bakım
    result->schedule = (MaintenanceTask*)malloc(schedule_size * sizeof(MaintenanceTask));
    
    int schedule_index = 0;
    int machine_state = (1 << n) - 1;
    
    for (int t = 1; t <= horizon; t++) {
        // Her 7 günde bir tüm makinelere bakım
        if (t % 7 == 0) {
            int maintenance_mask = machine_state;  // Tüm çalışan makineler
            
            for (int i = 0; i < n; i++) {
                if (maintenance_mask & (1 << i)) {
                    result->schedule[schedule_index].machine_id = i;
                    result->schedule[schedule_index].maintenance_time = t;
                    result->schedule[schedule_index].maintenance_type = 0; // Planlı
                    result->schedule[schedule_index].cost = machines[i].maintenance_cost;
                    
                    result->total_cost += machines[i].maintenance_cost;
                    result->total_cost += machines[i].production_loss * 8;
                    result->total_maintenance++;
                    result->total_downtime += 8;
                    
                    schedule_index++;
                }
            }
            
            machine_state = apply_maintenance(machines, n, machine_state, maintenance_mask);
        }
        
        result->total_cost += calculate_operational_cost(machines, n, machine_state, t);
    }
    
    result->schedule_size = schedule_index;
    
    return result;
}

void compare_algorithms(Machine *machines, int n, int horizon) {
    printf("ALGORITMA PERFORMANS KARSILASTIRMASI\n");
    printf("=====================================\n\n");
    
    clock_t start, end;
    double time_dp, time_greedy, time_fixed;
    
    // 1. Dinamik Programlama
    printf("1. Dinamik Programlama:\n");
    start = clock();
    OptimizationResult *dp_result = optimize_maintenance(machines, n, horizon);
    end = clock();
    time_dp = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   - Sure: %.3f saniye\n", time_dp);
    int dp_cost = (int)llround(dp_result->total_cost);
    printf("   - Maliyet: %d TL\n", dp_cost);
    printf("   - Bakim Sayisi: %d\n\n", dp_result->total_maintenance);
    
    // 2. Greedy Algoritma
    printf("2. Greedy Algoritma:\n");
    start = clock();
    OptimizationResult *greedy_result = greedy_optimization(machines, n, horizon);
    end = clock();
    time_greedy = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   - Sure: %.3f saniye\n", time_greedy);
    int greedy_cost = (int)llround(greedy_result->total_cost);
    printf("   - Maliyet: %d TL\n", greedy_cost);
    printf("   - Bakim Sayisi: %d\n\n", greedy_result->total_maintenance);
    
    // 3. Sabit Periyot
    printf("3. Sabit Periyotlu Planlama:\n");
    start = clock();
    OptimizationResult *fixed_result = fixed_schedule_optimization(machines, n, horizon);
    end = clock();
    time_fixed = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("   - Sure: %.3f saniye\n", time_fixed);
    int fixed_cost = (int)llround(fixed_result->total_cost);
    printf("   - Maliyet: %d TL\n", fixed_cost);
    printf("   - Bakim Sayisi: %d\n\n", fixed_result->total_maintenance);
    
    // Karşılaştırma tablosu
    printf("PERFORMANS KARSILASTIRMA TABLOSU:\n");
    printf("+-----------------+------------+------------+------------+\n");
    printf("| Algoritma       | Sure (sn)  | Maliyet    | Bakim Say. |\n");
    printf("+-----------------+------------+------------+------------+\n");
    printf("| Dinamik Prog.   | %10.3f | %10d | %10d |\n", 
           time_dp, dp_cost, dp_result->total_maintenance);
    printf("| Greedy          | %10.3f | %10d | %10d |\n", 
           time_greedy, greedy_cost, greedy_result->total_maintenance);
    printf("| Sabit Periyot   | %10.3f | %10d | %10d |\n", 
           time_fixed, fixed_cost, fixed_result->total_maintenance);
    printf("+-----------------+------------+------------+------------+\n\n");
    
    // İyileştirme yüzdeleri
    int best_cost = dp_cost;
    
    int improvement_greedy = ((greedy_cost - best_cost) * 100) / best_cost;
    int improvement_fixed = ((fixed_cost - best_cost) * 100) / best_cost;
    
    printf("DINAMIK PROGRAMLAMA AVANTAJLARI:\n");
    printf("  - Greedy algoritmaya gore: %%%d daha dusuk maliyet\n", 
           abs(improvement_greedy));
    printf("  - Sabit periyoda gore: %%%d daha dusuk maliyet\n", 
           abs(improvement_fixed));
    
    // Belleği temizle
    free_optimization_result(dp_result);
    free_optimization_result(greedy_result);
    free_optimization_result(fixed_result);
}