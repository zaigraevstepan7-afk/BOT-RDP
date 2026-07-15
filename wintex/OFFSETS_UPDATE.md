# wintex — статус обновления оффсетов под новый билд Standoff 2

> **Итог: статические адреса синглтонов из присланных дампов вывести НЕЛЬЗЯ.**
> Код в `sdk/*.h` возвращён к исходным значениям 0.38.2 (чтобы не оставлять
> нерабочие «псевдо-обновлённые» адреса). Что нужно, чтобы доделать — в конце.

## Присланные артефакты

| Артефакт | Дампер | Что внутри | Билд |
|---|---|---|---|
| a1 | custom «Unity6 IL2CPP dumper» | dump.cs + **script.json** + il2cpp.h | старее |
| a2 | custom «Unity6 IL2CPP dumper» | dump.cs + **script.json** + il2cpp.h | новый |
| dump_3 | **классический Il2CppDumper** | только dump.cs (нет script.json) | = a2 |

`a2` и `dump_3` — **один и тот же билд** (4670 методов с идентичными
обфусцированными именами; поля `PlayerController` совпадают побайтно).

## Что удалось подтвердить (не зависит от адресов)

**Раскладка полей классов не менялась** относительно 0.38.2 — это тот же
Unity/il2cpp. `PlayerController`: `PlayerCharacterView @0x48`,
`WeaponryController @0x88`, `MovementController @0x98`,
`ArmsAnimationController @0xA0`, `PlayerSoundController @0xE0`,
`PlayerMainCamera @0xE8`, `CharacterController @0x118`, `PhotonView @0x150`,
`PhotonPlayer @0x160`; `translationParameters @0xA8`/`translationData @0xB0`;
`PlayerMainCamera.Camera @0x20`/`CameraScopeZoomer @0x28`;
`PhotonPlayer.actorID @0x18`/`CustomProperties @0x38`. Оффсеты **полей** трогать
не нужно. (Внимание: внутренние поля `GameController` частично сдвинулись —
напр. `PlayerControls` теперь `@0x2B0`, было `0x2A0`; проверять по месту.)

## Почему адреса синглтонов НЕ вывести из этих дампов

Код резолвит синглтоны как `rpm(proc.base + X)`, где `X` — **истинный
module-RVA** (та же система координат, что у классического Il2CppDumper и у
исходных значений 0.38.2, напр. `RVA_LATEUPDATE = 0x76A839C`).

- **a1/a2 (custom dumper)** выдаёт адреса в **другой, синтетической системе
  координат.** Доказательство: для одного и того же билда (a2 = dump_3) метод
  `PlayerController.LateUpdate` имеет РАЗНЫЙ RVA в custom-дампе и в классическом.
  По 4670 совпавшим методам **постоянной дельты нет** — линейной конвертации
  custom → истинный RVA не существует. Таблица `_TypeInfo` в custom-дампе
  переупакована плотно и по алфавиту (0xAC37D40–0xAC6C2C0), тогда как истинные
  адреса разбросаны по бинарнику (адреса 0.38.2 охватывают ~1.7 MB). Поэтому
  `script.json` из a1/a2 для `proc.base + X` **не подходит.**
- **dump_3 (классический, верная система координат)** содержит истинные RVA
  методов и оффсеты полей, но **не содержит адресов `_TypeInfo`/статических
  полей** (они лежат в его `script.json`, которого в архиве нет).

Ранее закоммиченные значения a1/a2 (`0xAA…`, `0xAC5…`) были из custom-дампера —
**они в неверной системе координат и откачены.**

## Что нужно, чтобы доделать (одно из двух)

1. **`script.json` от классического Il2CppDumper для этого билда** (тот, что
   идёт в паре с `dump_3`). В его `ScriptMetadata` лежат адреса `*_TypeInfo` в
   истинной системе координат — тогда я сразу проставлю корректные
   `GameController` / `PhotonNetwork` / `PlayerManager` / `BombManager` и т.д.
2. **`libil2cpp.so`** этого билда (и `libunity.so` для body-hook). По нему
   выводятся и адреса статических полей корней (`MainController`/профиль-спуф,
   skiptable, high_jump, триггербот, skinchanger), которых нет ни в одном дампе,
   и code-cave для body-hook.

Как только пришлёте один из файлов — обновлю адреса и проверю их против
раскладки полей.

## Источники
- a2/dump.cs + script.json, dump_3/dump.cs (классический), a1/* — сверены.
