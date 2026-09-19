#!/usr/bin/env python3
"""Read-only PE inspection. Never loads or executes the target executable."""
import argparse
import bisect
import hashlib
import json
import re
from pathlib import Path

import pefile
from capstone import Cs, CS_ARCH_X86, CS_MODE_64
from capstone.x86 import X86_OP_IMM, X86_OP_MEM, X86_REG_RIP


def main():
    ap = argparse.ArgumentParser(description=__doc__)
    ap.add_argument('binary', type=Path)
    ap.add_argument('out', type=Path)
    args = ap.parse_args()
    data = args.binary.read_bytes()
    pe = pefile.PE(data=data)
    base = pe.OPTIONAL_HEADER.ImageBase
    out = args.out
    out.mkdir(parents=True, exist_ok=True)
    imports = {}
    for dll in getattr(pe, 'DIRECTORY_ENTRY_IMPORT', []):
        for imp in dll.imports:
            imports[imp.address] = dll.dll.decode() + '!' + (imp.name.decode() if imp.name else str(imp.ordinal))
    functions = [(base + f.struct.BeginAddress, base + f.struct.EndAddress) for f in getattr(pe, 'DIRECTORY_ENTRY_EXCEPTION', [])]
    functions.sort()
    starts = [a for a, b in functions]
    strings = {}
    records = []
    for section in pe.sections:
        raw = section.get_data()
        if section.Characteristics & 0x20000000:
            continue
        for m in re.finditer(rb'[\x20-\x7e\x80-\xff]{4,}', raw):
            b = m.group()
            for encoding in ('utf-8', 'gb18030'):
                try:
                    s = b.decode(encoding)
                except UnicodeDecodeError:
                    continue
                if not all(c.isprintable() for c in s):
                    continue
                addr = base + section.VirtualAddress + m.start()
                strings[addr] = s
                records.append({'va': hex(addr), 'offset': hex(section.PointerToRawData + m.start()), 'encoding': encoding, 'text': s})
                break
        for m in re.finditer(rb'(?:[\x20-\x7e]\x00){4,}', raw):
            addr = base + section.VirtualAddress + m.start()
            s = m.group().decode('utf-16le')
            strings[addr] = s
            records.append({'va': hex(addr), 'offset': hex(section.PointerToRawData + m.start()), 'encoding': 'utf-16le', 'text': s})
    string_starts = sorted(strings)

    def label(addr):
        if addr in imports:
            return imports[addr]
        idx = bisect.bisect_right(string_starts, addr) - 1
        if idx >= 0:
            a = string_starts[idx]
            if 0 <= addr - a < min(200, len(strings[a])):
                return json.dumps(strings[a], ensure_ascii=False) + (f'+{addr-a}' if addr != a else '')
        return ''

    md = Cs(CS_ARCH_X86, CS_MODE_64)
    md.detail = True
    md.skipdata = True
    lines = []
    refs = []
    for section in pe.sections:
        if not section.Characteristics & 0x20000000:
            continue
        for ins in md.disasm(section.get_data(), base + section.VirtualAddress):
            if ins.address in starts:
                lines.append(f'\n; FUNCTION {ins.address:#x}\n')
            notes = []
            if ins.id:
                for op in ins.operands:
                    addr = None
                    if op.type == X86_OP_MEM and op.mem.base == X86_REG_RIP:
                        addr = ins.address + ins.size + op.mem.disp
                    elif op.type == X86_OP_IMM:
                        addr = op.imm
                    if addr is not None:
                        # RIP-relative floating-point constants can resemble text.
                        # Only annotate strings when the instruction forms a pointer.
                        target = imports.get(addr, '')
                        if not target and (ins.mnemonic == 'lea' or op.type == X86_OP_IMM):
                            target = label(addr)
                        notes.append(f'{addr:#x}' + (' ' + target if target else ''))
                        if target:
                            idx = bisect.bisect_right(starts, ins.address) - 1
                            fn = starts[idx] if idx >= 0 and ins.address < functions[idx][1] else None
                            refs.append({'instruction': hex(ins.address), 'function': hex(fn) if fn else None, 'target': hex(addr), 'label': target})
            lines.append(f'{ins.address:016x}  {ins.mnemonic:8s} {ins.op_str:48s}' + (' ; ' + ' | '.join(notes) if notes else ''))
    (out / 'disassembly.asm').write_text('\n'.join(lines) + '\n')
    (out / 'strings.json').write_text(json.dumps(records, ensure_ascii=False, indent=2) + '\n')
    (out / 'references.json').write_text(json.dumps(refs, ensure_ascii=False, indent=2) + '\n')
    metadata = {'sha256': hashlib.sha256(data).hexdigest(), 'size': len(data), 'image_base': hex(base), 'entry_point': hex(base + pe.OPTIONAL_HEADER.AddressOfEntryPoint), 'imports': {hex(a): n for a, n in imports.items()}, 'functions': [{'start': hex(a), 'end': hex(b)} for a, b in functions]}
    (out / 'metadata.json').write_text(json.dumps(metadata, indent=2) + '\n')
    print(f'{len(imports)} imports, {len(functions)} runtime-function entries, {len(records)} string candidates, {len(refs)} annotated references')


if __name__ == '__main__':
    main()
