# R2K Open Points – Detaillierte Umsetzungsanweisungen

## Ziel dieses Dokuments
Dieses Dokument fasst **alle aktuell bekannten offenen Punkte** aus den bestehenden R2K-Karten- und Architekturkommentaren zusammen und überführt sie in **konkret umsetzbare Arbeitspakete**.

Es ist als „Execution Guide“ gedacht: jede Aufgabe enthält
- Zielbild,
- technische Umsetzungsschritte,
- Abnahmekriterien,
- Test-/Validierungsvorschläge,
- Risiken und Guardrails.

---

## Priorisierung (empfohlene Reihenfolge)
1. **Pass/Receive-Workflow stabilisieren** (`OffenseForwardPassCard`, `OffenseReceivePassCard`)
2. **LongShot/ClearOwnHalf harmonisieren** (`GoalieLongShotCard`, `ClearOwnHalfCard`, `ClearOwnHalfGoalieCard`)
3. **Shot-Konsolidierung Richtung GoalShot/SectorWheel** (`OffenseFastGoalKickCard` vs. `GoalShotCard`)
4. **ReferenceCard-OpenPoint schließen** (taktische Offense-Rollenpräzisierung)
5. **Systematische include/REQUIRES-Bereinigung + CI-Checks**
6. **Naming-Migration kontrolliert umsetzen**

---

## Arbeitspaket 1 – Pass/Receive-Workflow stabilisieren

### 1.1 Problemstellung
- `OffenseForwardPassCard`: Precondition ist als „needs to be fixed“ markiert.
- `OffenseReceivePassCard`: EBC-Verhalten und Robustheit der Kopplung sind offen.

### 1.2 Zielbild
Ein robuster, nachvollziehbarer Passablauf mit:
- deterministischer Pass-Auslösung,
- klarer Receiver-Selektion,
- sauberem Abbruchverhalten,
- eindeutigen Logging-/EBC-Signalen.

### 1.3 Umsetzungsschritte
1. **Precondition-Matrix definieren** (Pass nur wenn):
   - Ballführer bestätigt (`playsTheBall`),
   - mindestens ein valider Zielspieler vorhanden (aktiv, aufrecht, taktisch passend),
   - keine Konfliktaktivität (z. B. mehrere gleichzeitige Pass-/Clear-Aktionen),
   - Mindestabstand / Mindestwinkel für sinnvollen Vorwärtspass.
2. **Receiver-Auswahl deterministisch machen**:
   - Scoring-Funktion dokumentieren (x-Fortschritt, y-Offset, Gegnernähe optional),
   - Tie-Break über Roboternummer für reproduzierbares Verhalten.
3. **Pass-Handover explizit modellieren**:
   - Sender erzeugt `pass_intent` mit `target` und Timestamp,
   - Receiver akzeptiert nur konsistente, frische Intents,
   - Timeout/Abort bei ausbleibender Bestätigung.
4. **Receiver-Postconditions schärfen**:
   - Ausstieg bei verlorenem Passkontext,
   - Ausstieg bei Rollenwechsel/Strikerwechsel.
5. **R2KLOG-Events standardisieren**:
   - `pass_intent`, `pass_ack`, optional `pass_abort` mit eindeutigen Feldern.

### 1.4 Abnahmekriterien
- Keine zyklischen Übergänge zwischen Forward/Receive ohne neuen Kontext.
- Receiver wird nur durch valide und frische Passintention aktiviert.
- Bei Kommunikationsverlust fällt das Verhalten in sichere Standardkarten zurück.

### 1.5 Tests
- Unit-Tests für Selektion und Timeoutlogik (neue kleine Logic-Helper empfohlen).
- Simulationsszenario: 3v3/5v5 mit künstlicher Verzögerung in Teamdaten.
- Logprüfung: erwartete Event-Sequenz ohne doppelte Acks.

