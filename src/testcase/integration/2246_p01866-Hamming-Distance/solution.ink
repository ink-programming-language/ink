// Translated from solution.cpp.

func FOR(i: dynamic, s: dynamic, e: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=(s);(i)<(int)(e);(i)++)");
}

func REP(i: dynamic, e: dynamic) -> dynamic
{
  return cpp_expression("#include <");
}

func RFOR(i: dynamic, e: dynamic, s: dynamic) -> dynamic
{
  cpp_macro("for(int (i)=(e)-1;(i)>=(int)(s);(i)--)");
}

func RREP(i: dynamic, e: dynamic) -> dynamic
{
  return cpp_expression("#include <b");
}

func all(o: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.");
}

func psb(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bi");
}

func mp(x: dynamic, y: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/std");
}

var EPS: dynamic = 1e-10;

var N: dynamic = 1000;

var usd: dynamic = cpp_array(N);

func main() -> dynamic
{
  memset(usd, 0, cpp_sizeof((usd)));
  var n: dynamic = cpp_uninitialized();
  var d: dynamic = cpp_uninitialized();
  var x: dynamic = cpp_uninitialized();
  var res: dynamic = cpp_uninitialized();
  scanf("%d ", (&n));
  read(x);
  res = x;
  scanf("%d ", (&d));
  var i: dynamic = 0;
  var j: dynamic = 0;
  while (((i < n) && (j < d)))
  {
    if (((x[i] == cpp_char("0")) && (!usd[i])))
    {
      res[i] = cpp_char("1");
      usd[i] = 1;
      j += 1;
    }
    i += 1;
  }
  i = (n - 1);
  while (((i >= 0) && (j < d)))
  {
    if (((x[i] == cpp_char("1")) && (!usd[i])))
    {
      res[i] = cpp_char("0");
      usd[i] = 1;
      j += 1;
    }
    i -= 1;
  }
  write(res, "\n");
  return 0;
}
