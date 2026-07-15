# wintex — обновление оффсетов под Standoff 2 0.38.2

Внешний (external) проект `wintyx`, добавлен в репозиторий как папка `wintex`.
Оффсеты полей классов сверены с авторитетным дампом `deobfuscated_0.38.2.cs`
и `offsets0382.txt`.

## Что обновлено (PlayerController, версия 0.38.2)

В этой версии все поля `PlayerController`, начиная с `0xE0`, сдвинуты на `+0x8`
относительно прежней версии (перед камерой добавлен `PlayerSoundController @ 0xE0`).

| Поле | Было | Стало (0.38.2) | Где |
|---|---|---|---|
| PhotonPlayer (`get_photon`) | `0x158` | `0x160` | `player.h:79`, `player.h:778` (is_host), `game.h:633` (getPhoton) |
| PlayerMainCamera | `0xE0` | `0xE8` | `player.h:286`, `player.h:289`, `game.h:661` (getViewMatrix) |
| PhotonView | `0x148` | `0x150` | `player.h:296` |
| CharacterController (airjump) | `0x110` | `0x118` | `player.h:393` |

Подтверждено по дампу (`PlayerController`, TypeDefIndex 1892):
`MovementController @ 0x98`, `WeaponryController @ 0x88`, `ArmsAnimationController @ 0xA0`,
`PlayerSoundController @ 0xE0`, `PlayerMainCamera @ 0xE8`, `PhotonView @ 0x150`,
`CharacterController @ 0x118`, `PhotonPlayer @ 0x160`.

## Не менялось (по запросу / нет данных в дампах)

- **Базовые статик-адреса** оставлены как есть:
  `PlayerManager = proc.base + 0x9123CE8`, `GameController = 0x911C7A0`,
  `PhotonNetwork = 0x9120D60`, `MainController = 0x80D5C28`.
  Их нет в присланных дампах (берутся из global-metadata). **Проверь вручную.**
- `local_player = 0x70`, `all_players = 0x28` — заданы явно (уже совпадали).
- **WeaponController** (Gunparameters, GunController, reload и т.п.),
  **BombManager**, **ProfileController** (`get_photonA`, профиль-спуф) —
  не покрыты присланными дампами классов, оставлены без изменений.

## Источники
- `deobfuscated_0.38.2.cs` (полный IL2CPP дамп 0.38.2)
- `offsets0382.txt`, `outptCode.txt`, `api.h`
