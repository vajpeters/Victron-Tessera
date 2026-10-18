"""Repair the upstream preview bench's empty-attribute state subscriptions."""
import argparse
from pathlib import Path

OLD = "known['state'] if attribute is None else"
NEW = "known['state'] if not attribute else"


def patch(plugins: Path) -> bool:
    path = plugins / 'tools' / 'bench.py'
    original = path.read_bytes()
    old, new = OLD.encode(), NEW.encode()
    if original.count(old) == 1 and original.count(new) == 0:
        path.write_bytes(original.replace(old, new, 1))
        return True
    if original.count(old) == 0 and original.count(new) == 1:
        return False
    raise ValueError(f'Unexpected preview bench source: {path}; refusing to patch')


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--plugins', type=Path, required=True)
    args = parser.parse_args()
    try:
        changed = patch(args.plugins)
    except (OSError, ValueError) as error:
        parser.exit(1, f'{error}\n')
    print('Preview subscription fix applied' if changed else 'Preview subscription fix already applied')


if __name__ == '__main__':
    main()