### 1.6 Risiken / Guardrails
- Risiko: Deadlock Sender wartet auf Ack, Receiver wartet auf Bedingung.
- Guardrail: harte Zeitouts + Fallback auf Chase/Default.

---

## Arbeitspaket 2 – LongShot/ClearOwnHalf harmonisieren

### 2.1 Problemstellung
In mehreren Karten sind noch offene Punkte zu Schussvektor, `isDone()` und Parametrisierung dokumentiert.

### 2.2 Zielbild
Einheitliches Clearing-/LongShot-Verhalten mit:
- konsistenter Distanzparametrisierung,
- besserer Schussrichtung (freiere Bahn),
- reproduzierbarem Exit-Verhalten.

### 2.3 Umsetzungsschritte
1. **Gemeinsame Parameterfamilie definieren** (pro Karte konfigurierbar, aber gleich benannt):
   - `minOpponentDistanceMm`,
   - `emergencyOpponentDistanceMm`,
   - `shotDirectionScanAngleDeg`,
   - `postKickExitTimeoutMs`.
2. **Freie Schussrichtung einführen**:
   - einfache Sektorprüfung (falls SectorWheel noch nicht direkt integriert),
   - fallback auf zentrale Zielrichtung, wenn kein freier Kanal gefunden.
3. **`isDone()`-Strategie vereinheitlichen**:
   - Ende nach bestätigter Kickausführung oder Timeout,
   - kein Festhängen bei Ballstau.
4. **Kartenübergreifende Konsistenz**:
   - `DefenseLongShotCard`, `GoalieLongShotCard`, `ClearOwnHalfCard`, `ClearOwnHalfGoalieCard` auf gleiche Semantik bringen.

### 2.4 Abnahmekriterien
- Keine unbegrenzten Wiederholungsschleifen nach Fehlschuss.
- Schussrichtung reagiert messbar auf Hindernisse.
- Parameternamen sind in allen betroffenen Karten harmonisiert.

### 2.5 Tests
- Unit-Test für Zielrichtungswahl (falls Helper extrahiert).
- Regressions-Szenarien: Gegnerdruck nah/fern, Ball in eigener Hälfte, Wifi on/off.

---

## Arbeitspaket 3 – GoalShot/SectorWheel-Konsolidierung

### 3.1 Problemstellung
`OffenseFastGoalKickCard` enthält weiterhin den Hinweis, langfristig durch eine anspruchsvollere GoalShot-Variante ersetzt zu werden.

### 3.2 Zielbild
Eine konsolidierte Abschlusslogik (möglichst in `GoalShotCard`) mit klaren Entscheidungsstufen:
- hold,
- safety kick,
- precise kick,
- optional pass fallback.

### 3.3 Umsetzungsschritte
1. **Feature-Matrix erstellen**:
   - Welche Fähigkeiten sind in FastGoalKick vorhanden,
   - welche in GoalShot fehlen.
2. **Fehlende Features in GoalShot ergänzen**:
   - Ballsource-/Forecast-Einbindung,
   - logging parity,
   - ggf. SectorWheel-basierte Zielselektion.
3. **Card-Stack-Reihenfolge prüfen** (`gameplayCard.cfg`):
   - klare Priorität, keine redundante Konkurrenz zweier „Shot-First“-Karten.
4. **FastGoalKick entweder reduzieren oder als dünnen Wrapper belassen**.

### 3.4 Abnahmekriterien
- Keine widersprüchlichen Shot-Entscheidungen durch konkurrierende Karten.
- Logging-Felder für Abschlussentscheidungen sind vereinheitlicht.

---

## Arbeitspaket 4 – ReferenceCard OpenPoint finalisieren

### 4.1 Problemstellung
`ReferenceCard` enthält weiterhin offene Frage zur Offense-Rollenpräzisierung (`OFFENSE_LEFT`, `ANY_OFFENSE`).

### 4.2 Zielbild
Eindeutige, dokumentierte Regel:
- entweder jede taktische Offense-Rolle zulässig,
- oder gezielte Einschränkung mit nachvollziehbarer Begründung.

