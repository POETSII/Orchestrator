/* Defines a configuration for deploying multiple Aesop engines. */

#include "MultiAesopDeployer.h"

MultiAesopDeployer::MultiAesopDeployer(unsigned multiple)
{
    /* More boxes! */
    boxesInEngine *= multiple;

    /* More boards! (multiply the first dimension by multiple). The first
     * dimension is chosen purely arbitrarily. */
    boardsInEngine[0] *= multiple;

    /* Modify the address lengths to accomodate the new numbers of boxes and
     * boards. */
    boxWordLength = word_length_from_quantity(boxesInEngine);
    boardWordLengths[0] = word_length_from_quantity(boardsInEngine[0]);
}

/* Returns the word length needed to store a quantity of POETS
 * items.
 *
 * Basically returns (ceil(log_2(quantity)).
 *
 * Arguments:
 *
 * - quantity: Number of POETS items that need to fit. */
unsigned MultiAesopDeployer::word_length_from_quantity(unsigned quantity)
{
    /* NB: C++98 lacks a log-base-2 function, so we use:
     *   log_2(x) = log_10(x) / log_10(2) */
    return ceil(log10(static_cast<float>(quantity)) /
                log10(static_cast<float>(2)));
}
