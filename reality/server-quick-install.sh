# ==== Reality (VLESS+Vision) — быстрая установка. Вставь целиком в root-консоль сервера ====
export DEBIAN_FRONTEND=noninteractive
apt-get update -y; apt-get install -y curl unzip jq
if ss -tlnp 2>/dev/null | grep -q ':443 '; then
  echo ">>> ВНИМАНИЕ: порт 443 уже занят. Останавливаюсь, чтобы не сломать рабочее. Разберись и запусти снова."
else
  bash -c "$(curl -fsSL https://github.com/XTLS/Xray-install/raw/main/install-release.sh)" @ install
  [ -f /usr/local/etc/xray/config.json ] && cp -a /usr/local/etc/xray/config.json /usr/local/etc/xray/config.json.bak.$(date +%s)
  cat > /usr/local/etc/xray/config.json <<'JSON'
{"log":{"loglevel":"warning"},"inbounds":[{"listen":"0.0.0.0","port":443,"protocol":"vless","settings":{"clients":[{"id":"facf3911-7366-4f6e-9ab9-ca31bfbed2a1","flow":"xtls-rprx-vision"}],"decryption":"none"},"streamSettings":{"network":"tcp","security":"reality","realitySettings":{"show":false,"dest":"ya.ru:443","xver":0,"serverNames":["ya.ru"],"privateKey":"SCRNtPzsS79vV1LsO0TaVqRjDYYA9ex0DXVURaQshm0","shortIds":["d8d552dc89b5ec97"]}},"sniffing":{"enabled":true,"destOverride":["http","tls","quic"]}}],"outbounds":[{"protocol":"freedom","tag":"direct","settings":{"domainStrategy":"UseIP"}},{"protocol":"blackhole","tag":"block"}],"routing":{"domainStrategy":"IPIfNonMatch","rules":[{"type":"field","ip":["geoip:private"],"outboundTag":"block"}]}}
JSON
  if xray -test -config /usr/local/etc/xray/config.json; then
    systemctl enable xray; systemctl restart xray
    if command -v ufw >/dev/null && ufw status 2>/dev/null | grep -q active; then ufw allow 22/tcp; ufw allow 443/tcp; fi
    sleep 1
    systemctl is-active --quiet xray && echo ">>> OK. Reality поднят на $(curl -s -4 https://api.ipify.org):443  (sni=ya.ru)" || echo ">>> Сервис не активен: journalctl -u xray -e --no-pager"
  else
    echo ">>> Конфиг не прошёл проверку — ничего не запускаю."
  fi
fi
