// Translated from solution.cpp.

var f = cpp_array(3, 200000);

var N: dynamic;

var K: dynamic;

var s1 = cpp_array(200000);

var s2 = cpp_array(200000);

var ok: dynamic;

func dfs(x: dynamic, y: dynamic)
{
  if (ok)
  {
    return;
  }
  var Nxt = (x + K);
  if ((Nxt > N))
  {
    ok = 1;
    return;
  }
  if ((y == 1))
  {
    if ((((s2[Nxt] == cpp_char("-")) && ((f[x][1] + 1) < f[Nxt][2])) && (Nxt > (f[x][1] + 1))))
    {
      f[Nxt][2] = (f[x][1] + 1);
      dfs(Nxt, 2);
    }
    if (((s1[(x + 1)] == cpp_char("-")) && ((f[x][1] + 1) < f[(x + 1)][1])))
    {
      f[(x + 1)][1] = (f[x][1] + 1);
      dfs((x + 1), 1);
    }
    if ((((s1[(x - 1)] == cpp_char("-")) && ((f[x][1] + 1) < f[(x - 1)][1])) && ((x - 1) > (f[x][1] + 1))))
    {
      f[(x - 1)][1] = (f[x][1] + 1);
      dfs((x - 1), 1);
    }
  } else
  {
    if ((((s1[Nxt] == cpp_char("-")) && ((f[x][2] + 1) < f[Nxt][1])) && (Nxt > (f[x][2] + 1))))
    {
      f[Nxt][1] = (f[x][2] + 1);
      dfs(Nxt, 1);
    }
    if (((s2[(x + 1)] == cpp_char("-")) && ((f[x][2] + 1) < f[(x + 1)][2])))
    {
      f[(x + 1)][2] = (f[x][2] + 1);
      dfs((x + 1), 2);
    }
    if ((((s2[(x - 1)] == cpp_char("-")) && ((f[x][2] + 1) < f[(x - 1)][2])) && ((x - 1) > (f[x][2] + 1))))
    {
      f[(x - 1)][2] = (f[x][2] + 1);
      dfs((x - 1), 2);
    }
  }
}

func main()
{
  ios.sync_with_stdio(false);
  read(N, K);
  read((s1 + 1));
  read((s2 + 1));
  memset(f, 0x7f, cpp_sizeof((f)));
  f[1][1] = 0;
  dfs(1, 1);
  if (ok)
  {
    puts("YES");
  } else
  {
    puts("NO");
  }
  return 0;
}
