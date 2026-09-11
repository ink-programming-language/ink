// Translated from solution.cpp.

class node
{
  var x: dynamic = cpp_uninitialized();
  var t: dynamic = cpp_uninitialized();
  var id: dynamic = cpp_uninitialized();
}

var a: dynamic = cpp_array(200002);

var n: dynamic = cpp_uninitialized();

var h: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var k: dynamic = cpp_uninitialized();

var i: dynamic = cpp_uninitialized();

var j: dynamic = cpp_uninitialized();

func read() -> dynamic
{
  var c: dynamic = getchar();
  var w: dynamic = 0;
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    c = getchar();
  }
  while (((c <= cpp_char("9")) && (c >= cpp_char("0"))))
  {
    w = (((w * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  return w;
}

func cmp1(a: dynamic, b: dynamic) -> dynamic
{
  if ((a.x == b.x))
  {
    return (a.t < b.t);
  }
  return (a.x < b.x);
}

func cmp2(a: dynamic, b: dynamic) -> dynamic
{
  return (a.t < b.t);
}

func main() -> dynamic
{
  n = read();
  h = read();
  m = read();
  k = read();
  {
    i = 1;
    while ((i <= n))
    {
      a[i].x = read();
      a[i].t = (read() % ((m / 2)));
      a[i].id = i;
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), cmp2);
  {
    i = 1;
    while ((i <= n))
    {
      a[(n + i)] = [0, (a[i].t + (m / 2)), a[i].id];
      i += 1;
    }
  }
  var ans: dynamic = (1 << 30);
  var tim: dynamic = cpp_uninitialized();
  var l: dynamic = cpp_uninitialized();
  var r: dynamic = cpp_uninitialized();
  {
    i = (n + 1);
    j = 1;
    while ((i <= (2 * n)))
    {
      while ((((a[i].t - a[j].t) >= k) && (j <= (2 * n))))
      {
        j += 1;
      }
      if (((i - j) < ans))
      {
        ans = (i - j);
        l = j;
        r = (i - 1);
        tim = a[i].t;
      }
      i += 1;
    }
  }
  printf("%d %d\n", ans, (tim % ((m / 2))));
  if ((ans != 0))
  {
    {
      i = l;
      while ((i <= r))
      {
        printf("%d ", a[i].id);
        i += 1;
      }
    }
    puts("");
  }
  return 0;
}
