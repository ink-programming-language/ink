// Translated from solution.cpp.

var N: dynamic = 30005;

var M: dynamic = 550005;

var w: dynamic = cpp_uninitialized();

var n: dynamic = cpp_uninitialized();

var m: dynamic = cpp_uninitialized();

var pw: dynamic = cpp_array(15);

var a: dynamic = cpp_array(N);

var cc: dynamic = cpp_array(N);

var cnt: dynamic = cpp_array(M);

var s: dynamic = cpp_array(15);

func calc(num: dynamic) -> dynamic
{
  var ret: dynamic = 0;
  {
    var i: dynamic = 0;
    while ((i < w))
    {
      if ((num & ((1 << i))))
      {
        ret += pw[i];
      }
      i += 1;
    }
  }
  return ret;
}

func dfs(dep: dynamic, cur: dynamic) -> dynamic
{
  if ((dep == (w + 1)))
  {
    return cnt[cur];
  }
  var t: dynamic = pw[(w - dep)];
  if ((s[dep] == cpp_char("A")))
  {
    return (dfs((dep + 1), cur) + dfs((dep + 1), (cur + t)));
  }
  if ((s[dep] == cpp_char("O")))
  {
    return dfs((dep + 1), cur);
  }
  if ((s[dep] == cpp_char("X")))
  {
    return (dfs((dep + 1), cur) + dfs((dep + 1), (cur + (t * 2))));
  }
  if ((s[dep] == cpp_char("a")))
  {
    return dfs((dep + 1), (cur + (t * 2)));
  }
  if ((s[dep] == cpp_char("o")))
  {
    return (dfs((dep + 1), (cur + t)) + dfs((dep + 1), (cur + (t * 2))));
  }
  return dfs((dep + 1), (cur + t));
}

func main() -> dynamic
{
  ios.sync_with_stdio(false);
  read(w, n, m);
  {
    var i: dynamic = 1;
    while ((i <= n))
    {
      read(a[i]);
      cc[a[i]] += 1;
      i += 1;
    }
  }
  pw[0] = 1;
  {
    var i: dynamic = 1;
    while ((i <= 12))
    {
      pw[i] = (pw[(i - 1)] * 3);
      i += 1;
    }
  }
  {
    var i: dynamic = 0;
    while ((i < ((1 << w))))
    {
      {
        var j: dynamic = 0;
        while ((j < ((1 << w))))
        {
          cnt[(calc(i) + calc(j))] += (cc[i] * cc[j]);
          j += 1;
        }
      }
      i += 1;
    }
  }
  while (cpp_update(m, "--"))
  {
    read(((s + 1)));
    write(dfs(1, 0), "\n");
  }
  return 0;
}
