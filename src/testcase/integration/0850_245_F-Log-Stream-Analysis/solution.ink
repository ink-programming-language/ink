// Translated from solution.cpp.

var que: dynamic = cpp_array(10000000);

var ss: dynamic = cpp_array(10000000);

var day: dynamic = [0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335, 366];

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var m: dynamic = cpp_uninitialized();
  var head: dynamic = 0;
  var tail: dynamic = 0;
  var now: dynamic = cpp_uninitialized();
  var ye: dynamic = cpp_uninitialized();
  var mo: dynamic = cpp_uninitialized();
  var da: dynamic = cpp_uninitialized();
  var h: dynamic = cpp_uninitialized();
  var mi: dynamic = cpp_uninitialized();
  var s: dynamic = cpp_uninitialized();
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
