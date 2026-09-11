// Translated from solution.cpp.

func read(a: dynamic) -> dynamic
{
  var x: dynamic = 0;
  var f: dynamic = 1;
  var ch: dynamic = getchar();
  while (((ch < cpp_char("0")) || (ch > cpp_char("9"))))
  {
    if ((ch == cpp_char("-")))
    {
      f = -1;
    }
    ch = getchar();
  }
  while (((ch >= cpp_char("0")) && (ch <= cpp_char("9"))))
  {
    x = ((((x << 1)) + ((x << 3))) + ((ch ^ 48)));
    ch = getchar();
  }
  a = (x * f);
}

func write(x: dynamic) -> dynamic
{
  if ((x < 0))
  {
    putchar(cpp_char("-"));
    x = (-x);
  }
  if ((x < 10))
  {
    return cpp_cast(putchar((x + cpp_char("0"))));
  }
  write((x / 10));
  putchar(((x % 10) + cpp_char("0")));
}

func writeln(x: dynamic) -> dynamic
{
  write(x);
  putchar(cpp_char("\n"));
}

func writes(x: dynamic) -> dynamic
{
  write(x);
  putchar(cpp_char(" "));
}

func read(t: dynamic, args: dynamic...) -> dynamic
{
  read(t);
  read(cpp_expand(args));
}

func writes(t: dynamic, args: dynamic...) -> dynamic
{
  writes(t);
  writes(cpp_expand(args));
}

func writeln(t: dynamic, args: dynamic...) -> dynamic
{
  writes(t);
  writes(cpp_expand(args));
  putchar(cpp_char("\n"));
}

var mod: dynamic = 998244353;

var n: dynamic = cpp_uninitialized();

var head: dynamic = cpp_array(300005);

var pnt: dynamic = cpp_array((300005 << 1));

var nxt: dynamic = cpp_array((300005 << 1));

var E: dynamic = 0;

var dp: dynamic = cpp_array(3, 300005);

func add_edge(u: dynamic, v: dynamic) -> dynamic
{
  pnt[E] = v;
  nxt[E] = head[u];
  head[u] = cpp_update(E, "++");
}

func dfs(u: dynamic, f: dynamic) -> dynamic
{
  dp[u][0] = 1;
  {
    var i: dynamic = head[u];
    while ((i != -1))
    {
      var v: dynamic = pnt[i];
      if ((v == f))
      {
        i = nxt[i];
        continue;
      }
      dfs(v, u);
      dp[u][2] = (((((dp[u][0] * ((dp[v][0] + dp[v][1]))) + (dp[u][1] * ((dp[v][0] + dp[v][1])))) + (dp[u][2] * ((dp[v][0] + (dp[v][2] * 2)))))) % mod);
      dp[u][1] = ((((dp[u][0] * dp[v][2]) + (dp[u][1] * ((dp[v][0] + (2 * dp[v][2])))))) % mod);
      dp[u][0] = ((dp[u][0] * ((dp[v][0] + dp[v][2]))) % mod);
      i = nxt[i];
    }
  }
}

func main() -> dynamic
{
  memset(head, -1, cpp_sizeof((head)));
  read(n);
  {
    var i: dynamic = 1;
    while ((i < n))
    {
      var u: dynamic = cpp_uninitialized();
      var v: dynamic = cpp_uninitialized();
      read(u, v);
      add_edge(u, v);
      add_edge(v, u);
      i += 1;
    }
  }
  dfs(1, 0);
  writeln((((dp[1][0] + dp[1][2])) % mod));
  return 0;
}
