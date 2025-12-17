#include "main.h"

/* =====================================================
   MAKINE BELLEK YONETIMI
   ===================================================== */
Machine* initialize_machines(int n) {
    Machine *machines = malloc(n * sizeof(Machine));
    for (int i = 0; i < n; i++) {
        machines[i].id = i + 1;
        machines[i].age = 0;
        machines[i].last_maintenance = 0;
        machines[i].failure_probability = 0;
        machines[i].maintenance_cost = 0;
        machines[i].emergency_cost = 0;
        machines[i].production_loss = 0;
        machines[i].energy_consumption = 0;
    }
    return machines;
}

void free_machines(Machine *machines) {
    free(machines);
}

/* =====================================================
   TEST VERISI (DETERMINISTIK)
   ===================================================== */
void generate_test_data(Machine *machines, int n) {
    /* Not: Bu fonksiyon test verisi uretir. 
       DP'nin deterministik calismasi icin burada sabit seed kullanilir. */
    srand(42);

    for (int i = 0; i < n; i++) {
        machines[i].age = (rand() % 120) + 1;                 /* 1-120 */
        machines[i].last_maintenance = (rand() % 12) + 1;      /* 1-12 */
        machines[i].failure_probability = (rand() % 50) + 10;  /* %10-60 */
        machines[i].maintenance_cost = (rand() % 5000) + 1000; /* 1000-6000 */
        machines[i].emergency_cost = machines[i].maintenance_cost * ((rand() % 100) + 150) / 100; /* 1.5x-2.49x */
        machines[i].production_loss = (rand() % 200) + 50;     /* 50-250 */
        machines[i].energy_consumption = (rand() % 100) + 20;  /* 20-120 */
    }
}

/* =====================================================
   OPERASYONEL MALIYET (BAKIM YOKSA CEZA VAR!)
   ===================================================== */
double calculate_operational_cost(Machine *machines, int n, int state, int time) {
    (void)time;
    double cost = 0.0;

    for (int i = 0; i < n; i++) {
        /* State anlami (DP modeli): bit=1 => bu planlama ufkunda bakimi yapildi (daha dusuk risk)
           bit=0 => bakim yapilmadi (risk daha yuksek, zamanla artar) */

        double base_p = machines[i].failure_probability / 100.0;

        /* Basit risk artisi: son bakim + gecen gun ile lineer artir (cap ile) */
        double aging = 0.01 * (double)(machines[i].last_maintenance + time);
        double p_no_maint = base_p + aging;
        if (p_no_maint > 0.95) p_no_maint = 0.95;

        double p = (state & (1 << i)) ? (p_no_maint * 0.30) : p_no_maint;
        double energy_kwh = (state & (1 << i)) ? (machines[i].energy_consumption * 0.90)
                                               : (double)machines[i].energy_consumption;

        /* Gunluk enerji maliyeti + beklenen ariza/uretim kaybi */
        cost += energy_kwh * 0.5;                 /* 0.5 TL/kWh varsayimi */
        cost += p * machines[i].emergency_cost;
        cost += p * machines[i].production_loss * 8; /* 8 saat beklenen durus */
    }

    return cost;
}

/* =====================================================
   TAHMIN EDILEN SONRAKI DURUM
   (DP ICIN KASITLI OLARAK KULLANILMIYOR)
   ===================================================== */
int predict_next_state(Machine *machines, int n, int current_state, int time) {
    (void)machines;
    (void)n;
    (void)time;
    /* DP icin deterministik gecis: durumun kendisi. 
       Ariza riski beklenen maliyete yansitilir. */
    return current_state;
}

/* =====================================================
   PLANLI BAKIM MALIYETI
   ===================================================== */
double calculate_maintenance_cost(Machine *machines, int n, int maintenance_mask) {
    double cost = 0.0;

    for (int i = 0; i < n; i++) {
        if (maintenance_mask & (1 << i)) {

            /* Planli bakim maliyeti */
            cost += machines[i].maintenance_cost;

            /* Planli durus (4 saat) */
            cost += machines[i].production_loss * 4;
        }
    }

    return cost;
}

/* =====================================================
   BAKIM UYGULAMA (SADECE STATE DEGISTIRIR)
   ===================================================== */
int apply_maintenance(Machine *machines, int n, int state, int maintenance_mask) {
    (void)machines;
    (void)n;

    int new_state = state;

    for (int i = 0; i < n; i++) {
        if (maintenance_mask & (1 << i)) {
            /* Bakim / onarim yapildi -> makine calisir duruma gelir */
            new_state |= (1 << i);
        }
    }

    return new_state;
}

/* =====================================================
   UYGUN BAKIM ALT KUMELERI
   ===================================================== */
int* generate_feasible_subsets(int state, int n, int capacity, int *count) {

    int max_masks = 1 << n;
    int *subsets = malloc(max_masks * sizeof(int));
    *count = 0;

    for (int mask = 1; mask < max_masks; mask++) {

        /* Bit=1 => bakimi yapildi. Bir ufukta ayni makineye tekrar planli bakim yapmayalim. */
        if ((mask & state) != 0) continue;

        int c = 0;
        for (int i = 0; i < n; i++)
            if (mask & (1 << i)) c++;

        if (c <= capacity)
            subsets[(*count)++] = mask;
    }

    return subsets;
}
