# HMAC Service
___

## Зависимости

Для выполнения данного проекта предполагается использовать три сторонние библиотеки:

- `cpprestsdk` - простой интерфейс для создания веб-сервера
- `OpenSSL` - библиотека для использования алгоритмов криптографии (как правило, установлена по умолчанию)
- `nlohmann/json` - библиотека для работы с `JSON`-файлами

При использовании операционных систем на базе `Ubuntu` для установки этих библиотек вы можете воспользоваться пакетным менеджером:

```shell
sudo apt-get update
sudo apt-get install libssl-dev libcpprest-dev nlohmann-json3-dev
```

Если вы используете другие дистрибутивы `Linux`, то вы достаточно подготовлены, чтобы самостоятельно найти и установить
библиотеки в соответствие с вашей системой.

## Сборка и запуск проекта

Для сборки данного проекта используйте `cmake` - добавьте свои файлы решения в файл `CMakeLists.txt`.

Если вы выполняете этот проект, то вы достаточно подготовлены,
чтобы самостоятельно настроить и запустить его.

Скрипт автосборки через скрипт ./build.sh 

Запуск сервиса через ./build/hmac_service <path-to-config>

# Пример вызова метода sing/
```shell
curl -sS -X POST http://localhost:8080/sign \  
  -H 'Content-Type: application/json' \
  -d '{"msg":"hello"}'
```

# Пример вызова verify/
```shell
curl -sS -X POST http://localhost:8080/verify \
  -H 'Content-Type: application/json' \
  -d '{"msg":"hello","signature":"<скопировать из /sign>"}'
```

# Пример вызова ping/
```shell
curl -sS -X GET http://localhost:8080/ping -H 'Content-Type: application/json'
```