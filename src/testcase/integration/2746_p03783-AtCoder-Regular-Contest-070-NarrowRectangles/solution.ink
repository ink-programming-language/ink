// Translated from solution.cpp.

var db: dynamic = cpp_expression("#inclu");

var ls: dynamic = cpp_expression("#includ");

var rs: dynamic = cpp_expression("#include <b");

var pb: dynamic = cpp_expression("#include");

var ll: dynamic = dynamic;

var mp: dynamic = cpp_expression("#include");

var pii: dynamic = cpp_expression("#include <bits");

var X: dynamic = cpp_expression("#incl");

var Y: dynamic = cpp_expression("#inclu");

var pcc: dynamic = cpp_expression("#include <bits/s");

var vi: dynamic = cpp_expression("#include <b");

var vl: dynamic = cpp_expression("#include <");

func rep(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i = x - 1; i < y; i ++)");
}

func rrep(i: dynamic, x: dynamic, y: dynamic) -> dynamic
{
  cpp_macro("for(int i = x; i >= y; i - - )");
}

var eps: dynamic = cpp_expression("#inclu");

func all(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.");
}

func read() -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char(" - ")))
    {
      f = (-1);
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = (((x * 10) + ch) - cpp_char("0"));
    ch = getchar();
  }
  return (x * f);
}

var n: dynamic = cpp_uninitialized();

var l: dynamic = cpp_array(100001);

var r: dynamic = cpp_array(100001);

var L: dynamic = cpp_uninitialized();

var R: dynamic = cpp_uninitialized();

func main() -> dynamic
{
  n = read();
  rep(i, 1, n);
  scanf("%lld%lld", (&l[i]), (&r[i]));
  L.insert(((-1) << 60));
  R.insert((1 << 60));
  var ol: dynamic = 0;
  var Or: dynamic = 0;
  var res: dynamic = 0;
  rep(i, 1, n);
  {
    if ((i > 0))
    {
      ol -= (r[i] - l[i]);
      Or += (r[(i - 1)] - l[(i - 1)]);
    }
    if ((l[i] < ((*L.rbegin()) + ol)))
    {
      res += (((*L.rbegin()) + ol) - l[i]);
    } else if ((((*R.begin()) + Or) < l[i]))
    {
      res += (l[i] - (((*R.begin()) + Or)));
    }
    if ((l[i] < ((*L.rbegin()) + ol)))
    {
      R.insert((((*L.rbegin()) + ol) - Or));
      L.insert((l[i] - ol));
      L.insert((l[i] - ol));
      L.erase(L.find((*L.rbegin())));
    } else if ((((*R.begin()) + Or) < l[i]))
    {
      L.insert((((*R.begin()) + Or) - ol));
      R.insert((l[i] - Or));
      R.insert((l[i] - Or));
      R.erase(R.begin());
    } else
    {
      L.insert((l[i] - ol));
      R.insert((l[i] - Or));
    }
  }
  printf("%lld\n", res);
  return 0;
}
