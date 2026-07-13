#!/usr/bin/env bash
# =============================================================================
#  Reality (VLESS + XTLS-Vision) server setup  —  НЕразрушающий скрипт
#  Запускать НА СЕРВЕРЕ от root.   Маскировка (SNI/dest): ya.ru,  порт: 443
#  Идентичность (UUID/ключи/shortId) вшита и совпадает с клиентскими конфигами.
# =============================================================================
set -euo pipefail

### ==== Параметры (при желании можно поменять) ====
UUID="facf3911-7366-4f6e-9ab9-ca31bfbed2a1"
PRIVATE_KEY="SCRNtPzsS79vV1LsO0TaVqRjDYYA9ex0DXVURaQshm0"
PUBLIC_KEY="8FLNRBKDujmJZPoDCsUln7pRUqfjIvtnFw4YEDF9XH8"
SHORT_ID="d8d552dc89b5ec97"
SNI="ya.ru"
PORT=443
XRAY_CONF="/usr/local/etc/xray/config.json"

log(){ echo -e "\033[1;36m[*]\033[0m $*"; }
err(){ echo -e "\033[1;31m[!]\033[0m $*" >&2; }

# --- root ---
if [ "$(id -u)" -ne 0 ]; then err "Запусти от root:  sudo -i  затем снова этот скрипт"; exit 1; fi

# --- порт не занят? (не ломаем то, что уже работает) ---
log "Проверяю, свободен ли порт ${PORT}..."
if ss -tlnp 2>/dev/null | grep -q ":${PORT} "; then
  err "Порт ${PORT} уже занят другим процессом:"
  ss -tlnp | grep ":${PORT} " >&2 || true
  err "Скрипт остановлен, чтобы НЕ сломать существующий сервис."
  err "Освободи порт ${PORT} ИЛИ поменяй PORT= вверху скрипта и в клиентском конфиге, затем запусти снова."
  exit 1
fi

# --- зависимости ---
log "Ставлю зависимости..."
export DEBIAN_FRONTEND=noninteractive
apt-get update -y   >/dev/null 2>&1 || true
apt-get install -y curl unzip ca-certificates jq >/dev/null 2>&1 || true

# --- Xray (официальный установщик) ---
log "Устанавливаю/обновляю Xray..."
bash -c "$(curl -fsSL https://github.com/XTLS/Xray-install/raw/main/install-release.sh)" @ install >/dev/null

# --- бэкап старого конфига ---
if [ -f "$XRAY_CONF" ]; then
  BAK="${XRAY_CONF}.bak.$(date +%s)"
  cp -a "$XRAY_CONF" "$BAK"
  log "Старый конфиг сохранён: $BAK"
fi

# --- пишем конфиг Reality ---
log "Пишу конфиг Reality..."
mkdir -p "$(dirname "$XRAY_CONF")"
cat > "$XRAY_CONF" <<JSON
{
  "log": { "loglevel": "warning" },
  "inbounds": [
    {
      "listen": "0.0.0.0",
      "port": ${PORT},
      "protocol": "vless",
      "settings": {
        "clients": [ { "id": "${UUID}", "flow": "xtls-rprx-vision" } ],
        "decryption": "none"
      },
      "streamSettings": {
        "network": "tcp",
        "security": "reality",
        "realitySettings": {
          "show": false,
          "dest": "${SNI}:443",
          "xver": 0,
          "serverNames": [ "${SNI}" ],
          "privateKey": "${PRIVATE_KEY}",
          "shortIds": [ "${SHORT_ID}" ]
        }
      },
      "sniffing": { "enabled": true, "destOverride": [ "http", "tls", "quic" ] }
    }
  ],
  "outbounds": [
    { "protocol": "freedom",  "tag": "direct", "settings": { "domainStrategy": "UseIP" } },
    { "protocol": "blackhole", "tag": "block" }
  ],
  "routing": {
    "domainStrategy": "IPIfNonMatch",
    "rules": [
      { "type": "field", "ip": [ "geoip:private" ], "outboundTag": "block" }
    ]
  }
}
JSON

# --- проверка конфига ---
log "Проверяю конфиг (xray -test)..."
if ! xray -test -config "$XRAY_CONF"; then
  err "Конфиг не прошёл проверку. При необходимости восстанови бэкап (${XRAY_CONF}.bak.*)."
  exit 1
fi

# --- фаервол: открыть порт, НЕ потеряв SSH ---
log "Настраиваю фаервол (SSH сохраняю)..."
if command -v ufw >/dev/null 2>&1 && ufw status 2>/dev/null | grep -q "Status: active"; then
  ufw allow 22/tcp        >/dev/null 2>&1 || true   # гарантируем SSH
  ufw allow ${PORT}/tcp   >/dev/null 2>&1 || true
  log "ufw активен: разрешены 22/tcp и ${PORT}/tcp"
else
  log "ufw не активен — не трогаю его (чтобы не закрыть SSH)."
  log "ВАЖНО: убедись, что порт ${PORT}/tcp открыт в облачном фаерволе провайдера."
fi

# --- запуск ---
log "Запускаю сервис xray..."
systemctl enable xray >/dev/null 2>&1 || true
systemctl restart xray
sleep 1
if ! systemctl is-active --quiet xray; then
  err "Сервис xray не запустился. Смотри:  journalctl -u xray -e --no-pager"
  exit 1
fi

# --- внешний IP ---
IP4="$(curl -fsS -4 https://api.ipify.org 2>/dev/null || true)"
[ -z "${IP4:-}" ] && IP4="$(hostname -I 2>/dev/null | awk '{print $1}')"

LINK="vless://${UUID}@${IP4}:${PORT}?encryption=none&flow=xtls-rprx-vision&security=reality&sni=${SNI}&fp=chrome&pbk=${PUBLIC_KEY}&sid=${SHORT_ID}&type=tcp#Reality-${IP4}"

echo
echo "============================================================"
echo "  ГОТОВО ✅  Reality работает на ${IP4}:${PORT}  (маскировка: ${SNI})"
echo "============================================================"
echo "  Ссылка для быстрого импорта (v2rayNG / NekoBox / Hiddify / Streisand):"
echo
echo "  ${LINK}"
echo
echo "  UUID:       ${UUID}"
echo "  PublicKey:  ${PUBLIC_KEY}"
echo "  shortId:    ${SHORT_ID}"
echo "  SNI/dest:   ${SNI}"
echo "============================================================"
echo "  Полезное:"
echo "    статус:   systemctl status xray"
echo "    логи:     journalctl -u xray -e --no-pager"
echo "    рестарт:  systemctl restart xray"
echo "============================================================"
