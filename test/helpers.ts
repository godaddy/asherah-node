import crypto from 'crypto';

export const posix_log_levels = {
    emerg: 0,
    alert: 1,
    crit: 2,
    error: 3,
    warning: 4,
    notice: 5,
    info: 6,
    debug: 7
};

export function get_sample_json(size: number): string {
    return JSON.stringify({
        key1: 'value1b',
        nested: { secret: crypto.randomBytes(size).toString('base64') }
    }, ['nested.secret']);
}

export function get_string(size: number): string {
    return 'x'.repeat(size);
}

