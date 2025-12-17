#ifndef MAINTENANCE_OPTIMIZER_H
#define MAINTENANCE_OPTIMIZER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>
#include <limits.h>

/* =======================
   SABİTLER
   ======================= */
#define MAX_MACHINES 15
#define MAX_HORIZON 30
#define MAINTENANCE_CAPACITY 2

#define INF 1e15   /* double için güvenli INF */

/* =======================
   VERİ YAPILARI
   ======================= */
typedef struct {
    int id;
    int age;
    int last_maintenance;
    int failure_probability;
    int maintenance_cost;
    int emergency_cost;
    int production_loss;
    int energy_consumption;
} Machine;

typedef struct {
    int time;
    int state;
    double cost;        /* DÜZELTİLDİ */
    int decision;
    int prev_state;
} DPState;

typedef struct {
    int machine_id;
    int maintenance_time;
    int maintenance_type;
    int cost;
} MaintenanceTask;

typedef struct {
    double total_cost;  /* DÜZELTİLDİ */
    int total_maintenance;
    int total_downtime;
    int energy_savings;
    MaintenanceTask *schedule;
    int schedule_size;
} OptimizationResult;

/* =======================
   FONKSİYON PROTOTİPLERİ
   ======================= */
Machine* initialize_machines(int n);
void free_machines(Machine *machines);

double calculate_operational_cost(
    Machine *machines, int n, int state, int time
);

int predict_next_state(
    Machine *machines, int n, int current_state, int time
);

double calculate_maintenance_cost(
    Machine *machines, int n, int maintenance_mask
);

int apply_maintenance(
    Machine *machines, int n, int state, int maintenance_mask
);

int* generate_feasible_subsets(
    int state, int n, int capacity, int *count
);

OptimizationResult* optimize_maintenance(
    Machine *machines, int n, int horizon
);

void print_schedule(OptimizationResult *result);
void free_optimization_result(OptimizationResult *result);
void compare_algorithms(Machine *machines, int n, int horizon);
void generate_test_data(Machine *machines, int n);
void save_results_to_file(
    OptimizationResult *result, const char *filename
);

#endif
