#!/bin/bash
# Простой и надёжный AES-256 crypter

set -e

ARM64_IN="/storage/emulated/0/#Cheat/wintyx external/app/src/main/jniLibs/arm64-v8a/external.sh"
X86_IN="/storage/emulated/0/#Cheat/wintyx external/app/src/main/jniLibs/x86_64/external.sh"

ARM64_OUT="/storage/emulated/0/#Cheat/wintyx external/app/src/main/jniLibs/arm64-v8a/cheat1.sh"
X86_OUT="/storage/emulated/0/#Cheat/wintyx external/app/src/main/jniLibs/x86_64/cheat2.sh"

# ←←← СМЕНИ НА СВОЙ СИЛЬНЫЙ ПАРОЛЬ (только буквы и цифры, без !)
PASSWORD="SuperStrongPass2026ABC123"

echo "[+] Начинаем шифрование..."

encrypt() {
    input="$1"
    output="$2"
    name="$3"

    echo "[+] Шифрую $name → $output"

    # Простые временные имена
    tmp_gz="/tmp/crypt_${name}_gz"
    tmp_enc="/tmp/crypt_${name}_enc"
    tmp_b64="/tmp/crypt_${name}_b64"

    # Очистка на всякий случай
    rm -f "$tmp_gz" "$tmp_enc" "$tmp_b64" 2>/dev/null

    gzip -9 -c "$input" > "$tmp_gz"
    openssl enc -aes-256-cbc -salt -pbkdf2 -iter 300000 -in "$tmp_gz" -out "$tmp_enc" -pass pass:"$PASSWORD" 2>/dev/null
    base64 -w 0 "$tmp_enc" | sed 's/w/owo/g;s/e/eee/g;s/o/ooo/g;s/a/aaa/g' > "$tmp_b64"

    cat > "$output" << 'DECO'
#!/bin/bash
# Жёстко зашифровано AES-256

set -e 2>/dev/null || true

decode_and_run() {
    payload="$1"
    t1="/tmp/.c1_\[ "
    t2="/tmp/.c2_ \]"
    t3="/tmp/.run_$$"

    printf '%s' "$payload" | sed 's/aaa/a/g;s/ooo/o/g;s/eee/e/g;s/owo/w/g' | base64 -d > "$t1" 2>/dev/null

    openssl enc -d -aes-256-cbc -salt -pbkdf2 -iter 300000 -in "$t1" -out "$t2" -pass pass:"SuperStrongPass2026ABC123" 2>/dev/null || {
        echo "[-] Ошибка: неверный пароль"
        rm -f "$t1" "$t2" "$t3" 2>/dev/null
        exit 1
    }

    gunzip -f "$t2" 2>/dev/null && mv "$t2" "$t3" || gunzip -c "$t2" > "$t3" 2>/dev/null

    chmod +x "$t3" 2>/dev/null
    "\( t3" " \)@"
    res=$?
    rm -f "$t1" "$t2" "$t3" 2>/dev/null &
    exit $res
}

PAYLOAD='''PASTE_HERE'''

decode_and_run "$PAYLOAD"
DECO

    sed -i "s|PAYLOAD='''PASTE_HERE'''|PAYLOAD='$(cat "$tmp_b64")'|" "$output"
    sed -i "s|SuperStrongPass2026ABC123|$PASSWORD|g" "$output"

    chmod +x "$output"

    rm -f "$tmp_gz" "$tmp_enc" "$tmp_b64" 2>/dev/null
    echo "[+] $name готов"
}

encrypt "$ARM64_IN" "$ARM64_OUT" "arm64"
encrypt "$X86_IN" "$X86_OUT" "x86_64"

echo "[+]"
echo "[+] Готово!"
echo "[+] cheat1.sh (arm64) создан"
echo "[+] cheat2.sh (x86_64) создан"
echo "[+] Запуск:"
echo "    ./cheat1.sh     (в папке arm64-v8a)"
echo "    ./cheat2.sh     (в папке x86_64)"
