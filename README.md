# **Summer-Internship-2026**
This is a journal of all my tasks and projects of my Summer Internship. Each task will have its own file. Here I will give breif explanation of each task

*Task 1:*

The objective of this first internship task was to evaluate the mathematical efficiency of the BLIS library within an Ubuntu environment by benchmarking 
actual double-precision matrix multiplication performance across sizes up to $20,000 \times 20,000$ and comparing the resulting GFLOPS directly against my 
hardware's theoretical peak limits.

*Task 2:*

This task implements the header and metadata parser for a custom GGUF file reader, `gguf_dump`, as part of a 3-person team building a tool to inspect local LLM model files. My part opens a `.gguf` file with `mmap`, verifies the "GGUF" magic number, reads the fixed header fields (version, tensor count, metadata count), and then loops through every metadata key-value pair, using each entry's 4-byte type ID to correctly parse and print integers, floats, booleans, strings, and arrays of arbitrary type — handling GGUF's non-null-terminated length-prefixed strings along the way. This header/metadata layer feeds directly into Member 1's tensor table parsing and Member 2's dequantization work, since all three depend on the same underlying file pointer staying correctly aligned as it walks through the file.
