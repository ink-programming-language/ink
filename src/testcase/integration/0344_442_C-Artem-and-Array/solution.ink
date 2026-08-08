// Translated from solution.cpp.

var Imx = 2147483647;

var mod = 1000000007;

var Lbig = 2e18;

func getnum()
{
  var r = 0;
  var ng = 0;
  var c: dynamic;
  c = getchar();
  while (((c != cpp_char("-")) && (((c < cpp_char("0")) || (c > cpp_char("9"))))))
  {
    c = getchar();
  }
  if ((c == cpp_char("-")))
  {
    ng = 1;
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    r = (((r * 10) + c) - cpp_char("0"));
    c = getchar();
  }
  if (ng)
  {
    r = (-r);
  }
  return r;
}

func putnum(x: dynamic)
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  var a = [];
  var sz = 0;
  while ((x > 0))
  {
    a[cpp_update(sz, "++")] = (x % 10);
    x /= 10;
  }
  if ((sz == 0))
  {
    putchar(cpp_char("0"));
  }
  {
    var i = (sz - 1);
    while ((i >= 0))
    {
      putchar((cpp_char("0") + a[i]));
      i -= 1;
    }
  }
}

func putsp()
{
  putchar(cpp_char(" "));
}

func putendl()
{
  putchar(cpp_char("\n"));
}

func mygetchar()
{
  var c = getchar();
  while (((c == cpp_char(" ")) || (c == cpp_char("\n"))))
  {
    c = getchar();
  }
  return c;
}

var n: dynamic;

var nxt = cpp_array(500111);

var pre = cpp_array(500111);

var a = cpp_array(500111);

var f = cpp_array(500111);

func del(x: dynamic)
{
  f[x] = 1;
  nxt[pre[x]] = nxt[x];
  pre[nxt[x]] = pre[x];
}

func check(x: dynamic)
{
  if ((((((!f[x]) && (x != 1)) && (x != n)) && (a[pre[x]] >= a[x])) && (a[x] <= a[nxt[x]])))
  {
    del(x);
    return ((min(a[nxt[x]], a[pre[x]]) + check(pre[x])) + check(nxt[x]));
  }
  return 0;
}

func main()
{
  n = getnum();
  {
    var i = 1;
    while ((i <= n))
    {
      a[i] = getnum();
      nxt[i] = (i + 1);
      pre[i] = (i - 1);
      i += 1;
    }
  }
  var ans = 0;
  {
    var i = 2;
    while ((i < n))
    {
      if ((!f[i]))
      {
        ans += check(i);
      }
      i += 1;
    }
  }
  var p = 1;
  while (((p >= 1) && (p <= n)))
  {
    ans += min(a[pre[p]], a[nxt[p]]);
    p = nxt[p];
  }
  write(ans, "\n");
  return 0;
}
