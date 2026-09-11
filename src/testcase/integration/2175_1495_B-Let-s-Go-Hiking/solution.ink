// Translated from solution.cpp.

var pb: dynamic = cpp_expression("#include");

var fst: dynamic = cpp_expression("#incl");

var snd: dynamic = cpp_expression("#inclu");

func fore(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i=a,ggdem=b;i<ggdem;++i)");
}

func SZ(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/");
}

func ALL(x: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/st");
}

func mset(a: dynamic, v: dynamic) -> dynamic
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var FIN: dynamic = cpp_expression("#include <bits/stdc++.h>");

func main() -> dynamic
{
  FIN;
  var n: dynamic = cpp_uninitialized();
  read(n);
  fore(i, 0, n);
  read(a[i]);
  var d: dynamic = cpp_uninitialized();
  fore(i, 0, (n - 1)).pb((a[(i + 1)] > a[i]));
  var c: dynamic = cpp_uninitialized();
  var va: dynamic = 0;
  fore(i, 0, SZ(d));
  {
    va += 1;
    if (((i == (SZ(d) - 1)) || (d[i] != d[(i + 1)])))
    {
      c.pb(va);
      va = 0;
    }
  }
  var maxi: dynamic = [0, -1];
  fore(i, 0, SZ(c)) = max(maxi, [c[i], i]);
  var cant: dynamic = 0;
  fore(i, 0, SZ(c)) += ((c[i] == maxi.fst));
  var res: dynamic = 0;
  if (((((cant == 2) && ((maxi.snd - 1) >= 0)) && (c[(maxi.snd - 1)] == maxi.fst)) && (((maxi.fst % 2) == 0))))
  {
    res = (d[0] ^ ((((maxi.snd - 1)) & 1)));
  }
  write(res, "\n");
  return 0;
}
