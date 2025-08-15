#!/usr/bin/env node

// Investigation script to check signal handling differences between Node.js and Bun
const os = require('os');
const fs = require('fs');

console.log('=== Signal Investigation ===');
console.log('Runtime:', typeof Bun !== 'undefined' ? 'Bun' : 'Node.js');
console.log('Platform:', os.platform());
console.log('Architecture:', os.arch());

// Check process info
console.log('\n=== Process Info ===');
console.log('PID:', process.pid);
console.log('PPID:', process.ppid);

// Check signal handlers
console.log('\n=== Signal Handlers ===');
const importantSignals = ['SIGTERM', 'SIGINT', 'SIGQUIT', 'SIGPROF', 'SIGALRM', 'SIGUSR1', 'SIGUSR2'];

importantSignals.forEach(sig => {
    try {
        const listeners = process.listeners(sig);
        console.log(`${sig}: ${listeners.length} listeners`);
    } catch (e) {
        console.log(`${sig}: Error checking - ${e.message}`);
    }
});

// Check if we can read signal mask (Linux/macOS specific)
if (os.platform() !== 'win32') {
    console.log('\n=== Signal Mask Info ===');
    try {
        const statusPath = `/proc/${process.pid}/status`;
        if (fs.existsSync(statusPath)) {
            const status = fs.readFileSync(statusPath, 'utf8');
            const sigMask = status.match(/SigMsk:\s*([a-f0-9]+)/);
            const sigPnd = status.match(/SigPnd:\s*([a-f0-9]+)/);
            const sigBlk = status.match(/SigBlk:\s*([a-f0-9]+)/);
            if (sigMask) console.log('Signal Mask:', sigMask[1]);
            if (sigPnd) console.log('Pending Signals:', sigPnd[1]);
            if (sigBlk) console.log('Blocked Signals:', sigBlk[1]);
        } else {
            console.log('Cannot read /proc/PID/status (not Linux or permission denied)');
        }
    } catch (e) {
        console.log('Error reading signal mask:', e.message);
    }
}

// Check thread info
console.log('\n=== Thread Info ===');
try {
    const tasksPath = `/proc/${process.pid}/task`;
    if (fs.existsSync(tasksPath)) {
        const threads = fs.readdirSync(tasksPath);
        console.log('Thread count:', threads.length);
        console.log('Thread IDs:', threads.join(', '));
    }
} catch (e) {
    console.log('Cannot read thread info:', e.message);
}

// Check environment
console.log('\n=== Environment Variables ===');
const goRelated = Object.keys(process.env).filter(key => 
    key.startsWith('GO') || key.includes('CGO') || key.includes('STACK')
);
goRelated.forEach(key => {
    console.log(`${key}=${process.env[key]}`);
});

// Check ulimits
console.log('\n=== Resource Limits ===');
try {
    const { execSync } = require('child_process');
    const ulimits = execSync('ulimit -a', { encoding: 'utf8' });
    const stackLine = ulimits.split('\n').find(line => line.includes('stack'));
    if (stackLine) {
        console.log('Stack size limit:', stackLine.trim());
    }
} catch (e) {
    console.log('Cannot check ulimits:', e.message);
}

console.log('\n=== Main Thread Stack Info ===');
try {
    // Try to detect current stack size through pthread_attr
    const asherah = require('./build/Release/asherah.node');
    console.log('Native module loaded successfully');
} catch (e) {
    console.log('Cannot load native module:', e.message);
}