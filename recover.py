import json

transcript_path = r'C:\Users\jp18b\.gemini\antigravity-cli\brain\83087d19-2f93-45a2-91ea-ef8ac103c580\.system_generated\logs\transcript_full.jsonl'
file_path = r'C:\Users\jp18b\Desktop\sketch\src\app\IupacNamer.cpp'
out_path = r'C:\Users\jp18b\Desktop\sketch\src\app\IupacNamer.cpp.recovered'

with open(file_path, 'r', encoding='utf-8') as f:
    content = f.read()

lines = content.split('\n')

edits_applied = 0
with open(transcript_path, 'r', encoding='utf-8') as f:
    for line_str in f:
        try:
            entry = json.loads(line_str)
        except:
            continue
        
        if 'tool_calls' in entry:
            for tc in entry['tool_calls']:
                name = tc.get('name')
                args = tc.get('args', {})
                if name and ('multi_replace_file_content' in name or 'replace_file_content' in name):
                    target = args.get('TargetFile', '')
                    if 'IupacNamer.cpp' in target:
                        chunks = []
                        if name == 'replace_file_content':
                            chunks = [args]
                        else:
                            chunks = args.get('ReplacementChunks', [])
                        
                        # Apply chunks in reverse order to not mess up line numbers
                        chunks = sorted(chunks, key=lambda x: x.get('StartLine', 0), reverse=True)
                        for chunk in chunks:
                            start = chunk.get('StartLine', 1) - 1
                            end = chunk.get('EndLine', 1)
                            replacement = chunk.get('ReplacementContent', '').split('\n')
                            
                            # Replace lines
                            lines = lines[:start] + replacement + lines[end:]
                        edits_applied += 1
                        print(f"Applied edit {edits_applied}")

with open(out_path, 'w', encoding='utf-8') as f:
    f.write('\n'.join(lines))

print(f"Saved recovered file with {edits_applied} tool calls applied.")
