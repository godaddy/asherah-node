const { execSync } = require('child_process');

console.log('Runtime:', process.title);
console.log('Signal status in', process.title + ':');

try {
    const result = execSync('./test-signal-mask-check', { encoding: 'utf8' });
    console.log(result);
} catch (e) {
    console.error('Error:', e.message);
}