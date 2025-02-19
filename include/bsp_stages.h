#ifndef BSP_STAGES_H_
#define BSP_STAGES_H_

typedef enum BSP_Stage {
    S1_INITIALIZING,
    S1_PENDING,
    S1_FAILED,
    S1_COMPLETED,

    S2_INITIALIZING,
    S2_PENDING,
    S2_FAILED,
    S2_COMPLETED,
} BSP_Stage;

#endif // BSP_STAGES_H_
