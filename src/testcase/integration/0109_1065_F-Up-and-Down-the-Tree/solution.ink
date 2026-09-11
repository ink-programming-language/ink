// Translated from solution.cpp.

func read() -> dynamic
{
  var c: dynamic = getchar();
  var x: dynamic = 0;
  while (((c < cpp_char("0")) || (c > cpp_char("9"))))
  {
    c = getchar();
  }
  while (((c >= cpp_char("0")) && (c <= cpp_char("9"))))
  {
    x = ((x * 10) + ((c - cpp_char("0"))));
    c = getchar();
  }
  return x;
}

func MOD(x: dynamic) -> dynamic
{
  if ((x >= 998244353))
  {
    x -= 998244353;
  }
}

var m: dynamic = cpp_uninitialized();

var l: dynamic = cpp_uninitialized();

var nxt: dynamic = cpp_array(1000010);

var head: dynamic = cpp_array(1000010);

var to: dynamic = cpp_array(1000010);

func add(x: dynamic, y: dynamic) -> dynamic
{
  l += 1;
  nxt[l] = head[x];
  head[x] = l;
  to[l] = y;
}

var low: dynamic = cpp_array(1000010);

var f: dynamic = cpp_array(1000010);

var d: dynamic = cpp_array(1000010);

func dfs(x: dynamic) -> dynamic
{
  low[x] = ((1 << 30));
  var fl: dynamic = 1;
  {
    var i: dynamic = head[x];
    while (i)
    {
      var c: dynamic = to[i];
      d[c] = (d[x] + 1);
      dfs(c);
      if (((low[c] - d[x]) <= m))
      {
        f[x] += f[c];
        f[c] = 0;
      }
      low[x] = min(low[x], low[c]);
      fl = 0;
      i = nxt[i];
    }
  }
  if (fl)
  {
    low[x] = d[x];
    f[x] = 1;
  }
}

func getans(x: dynamic) -> dynamic
{
  var ans: dynamic = 0;
  {
    var i: dynamic = head[x];
    while (i)
    {
      var c: dynamic = to[i];
      ans = max(ans, getans(c));
      i = nxt[i];
    }
  }
  return (ans + f[x]);
}

func main() -> dynamic
{
  var n: dynamic = cpp_uninitialized();
  n = read();
  m = read();
  {
    var i: dynamic = 2;
    while ((i <= n))
    {
      add(read(), i);
      i += 1;
    }
  }
  dfs(1);
  printf("%d\n", getans(1));
}
