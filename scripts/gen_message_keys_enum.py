#!/usr/bin/env python3
"""Pre-compile codegen: turn build/src/message_keys.auto.c into a C99 enum.

The Pebble SDK derives build/src/message_keys.auto.c from package.json
"messageKeys", one line per key:

    uint32_t MESSAGE_KEY_EnableSecondsHand = 10000;
    ...

Those are plain mutable globals, not constant expressions, so they cannot be
used in contexts that require compile-time constants (case labels, #if, array
bounds, designated initializer indices, ...).

This pre-compile step reads that generated file and emits a matching C99 enum
typedef into the build output tree, e.g. build/include/message_keys.enum.h:

    typedef enum {
      EMSGKEY_EnableSecondsHand = 10000,
      ...
    } MessageKeyId;

The enumerators are prefixed EMSGKEY_ rather than MESSAGE_KEY_ so they do not
collide with the uint32_t globals declared in message_keys.auto.h when both
headers are used in the same translation unit.

Usage:
    gen_message_keys_enum.py <message_keys.auto.c> <output header>
"""
import os
import re
import sys

_LINE = re.compile(r'^uint32_t\s+([A-Za-z_]\w*)\s*=\s*(\d+)\s*;')


def main(argv):
    if len(argv) != 3:
        sys.exit('usage: gen_message_keys_enum.py <in.auto.c> <out.h>')
    in_path, out_path = argv[1], argv[2]

    pairs = []
    with open(in_path, 'r', encoding='utf-8') as handle:
        for raw in handle:
            match = _LINE.match(raw.strip())
            if match:
                pairs.append((match.group(1), int(match.group(2))))

    if not pairs:
        sys.exit('error: no MESSAGE_KEY* entries found in %r' % in_path)

    guard = 'ARC_VIEW_MESSAGE_KEYS_ENUM_H'
    lines = [
        '/* AUTO-GENERATED from %s. DO NOT EDIT.' % in_path,
        ' *',
        ' * C99 enum typedef mirroring the message key ids so they are usable',
        ' * as compile-time constants (case labels, #if, initializers, ...).',
        ' */',
        '#ifndef %s' % guard,
        '#define %s' % guard,
        '',
        'typedef enum {',
    ]
    lines += ['  EMSGKEY_%s = %d,' % (name[len('MESSAGE_KEY_'):], value)
              for name, value in pairs]
    lines += [
        '} MessageKeyId;',
        '',
        '#endif /* %s */' % guard,
        '',
    ]

    out_dir = os.path.dirname(out_path) or '.'
    os.makedirs(out_dir, exist_ok=True)
    with open(out_path, 'w', encoding='utf-8') as f:
        f.write('\n'.join(lines))


if __name__ == '__main__':
    main(sys.argv)
