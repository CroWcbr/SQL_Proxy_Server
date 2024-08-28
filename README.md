# TCP_proxy_server

## Задание
TCP прокси-сервер для СУБД Postgresql на C++ с возможностью логирования всех SQL запросов, проходящих через него.

## Детали
- Используются Berkley sockets (select/poll/epoll).
- Обрабатывает большое количество соединений без создания потока (thread) на каждое соединение.
- Записать в файл SQL запросы проходящие через прокси сервер.

## Тестирование
- мой test_serv
- sql запросы
- sysbench

## Дополнительно
### PostgreSQL и pgAdmin
- Необходимо отключить ssl шифрование в PostgreSQL
- Создаем пользователя для тестов
```
sudo -i -u postgres
psql

CREATE USER test_user WITH PASSWORD 'test_password';
CREATE DATABASE test_database WITH OWNER test_user;
GRANT ALL PRIVILEGES ON DATABASE test_database TO test_user;

\q
exit
```
