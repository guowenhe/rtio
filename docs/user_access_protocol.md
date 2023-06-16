# user access

## 样例

```text
curl -X POST "http://127.0.0.1:17317/cfa09baa-4913-4ad7-a936-2e26f9671b04/post_handler"  -d '{"uri":"/printer/action","id":12667,"data":"MTIzNDk5MA=="}'

curl -X GET "http://127.0.0.1:17317/cfa09baa-4913-4ad7-a936-2e26f9671b04/obget_handler?uri=%2Fprinter%2Fstatus&id=12334&data=MTIzNDU%3D"

```
