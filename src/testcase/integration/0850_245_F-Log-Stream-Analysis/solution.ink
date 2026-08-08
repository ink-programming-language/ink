// Translated from solution.cpp.

var que = cpp_array(10000000);

var ss = cpp_array(10000000);

var day = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366];

func main()
{
  var n: dynamic;
  var m: dynamic;
  var head = 0;
  var tail = 0;
  var now: dynamic;
  var ye: dynamic;
  var mo: dynamic;
  var da: dynamic;
  var h: dynamic;
  var mi: dynamic;
  var s: dynamic;
  read(n, m);
  while ((scanf("%d-%d-%d %d:%d:%d:", (&ye), (&mo), (&da), (&h), (&mi), (&s)) != EOF))
  {
    gets(ss);
    now = day[(mo - 1)];
    now = (((((((((now + da) - 1)) * 24) * 60) * 60) + ((h * 60) * 60)) + (mi * 60)) + s);
    que[cpp_update(tail, "++")] = now;
    while ((que[head] <= (now - n)))
    {
      head += 1;
    }
    if (((tail - head) >= m))
    {
      printf("2012-%02d-%02d %02d:%02d:%02d\n", mo, da, h, mi, s);
      return 0;
    }
  }
  write(-1, "\n");
}
