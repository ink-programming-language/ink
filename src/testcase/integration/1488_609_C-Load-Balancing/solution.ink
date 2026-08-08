// Translated from solution.cpp.

func read()
{
  var x = 0;
  var f = 1;
  var c = getchar();
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

var t: dynamic;

var n: dynamic;

var a = cpp_array(100005);

var tot: dynamic;

var bal: dynamic;

var ans: dynamic;

func abs(x: dynamic)
{
  return if ((x > 0)) x else (-x);
}

func cmp(x: dynamic, y: dynamic)
{
  return (x > y);
}

func main()
{
  n = read();
  {
    var i = 1;
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
    var i = 1;
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
