// Translated from solution.cpp.

func range(i: dynamic, a: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i = (a); i < (b); i++)");
}

func rep(i: dynamic, b: dynamic) -> dynamic
{
  cpp_macro("for(int i = 0; i < (b); i++)");
}

func all(a: dynamic) -> dynamic
{
  return cpp_expression("#include<bits/stdc++.h");
}

func show(x: dynamic) -> dynamic
{
  cpp_macro("cerr << #x << \" = \" << (x) << endl;");
}

func debug(x: dynamic) -> dynamic
{
  cpp_macro("cerr << #x << \" = \" << (x) << \" (L\" << __LINE__ << \")\" << \" \" << __FILE__ << endl;");
}

var INF: dynamic = 2000000000;

func toStr(n: dynamic) -> dynamic
{
  var str: dynamic = cpp_uninitialized();
  var len: dynamic = static_cast(log10(n));
  var K: dynamic = 1;
  rep(i, len) *= 10;
  rep(i, (len + 1));
  {
    if (((n / K) == 0))
    {
      str += cpp_char("0");
    } else
    {
      str += ((cpp_char("0") + (n / K)));
    }
    n %= K;
    K /= 10;
  }
  return str;
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  var ans: dynamic = cpp_array(2);
  read(n);
  var cnt: dynamic = 0;
  while (true)
  {
    var s: dynamic = toStr((n + cnt));
    var r: dynamic = s;
    reverse(all(r));
    if ((s == r))
    {
      ans[0] = cnt;
      break;
    }
    cnt += 1;
  }
  cnt = 0;
  while (true)
  {
    var s: dynamic = toStr((n - cnt));
    var r: dynamic = s;
    reverse(all(r));
    if ((s == r))
    {
      ans[1] = cnt;
      break;
    } else if (((n - cnt) < 0))
    {
      ans[1] = INF;
      break;
    }
    cnt += 1;
  }
  if ((ans[1] < ans[0]))
  {
    write((n - ans[1]), "\n");
  } else if ((ans[1] > ans[0]))
  {
    write((n + ans[0]), "\n");
  } else
  {
    write((n - ans[1]), "\n");
  }
  return 0;
}
