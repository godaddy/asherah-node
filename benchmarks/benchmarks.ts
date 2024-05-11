//import setup from '../dist/asherah'
import { IsoBench } from 'iso-bench';

// Define asynchronous test functions with TypeScript
async function fetchData(): Promise<string> {
    return new Promise(resolve => setTimeout(() => resolve("data"), 100));
}

async function processItems(): Promise<string> {
    return new Promise(resolve => setTimeout(() => resolve("processed"), 150));
}

// Function to run benchmarks
async function runBenchmarks(): Promise<void> {
    const isoBench = new IsoBench("Example Benchmarks", {
        samples: 10,   // Number of times each function is run
        time: 500      // Maximum time (ms) for each test
    });

    // Add tests to the benchmark
    isoBench.add('Fetch Data', async () => await fetchData());
    isoBench.add('Process Items', async () => await processItems());

    // Optionally, add console logging
    isoBench.consoleLog();

    // Run the benchmarks
    await isoBench.run();

    console.log("Benchmarks completed.");
}

runBenchmarks();

