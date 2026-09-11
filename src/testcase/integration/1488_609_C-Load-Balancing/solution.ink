// Translated from solution.cpp.

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var c: dynamic = getchar();
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    if ((c == cpp_char("-")))
    {
      f = -1;
    }
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((c ^ 48)));
    c = getchar();
  }
  return (x * f);
}

var t: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var a: dynamic = cpp_array(100005);

var tot: dynamic = cpp_uninitialized();

var bal: dynamic = cpp_uninitialized();

var ans: dynamic = cpp_uninitialized();

func abs(x: dynamic) -> dynamic
{
  return  ((x > 0)) ? x : (-x);
}

func cmp(x: dynamic, y: dynamic) -> dynamic
{
  return (x > y);
}

func main() -> dynamic
{
  n = read();
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      a[i] = read();
      tot += a[i];
      i += 1;
    }
  }
  sort((a + 1), ((a + n) + 1), cmp);
  bal = (tot % n);
  tot /= n;
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      if (bal)
      {
        bal -= 1;
        ans += abs(((a[i] - tot) - 1));
      } else
      {
        ans += abs((a[i] - tot));
      }
      i += 1;
    }
  }
  ans /= 2;
  printf("%d\n", ans);
  ans = cpp_assign(tot, "=", 0);
  return 0;
}
