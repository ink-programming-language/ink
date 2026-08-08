// Translated from solution.cpp.

func Next(i: dynamic, x: dynamic)
{
  cpp_macro("for( register int i = head[x]; i; i = e[i].next )");
}

func rep(i: dynamic, s: dynamic, t: dynamic)
{
  cpp_macro("for( register int i = (s); i <= (t); ++ i )");
}

func drep(i: dynamic, s: dynamic, t: dynamic)
{
  cpp_macro("for( register int i = (t); i >= (s); -- i )");
}

var re = cpp_expression("#include");

var int_cpp = dynamic;

func gi()
{
  var cc = getchar();
  var cn = 0;
  var flus = 1;
  while (((cc < cpp_char("0")) || (cc > cpp_char("9"))))
  {
    if ((cc == cpp_char("-")))
    {
      flus = (-flus);
    }
    cc = getchar();
  }
  while (((cc >= cpp_char("0")) && (cc <= cpp_char("9"))))
  {
    cn = (((cn * 10) + cc) - cpp_char("0"));
    cc = getchar();
  }
  return (cn * flus);
}

var N = (1e6 + 5);

var P = (1e9 + 7);

var a: dynamic;

var b: dynamic;

var c: dynamic;

var n: dynamic;

var m: dynamic;

var Ans: dynamic;

var fac = cpp_array(N);

var inv = cpp_array(N);

func fpow(x: dynamic, k: dynamic)
{
  var ans = 1;
  var base = x;
  while (k)
  {
    if ((k & 1))
    {
      ans = ((ans * base) % P);
    }
    base = ((base * base) % P);
    k >>= 1;
  }
  return (ans % P);
}

func C(x: dynamic, y: dynamic)
{
  if ((((y > x) || (x < 0)) || (y < 0)))
  {
    return 0;
  }
  return ((((fac[x] * inv[y]) % P) * inv[(x - y)]) % P);
}

func main()
{
  a = gi();
  b = gi();
  c = gi();
  n = ((a + b) + c);
  fac[0] = cpp_assign(inv[0], "=", 1);
  rep(i, 1, n)[i] = ((fac[(i - 1)] * i) % P);
  inv[i] = fpow(fac[i], (P - 2));
  var f = 1;
  var l = (-c);
  a -= 1;
  {
    var int_cpp = a;
    while ((i < n))
    {
      var x = (i - a);
      if ((l > b))
      {
        break;
      }
      Ans = (((Ans + ((((f * C(i, a)) % P) * fpow(3, ((n - i) - 1))) % P))) % P);
      f = (((((((f * 2) + P) - C(x, b)) + P) - C(x, l))) % P);
      l += 1;
      i += 1;
    }
  }
  printf("%lld\n", (Ans % P));
  return 0;
}
