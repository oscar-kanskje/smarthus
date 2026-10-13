
# Nyttige git operasjoner

- `git clone "git@github.com:oscar-kanskje/smarthus"` kopier dette git repositoret til pc-en din


- `git pull` hent andres endringer av repoet fra github
- `git add` legg til endrede filer (kan bruke `git add *` for å legge til alle endrede filer)
- `git commit -m 'beskrivelse av endringer'` bekreft endringer
- `git push` send dine endringer av repoet til github

## Info om git operasjoner som kobler seg til github
Når man bruker clone, push og pull må man verifisere identiteten
sin ovenfor github. det er litt irriterende.. Og mange måter å
gjøre det på.

Den beste er nok å bruke en ssh-nøkkel

### Sette opp ssh-nøkkel:
1. Generere ssh-nøkkel.
  - kjør `ssh-keygen` i terminalen, bare trykk enter for alle spørsmål
2. Last din public key opp på github profilen din
  - Logg in på github.com
  - Trykk på profilbildet ditt
  - Trykk på `Settings`
  - Trykk på `SSH and GPG keys`
  - Trykk på `New SSH key`
  - Kopier teksten i filen .ssh/<filnavn>.pub over til feltet i github
  - Nå bør det fungere


