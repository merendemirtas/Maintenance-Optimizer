#include "main.h"

/* =====================================================
   PROTOTİP (C99 ZORUNLU)
   ===================================================== */
OptimizationResult* reconstruct_schedule(
    Machine *machines,
    int n,
    int horizon,
    int final_state,
    double total_cost
);

/* =====================================================
   GLOBAL DP TABLOSU
   ===================================================== */
static DPState ***dp_table = NULL;

static int popcount_int(int x) {
    int c = 0;
    while (x) { c += (x & 1); x >>= 1; }
    return c;
}

/* =====================================================
   ANA OPTİMİZASYON (DETERMINISTIK DP)
   ===================================================== */
OptimizationResult* optimize_maintenance(Machine *machines, int n, int horizon)
{
    const int TOTAL_STATES = 1 << n;

    dp_table = calloc(horizon + 1, sizeof(DPState **));
    for (int t = 0; t <= horizon; t++) {
        dp_table[t] = calloc(TOTAL_STATES, sizeof(DPState *));
        for (int s = 0; s < TOTAL_STATES; s++) {
            dp_table[t][s] = malloc(sizeof(DPState));
            dp_table[t][s]->time = t;
            dp_table[t][s]->state = s;
            dp_table[t][s]->cost = INF;
            dp_table[t][s]->decision = 0;
            dp_table[t][s]->prev_state = -1;
        }
    }

    /* Baslangic: hic makineye planli bakim yapilmamis (bit=1 => bu horizon icinde bakimi yapildi) */
    const int initial_state = 0;
    dp_table[0][initial_state]->cost = 0.0;
    dp_table[0][initial_state]->prev_state = initial_state;

    /* ================= DP DONGUSU ================= */
    for (int t = 0; t < horizon; t++) {
        for (int s = 0; s < TOTAL_STATES; s++) {

            DPState *cur = dp_table[t][s];
            if (cur->cost == INF) continue;

            double base_cost = cur->cost;

            /* 1️⃣ BAKIM YAPMA: o gun bakimi olmayan makineler daha riskli calisir */
            double op_cost = calculate_operational_cost(machines, n, s, t);
            int next_state = s;

            if (base_cost + op_cost < dp_table[t + 1][next_state]->cost) {
                dp_table[t + 1][next_state]->cost = base_cost + op_cost;
                dp_table[t + 1][next_state]->decision = 0;
                dp_table[t + 1][next_state]->prev_state = s;
            }

            /* 2️⃣ BAKIM YAP */
            int subset_count = 0;
            int *subsets = generate_feasible_subsets(
                s, n, MAINTENANCE_CAPACITY, &subset_count
            );

            for (int i = 0; i < subset_count; i++) {
                int mask = subsets[i];

                /* Not: generate_feasible_subsets kapasiteyi saglar.
                   Burada ek bir filtre uygulamiyoruz; aksi halde tum maskeler elenebilir. */

                /* Bakim yapilan makineler bu gunden itibaren 'bakimi yapildi' kabul edilir */
                int day_state = apply_maintenance(machines, n, s, mask);
                double day_op_cost = calculate_operational_cost(machines, n, day_state, t);
                double m_cost = calculate_maintenance_cost(machines, n, mask);

                int new_state = day_state;
                double cand = base_cost + day_op_cost + m_cost;
                if (cand < dp_table[t + 1][new_state]->cost) {
                    dp_table[t + 1][new_state]->cost = cand;
                    dp_table[t + 1][new_state]->decision = mask;
                    dp_table[t + 1][new_state]->prev_state = s;
                }
            }

            free(subsets);
        }
    }

    /* ================= FINAL STATE ================= */
    double min_cost = INF;
    int final_state = 0;

    for (int s = 0; s < TOTAL_STATES; s++) {
        if (dp_table[horizon][s]->cost < min_cost) {
            min_cost = dp_table[horizon][s]->cost;
            final_state = s;
        }
    }

    return reconstruct_schedule(machines, n, horizon, final_state, min_cost);
}

/* =====================================================
   OPTIMAL PLAN GERİ OLUSTURMA
   ===================================================== */
OptimizationResult* reconstruct_schedule(
    Machine *machines,
    int n,
    int horizon,
    int final_state,
    double total_cost
)
{
    OptimizationResult *res = malloc(sizeof(OptimizationResult));

    res->total_cost = total_cost;
    res->total_maintenance = 0;
    res->total_downtime = 0;
    res->energy_savings = 0;
    res->schedule_size = 0;

    int state = final_state;

    /* Plan boyutu: bir gunde birden fazla makine bakima girebilir.
       schedule_size toplam gorev sayisi olmalidir (gun sayisi degil). */
    for (int t = horizon; t > 0; t--) {
        int dec = dp_table[t][state]->decision;
        if (dec > 0) res->schedule_size += popcount_int(dec);
        state = dp_table[t][state]->prev_state;
    }

    res->schedule = malloc(res->schedule_size * sizeof(MaintenanceTask));

    state = final_state;
    int idx = 0;

    for (int t = horizon; t > 0; t--) {
        DPState *cur = dp_table[t][state];

        if (cur->decision > 0) {
            int mask = cur->decision;
            for (int i = 0; i < n; i++) {
                if (mask & (1 << i)) {
                    res->schedule[idx++] = (MaintenanceTask){
                        .machine_id = i,
                        .maintenance_time = t,
                        .maintenance_type = 0,
                        .cost = machines[i].maintenance_cost
                    };
                    res->total_maintenance++;
                    res->total_downtime += 4; /* Sabit 4 saat */
                    res->energy_savings += machines[i].energy_consumption * 0.2;
                }
            }
        }

        state = cur->prev_state;
    }

    /* BELLEK TEMIZLIGI */
    for (int t = 0; t <= horizon; t++) {
        for (int s = 0; s < (1 << n); s++)
            free(dp_table[t][s]);
        free(dp_table[t]);
    }
    free(dp_table);

    return res;
}
