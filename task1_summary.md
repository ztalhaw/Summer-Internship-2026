# BLIS Library Matrix Multiplication Benchmarking

## Objective
The core objective of my first task was to evaluate the efficiency of the **BLIS (BLAS-like Library Instantiation Software)** repository by calculating the actual performance of double-precision matrix multiplication (DGEMM) and comparing it directly against the **theoretical peak FLOPS** of my hardware. By testing various square matrix sizes ranging from $2,000 \times 2,000$ up to $20,000 \times 20,000$, I aimed to analyze how close the software comes to reaching maximum hardware limits and how it scales under intensive multi-threaded workloads.

---

## Step-by-Step Execution Guide

### Step 1: Environment Setup & File Organization
To ensure the compiler could resolve paths correctly without global environment errors, I organized my working directory inside my dedicated repository space:
* I navigated to my active folder: `/home/talha-zaman/Summer-Internship-26/task1`
* I placed my source code file (`benchmark.c`) directly in this root directory, positioned right alongside the local `blis/` subfolder.

### Step 2: Mapping Headers and Static Libraries
Since BLIS was compiled locally rather than installed globally on the system, I had to locate the exact internal paths of the build artifacts to feed into the compiler. I mapped them to:
* **Header File (`blis.h`):** `./blis/install/include/blis/blis.h`
* **Static Library (`libblis.a`):** `./blis/install/lib/libblis.a`

### Step 3: Compiling the Source Code with Optimization Flags
Because standard compilation commands fail to link the required local files or the necessary multi-threading hooks, I compiled my program using aggressive compiler optimizations (`-O3`) and explicitly enabled OpenMP (`-fopenmp`) to handle the library's parallel execution paths:

```bash
gcc -O3 -fopenmp -I./blis/install/include/blis benchmark.c ./blis/install/lib/libblis.a -o benchmark -lm -lpthread
```
### Step 4: Automating and Running the Benchmarks
To gather statistically clean performance data, I configured the execution to run each specific matrix size exactly 10 times consecutively before moving on to the next size. I utilized two structural methods for tracking:

* **The Internal Loop Structure:** Modified the code to manage the 10-run iteration internally and print a live, readable table straight to my terminal screen.
* **The Script-Driven Argument Structure:** Passed the matrix size ($N$) and run number as command-line inputs (e.g., `./benchmark 2000 1`). This setup allowed me to check the `BLIS_NUM_THREADS` environment variable dynamically and write the exact execution time, raw FLOPs, and GFLOPS directly into a master data file named `blis_benchmark_results.csv`.

### Step 5: Calculating and Comparing Performance
For each run, the program captured high-resolution monotonic time and computed the actual **GFLOPS** (Giga Floating-Point Operations Per Second) using the formula:

$$\text{GFLOPS} = \frac{2 \times N^3}{\text{Time in seconds} \times 10^9}$$

By logging these values all the way up to a massive $20,000 \times 20,000$ size (which tests memory allocation up to ~9.6 GB of RAM), I was able to plot the calculated performance and compare it against my processor's theoretical peak performance to determine the library's true mathematical efficiency.

> ⚠️ **Hardware Limitation Note:** Due to the hardware constraints of my personal computer, I was unable to successfully complete the benchmarks for the largest matrix sizes (approaching $20,000 \times 20,000$). The heavy memory usage overwhelmed my system's available RAM, forcing the operating system to terminate the execution process mid-run. Consequently, the performance analysis focuses on the successfully calculated smaller and mid-sized matrix limits where data could be plotted safely against my processor's theoretical peak performance.
