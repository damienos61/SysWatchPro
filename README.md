# 🛡️ SysWatch Pro v2.0

> Moniteur système avancé en C pur pour Windows — surveillance temps réel,
> détection IA d'anomalies, dashboard web, logs forensics chiffrés.

---

## 📋 Table des matières

1. [Présentation](#présentation)
2. [Fonctionnalités](#fonctionnalités)
3. [Architecture du projet](#architecture-du-projet)
4. [Prérequis](#prérequis)
5. [Installation](#installation)
6. [Compilation](#compilation)
7. [Lancement](#lancement)
8. [Interface console](#interface-console)
9. [Touches clavier F1-F10](#touches-clavier-f1-f10)
10. [Dashboard Web](#dashboard-web)
11. [API JSON](#api-json)
12. [Moteur IA — Scoring comportemental](#moteur-ia--scoring-comportemental)
13. [Logs forensics chiffrés](#logs-forensics-chiffrés)
14. [Tuer un processus](#tuer-un-processus)
15. [Alertes sonores](#alertes-sonores)
16. [Modules détaillés](#modules-détaillés)
17. [API Windows utilisées](#api-windows-utilisées)
18. [Dépannage](#dépannage)
19. [Roadmap future](#roadmap-future)

---

## Présentation

**SysWatch Pro** est un moniteur système complet développé entièrement en **C pur**
pour Windows. Il surveille en temps réel tous les aspects critiques du système :
processeur, mémoire, processus, réseau, et détecte automatiquement les comportements
suspects grâce à un moteur de scoring comportemental inspiré des SIEM professionnels.

Le programme expose également un **dashboard web** accessible depuis n'importe quel
navigateur sur `http://localhost:8080`, avec graphiques interactifs mis à jour
toutes les 2 secondes.

---

## Fonctionnalités

### 🖥️ Surveillance système
- Charge CPU globale avec historique graphique 60 secondes
- Consommation RAM physique et virtuelle avec historique 60 secondes
- Liste complète des processus actifs (jusqu'à 1024)
- Par processus : PID, CPU%, RAM (KB), nombre de threads, chemin complet de l'exécutable
- Détection des nouveaux processus apparus depuis moins de 60 secondes

### 🌐 Surveillance réseau
- Toutes les connexions TCP actives (ESTABLISHED, LISTEN)
- Identification du processus propriétaire de chaque connexion
- Détection des connexions vers des ports malveillants connus
- Distinction IP privées / IP publiques

### 🧠 Détection IA des anomalies
- Moteur de scoring comportemental sur 8 règles
- 4 niveaux : OK / WARN / ALERTE / CRITIQUE
- Détection des binaires système exécutés hors de system32
- Surveillance des processus à CPU élevé en continu

### 🔐 Logs forensics
- Enregistrement chiffré XOR de tous les événements critiques
- Horodatage précis de chaque entrée
- Rotation automatique des logs à 5 MB
- Mode déchiffrement intégré (`--decrypt`)

### 🌍 Dashboard web
- Serveur HTTP embarqué sur le port 8080
- Interface HTML/CSS/JS moderne, auto-refresh 2s
- Graphiques sparklines CPU et RAM
- Tableau des anomalies IA coloré par niveau
- Vue réseau et top processus

### ⌨️ Navigation avancée
- Touches F1-F10 pour changer de vue instantanément
- Tri par CPU ou par RAM (F8)
- Kill de processus par PID (F9)
- Quitter proprement avec sauvegarde des logs (F10)

### 🔔 Alertes sonores
- Beep d'avertissement si CPU > 85% ou RAM > 90%
- Double beep critique si CPU > 95% ou RAM > 95%
- Alerte sonore une seule fois par dépassement de seuil

---

## Architecture du projet

```
SysWatchPro/
│
├── main.c                        ← Point d'entrée, orchestrateur principal
├── Makefile                      ← Compilation en une commande
├── README.md                     ← Cette documentation
├── SysWatchPro.exe               ← Exécutable (généré après compilation)
├── syswatch_forensic.log         ← Logs chiffrés (généré au lancement)
│
├── core/                         ← Moteur C — logique métier pure
│   ├── process.h / process.c     ← Snapshot processus, CPU/RAM par PID, kill
│   ├── cpu.h     / cpu.c         ← Charge CPU globale + historique 60s
│   ├── memory.h  / memory.c      ← RAM physique + virtuelle + historique 60s
│   ├── network.h / network.c     ← Connexions TCP actives via iphlpapi
│   ├── anomaly.h / anomaly.c     ← Moteur IA scoring comportemental
│   └── logger.h  / logger.c      ← Logs forensics chiffrés XOR + rotation
│
├── gui/                          ← Interface console Windows
│   ├── widgets.h / widgets.c     ← Barres colorées, sections, alertes visuelles
│   └── hotkeys.h / hotkeys.c     ← F1-F10 navigation non-bloquante
│
└── web/                          ← Serveur HTTP embarqué
    ├── httpserver.h / httpserver.c  ← Serveur TCP minimal, thread dédié
    └── dashboard.h               ← Dashboard HTML/CSS/JS complet en string C
```

**Dépendances entre couches :**
```
web/  ──────────────────────────┐
gui/  ──────────────────────────┤──► core/  (core ne dépend de rien)
main.c ─────────────────────────┘
```

---

## Prérequis

- **Windows 7 / 8 / 10 / 11** (32 ou 64 bits)
- **MinGW-w64** (GCC pour Windows) installé et dans le PATH
  - Vérifier : `gcc --version` dans cmd
  - Télécharger : https://www.mingw-w64.org/ ou via MSYS2

---

## Installation

**1. Créer le dossier du projet**
```bat
mkdir C:\Users\LENOVO\Documents\SysWatchPro
cd C:\Users\LENOVO\Documents\SysWatchPro
```

**2. Créer les sous-dossiers**
```bat
mkdir core
mkdir gui
mkdir web
```

**3. Copier tous les fichiers source** dans leurs dossiers respectifs
selon la structure ci-dessus.

---

## Compilation

Ouvrir un terminal (`cmd`) dans le dossier racine du projet et lancer :

```bat
gcc -Wall -O2 -std=c11 -o SysWatchPro.exe main.c core/process.c core/cpu.c core/memory.c core/network.c core/anomaly.c core/logger.c gui/widgets.c gui/hotkeys.c web/httpserver.c -lpsapi -lkernel32 -liphlpapi -lws2_32
```

### Bibliothèques liées

| Lib | Rôle |
|-----|------|
| `-lpsapi` | `GetProcessMemoryInfo` — mémoire par processus |
| `-lkernel32` | API Windows de base |
| `-liphlpapi` | `GetExtendedTcpTable` — connexions réseau |
| `-lws2_32` | Winsock2 — serveur HTTP |

### Message attendu après compilation réussie
```
(aucune ligne "error:")
```
Le warning `ignoring #pragma comment` est **normal et inoffensif**.

---

## Lancement

```bat
SysWatchPro.exe
```

Pour lancer en mode administrateur (recommandé pour tuer des processus système) :
```
Clic droit sur cmd.exe → "Exécuter en tant qu'administrateur"
cd C:\...\SysWatchPro
SysWatchPro.exe
```

---

## Interface console

```
  +===============================================================+
  |  SysWatch Pro v2.0  |  2026-03-11  15:07:14  |  Tout        |
  +===============================================================+

  >> PROCESSEUR (CPU) ----------------------------------------
  Coeurs : 2
  CPU    : [|||||||||||||||||||||||-----------------]  58.4%
  CPU 60s [._:iIHH#@@#IIi:._                              ]

  >> MEMOIRE (RAM) -------------------------------------------
  Totale  : 3.92 GB
  Utilisee: 3.19 GB
  Libre   : 738.0 MB
  Charge  : [||||||||||||||||||||||||||||||||--------]  81.6%
  RAM 60s [HH@@@@HHIi:.                                    ]

  >> TOP PROCESSUS par CPU  [F8=Basculer tri] ----------------
  PID     CPU%    RAM(KB)     Threads  Nom
  ------  ------  ----------  -------  ------------------------
  8368    13.6%      266568   16       chrome.exe
           C:\Program Files\Google\Chrome\Application\chrome.exe
  ...

  >> RESEAU - Connexions actives -----------------------------
  PID     Local IP          L.Port  Remote IP         R.Port  Etat
  ...

  >> IA - DETECTION D'ANOMALIES ------------------------------
  PID     Score  Nom                      Raisons
  ...

  Dashboard web: http://localhost:8080

  Procs:224  Suspects:0  Anomalies:2  Net:0
  [F1]CPU [F2]RAM [F3]Procs [F4]Reseau [F5]Anomalies [F6]Logs [F9]Kill [F10]Quitter
```

### Codes couleur

| Couleur | Signification |
|---------|--------------|
| 🟢 Vert | Normal (CPU < 50%, RAM < 60%) |
| 🟡 Jaune | Attention (CPU 50-80%, RAM 60-85%) |
| 🔴 Rouge | Critique (CPU > 80%, RAM > 85%) |
| 🔵 Cyan | Informations système |
| ⚪ Blanc | Données neutres |
| 🔘 Gris | Valeurs inactives / historique vide |

---

## Touches clavier F1-F10

| Touche | Vue affichée | Description |
|--------|-------------|-------------|
| **F1** | CPU | Charge CPU + graphique 60s uniquement |
| **F2** | RAM | Mémoire + graphique 60s uniquement |
| **F3** | Processus | Liste complète des processus |
| **F4** | Réseau | Connexions TCP actives |
| **F5** | IA Anomalies | Rapport du moteur de détection |
| **F6** | Logs | Informations sur le fichier forensic |
| **F7** | Tout | Afficher tous les modules (défaut) |
| **F8** | — | Basculer tri Processus : CPU ↔ RAM |
| **F9** | — | Saisir un PID pour tuer un processus |
| **F10** | — | Quitter proprement (sauvegarde logs) |
| **Ctrl+C** | — | Arrêt d'urgence |

---

## Dashboard Web

Ouvrir dans n'importe quel navigateur :
```
http://localhost:8080
```

### Contenu du dashboard

- **Carte CPU** : valeur actuelle + barre colorée + sparkline 60s
- **Carte RAM** : usage en GB + barre colorée + sparkline 60s
- **Carte IA Anomalies** : compteurs CRITIQUE/ALERTE/WARN + tableau détaillé
- **Carte Réseau** : statistiques TCP/UDP + liste des connexions actives
- **Tableau Processus** : top 20 avec PID, CPU%, RAM, threads, statut

Le dashboard se rafraîchit automatiquement toutes les **2 secondes** via l'API JSON.

---

## API JSON

### Endpoint

```
GET http://localhost:8080/api/data
```

### Structure de la réponse

```json
{
  "cpu": {
    "usage": 58.4,
    "cores": 2,
    "history": [45.1, 52.3, 58.4, ...]
  },
  "mem": {
    "usage": 81.6,
    "used": 3427000000,
    "total": 4200000000,
    "history": [79.2, 80.1, 81.6, ...]
  },
  "anomalies": {
    "crit": 0,
    "alert": 1,
    "warn": 2,
    "list": [
      { "pid": 1234, "name": "svchost.exe", "score": 70, "level": 2 }
    ]
  },
  "network": {
    "tcp": 42,
    "udp": 0,
    "suspects": 0,
    "list": [
      { "pid": 8368, "proc": "chrome.exe", "local": "192.168.1.5",
        "lport": 54321, "remote": "142.250.74.46", "rport": 443,
        "state": "ESTABLISHED", "suspicious": false }
    ]
  },
  "procs": [
    { "pid": 8368, "name": "chrome.exe", "cpu": 13.6,
      "mem": 266568, "threads": 16, "suspicious": false }
  ]
}
```

---

## Moteur IA — Scoring comportemental

Chaque processus reçoit un **score de 0 à 100** calculé selon 8 règles :

| Règle | Points | Condition |
|-------|--------|-----------|
| Nom malveillant connu | +50 | Correspond à la liste noire intégrée |
| Binaire système hors system32 | +35 | svchost.exe, lsass.exe hors de System32 |
| CPU élevé en continu | +30 | CPU > 80% pendant 10 secondes consécutives |
| Connexion réseau suspecte | +25 | Connexion vers port malveillant connu |
| Nouveau processus | +20 | Apparu depuis moins de 60 secondes |
| Nouveau + mémoire élevée | +20 | Nouveau processus utilisant > 100 MB |
| Threads excessifs | +15 | Plus de 100 threads pour un petit processus |
| Petit processus + réseau | +15 | Processus < 5 MB avec activité réseau |

### Niveaux d'alerte

| Score | Niveau | Couleur | Action |
|-------|--------|---------|--------|
| 0-39 | OK | Vert | Aucune |
| 40-64 | WARN | Cyan | Affiché dans le tableau |
| 65-84 | ALERTE | Jaune | Affiché + log forensic |
| 85-100 | CRITIQUE | Rouge | Affiché + log + beep |

### Liste noire intégrée

```
keylogger, rootkit, backdoor, trojan, miner, cryptominer,
stealer, rat.exe, spy, ransom, inject, hook, dump,
pwdump, mimikatz, netcat, nc.exe, ncat, psexec
```

Pour ajouter des noms, éditer le tableau `SUSPICIOUS_NAMES[]` dans `core/process.c`.

---

## Logs forensics chiffrés

### Fichier généré
```
syswatch_forensic.log  (dans le dossier du .exe)
```

### Chiffrement
Chiffrement **XOR** avec clé `0xA7` — protection légère contre lecture accidentelle.
Rotation automatique à **5 MB** (ancien fichier renommé `syswatch_TIMESTAMP.log.old`).

### Contenu enregistré

| Catégorie | Déclencheur |
|-----------|------------|
| `[ANOMALY]` | Tout processus niveau ALERTE ou CRITIQUE |
| `[KILL]` | Chaque tentative de kill (succès ou échec) |
| `[NET]` | Connexions TCP vers ports suspects |
| `[STARTUP]` | Démarrage du programme |
| `[SHUTDOWN]` | Arrêt propre du programme |

### Lire les logs en clair

```bat
SysWatchPro.exe --decrypt syswatch_forensic.log
```

Exemple de sortie déchiffrée :
```
[SESSION_START] 2026-03-11 15:07:14  SysWatchPro v2.0
[ANOMALY]  2026-03-11 15:07:45  PID=9876   Score= 70  Level=ALERTE    Name=svchost.exe   [CHEMIN ANORMAL]
[KILL]     2026-03-11 15:08:12  PID=8368   Name=chrome.exe             Result=SUCCESS
[SESSION]  2026-03-11 15:09:00  SESSION_END
```

---

## Tuer un processus

**Méthode 1 — Touche F9 :**
1. Appuyer sur **F9**
2. Taper le PID visible dans la liste
3. Appuyer sur **Entrée**

**Méthode 2 — Prompt en bas de l'écran :**
Le prompt `Entrez PID a tuer:` est toujours visible en bas.

### Règles de sécurité
- PID **0, 4** (System, Idle) → refusés automatiquement
- Processus système (`lsass.exe`, `csrss.exe`) → Windows refusera même en admin
- En cas d'échec → message `ECHEC (droits?)` + log forensic

### Recommandation
Lancer SysWatch Pro **en mode Administrateur** pour maximiser les droits de kill :
```
Clic droit cmd.exe → Exécuter en tant qu'administrateur
```

---

## Alertes sonores

| Situation | Son |
|-----------|-----|
| CPU dépasse 85% | 1 beep 900 Hz |
| RAM dépasse 90% | 1 beep 900 Hz |
| CPU dépasse 95% | 3 beeps 1400 Hz rapides |
| RAM dépasse 95% | 3 beeps 1400 Hz rapides |

L'alerte ne se répète **qu'une seule fois** par dépassement de seuil
(pas de beep en boucle tant que la valeur reste haute).

---

## Modules détaillés

### `core/process.c`
- `CreateToolhelp32Snapshot` pour lister tous les processus
- `GetProcessMemoryInfo` (psapi) pour la RAM par processus
- `GetProcessTimes` + `GetSystemTimes` pour le CPU par processus
- `QueryFullProcessImageNameA` pour le chemin complet
- `count_threads()` : snapshot des threads via `TH32CS_SNAPTHREAD`
- Registre first-seen : détecte les nouveaux processus < 60s
- Vérification chemin : binaire système hors system32 = suspect

### `core/cpu.c`
- `GetSystemTimes` : calcul du delta idle/kernel/user entre deux mesures
- Ring buffer 60 entrées pour l'historique
- Sparkline ASCII avec 9 niveaux de densité

### `core/memory.c`
- `GlobalMemoryStatusEx` : RAM totale, libre, virtuelle
- Ring buffer 60 entrées identique au CPU

### `core/network.c`
- `GetExtendedTcpTable` (iphlpapi) : toutes les connexions TCP avec PID
- Détection IP privées (10.x, 192.168.x, 172.16-31.x, 127.x)
- Liste de 14 ports malveillants connus (RAT, miners, RDP sortant)

### `core/anomaly.c`
- Scoring par processus avec accumulation des règles
- Suivi du streak CPU élevé par PID (10s consécutives)
- Corrélation processus ↔ connexions réseau

### `core/logger.c`
- `fopen` en mode binaire append
- XOR byte-à-byte avec clé 0xA7
- `ftell` pour détecter la rotation à 5 MB
- Mode `--decrypt` : lecture + XOR inverse vers stdout

### `gui/widgets.c`
- `SetConsoleTextAttribute` : 8 couleurs utilisées
- `FillConsoleOutputCharacterA` : clear screen sans flash
- `Beep()` : alertes sonores natives Windows

### `gui/hotkeys.c`
- `GetNumberOfConsoleInputEvents` : vérification non-bloquante
- `ReadConsoleInput` : lecture des `KEY_EVENT`
- Virtual key codes `VK_F1` à `VK_F10`

### `web/httpserver.c`
- `socket()` + `bind()` + `listen()` sur `127.0.0.1:8080`
- `ioctlsocket(FIONBIO)` : mode non-bloquant
- Thread dédié via `CreateThread`
- Routing : `/api/data` → JSON | `/` → HTML dashboard

---

## API Windows utilisées

| Module | Fonction | Description |
|--------|----------|-------------|
| kernel32 | `GetSystemTimes` | Temps CPU idle/kernel/user |
| kernel32 | `GetSystemInfo` | Nombre de cœurs |
| kernel32 | `GlobalMemoryStatusEx` | Stats mémoire |
| kernel32 | `CreateToolhelp32Snapshot` | Snapshot processus |
| kernel32 | `Process32First/Next` | Itération processus |
| kernel32 | `Thread32First/Next` | Itération threads |
| kernel32 | `GetTickCount` | Horodatage first-seen |
| kernel32 | `OpenProcess` | Handle par PID |
| kernel32 | `TerminateProcess` | Kill processus |
| kernel32 | `GetProcessTimes` | CPU par processus |
| kernel32 | `QueryFullProcessImageNameA` | Chemin exe |
| kernel32 | `SetConsoleTextAttribute` | Couleurs console |
| kernel32 | `Beep` | Alertes sonores |
| psapi | `GetProcessMemoryInfo` | RAM par processus |
| iphlpapi | `GetExtendedTcpTable` | Connexions TCP avec PID |
| ws2_32 | `socket/bind/listen/accept` | Serveur HTTP |
| ws2_32 | `inet_ntoa/ntohs` | Conversion adresses IP |

---

## Dépannage

### `error: undefined reference to GetSystemTimes`
→ Ton MinGW est trop ancien. Les déclarations manuelles sont déjà dans
`process.h` et `cpu.c`. Vérifie que tu utilises bien les fichiers fournis.

### `error: undefined reference to GetExtendedUdpTable`
→ Non supporté sur MinGW 32-bit. La section UDP a été retirée.
Seul TCP est surveillé — c'est suffisant pour détecter les connexions suspectes.

### Le dashboard web ne s'ouvre pas
→ Vérifier qu'aucun autre programme n'utilise le port 8080 :
```bat
netstat -ano | findstr :8080
```
Si occupé, modifier `HTTP_PORT` dans `web/httpserver.h`.

### `warning: ignoring #pragma comment`
→ Inoffensif. MinGW ignore ce pragma MSVC. Les libs sont passées via `-l` en ligne de commande.

### Processus non tué — `droits insuffisants`
→ Relancer SysWatch Pro en **Administrateur**.

### Les logs sont illisibles
→ Normal — ils sont chiffrés XOR. Utiliser :
```bat
SysWatchPro.exe --decrypt syswatch_forensic.log
```

---

## Roadmap future

- [ ] Support IPv6 pour le module réseau
- [ ] Surveillance des clés de registre (HKLM Run)
- [ ] Détection des DLL injectées dans les processus
- [ ] Export rapport HTML automatique quotidien
- [ ] Seuils d'alerte configurables via fichier `.ini`
- [ ] Mode silencieux (pas d'interface console, daemon pur)
- [ ] Support HTTPS pour le dashboard (TLS via mbedTLS)
- [ ] Notifications Windows toast (WinRT)
- [ ] Base de données SQLite pour l'historique long terme

---

*SysWatch Pro — Développé en C pur pour Windows*
*Architecture modulaire — Zéro dépendance externe*
