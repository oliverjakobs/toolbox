#ifndef TOOLBOX_H
#define TOOLBOX_H

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>

#include <string.h>

void tb_swap(void** a, void** b);
void tb_swapi(int* a, int* b);
void tb_swapi32(int32_t* a, int32_t* b);
void tb_swapi64(int64_t* a, int64_t* b);
void tb_swapf(float* a, float* b);
void tb_swapd(double* a, double* b);
void tb_swapstr(char** a, char** b);

inline int tb_max(int a, int b) { return a > b ? a : b; }
inline int32_t tb_max32(int32_t a, int32_t b) { return a > b ? a : b; }
inline int64_t tb_max64(int64_t a, int64_t b) { return a > b ? a : b; }
inline float tb_maxf(float a, float b) { return a > b ? a : b; }
inline double tb_maxd(double a, double b) { return a > b ? a : b; }

inline int tb_min(int a, int b) { return a < b ? a : b; }
inline int32_t tb_min32(int32_t a, int32_t b) { return a < b ? a : b; }
inline int64_t tb_min64(int64_t a, int64_t b) { return a < b ? a : b; }
inline float tb_minf(float a, float b) { return a < b ? a : b; }
inline double tb_mind(double a, double b) { return a < b ? a : b; }

inline uint8_t tb_between(int start, int end, int value) { return (value <= end) && (value >= start); }
inline uint8_t tb_between32(int32_t start, int32_t end, int32_t value) { return (value <= end) && (value >= start); }
inline uint8_t tb_between64(int64_t start, int64_t end, int64_t value) { return (value <= end) && (value >= start); }
inline uint8_t tb_betweenf(float start, float end, float value) { return (value <= end) && (value >= start); }
inline uint8_t tb_betweend(double start, double end, double value) { return (value <= end) && (value >= start); }

int tb_clamp(int value, int min, int max);
int32_t tb_clamp32(int32_t value, int32_t min, int32_t max);
int64_t tb_clamp64(int64_t value, int64_t min, int64_t max);
float tb_clampf(float value, float min, float max);
double tb_clampd(double value, double min, double max);

#endif /* !TOOLBOX_H */
