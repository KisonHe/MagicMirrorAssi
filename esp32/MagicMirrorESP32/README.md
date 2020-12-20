# MagicMirrorAssi
My little stuff to make my MM smarter.

Using a 

外部信号-touch,or ir---->释放信号量
touch --> isr
ir --> keep reading in maintask

一个task---->尝试take？for 45s?
if fail? close monitor
if success? taskdelay 5s
