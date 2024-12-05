// gcc -shared -o Analyzers/indicators.dll Analyzers/indicators.c

#include <stdlib.h>
#include <stdio.h>


void sma(float* data_close, int data_len, int period, float* output, int* output_len) {
    *output_len = data_len - period + 1;
    for (int x = 0; x < *output_len; x++) {
        float sum = 0.0f;
        for (int y = 0; y < period; y++) {
            sum = sum + data_close[x + y];
        }
        output[x] = sum / (float)period;
    }
}

void ema(float* data_close, int data_len, int period, float* output, int* output_len) {
    *output_len = data_len - period + 1;
    float sf = 2.0f / ((float)period + 1.0f);
    float sum = 0.0f;
    for (int x = 0; x < period; x++) {
        sum = sum + data_close[x];
    }
    output[0] = sum / (float)period;
    for (int x = 1; x < data_len - period + 1; x++) {
        output[x] = (data_close[x + period - 1] - output[x - 1]) * sf + output[x - 1];
    }
}

void rsi(float* data_close, int data_len, int period, float* output, int* output_len) {
    *output_len = data_len - period;
    float* gain = (float*)malloc(*output_len * sizeof(float));
    float* loss = (float*)malloc(*output_len * sizeof(float));
    float* delta = (float*)malloc((data_len - 1) * sizeof(float));
    float* delta_positive = (float*)malloc((data_len - 1) * sizeof(float));
    float* delta_negative = (float*)malloc((data_len - 1) * sizeof(float));
    for (int x = 0; x < data_len - 1; x++) {
        delta[x] = data_close[x + 1] - data_close[x];
    }
    for (int x = 0; x < data_len - 1; x++) {
        float d = delta[x];
        if (d > 0.0f) {
            delta_positive[x] = d;
            delta_negative[x] = 0.0f;
        }
        else if (d < 0.0f) {
            delta_positive[x] = 0.0f;
            delta_negative[x] = -d;
        }
        else {
            delta_positive[x] = 0.0f;
            delta_negative[x] = 0.0f;
        }
    }

    float sum_positive = 0.0f;
    float sum_negative = 0.0f;
    for (int x = 0; x < period; x++) {
        sum_positive = sum_positive + delta_positive[x];
        sum_negative = sum_negative + delta_negative[x];
    }
    gain[0] = sum_positive / (float)period;
    loss[0] = sum_negative / (float)period;
    for (int x = 1; x < *output_len; x++) {
        gain[x] = (gain[x - 1] * ((float)period - 1.0f) + delta_positive[x + period - 1]) / (float)period;
        loss[x] = (loss[x - 1] * ((float)period - 1.0f) + delta_negative[x + period - 1]) / (float)period;
    }
    for (int x = 0; x < *output_len; x++) {
        float rs = gain[x];
        if (loss[x] != 0.0f) {
            rs = rs / loss[x];
        }
        else {
            rs = rs / 0.0000000001f;
        }
        output[x] = 100.0f - (100.0f / (1.0f + rs));
    }
    free(gain);
    free(loss);
    free(delta);
    free(delta_positive);
    free(delta_negative);
}

void mfi(float* data_high, float* data_low, float* data_close, float* data_volume, int data_len, int period, float* output, int* output_len) {
    *output_len = data_len - 1;
    float* typical_price = (float*)malloc(data_len * sizeof(float));
    float* delta = (float*)malloc((data_len - 1) * sizeof(float));
    float* raw_money_flow = (float*)malloc(data_len * sizeof(float));
    float* positive_money_flow = (float*)malloc((data_len - 1) * sizeof(float));
    float* negative_money_flow = (float*)malloc((data_len - 1) * sizeof(float));
    for (int x = 0; x < data_len; x++) {
        typical_price[x] = (data_high[x] + data_low[x] + data_close[x]) / 3.0f;
        raw_money_flow[x] = typical_price[x] * data_volume[x];
    }
    for (int x = 0; x < data_len - 1; x++) {
        delta[x] = typical_price[x + 1] - typical_price[x];
    }
    for (int x = 0; x < data_len - 1; x++) {
        if (delta[x] > 0.0f) {
            positive_money_flow[x] = raw_money_flow[x + 1];
            negative_money_flow[x] = 0.0f;
        }
        else if (delta[x] < 0.0f) {
            positive_money_flow[x] = 0.0f;
            negative_money_flow[x] = raw_money_flow[x + 1];
        }
        else {
            positive_money_flow[x] = 0.0f;
            negative_money_flow[x] = 0.0f;
        }
    }
    for (int x = 1; x < data_len; x++) {
        float gain = 0.0f;
        float loss = 0.0f;
        int start = x - period;
        if (start < 0) {
            start = 0;
        }
        for (int y = start; y < x; y++) {
            gain = gain + positive_money_flow[y];
            loss = loss + negative_money_flow[y];
        }
        if (loss == 0.0f) {
            loss = 0.000000001f;
        }
        output[x - 1] = 100.0f - (100.0f / (1.0f + (gain / loss)));
    }

    free(typical_price);
    free(delta);
    free(raw_money_flow);
    free(positive_money_flow);
    free(negative_money_flow);
}
