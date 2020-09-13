import requests
import csv
from collections import defaultdict
from pprint import pprint
import json

fieldnames = ['word', 'kind', 'definition']
resp = requests.get('https://raw.githubusercontent.com/jan-Lope/Toki_Pona_lessons_English/gh-pages/nimi_pi_toki_pona.csv')

words = defaultdict(list)
for row in csv.DictReader(resp.text.splitlines(), fieldnames=fieldnames):
    key = row['word'].replace(' (e )', '').replace('!', '').strip()

    # merge equal definitions
    dupe = False
    for d in words[key]:
        if d['definition'] == row['definition']:
            d['kind'] = f"{d['kind']}, {row['kind']}"
            dupe = True
            break

    if dupe:
        continue

    words[key].append({
        'kind': row['kind'],
        'definition': row['definition'],
    })

# for (word, defs) in words.items():
#     print(word)
#     for d in defs:
#         print(f"  {d['kind']}: {d['definition']}")

json.dump(words, open('dictionary-jan-lope.json', 'w'), indent=2)
