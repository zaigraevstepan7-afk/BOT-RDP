# wintex — обновление оффсетов под новый Unity6-билд Standoff 2

Обновление поверх `OFFSETS_0.38.2.md`. Сверено с присланным дампом:
`a1/dump.cs` (кастомный Unity6 IL2CPP dumper, Types: 31978, Methods: 191127)
и `a1/script (1).json` (`ScriptMetadata` — адреса указателей на классы `*_TypeInfo`).

## Главный вывод

**Раскладка полей классов НЕ изменилась** — версия Unity/il2cpp та же, что и в
0.38.2. `PlayerController` в новом дампе байт-в-байт совпал с текущими оффсетами
исходника (см. таблицу «сверено» ниже). Сдвинулись только **статические адреса
`*_TypeInfo`**: бинарник вырос, и таблица метадаты уехала с диапазона `~0x91xxxxx`
на `~0xAAxxxxx`. Поэтому обновлять нужно было только базовые статик-адреса
синглтонов, а не оффсеты полей.

## Что обновлено (применено в коде)

| Синглтон (`*_TypeInfo`) | Было (0.38.2) | Стало (новый билд) | Где |
|---|---|---|---|
| `PlayerManager` | `0x92BDA78` (153868920) | `0xAA181F8` (178356728) | `game.h:32` (resolvePlayerManager), `functions.h:165` (OFF_PM_STATIC) |
| `GameController` | `0x911C7A0` | `0xAA12E60` | `player.h:289`, `game.h:2695`, `game.h:2731` |
| `PhotonNetwork` | `0x9120D60` | `0xAA17EA0` | `player.h:298`, `player.h:755` |
| `BombManager` | `0x9114DC0` | `0xAA09DB8` | `functions.h:531` |

Точные имена классов из `ScriptMetadata`:
`Axlebolt.Standoff.Player.PlayerManager_TypeInfo`,
`Axlebolt.Standoff.Game.GameController_TypeInfo`,
`PhotonNetwork_TypeInfo`,
`Axlebolt.Standoff.Inventory.Bomb.BombManager_TypeInfo`.

**Проверка порядком:** относительный порядок этих четырёх адресов совпадает в
обоих билдах — `BombManager < GameController < PhotonNetwork < PlayerManager`
(было `0x9114DC0 < 0x911C7A0 < 0x9120D60 < 0x92BDA78`,
стало `0xAA09DB8 < 0xAA12E60 < 0xAA17EA0 < 0xAA181F8`). Это подтверждает
корректность сопоставления по именам.

Навигационные оффсеты внутри Il2CppClass (`+0x60`/`+0x190`/`+0x198` static_fields,
`+0x10`/`+0x8` instance) **не трогались** — они зависят от версии Unity, а она
не менялась.

## Сверено по дампу — оффсеты полей НЕ изменились (правки не требуются)

- **PlayerController** (TypeDefIndex 848): `PlayerCharacterView @ 0x48`,
  `WeaponryController @ 0x88`, `MovementController @ 0x98`,
  `ArmsAnimationController @ 0xA0`, `PlayerSoundController @ 0xE0`,
  `PlayerMainCamera @ 0xE8`, `CharacterController @ 0x118`,
  `PhotonView @ 0x150`, `PhotonPlayer @ 0x160`.
- **MovementController**: `translationParameters @ 0xA8`, `translationData @ 0xB0`.
- **PlayerMainCamera**: `Camera @ 0x20`, `CameraScopeZoomer @ 0x28`.
- **PhotonView**: `synchronization @ 0x50` (clumsy).
- **PhotonPlayer**: `actorID @ 0x18`, `CustomProperties @ 0x38`.
- **WeaponryController**: `WeaponManager @ 0xA0`, invis-флаги `@ 0x88/0x89`.
- **PlayerManager**: `local_player @ 0x70`, `all_players @ 0x28`.

## НЕ обновлено — требует ручной проверки по .so

Эти адреса указывают на **обфусцированные** корневые синглтоны (их имён нет в
дампе) либо лежат в другой таблице метадаты, которую нельзя однозначно
сопоставить из `dump.cs`/`script.json`. Оставлены как есть, чтобы не подставить
неверный адрес (особенно для write-функций):

- `MainController` (`player.h` getMainController, `0x80CEF68`) и вся цепочка
  `ProfileController` (профиль-спуф: gold/level, `get_photonA`) — классы
  обфусцированы, по именам в дампе не находятся.
- `functions.h:515` «gameManager» (`0x80CB658`) — корневой синглтон обфусцирован
  (фича skiptable).
- `functions.h:562` «playercontrols» (`0x9086CA0`) — high_jump; нестандартная
  навигация (`+0x30`), не подтверждена.
- `game.h:2366/2389` (`0x7EF6150`) — isFiring/fisting (триггербот). Корень
  обфусцирован; он *ведёт* к `PlayerControls`, но сам ≠ `PlayerControls`.
- `skinchanger.h` (`0x91143F8`) — смена скинов; обфусцированный inventory-корень.
- `resolvePlayerManager` fallback-кандидат #2 (`132435632` / `0x7E4CEB0`) —
  оставлен: используется только если #1 не срезолвился.
- **Body-hook** (`functions.h`, namespace `tpbody`, «показать своё тело»):
  `RVA_LATEUPDATE`, `RVA_CAVE1/2`, `RVA_SETCHARVIS`, `RVA_SETTPSVIEW`,
  `ORIG_LU_INSN`. `PlayerController.LateUpdate` в новом дампе имеет
  `RVA 0x8789904` (было `0x76A839C`), но `SetCharacterVisible`/`SetTPSView`
  обфусцированы (по именам отсутствуют), а RVA code-cave лежат в **libunity** и
  из il2cpp-дампа не выводятся. Хук нужно пере-вывести из самой `.so` перед
  включением — иначе он пишет ветвление в неверный адрес.

## Справочно — новые адреса, найденные в дампе (для ручного применения)

Если после проверки понадобится подключить связанные фичи:

- `Axlebolt.Standoff.Game.GameManager_TypeInfo` = `0xAA12EB8`
- `Axlebolt.Standoff.Controls.PlayerControls_TypeInfo` = `0xAA18148`
- `PlayerController.LateUpdate` RVA = `0x8789904`

## Источники
- `a1/dump.cs` (полный Unity6 IL2CPP дамп)
- `a1/script (1).json` (`ScriptMetadata`: `*_TypeInfo`)
- `a1/il2cpp.h`
