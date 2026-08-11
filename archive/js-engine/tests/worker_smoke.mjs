import { spawn } from 'child_process';
import { fileURLToPath } from 'url';
import { dirname, join } from 'path';

const __filename = fileURLToPath(import.meta.url);
const __dirname = dirname(__filename);

const workerPath = join(__dirname, '..', 'src', 'v8_worker.js');

const child = spawn('node', [workerPath], {
    cwd: join(__dirname, '..')
});

const requests = [
    { cmd: 'init', args: [] },
    { cmd: 'addAtom', args: ['C', 0, 0, 0] },
    { cmd: 'getStructure', args: ['mol', 'save'] },
    { cmd: 'getMoleculeName', args: [] },
    { cmd: 'getSdfProps', args: [] },
    { cmd: 'unknown_command', args: [] } // unrecognized commands must error, not silently broadcast state
];

let currentReq = 0;
let outputBuffer = '';

child.stdout.on('data', (data) => {
    outputBuffer += data.toString();
    
    let lines = outputBuffer.split('\n');
    outputBuffer = lines.pop(); // Keep the incomplete line, if any
    
    for (const line of lines) {
        if (!line.trim()) continue;
        
        let msg;
        try {
            msg = JSON.parse(line);
        } catch (e) {
            console.error('Failed to parse output:', line);
            process.exit(1);
        }
        
        handleResponse(msg);
    }
});

let moleculeNameSent = false;
let sdfPropsSent = false;

function handleResponse(msg) {
    if (msg.status === 'error') {
        if (requests[currentReq - 1]?.cmd === 'unknown_command' && msg.message.includes('unknown command')) {
            console.log('Passed unknown command test');
        } else {
            console.error('Unexpected error response:', msg);
            process.exit(1);
        }
    } else if (requests[currentReq - 1]?.cmd === 'unknown_command') {
        console.error('unknown_command should have produced an error response, got:', msg);
        process.exit(1);
    } else if (msg.type === 'structureResponse') {
        if (msg.reqId === 'save') {
            if (!msg.data.includes('M  END')) {
                console.error('getStructure did not contain M  END');
                process.exit(1);
            }
            console.log('Passed getStructure test');
        } else if (msg.reqId === 'mol_name') {
            console.log('Passed getMoleculeName test');
            moleculeNameSent = true;
        } else if (msg.reqId === 'sdf_props') {
            console.log('Passed getSdfProps test');
            sdfPropsSent = true;
        }
    } else if (msg.status === 'ok') {
        if (requests[currentReq - 1]?.cmd === 'getMoleculeName' && moleculeNameSent) {
            console.error('getMoleculeName should not broadcast status:ok');
            process.exit(1);
        }
        if (requests[currentReq - 1]?.cmd === 'getSdfProps' && sdfPropsSent) {
            console.error('getSdfProps should not broadcast status:ok');
            process.exit(1);
        }
        if (requests[currentReq - 1]?.cmd === 'addAtom') {
            if (!msg.state || !msg.state.atoms) {
                console.error('addAtom response did not contain atoms state');
                process.exit(1);
            }
            console.log('Passed addAtom test');
        }
    }
    
    sendNext();
}

child.stderr.on('data', (data) => {
    console.error(`stderr: ${data}`);
});

child.on('close', (code) => {
    if (code !== 0) {
        console.error(`child process exited with code ${code}`);
        process.exit(code);
    }
    console.log('All smoke tests passed!');
});

function sendNext() {
    if (currentReq < requests.length) {
        const req = requests[currentReq++];
        moleculeNameSent = false;
        sdfPropsSent = false;
        child.stdin.write(JSON.stringify(req) + '\n');
    } else {
        child.stdin.end();
    }
}

// Start
sendNext();
