// Translated from solution.cpp.

func read()
{
  var c = getchar();
  var x = 0;
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

func MOD(x: dynamic)
{
  if ((x >= 998244353))
  {
    x -= 998244353;
  }
}

var m: dynamic;

var l: dynamic;

var nxt = cpp_array(1000010);

var head = cpp_array(1000010);

var to = cpp_array(1000010);

func add(x: dynamic, y: dynamic)
{
  l += 1;
  nxt[l] = head[x];
  head[x] = l;
  to[l] = y;
}

var low = cpp_array(1000010);

var f = cpp_array(1000010);

var d = cpp_array(1000010);

func dfs(x: dynamic)
{
  low[x] = ((1 << 30));
  var fl = 1;
  {
    var i = head[x];
    while (i)
    {
      var c = to[i];
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

func getans(x: dynamic)
{
  var ans = 0;
  {
    var i = head[x];
    while (i)
    {
      var c = to[i];
      ans = max(ans, getans(c));
      i = nxt[i];
    }
  }
  return (ans + f[x]);
}

func main()
{
  var n: dynamic;
  n = read();
  m = read();
  {
    var i = 2;
    while ((i <= n))
    {
      add(read(), i);
      i += 1;
    }
  }
  dfs(1);
  printf("%d\n", getans(1));
}
