"""Reviewed PS2 identities. Names and candidate similarity never prove behavior."""
import argparse
from collections import Counter
import hashlib
import json
from pathlib import Path
import re

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_MANIFEST = Path(__file__).with_name('ps2-identities-reviewed.json')
DEFAULT_COHORT = Path(__file__).with_name('fixtures') / 'ps2-identity-baseline.json'
FIELDS = ('entry', 'name', 'namespace', 'signature', 'body')


def digest(data):
    return hashlib.sha256(data).hexdigest()


def declaration(fn):
    return ' '.join(fn['source'].split('{', 1)[0].split())


def source_identity(fn):
    # IDs include byte positions and change when preceding source changes.
    # Locate reviews by declaration too, but always revalidate the complete body.
    return dict(path=fn['path'], name=fn['name'], declaration=declaration(fn),
                source_sha256=digest(fn['source'].replace('\r\n', '\n').encode()))


def qualified_name(fn):
    return (fn['name'] if '::' in fn['name'] or fn['namespace'] in ('', 'Global')
            else fn['namespace'] + '::' + fn['name'])


def metadata(fn):
    return {key: fn[key] for key in FIELDS}


def listing(response):
    functions = response['functions']
    if len(functions) >= response.get('limit', 1000):
        raise ValueError('Truncated PS2 name listing')
    return functions


class Manifest:
    def __init__(self, document, root=ROOT):
        if document.get('schema') != 1:
            raise ValueError('Unsupported identity manifest schema')
        self.document, self.root = document, root
        self.records, self.by_declaration = {}, {}
        for row in document['records']:
            key = tuple(row['source'][field] for field in ('path', 'name', 'declaration'))
            if row['id'] in self.records or key in self.by_declaration:
                raise ValueError('Duplicate/conflicting identity review: ' + row['id'])
            if row['disposition'] not in ('mapped', 'non-standalone', 'unresolved'):
                raise ValueError('Invalid identity disposition')
            if not row.get('obligation') or not row.get('evidence'):
                raise ValueError('Incomplete identity review: ' + row['id'])
            if row['disposition'] == 'mapped':
                original = row.get('ps2', {})
                if any(not original.get(field) for field in FIELDS):
                    raise ValueError('Incomplete mapped PS2 metadata')
                if not re.fullmatch('[0-9a-f]{8}', original['entry']):
                    raise ValueError('Invalid PS2 entry')
                instances = row.get('ps2_instances', [original])
                if not isinstance(instances, list) or not instances:
                    raise ValueError('Mapped PS2 instances must be a non-empty list')
                entries = set()
                for instance in instances:
                    if any(not instance.get(field) for field in FIELDS):
                        raise ValueError('Incomplete mapped PS2 instance')
                    if not re.fullmatch('[0-9a-f]{8}', instance['entry']):
                        raise ValueError('Invalid PS2 instance entry')
                    if instance['entry'] in entries:
                        raise ValueError('Duplicate mapped PS2 instance entry')
                    entries.add(instance['entry'])
                if original not in instances:
                    raise ValueError('Primary PS2 entry is absent from mapped instances')
                if not any(item.get('kind') == 'live-review' for item in row['evidence']):
                    raise ValueError('Mapped identity lacks live review evidence')
            if row['disposition'] == 'non-standalone' and not any(
                    item.get('kind') == 'source-review' for item in row['evidence']):
                raise ValueError('Non-standalone identity lacks source review')
            for evidence in row['evidence']:
                if evidence.get('kind') in ('live-review', 'source-review') and not all(
                        evidence.get(key) for key in ('path', 'sha256', 'summary')):
                    raise ValueError('Incomplete review evidence')
                if 'path' in evidence:
                    path = (root / evidence['path']).resolve()
                    if not path.is_relative_to(root.resolve()) or digest(path.read_bytes()) != evidence.get('sha256'):
                        raise ValueError('Changed identity evidence: ' + evidence['path'])
            self.records[row['id']] = row
            self.by_declaration[key] = row

    def find(self, fn):
        key = (fn['path'], fn['name'], declaration(fn))
        return self.records.get(fn['id']) or self.by_declaration.get(key)

    def validate(self, fn, elf_sha256):
        row = self.find(fn)
        if row is None:
            return None
        if source_identity(fn) != row['source']:
            raise ValueError('Stale identity source/declaration')
        if elf_sha256 != self.document['original_elf_sha256']:
            raise ValueError('Stale identity ELF (supply the reviewed original ELF)')
        return row


