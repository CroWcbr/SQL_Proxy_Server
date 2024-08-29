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
```
Подключаемся напрямую:
psql "dbname=test_database user=test_user password=test_password"

Подключаемся через proxy_server
psql "host=localhost port=5000 dbname="test_database" user=test_user password=test_password sslmode=disable"
```
- sysbench
```
подготовка данных
sysbench \
  --db-driver=pgsql \
  --pgsql-host=localhost \
  --pgsql-port=5000 \
  --pgsql-user=test_user \
  --pgsql-password=test_password \
  --pgsql-db=test_database \
  --tables=10 \
  --table-size=1000000 \
  /usr/share/sysbench/oltp_read_write.lua prepare
```
```
тест
sysbench \
  --db-driver=pgsql \
  --pgsql-host=localhost \
  --pgsql-port=5000 \
  --pgsql-user=test_user \
  --pgsql-password=test_password \
  --pgsql-db=test_database \
  --tables=10 \
  --table-size=1000000 \
  --threads=30 \
  --time=300 \
  --report-interval=5 \
  /usr/share/sysbench/oltp_read_write.lua run
```
```
очистка
sysbench \
  --db-driver=pgsql \
  --pgsql-host=localhost \
  --pgsql-port=5000 \
  --pgsql-user=test_user \
  --pgsql-password=test_password \
  --pgsql-db=test_database \
  --tables=10 \
  /usr/share/sysbench/oltp_read_write.lua cleanup
```
```
SQL statistics:
    queries performed:
        read:                            3852478
        write:                           1100677
        other:                           550365
        total:                           5503520
    transactions:                        275167 (916.64 per sec.)
    queries:                             5503520 (18333.50 per sec.)
    ignored errors:                      10     (0.03 per sec.)
    reconnects:                          0      (0.00 per sec.)

General statistics:
    total time:                          300.1886s
    total number of events:              275167

Latency (ms):
         min:                                    4.62
         avg:                                   32.71
         max:                                  770.37
         95th percentile:                       74.46
         sum:                              9000804.80

Threads fairness:
    events (avg/stddev):           9172.2333/1040.47
    execution time (avg/stddev):   300.0268/0.04
```
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
