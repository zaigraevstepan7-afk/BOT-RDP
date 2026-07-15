# wintex — обновление оффсетов под новый Unity6-билд Standoff 2

Обновление поверх `OFFSETS_0.38.2.md`. Присланы два близких дампа, сверены оба:

| Дамп | Types | Methods | Image 0 | Итоговые адреса |
|---|---|---|---|---|
| a1 | 31978 | 191127 | `Assembly-CSharp.dll` | `~0xAA1xxxx` |
| **a2** (актуальный) | 32644 | 194219 | `CSharp.dll` | `~0xAC5xxxx` |

**В коде применены адреса из a2** (более новый билд, прислан последним).
Раскладка полей классов в обоих дампах одинаковая — это тот же Unity/il2cpp,
что и в 0.38.2; поменялись только статические адреса `*_TypeInfo`
(таблица метадаты уехала из-за роста бинарника).

## Что обновлено (применено в коде, значения a2)

| Синглтон (`*_TypeInfo`) | 0.38.2 | a1 | **a2 (в коде)** | Где |
|---|---|---|---|---|
| `PlayerManager` | `0x92BDA78` | `0xAA181F8` | **`0xAC5E190`** (180740496) | `game.h:34`, `functions.h:165` |
| `GameController` | `0x911C7A0` | `0xAA12E60` | **`0xAC58BB0`** | `player.h:289`, `game.h:2697/2733` |
| `PhotonNetwork` | `0x9120D60` | `0xAA17EA0` | **`0xAC5DE18`** | `player.h:298`, `player.h:755` |
| `BombManager` | `0x9114DC0` | `0xAA09DB8` | **`0xAC4FAC0`** | `functions.h:531` |

Точные имена из `ScriptMetadata`:
`Axlebolt.Standoff.Player.PlayerManager_TypeInfo`,
`Axlebolt.Standoff.Game.GameController_TypeInfo`,
`PhotonNetwork_TypeInfo`,
`Axlebolt.Standoff.Inventory.Bomb.BombManager_TypeInfo`.

**Проверка порядком** (устойчива к вставкам типов между билдами): относительный
порядок совпадает во всех трёх версиях —
`BombManager < GameController < PhotonNetwork < PlayerManager`
(a2: `0xAC4FAC0 < 0xAC58BB0 < 0xAC5DE18 < 0xAC5E190`). Подтверждает сопоставление.

Навигационные оффсеты внутри Il2CppClass (`+0x60`/`+0x190`/`+0x198` static_fields,
`+0x10`/`+0x8` instance) не трогались — версия Unity не менялась.

## Сверено по дампу a2 — оффсеты полей НЕ изменились

`PlayerController` (в a2 TypeDefIndex 776) совпал с исходником:
`PlayerCharacterView @ 0x48`, `WeaponryController @ 0x88`, `MovementController @ 0x98`,
`ArmsAnimationController @ 0xA0`, `PlayerSoundController @ 0xE0`,
`PlayerMainCamera @ 0xE8`, `CharacterController @ 0x118`,
`PhotonView @ 0x150`, `PhotonPlayer @ 0x160`. А также:
`MovementController.translationParameters @ 0xA8` / `translationData @ 0xB0`,
`PlayerMainCamera.Camera @ 0x20` / `CameraScopeZoomer @ 0x28`,
`PhotonView.synchronization @ 0x50`,
`PhotonPlayer.actorID @ 0x18` / `CustomProperties @ 0x38`,
`PlayerManager.local_player @ 0x70` / `all_players @ 0x28`.

## НЕ обновлено — требует ручной проверки по .so

Обфусцированные корневые синглтоны (имён нет в дампе) или адреса из другой
таблицы метадаты — не выводятся однозначно из `dump.cs`/`script.json`,
оставлены как есть (риск неверной записи в память):

- `MainController` (`player.h`, `0x80CEF68`) и цепочка `ProfileController`
  (профиль-спуф gold/level, `get_photonA`).
- `functions.h:515` «gameManager» (`0x80CB658`) — skiptable.
- `functions.h:562` «playercontrols» (`0x9086CA0`) — high_jump.
- `game.h:2366/2389` (`0x7EF6150`) — isFiring/fisting (триггербот).
- `skinchanger.h` (`0x91143F8`) — смена скинов.
- `resolvePlayerManager` fallback #2 (`132435632`) — используется только если #1 не сработал.
- **Body-hook** (`functions.h`, `tpbody`): `SetCharacterVisible`/`SetTPSView`
  обфусцированы, RVA code-cave лежат в libunity — из il2cpp-дампа не выводятся.
  Пере-вывести из `.so` перед включением.

## Справочно — новые адреса a2 (для ручного применения после проверки)

- `Axlebolt.Standoff.Game.GameManager_TypeInfo` = `0xAC58C00`
- `Axlebolt.Standoff.Controls.PlayerControls_TypeInfo` = `0xAC5E0E0`

## Источники
- a2: `dump (2).cs`, `script (2).json`, `il2cpp (1).h` (Unity6, Types 32644) — **применён**
- a1: `dump.cs`, `script (1).json` (Unity6, Types 31978) — сверен