def load_manifest(path=DEFAULT_MANIFEST):
    return Manifest(json.loads(Path(path).read_text()))


def resolve(fn, candidates, manifest, elf_sha256, get_function):
    """Return a selection and diagnostics; fail closed on invalid reviews."""
    exact = [metadata(f) for f in candidates if qualified_name(f) == fn['name']]
    diagnostic = dict(status='missing' if not exact else 'ambiguous' if len(exact) > 1 else 'exact',
                      candidates=exact)
    try:
        review = manifest.validate(fn, elf_sha256)
        if review:
            diagnostic.update(review_id=review['id'], obligation=review['obligation'])
            if review['disposition'] == 'non-standalone':
                diagnostic['status'] = 'non-standalone'
                return None, diagnostic
            if review['disposition'] == 'mapped':
                instances = review.get('ps2_instances', [review['ps2']])
                live_instances = [metadata(get_function(item['entry'])) for item in instances]
                if live_instances != instances:
                    raise ValueError('Changed live PS2 metadata or entry boundary')
                live = live_instances[instances.index(review['ps2'])]
                diagnostic.update(status='reviewed', selected=live)
                diagnostic['selected_instances'] = live_instances
                return live, diagnostic
        return (exact[0] if len(exact) == 1 else None), diagnostic
    except (ValueError, RuntimeError, KeyError) as error:
        diagnostic.update(status='stale', error=str(error))
        return None, diagnostic


def validate_export(fn, original, diagnostic, manifest, elf_sha256, get_function):
    if not original['complete']:
        raise ValueError('Incomplete PS2 export')
    review = manifest.validate(fn, elf_sha256)
    if review and review['disposition'] == 'non-standalone':
        raise ValueError('Non-standalone review cannot authorize an export')
    if diagnostic.get('status') == 'reviewed' or (review and review['disposition'] == 'mapped'):
        if not review or review['disposition'] != 'mapped':
            raise ValueError('Missing reviewed export identity')
        if diagnostic.get('status') != 'reviewed':
            raise ValueError('Missing reviewed export diagnostic')
        expected = review['ps2']
        instances = review.get('ps2_instances', [expected])
        live_instances = [metadata(get_function(item['entry'])) for item in instances]
        if (diagnostic.get('selected') != expected or metadata(get_function(expected['entry'])) != expected
                or live_instances != instances or diagnostic.get('selected_instances', [expected]) != instances):
            raise ValueError('Changed reviewed PS2 metadata')
        if (original['entry'] != expected['entry'] or original['name'] != qualified_name(expected)
                or original['executable_sha256'] != elf_sha256):
            raise ValueError('Changed reviewed PS2 export identity')
    elif original['name'] != fn['name']:
        raise ValueError('Original identity changed')


def reconcile(cohort, rows):
    """Account for the fixed cohort independently of fresh survey selection."""
    by_id = {row['id']: row for row in rows}
    result = []
    for baseline in cohort['functions']:
        row = by_id.get(baseline['id'])
        if row is None:
            same = [r for r in rows if r['path'] == baseline['path'] and r['name'] == baseline['name']
                    and r['source_sha256'] == baseline['source_sha256']]
            row = same[0] if len(same) == 1 else None
        item = dict(id=baseline['id'], name=baseline['name'], path=baseline['path'])
        if row is None:
            item['status'] = 'not-selected-or-changed'
        else:
            item.update(status=row.get('ps2_identity', {}).get('status', 'blocked-before-identity'),
                        result=row['result'], reason=row.get('reason'), ps2_entry=row.get('ps2_entry'))
        result.append(item)
    return dict(total=len(result), counts=dict(Counter(r['status'] for r in result)), functions=result)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report', type=Path, required=True)
    parser.add_argument('--cohort', type=Path, default=DEFAULT_COHORT)
    parser.add_argument('--output', type=Path, required=True)
    args = parser.parse_args()
    document = reconcile(json.loads(args.cohort.read_text()), json.loads(args.report.read_text())['functions'])
    args.output.parent.mkdir(parents=True, exist_ok=True)
    args.output.write_text(json.dumps(document, indent=2) + '\n')
    print(document['counts'])


if __name__ == '__main__':
    main()
