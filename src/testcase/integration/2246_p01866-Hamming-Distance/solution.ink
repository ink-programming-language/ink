// Translated from solution.cpp.

func FOR(i: dynamic, s: dynamic, e: dynamic)
{
  cpp_macro("for(int (i)=(s);(i)<(int)(e);(i)++)");
}

func REP(i: dynamic, e: dynamic)
{
  return cpp_expression("#include <");
}

func RFOR(i: dynamic, e: dynamic, s: dynamic)
{
  cpp_macro("for(int (i)=(e)-1;(i)>=(int)(s);(i)--)");
}

func RREP(i: dynamic, e: dynamic)
{
  return cpp_expression("#include <b");
}

func all(o: dynamic)
{
  return cpp_expression("#include <bits/stdc++.");
}

func psb(x: dynamic)
{
  return cpp_expression("#include <bi");
}

func mp(x: dynamic, y: dynamic)
{
  return cpp_expression("#include <bits/std");
}

var EPS = 1e-10;

var N = 1000;

var usd = cpp_array(N);

func main()
{
  memset(usd, 0, cpp_sizeof((usd)));
  var n: dynamic;
  var d: dynamic;
  var x: dynamic;
  var res: dynamic;
  scanf("%d ", (&n));
  read(x);
  res = x;
  scanf("%d ", (&d));
  var i = 0;
  var j = 0;
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
