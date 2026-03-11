# 🤝 Guide de Contribution — SysWatch Pro

Merci de l'intérêt que tu portes à **SysWatch Pro** !
Ce document explique tout ce qu'il faut savoir pour contribuer
au projet de manière efficace et cohérente.

---

## 📋 Table des matières

1. [Code de conduite](#code-de-conduite)
2. [Comment contribuer](#comment-contribuer)
3. [Signaler un bug](#signaler-un-bug)
4. [Proposer une fonctionnalité](#proposer-une-fonctionnalité)
5. [Mettre en place l'environnement de développement](#mettre-en-place-lenvironnement-de-développement)
6. [Workflow Git](#workflow-git)
7. [Standards de code C](#standards-de-code-c)
8. [Architecture et règles de module](#architecture-et-règles-de-module)
9. [Tests et validation](#tests-et-validation)
10. [Soumettre une Pull Request](#soumettre-une-pull-request)
11. [Processus de review](#processus-de-review)
12. [Domaines ouverts aux contributions](#domaines-ouverts-aux-contributions)
13. [Questions fréquentes](#questions-fréquentes)

---

## Code de conduite

En participant à ce projet, tu acceptes de respecter les principes suivants :

- **Respect** : traiter tous les contributeurs avec courtoisie, quelle que soit
  leur expérience.
- **Bienveillance** : les critiques doivent porter sur le code, jamais sur
  la personne.
- **Inclusion** : toute contribution constructive est la bienvenue, des
  débutants aux experts.
- **Honnêteté** : signaler clairement les bugs, les limitations et les
  incompatibilités connues.

Tout comportement abusif, harcelant ou discriminatoire entraînera l'exclusion
du projet.

---

## Comment contribuer

Il existe plusieurs façons de contribuer à SysWatch Pro :

| Type | Description |
|------|-------------|
| 🐛 **Bug fix** | Corriger un bug existant |
| ✨ **Feature** | Ajouter une nouvelle fonctionnalité |
| 📝 **Docs** | Améliorer la documentation |
| 🔧 **Refactor** | Améliorer la qualité du code sans changer le comportement |
| 🧪 **Test** | Ajouter des scénarios de test |
| 🌐 **Port** | Adapter le code à d'autres compilateurs ou versions Windows |
| 🔒 **Security** | Améliorer la détection ou les mécanismes de sécurité |

---

## Signaler un bug

Avant de créer un rapport de bug, vérifie que :
- Le bug n'a pas déjà été signalé dans les
  [Issues](https://github.com/damienos61/SysWatchPro/issues)
- Tu utilises bien la dernière version du code (`git pull`)

### Créer une Issue de bug

Utilise ce modèle :

```
**Titre** : [BUG] Description courte du problème

**Environnement**
- OS : Windows 10 / 11 (version exacte)
- Compilateur : gcc --version
- Version MinGW : 32-bit / 64-bit

**Description du bug**
Décrire clairement ce qui se passe.

**Comportement attendu**
Décrire ce qui devrait se passer.

**Étapes pour reproduire**
1. Lancer SysWatchPro.exe
2. Appuyer sur F4
3. ...

**Messages d'erreur**
Coller ici la sortie exacte du terminal ou du compilateur.

**Captures d'écran**
Si applicable.
```

### Niveaux de sévérité

| Label | Description |
|-------|-------------|
| `critical` | Crash du programme, perte de données |
| `high` | Fonctionnalité majeure cassée |
| `medium` | Fonctionnalité dégradée mais contournable |
| `low` | Problème cosmétique ou mineur |

---

## Proposer une fonctionnalité

### Avant de proposer

Vérifie dans la [Roadmap du README](README.md#roadmap-future) si la
fonctionnalité est déjà planifiée. Si oui, tu peux directement l'implémenter
et soumettre une PR.

### Créer une Issue de fonctionnalité

```
**Titre** : [FEATURE] Description courte

**Problème résolu**
Quel problème cette fonctionnalité résout-elle ?

**Solution proposée**
Décrire l'implémentation envisagée.

**Module(s) concerné(s)**
core/network.c, gui/widgets.c, etc.

**Compatibilité**
Impact sur les vieilles versions de MinGW ?
Nécessite-t-il de nouvelles bibliothèques Windows ?

**Alternatives considérées**
D'autres approches ont-elles été envisagées ?
```

---

## Mettre en place l'environnement de développement

### 1. Prérequis

```bat
REM Vérifier GCC
gcc --version

REM Vérifier Git
git --version
```

Si GCC n'est pas installé :
- **MinGW-w64** : https://www.mingw-w64.org/
- **MSYS2** (recommandé) : https://www.msys2.org/
  ```bash
  pacman -S mingw-w64-x86_64-gcc
  ```

### 2. Forker et cloner le repo

```bat
REM 1. Forker sur GitHub (bouton "Fork" en haut à droite)

REM 2. Cloner ton fork
git clone https://github.com/TON_USERNAME/SysWatchPro.git
cd SysWatchPro

REM 3. Ajouter le repo original comme remote "upstream"
git remote add upstream https://github.com/damienos61/SysWatchPro.git

REM 4. Vérifier les remotes
git remote -v
```

### 3. Créer les dossiers si absents

```bat
mkdir core
mkdir gui
mkdir web
```

### 4. Compiler et vérifier que tout fonctionne

```bat
gcc -Wall -O2 -std=c11 -o SysWatchPro.exe main.c core/process.c core/cpu.c ^
core/memory.c core/network.c core/anomaly.c core/logger.c ^
gui/widgets.c gui/hotkeys.c web/httpserver.c ^
-lpsapi -lkernel32 -liphlpapi -lws2_32
```

Si la compilation produit **zéro ligne `error:`**, l'environnement est prêt.

### 5. Rester à jour avec upstream

```bat
git fetch upstream
git checkout main
git merge upstream/main
```

---

## Workflow Git

### Nommage des branches

| Type | Format | Exemple |
|------|--------|---------|
| Bug fix | `fix/description-courte` | `fix/network-crash-on-empty-table` |
| Nouvelle feature | `feature/nom-feature` | `feature/ipv6-support` |
| Documentation | `docs/sujet` | `docs/api-json-examples` |
| Refactoring | `refactor/module` | `refactor/anomaly-engine` |
| Sécurité | `security/sujet` | `security/stronger-log-encryption` |

### Créer une branche de travail

```bat
REM Toujours partir de main à jour
git checkout main
git pull upstream main

REM Créer la branche
git checkout -b feature/ma-nouvelle-feature
```

### Convention des commits

Le projet suit la convention **Conventional Commits** :

```
type(scope): description courte en français ou anglais

Corps optionnel : expliquer le POURQUOI, pas le COMMENT.
Le code explique le comment.

Footer optionnel : Closes #42
```

#### Types de commit

| Type | Quand l'utiliser |
|------|-----------------|
| `feat` | Nouvelle fonctionnalité |
| `fix` | Correction de bug |
| `docs` | Documentation uniquement |
| `style` | Formatage, espaces (pas de logique) |
| `refactor` | Refactoring sans changement de comportement |
| `perf` | Amélioration de performance |
| `test` | Ajout ou correction de tests |
| `chore` | Tâches de maintenance (Makefile, .gitignore) |
| `security` | Correctif de sécurité |

#### Exemples de bons commits

```
feat(network): ajouter surveillance des connexions IPv6

fix(anomaly): corriger faux positif sur svchost.exe en syswow64

docs(readme): ajouter section dépannage MinGW 32-bit

perf(process): optimiser count_threads() avec cache PID

security(logger): renforcer chiffrement XOR avec clé rotative

Closes #17
```

#### Exemples de mauvais commits ❌

```
fix bug          ← trop vague
WIP              ← ne jamais pusher du WIP sur une PR
update files     ← ne décrit rien
```

### Règle des commits atomiques

Chaque commit doit représenter **une seule modification logique**.
Éviter les commits qui mélangent un bug fix et une nouvelle feature.

---

## Standards de code C

### Style général

Le projet suit un style C11 cohérent. Voici les règles à respecter :

#### Indentation et espacement

```c
/* ✅ Correct */
int process_kill(DWORD pid) {
    HANDLE h = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    if (!h) return 0;
    int ok = (TerminateProcess(h, 1) != 0);
    CloseHandle(h);
    return ok;
}

/* ❌ Incorrect */
int process_kill(DWORD pid){
HANDLE h=OpenProcess(PROCESS_TERMINATE,FALSE,pid);
if(!h)return 0;
int ok=(TerminateProcess(h,1)!=0);
CloseHandle(h);return ok;}
```

#### Nommage

| Élément | Convention | Exemple |
|---------|-----------|---------|
| Fonctions | `snake_case` avec préfixe module | `process_kill()`, `cpu_info_update()` |
| Variables locales | `snake_case` court | `pid`, `hProc`, `nc` |
| Constantes / macros | `SCREAMING_SNAKE_CASE` | `MAX_PROCESSES`, `HISTORY_LEN` |
| Types struct | `PascalCase` avec typedef | `ProcessInfo`, `CpuInfo` |
| Fichiers | `snake_case` | `process.c`, `network.h` |

#### Headers et guards

```c
/* Chaque .h doit avoir un include guard */
#ifndef MODULE_NAME_H
#define MODULE_NAME_H

/* includes système d'abord */
#include <windows.h>
#include <stdio.h>

/* includes locaux ensuite */
#include "autre_module.h"

/* ... contenu ... */

#endif // MODULE_NAME_H
```

#### Commentaires

```c
/* ── Commentaire de section ──────────────────────────────── */

/* Commentaire de fonction : expliquer le POURQUOI */
/* Calcule le delta CPU entre deux appels GetSystemTimes     */
/* Retourne 0 si les compteurs ne sont pas encore initialisés */
int cpu_info_update(CpuInfo* ci) {
    /* ... */
}

/* Commentaire inline : uniquement pour les parties non évidentes */
int streak = get_cpu_streak(p->pid, p->cpuUsage);
if (streak >= 10) {   /* 10 secondes consécutives > 80% CPU */
```

#### Gestion des erreurs

```c
/* ✅ Toujours vérifier les retours des API Windows */
HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
if (!hProc) return 0;   /* échec silencieux acceptable ici */

/* ✅ Libérer les ressources dans tous les cas */
if (hSnap != INVALID_HANDLE_VALUE) {
    /* ... */
    CloseHandle(hSnap);
}

/* ✅ Vérifier les allocations mémoire */
char* buf = malloc(size);
if (!buf) return;
/* utiliser buf */
free(buf);
```

#### Taille des fonctions

- Une fonction = une responsabilité unique
- Maximum **80 lignes** par fonction
- Si une fonction dépasse 80 lignes → la découper

#### Longueur des lignes

- Maximum **100 caractères** par ligne
- Couper les longues expressions :

```c
/* ✅ */
pos += snprintf(tmp + pos, sizeof(tmp) - pos,
    "{\"pid\":%lu,\"name\":\"%s\",\"cpu\":%.1f}",
    (unsigned long)p->pid, p->name, p->cpuUsage);

/* ❌ */
pos += snprintf(tmp+pos,sizeof(tmp)-pos,"{\"pid\":%lu,\"name\":\"%s\",\"cpu\":%.1f}",(unsigned long)p->pid,p->name,p->cpuUsage);
```

#### Compatibilité MinGW 32-bit

Le projet doit compiler **sans erreur** sur MinGW 32-bit (l'ancien `c:\mingw`).
Règles à respecter :

```c
/* ❌ Éviter %llu et %zu — non supportés sur MinGW 32-bit */
printf("%llu", (unsigned long long)value);
printf("%zu", (size_t)size);

/* ✅ Utiliser %lu avec cast explicite */
printf("%lu", (unsigned long)value);

/* ❌ Éviter les fonctions Vista+ sans déclaration manuelle */
GetSystemTimes(&ftI, &ftK, &ftU);

/* ✅ Déclarer manuellement si nécessaire */
WINBASEAPI BOOL WINAPI GetSystemTimes(LPFILETIME, LPFILETIME, LPFILETIME);
```

---

## Architecture et règles de module

### Règle d'or : dépendances à sens unique

```
web/   → core/  ✅
gui/   → core/  ✅
core/  → gui/   ❌  (jamais !)
core/  → web/   ❌  (jamais !)
```

Le module `core/` ne doit **jamais** inclure de headers de `gui/` ou `web/`.

### Ajouter un nouveau module

1. Créer `core/monmodule.h` et `core/monmodule.c`
2. Respecter la convention de nommage : `monmodule_init()`, `monmodule_update()`, `monmodule_print()`
3. Ajouter au Makefile :
   ```makefile
   SRCS = ... core/monmodule.c ...
   ```
4. Intégrer dans `main.c` :
   ```c
   #include "core/monmodule.h"
   /* dans monitor_init() */
   monmodule_init(&g_monmodule);
   /* dans la boucle principale */
   monmodule_update(&g_monmodule);
   ```
5. Documenter dans le README

### Ajouter une règle IA

Dans `core/anomaly.c`, ajouter dans `anomaly_analyze()` :

```c
/* ── Ma nouvelle règle ─────────────────────── */
if (ma_condition) {
    score += MON_SCORE;
    strncat(reasons, "[MA_REGLE] ",
            sizeof(reasons) - strlen(reasons) - 1);
}
```

Et définir la constante dans `core/anomaly.h` :
```c
#define SCORE_MA_REGLE   20
```

### Ajouter un endpoint API JSON

Dans `web/httpserver.c`, dans `build_json()` :

```c
/* ── Ma section JSON ── */
pos += snprintf(tmp + pos, sizeof(tmp) - pos,
    "\"masection\":{\"valeur\":%lu},",
    (unsigned long)ma_valeur);
```

Et dans `web/dashboard.h`, dans le JavaScript :
```javascript
const ms = d.masection;
document.getElementById('ma-div').textContent = ms.valeur;
```

---

## Tests et validation

Le projet n'a pas encore de framework de test automatisé.
La validation se fait manuellement selon cette checklist :

### Checklist de validation avant PR

```
Compilation
  [ ] Compile sans aucune ligne "error:"
  [ ] Les warnings sont réduits au minimum
  [ ] Testé sur MinGW 32-bit (c:\mingw)

Fonctionnement
  [ ] Le programme se lance sans crash
  [ ] Les données CPU et RAM s'affichent correctement
  [ ] Le rafraîchissement toutes les secondes fonctionne
  [ ] F1-F10 répondent correctement
  [ ] F9 (kill) fonctionne sur un processus de test (notepad.exe)
  [ ] Ctrl+C quitte proprement

Dashboard web
  [ ] http://localhost:8080 s'ouvre dans le navigateur
  [ ] Les données se mettent à jour toutes les 2s
  [ ] /api/data retourne du JSON valide

Logs forensics
  [ ] syswatch_forensic.log est créé au lancement
  [ ] --decrypt affiche les logs en clair
  [ ] Les events KILL sont bien enregistrés

Réseau
  [ ] La section réseau affiche des connexions TCP
  [ ] Aucun crash si aucune connexion active

Anomalies
  [ ] Le scoring s'affiche pour les processus actifs
  [ ] Aucun faux positif sur les processus système normaux

Mémoire
  [ ] Pas de fuite mémoire évidente (lancer 5+ minutes)
  [ ] Les handles Windows sont bien fermés (CloseHandle)
```

### Tester un crash potentiel

```bat
REM Lancer en boucle 10 fois pour détecter les crashs intermittents
for /L %i in (1,1,10) do (SysWatchPro.exe & timeout /t 3)
```

### Valider le JSON de l'API

```bat
REM Avec curl (disponible sur Windows 10+)
curl http://localhost:8080/api/data
```

---

## Soumettre une Pull Request

### Avant de soumettre

```bat
REM 1. S'assurer que la branche est à jour avec main
git fetch upstream
git rebase upstream/main

REM 2. Compiler une dernière fois proprement
gcc -Wall -O2 -std=c11 -o SysWatchPro.exe main.c core/process.c ^
core/cpu.c core/memory.c core/network.c core/anomaly.c core/logger.c ^
gui/widgets.c gui/hotkeys.c web/httpserver.c ^
-lpsapi -lkernel32 -liphlpapi -lws2_32

REM 3. Pusher la branche
git push origin feature/ma-nouvelle-feature
```

### Template de Pull Request

Sur GitHub, utilise ce modèle pour la description de ta PR :

```markdown
## Description
<!-- Décrire clairement ce que fait cette PR -->

## Type de changement
- [ ] Bug fix (ne casse pas l'API existante)
- [ ] Nouvelle fonctionnalité (ne casse pas l'API existante)
- [ ] Breaking change (modifie l'API ou le comportement existant)
- [ ] Documentation

## Modules modifiés
- [ ] core/process.c/h
- [ ] core/cpu.c/h
- [ ] core/memory.c/h
- [ ] core/network.c/h
- [ ] core/anomaly.c/h
- [ ] core/logger.c/h
- [ ] gui/widgets.c/h
- [ ] gui/hotkeys.c/h
- [ ] web/httpserver.c/h
- [ ] web/dashboard.h
- [ ] main.c
- [ ] Documentation

## Checklist
- [ ] Le code compile sans erreur sur MinGW 32-bit
- [ ] J'ai testé manuellement avec la checklist de validation
- [ ] J'ai mis à jour le README si nécessaire
- [ ] Mes commits suivent la convention Conventional Commits
- [ ] Je n'ai pas introduit de dépendance core/ → gui/ ou core/ → web/

## Issue liée
Closes #XXX

## Captures d'écran (si applicable)
```

### Règles des PR

- Une PR = une fonctionnalité ou un bug fix
- Ne pas mélanger plusieurs sujets dans une même PR
- La PR doit compiler **sans aucune erreur**
- Le titre de la PR suit la même convention que les commits :
  `feat(network): ajouter support IPv6`

---

## Processus de review

### Délai de réponse

Les PRs seront reviewées dans un délai de **7 jours ouvrés**.

### Critères de review

Le reviewer vérifiera :

1. **Compilation** : zéro erreur sur MinGW 32-bit
2. **Cohérence** : respect des standards de code C du projet
3. **Architecture** : respect des dépendances entre modules
4. **Compatibilité** : pas de régression sur les fonctionnalités existantes
5. **Documentation** : README mis à jour si nécessaire
6. **Commits** : respect de la convention Conventional Commits

### Répondre aux commentaires de review

```bat
REM Faire les modifications demandées, puis :
git add .
git commit -m "fix(network): adresser commentaires review PR #42"
git push origin feature/ma-branche
```

Ne pas forcer-pusher (`--force`) sauf demande explicite du reviewer.

### Merge

Le merge sera effectué par le mainteneur du projet une fois :
- Tous les commentaires résolus
- La checklist complète cochée
- Aucun conflit avec `main`

---

## Domaines ouverts aux contributions

Voici les contributions les plus utiles en ce moment :

### 🔴 Priorité haute
- **Support IPv6** dans `core/network.c`
  — actuellement limité à `AF_INET`
- **Détection DLL injectées** dans `core/process.c`
  — via `EnumProcessModules` + vérification chemin
- **Seuils configurables** via fichier `.ini`
  — permettre de changer `CPU_ALERT`, `RAM_ALERT` sans recompiler

### 🟡 Priorité moyenne
- **Export rapport HTML** quotidien automatique
- **Surveillance registre** `HKLM\SOFTWARE\Microsoft\Windows\CurrentVersion\Run`
- **Mode daemon** sans interface console
- **Amélioration dashboard** : filtres, tri côté client

### 🟢 Priorité basse / Bonne première contribution
- Améliorer les messages d'erreur dans `core/logger.c`
- Ajouter des noms à la liste noire dans `core/process.c`
- Améliorer les commentaires de code
- Corriger des warnings `-Wextra` restants
- Traduire les commentaires du code en anglais

---

## Questions fréquentes

**Q : Dois-je parler français ou anglais dans mes commits ?**
> Les deux sont acceptés. La cohérence à l'intérieur d'une PR est
> plus importante que la langue choisie.

**Q : Puis-je contribuer si je débute en C ?**
> Absolument. Les contributions de documentation, de tests manuels,
> et les petits bug fixes sont parfaits pour débuter.

**Q : Mon code compile sur MinGW-w64 64-bit mais pas 32-bit. Que faire ?**
> Signale-le clairement dans ta PR. On essaiera de trouver ensemble
> une solution compatible. Les déclarations manuelles (`WINBASEAPI BOOL WINAPI ...`)
> sont souvent la solution.

**Q : Puis-je ajouter une dépendance externe ?**
> Le projet a pour objectif de rester **zéro dépendance externe**.
> Toute bibliothèque tierce doit faire l'objet d'une discussion dans
> une Issue avant implémentation.

**Q : Puis-je porter le projet sur Linux ?**
> C'est une idée intéressante mais qui représente un refactoring majeur
> (toutes les API Windows seraient à remplacer). Ouvrir une Issue pour
> en discuter.

**Q : Comment tester sans casser mon système ?**
> Utilise une machine virtuelle Windows ou un compte utilisateur sans
> droits admin pour les tests de kill de processus.

---

*Merci de contribuer à SysWatch Pro —*
*chaque amélioration compte, qu'elle soit grande ou petite.* 🛡️
