#include "stochastic_gradient_descent.h"

#include "../network.h"

#include "../network_utils/batch.h"
// #include "../network_utils/gradiant.h"

#include "update_mini_batch.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int stochastic_gradiant_descent(
    Network *network, Batch *batch, size_t epochs, uint16_t mini_batch_size,
    float eta, float lambda, Batch *validation_batch
)
{
    uint16_t total_training_size = batch->batchSize;
    uint16_t total_evaluation_size = 0;
    if (validation_batch != NULL)
        total_evaluation_size = validation_batch->batchSize;

    for (size_t epoch = 0; epoch < epochs; epoch++)
    {
        batch_shuffle(batch);

        uint16_t num_mini_batches = 0;

        Batch **mini_batches = create_mini_batches(
            network, batch, mini_batch_size, &num_mini_batches
        );
        if (mini_batches == NULL)
        {
            return 0;
        }

        for (size_t i = 0; i < num_mini_batches; i++)
        {
            update_mini_batch(
                network, mini_batches[i], eta, lambda, total_training_size
            );

            batch_free(mini_batches[i]);
        }

        printf("\n\nEpoch %zu training complete\n\n\n", epoch);
        free(mini_batches);
    }

    return 1;
}

Batch **create_mini_batches(
    Network *network, Batch *batch, uint16_t mini_batch_size,
    uint16_t *num_mini_batches
)
{
    uint16_t batch_size = batch->batchSize;
    *num_mini_batches = (batch_size + mini_batch_size - 1) / mini_batch_size;

    size_t input_size = network->entryCount;
    size_t output_size = network->layers[network->layerCount - 1]->nodeCount;

    Batch **mini_batches =
        (Batch **)malloc(*num_mini_batches * sizeof(Batch *));
    if (mini_batches == NULL)
        return NULL;

    for (uint16_t k = 0; k < *num_mini_batches; k++)
    {
        // ternary operator was usable in C and I had no idea lol
        uint16_t current_batch_size = (k == *num_mini_batches - 1)
                                          ? (batch_size - k * mini_batch_size)
                                          : mini_batch_size;

        mini_batches[k] =
            batch_new(current_batch_size, input_size, output_size);
        if (mini_batches[k] == NULL)
        {
            // Free previously
            for (uint16_t j = 0; j < k; j++)
            {
                free(mini_batches[j]);
            }
            free(mini_batches);
            return NULL;
        }

        // Copy the layers from the batch to the mini_batch
        for (uint16_t j = 0; j < current_batch_size; j++)
        {
            mini_batches[k]->layers[j] = batch->layers[k * mini_batch_size + j];
        }
    }

    return mini_batches;
}
