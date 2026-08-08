// Translated from solution.cpp.

var pb = cpp_expression("#include");

var fst = cpp_expression("#incl");

var snd = cpp_expression("#inclu");

func fore(i: dynamic, a: dynamic, b: dynamic)
{
  cpp_macro("for(int i=a,ggdem=b;i<ggdem;++i)");
}

func SZ(x: dynamic)
{
  return cpp_expression("#include <bits/");
}

func ALL(x: dynamic)
{
  return cpp_expression("#include <bits/st");
}

func mset(a: dynamic, v: dynamic)
{
  return cpp_expression("#include <bits/stdc++.h>");
}

var FIN = cpp_expression("#include <bits/stdc++.h>");

func main()
{
  FIN;
  var n: dynamic;
  read(n);
  fore(i, 0, n);
  read(a[i]);
  var d: dynamic;
  fore(i, 0, (n - 1)).pb((a[(i + 1)] > a[i]));
  var c: dynamic;
  var va = 0;
  fore(i, 0, SZ(d));
  {
    va += 1;
    if (((i == (SZ(d) - 1)) || (d[i] != d[(i + 1)])))
    {
      c.pb(va);
      va = 0;
    }
  }
  var maxi = [0, -1];
  fore(i, 0, SZ(c)) = max(maxi, [c[i], i]);
  var cant = 0;
  fore(i, 0, SZ(c)) += ((c[i] == maxi.fst));
  var res = 0;
  if (((((cant == 2) && ((maxi.snd - 1) >= 0)) && (c[(maxi.snd - 1)] == maxi.fst)) && (((maxi.fst % 2) == 0))))
  {
    res = (d[0] ^ ((((maxi.snd - 1)) & 1)));
  }
  write(res, "\n");
  return 0;
}
