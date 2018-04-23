# MulticoreProject
Course Project for Multicore Processors: Architecture &amp; Programming

Parallelizing Typical Approximate Member Query(AMQ) Filters Using OpenMP

## Prerequisties

g++-6.2.0

(with openmp and c++11 support)

## How to Compile

`cd src`

`make`

## How to Run

`amq -t <numThreads> -tc <numTestCases> -bs <BFsize> -k <BFNumHash> -q <QFQuoSize> -r <QFRemSize>`

## Example

### Use default parameters

`amq -t 8 -tc 6000000`

### Use customize parameters

`amq -t 32 -tc 8000000 -bs 128000000 -k 16 -q 26 -r 13`
