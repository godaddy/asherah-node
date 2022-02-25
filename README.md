Asherah envelope encryption and key rotation library

This is a wrapper of the Asherah Go implementation using the Cobhan FFI library

Example code: 


```javascript
import { setup, encrypt, decrypt } from 'asherah'

setup({
  kmsType: 'static',
  metastore: 'memory',
  serviceName: 'TestService',
  productId: 'TestProduct',
  verbose: true,
  sessionCache: true
});

const data = Buffer.from('mysecretdata', 'utf8');

const encrypted = encrypt('partition', data);
console.log(encrypted);

const decrypted = decrypt('partition', encrypted);
console.log("Decrypted: " + decrypted.toString('utf8'));
```
