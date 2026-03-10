## TODO
- проверить самодельную схему борьбы с дребезгом
- проверить готовое решение
- принести в уник и написать тест для всех устройств ввода
- подключить 74HC165 к SPI через резистор
- сравнить bit-banging и SPI для 74HC165

## Bookmarks
- [toolchain](https://habr.com/ru/articles/854050/)
- [uploader](https://docs.mikron.ru/wiki/dev-tools/soft/uploader.html)
- [debouncer IC](https://www.chipdip.ru/product/mc14490dwg-ic-digital-contact-bounce-on-semiconductor-8051233161)

## rev. 1
![Schematics](./images/rev1.png)

### Особенности
- 12 кнопок через 2 74HC165
- готовый китайский модуль энкодера
- 8 рычажковых переключателей подключены напрямую без борьбы с дребезгом.
- 2 потенциометра от 0v до 1v1
- все устройства ввода проверены с Arduino -> все собрано верно

### Список компонентов
TODO
