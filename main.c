#include "main.h"

int main() {
    printf("=============================================\n");
    printf("MAKINE BAKIM PLANLAMA OPTIMIZASYON SISTEMI\n");
    printf("Dinamik Programlama Tabanlı Çözüm\n");
    printf("=============================================\n\n");
    
    int num_machines, horizon;
    
    // Kullanıcı girişi
    printf("Makine sayisini girin (1-%d): ", MAX_MACHINES);
    scanf("%d", &num_machines);
    printf("\n");  // okunabilirlik: sonraki istemi yeni satira al
    
    if (num_machines < 1 || num_machines > MAX_MACHINES) {
        printf("Hatali giris! Makine sayisi 1-%d arasinda olmali.\n", MAX_MACHINES);
        return 1;
    }
    
    printf("Planlama periyodunu girin (gun): ");
    scanf("%d", &horizon);
    
    if (horizon < 1 || horizon > MAX_HORIZON) {
        printf("Hatali giris! Periyot 1-%d arasinda olmali.\n", MAX_HORIZON);
        return 1;
    }
    
    // Makineleri başlat
    Machine *machines = initialize_machines(num_machines);
    generate_test_data(machines, num_machines);
    
    printf("\n=== TEST VERILERI OLUSTURULDU ===\n");
    printf("Toplam Makine Sayisi: %d\n", num_machines);
    printf("Planlama Periyodu: %d gun\n", horizon);
    printf("Eszamanli Bakim Kapasitesi: %d makine\n\n", MAINTENANCE_CAPACITY);
    
    // Makine bilgilerini göster
    printf("MAKINE BILGILERI:\n");
    printf("ID | Yas | Son Bakim | Ariza Olasiligi | Planli Maliyet | Acil Maliyet | Uretim Kaybi\n");
    printf("------------------------------------------------------------------------------------\n");
    
    for (int i = 0; i < num_machines; i++) {
        printf("%2d | %3d | %9d | %15d%% | %13d TL | %11d TL | %11d TL/saat\n",
               machines[i].id,
               machines[i].age,
               machines[i].last_maintenance,
               machines[i].failure_probability,
               machines[i].maintenance_cost,
               machines[i].emergency_cost,
               machines[i].production_loss);
    }
    printf("\n");
    
    // Optimizasyonu çalıştır
    printf("=== DINAMIK PROGRAMLAMA ILE OPTIMIZASYON CALISTIRILIYOR ===\n");
    clock_t start = clock();
    
    OptimizationResult *result = optimize_maintenance(machines, num_machines, horizon);
    
    clock_t end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Optimizasyon tamamlandi! Sure: %.3f saniye\n\n", cpu_time_used);
    
    // Sonuçları göster
    print_schedule(result);
    
    // Alternatif algoritmalarla karşılaştır
    printf("\n=== ALGORITMA KARSILASTIRMASI ===\n");
    compare_algorithms(machines, num_machines, horizon);
    
    // Sonuçları dosyaya kaydet
    save_results_to_file(result, "optimization_results.txt");
    printf("\nSonuclar 'optimization_results.txt' dosyasina kaydedildi.\n");
    
    // Belleği temizle
    free_optimization_result(result);
    free_machines(machines);
    
    return 0;
}