### 4.3 Umsetzungsschritte
1. Rolle explizit entscheiden (Designentscheidung im Kommentar + Code).
2. Precondition entsprechend anpassen.
3. Mini-Test/Log-Szenario zur Verifikation hinzufügen.

### 4.4 Abnahmekriterien
- Kein uneindeutiger Rollenkommentar mehr.
- Verhalten entspricht dokumentierter Taktik.

---

## Arbeitspaket 5 – Includes/REQUIRES Cleanup + CI-Schutz

### 5.1 Problemstellung
Der Plan nennt systematischen Cleanup und CI-Absicherung als nächste Phase.

### 5.2 Zielbild
Weniger technische Schulden und frühzeitige Regressionserkennung.

### 5.3 Umsetzungsschritte
1. **Per-Card Matrix erstellen**:
   - deklarierte `REQUIRES` vs. tatsächlich verwendete Symbole,
   - unnötige Includes entfernen.
2. **Automatisierte Checks ergänzen**:
   - bestehendes `check_r2k_cards.py` als CI-Step,
   - fokussierte GTest-Compile/Run als CI-Step.
3. **Linter schrittweise verbessern**:
   - optional strukturierter Parser statt reiner String-Heuristik.

### 5.4 Abnahmekriterien
- Keine toten `REQUIRES` in den bearbeiteten Karten.
- Lint + fokussierte Tests laufen reproduzierbar in CI.

---

## Arbeitspaket 6 – Naming-Migration planvoll umsetzen

### 6.1 Problemstellung
Eine Mapping-Strategie liegt vor, die eigentliche Migration ist aber noch nicht abgeschlossen.

### 6.2 Zielbild
Konsistente Kartennamen gemäß kanonischem Schema ohne Runtime-Brüche.

### 6.3 Umsetzungsschritte
1. Migration in kleinen Gruppen (z. B. zuerst Offense-Chase/Pass).
2. Konfigurationsstapel kontrolliert umstellen.
3. Alias-/Kompatibilitätsphase einplanen (falls Framework-seitig nötig).
4. Dokumentation + Mappings aktuell halten.

### 6.4 Abnahmekriterien
- Card-Stacks lösen alle Karten korrekt auf.
- Keine semantischen Änderungen durch reine Umbenennung.

---

## Querschnittsanforderungen für **alle** Arbeitspakete

### A) Logging-Disziplin
- Einheitliches Feldschema pro Eventtyp,
- numerische Werte als integer-string (stabil parsbar),
- keine Event-Flut (nur bei Zustandswechseln oder signifikanten Entscheidungen).

### B) Fallback-Disziplin
- Jede neue Bedingung braucht einen sicheren Fallbackpfad.
- Timeout- und Kommunikationsfehler müssen zu stabilen Basiskarten zurückführen.

### C) Pre/Postcondition-Disziplin
- Standard: `postconditions() == !preconditions()`.
- Ausnahmen nur mit expliziter Begründung im Kommentar.

### D) Validierungs-Standard vor Merge
1. `python Scripts/behavior/check_r2k_cards.py`
2. Fokus-GTests bauen und ausführen (`R2KTeamLogic`, `R2KAttackLogic`, `R2KDecisionLog`, `R2KBallSourceLogic`, `R2KDribbleLogic`)
3. `git diff --check`
4. Optional: kurzes Simulationsprotokoll mit zentralen R2KLOG-Events

---

## Definition of Done (gesamt)
Ein Open Point gilt als abgeschlossen, wenn:
1. der zugehörige Kommentar im Code auf „implemented“ oder „remaining future work“ präzisiert wurde,
2. die Änderung mit mindestens einem automatisierten Check abgesichert ist,
3. das Verhalten per Log/Simulation nachvollziehbar ist,
4. keine neuen Linter- oder Whitespace-Verstöße entstehen.

